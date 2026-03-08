/**
 * @file   sensor_proc.c
 * @brief  Sensor Processing Module - implementation
 * @doc    WMC-SUD-001 §4.2 Sensor Processing Specification
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "sensor_proc.h"
#include "hal.h"

/* ========================================================================
 * [SAFETY VARIABLE REGION]
 * Variables in this section are safety-critical sensor data.
 * In the linker script these should be placed in a dedicated
 * .safety_data section separated from control variables.
 * ======================================================================== */

/** Filtered motor current in milliamps */
static uint16_t s_current_ma;

/** Unfiltered motor current in milliamps (for vibration detection) */
static uint16_t s_current_ma_raw;

/** Supply voltage in millivolts */
static uint16_t s_voltage_mv;

/** Motor RPM from hall sensor capture */
static uint16_t s_rpm;

/** Debounced lid state */
static lid_state_t s_lid_state;

/** ADC diagnostic fault flag */
static uint8_t s_adc_diag_fault;

/* ========================================================================
 * [CONTROL VARIABLE REGION]
 * Internal working variables for filtering and debounce logic.
 * ======================================================================== */

/** IIR filter accumulator for current (scaled by IIR denominator) */
static uint16_t s_current_filtered;

/** Voltage update divider counter (10Hz from 100Hz base) */
static uint8_t s_voltage_divider;

/** Lid GPIO debounce counter */
static uint8_t s_lid_debounce_cnt;

/** Raw lid GPIO reading (unfiltered) */
static uint8_t s_lid_raw;

/** ADC stuck-at counters per channel */
static uint8_t s_adc_stuck_low_cnt[2];
static uint8_t s_adc_stuck_high_cnt[2];

/** Hall sensor period for RPM calculation (timer capture value) */
static uint16_t s_hall_period;

/* ========================================================================
 * Internal Helper: Convert ADC raw to milliamps
 * Current sense: 1V/A shunt + amplifier
 * ADC range: 0-1023 maps to 0-5000mV maps to 0-10000mA
 * Formula: mA = (raw * ADC_VREF_MV) / ADC_MAX_VALUE * 2
 *        = (raw * 10000) / 1023
 * Using integer math: (raw * 10000U + 511U) / 1023U
 * ======================================================================== */

static uint16_t AdcToCurrentMa(uint16_t raw)
{
    uint32_t temp;

    temp = (uint32_t)raw * 10000UL;
    temp = (temp + 511UL) / 1023UL;

    /* Clamp to uint16_t range */
    if (temp > 65535UL)
    {
        temp = 65535UL;
    }

    return (uint16_t)temp;
}

/* ========================================================================
 * Internal Helper: Convert ADC raw to millivolts
 * Voltage divider: 1:1 ratio assumed
 * ADC range: 0-1023 maps to 0-5000mV
 * Formula: mV = (raw * ADC_VREF_MV) / ADC_MAX_VALUE
 * ======================================================================== */

static uint16_t AdcToVoltageMv(uint16_t raw)
{
    uint32_t temp;

    temp = (uint32_t)raw * (uint32_t)ADC_VREF_MV;
    temp = (temp + 511UL) / 1023UL;

    if (temp > 65535UL)
    {
        temp = 65535UL;
    }

    return (uint16_t)temp;
}

/* ========================================================================
 * Internal Helper: ADC stuck-at diagnostic (DR-003)
 * Detects ADC readings stuck at 0 or 1023 for SENSOR_ADC_STUCK_LIMIT
 * consecutive readings.
 * ======================================================================== */

static void AdcDiagCheck(uint8_t ch, uint16_t raw)
{
    if (ch > HAL_ADC_CH_VOLTAGE)
    {
        return;
    }

    /* Check stuck-at-low */
    if (raw == SENSOR_ADC_STUCK_LOW)
    {
        if (s_adc_stuck_low_cnt[ch] < 255U)
        {
            s_adc_stuck_low_cnt[ch]++;
        }
    }
    else
    {
        s_adc_stuck_low_cnt[ch] = 0U;
    }

    /* Check stuck-at-high */
    if (raw == SENSOR_ADC_STUCK_HIGH)
    {
        if (s_adc_stuck_high_cnt[ch] < 255U)
        {
            s_adc_stuck_high_cnt[ch]++;
        }
    }
    else
    {
        s_adc_stuck_high_cnt[ch] = 0U;
    }

    /* Set fault if either counter exceeds limit */
    if ((s_adc_stuck_low_cnt[ch] >= SENSOR_ADC_STUCK_LIMIT) ||
        (s_adc_stuck_high_cnt[ch] >= SENSOR_ADC_STUCK_LIMIT))
    {
        s_adc_diag_fault = FLAG_SET;
    }
}

/* ========================================================================
 * Internal Helper: IIR low-pass filter for current
 * new_filtered = old_filtered + (raw - old_filtered) >> SHIFT
 * ======================================================================== */

static uint16_t IirFilter(uint16_t old_val, uint16_t new_sample)
{
    uint16_t result;
    int32_t  diff;

    diff = (int32_t)new_sample - (int32_t)old_val;
    diff = diff >> SENSOR_IIR_SHIFT;

    result = (uint16_t)((int32_t)old_val + diff);

    return result;
}

/* ========================================================================
 * Internal Helper: Lid GPIO debounce
 * Requires DEBOUNCE_COUNT consecutive same-state readings to change state.
 * ======================================================================== */

static void LidDebounce(void)
{
    uint8_t raw_pin;

    raw_pin = HAL_GPIO_Read(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR);

    /*
     * Lid sensor is active-low: LOW = closed, HIGH = open
     * Invert for logical state
     */
    if (raw_pin == HAL_GPIO_LOW)
    {
        s_lid_raw = (uint8_t)LID_CLOSED;
    }
    else
    {
        s_lid_raw = (uint8_t)LID_OPEN;
    }

    /* Debounce logic */
    if (s_lid_raw == (uint8_t)s_lid_state)
    {
        /* Same as current state - reset counter */
        s_lid_debounce_cnt = 0U;
    }
    else
    {
        /* Different from current state - increment counter */
        s_lid_debounce_cnt++;

        if (s_lid_debounce_cnt >= DEBOUNCE_COUNT)
        {
            /* State change confirmed */
            s_lid_state = (lid_state_t)s_lid_raw;
            s_lid_debounce_cnt = 0U;
        }
    }
}

/* ========================================================================
 * Internal Helper: RPM calculation from hall sensor period
 *
 * RPM = 60 * f_timer / (period * pole_pairs)
 * For RL78/G14 timer at 1 MHz capture clock:
 *   RPM = 60,000,000 / (period * MOTOR_POLE_PAIRS)
 *
 * In host environment, s_hall_period is set externally for testing.
 * ======================================================================== */

static void CalculateRpm(void)
{
    uint32_t numerator;

    if (s_hall_period == 0U)
    {
        /* No hall pulse detected - motor stopped or startup */
        s_rpm = 0U;
    }
    else
    {
        /*
         * Target: Read TAU capture register for hall sensor period
         * s_hall_period = TDR04 (capture value from hall input)
         *
         * RPM = 60,000,000 / (period * pole_pairs)
         * Using 1 MHz timer clock
         */
        numerator = 60000000UL;
        s_rpm = (uint16_t)(numerator / ((uint32_t)s_hall_period * (uint32_t)MOTOR_POLE_PAIRS));
    }
}

/* ========================================================================
 * Public Functions
 * ======================================================================== */

void SensorProc_Init(void)
{
    s_current_ma       = 0U;
    s_current_ma_raw   = 0U;
    s_voltage_mv       = 0U;
    s_rpm              = 0U;
    s_lid_state        = LID_CLOSED;
    s_adc_diag_fault   = FLAG_CLEAR;

    s_current_filtered = 0U;
    s_voltage_divider  = (uint16_t)((ADC_CURRENT_HZ / ADC_VOLTAGE_HZ) - 1U);
    s_lid_debounce_cnt = 0U;
    s_lid_raw          = (uint8_t)LID_CLOSED;
    s_hall_period      = 0U;

    s_adc_stuck_low_cnt[0]  = 0U;
    s_adc_stuck_low_cnt[1]  = 0U;
    s_adc_stuck_high_cnt[0] = 0U;
    s_adc_stuck_high_cnt[1] = 0U;
}

void SensorProc_Update(void)
{
    uint16_t raw_current;
    uint16_t raw_voltage;
    uint16_t current_ma_raw;

    /* --- Current ADC: every call (100Hz at 10ms tick) --- */
    raw_current = HAL_ADC_Read(HAL_ADC_CH_CURRENT);
    AdcDiagCheck(HAL_ADC_CH_CURRENT, raw_current);

    current_ma_raw = AdcToCurrentMa(raw_current);
    s_current_ma_raw = current_ma_raw;
    s_current_filtered = IirFilter(s_current_filtered, current_ma_raw);
    s_current_ma = s_current_filtered;

    /* --- Voltage ADC: every 10th call (10Hz) --- */
    s_voltage_divider++;
    if (s_voltage_divider >= (ADC_CURRENT_HZ / ADC_VOLTAGE_HZ))
    {
        s_voltage_divider = 0U;

        raw_voltage = HAL_ADC_Read(HAL_ADC_CH_VOLTAGE);
        AdcDiagCheck(HAL_ADC_CH_VOLTAGE, raw_voltage);

        s_voltage_mv = AdcToVoltageMv(raw_voltage);
    }

    /* --- Lid GPIO: debounce every call --- */
    LidDebounce();

    /* --- RPM: calculate from hall capture --- */
    CalculateRpm();
}

uint16_t SensorProc_GetRpm(void)
{
    return s_rpm;
}

uint16_t SensorProc_GetCurrentMa(void)
{
    return s_current_ma;
}

uint16_t SensorProc_GetVoltageMv(void)
{
    return s_voltage_mv;
}

lid_state_t SensorProc_GetLidState(void)
{
    return s_lid_state;
}

uint16_t SensorProc_GetRawCurrentMa(void)
{
    return s_current_ma_raw;
}

uint8_t SensorProc_GetAdcDiagFault(void)
{
    return s_adc_diag_fault;
}
