/**
 * @file   safety_mon.c
 * @brief  Safety Monitor Module - implementation
 * @doc    WMC-SUD-001 §4.3 Safety Monitor Specification
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "safety_mon.h"
#include "sensor_proc.h"

/* ========================================================================
 * [SAFETY VARIABLE REGION]
 * Safety flags structure - CRC-16 protected.
 * In linker script: place in .safety_data section.
 * ======================================================================== */

/** Safety flags with CRC-16 integrity protection */
static safety_flags_t s_flags;

/* ========================================================================
 * [CONTROL VARIABLE REGION]
 * Working variables for vibration detection.
 * ======================================================================== */

/** Circular buffer for current samples (vibration analysis) */
static uint16_t s_current_history[SAFETY_VIBRATION_SAMPLES];

/** Circular buffer write index */
static uint8_t s_current_hist_idx;

/** Number of valid samples in history buffer */
static uint8_t s_current_hist_count;

/* ========================================================================
 * Internal Helper: CRC-16 CCITT calculation
 * Polynomial: 0x1021, Init: 0xFFFF
 * Operates on safety_flags_t data bytes (excluding the CRC field itself)
 * ======================================================================== */

static uint16_t CalcFlagsCrc(const safety_flags_t *flags)
{
    uint16_t crc = CRC16_INIT;
    const uint8_t *data = (const uint8_t *)flags;
    uint8_t num_bytes;
    uint8_t i;
    uint8_t bit;

    /*
     * CRC covers all fields except the crc field itself.
     * safety_flags_t layout: 6 x uint8_t fields, then uint16_t crc
     * So we CRC over the first 6 bytes.
     */
    num_bytes = (uint8_t)(sizeof(safety_flags_t) - sizeof(uint16_t));

    for (i = 0U; i < num_bytes; i++)
    {
        crc ^= ((uint16_t)data[i] << 8U);

        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (uint16_t)((crc << 1U) ^ CRC16_POLY);
            }
            else
            {
                crc = (uint16_t)(crc << 1U);
            }
        }
    }

    return crc;
}

/* ========================================================================
 * Internal Helper: Update CRC in safety flags
 * ======================================================================== */

static void UpdateFlagsCrc(void)
{
    s_flags.crc = CalcFlagsCrc(&s_flags);
}

/* ========================================================================
 * Internal Helper: Overspeed check with hysteresis
 * SET at MOTOR_OVERSPEED_RPM, CLEAR at OVERSPEED_HYST_RPM
 * ======================================================================== */

static void CheckOverspeed(uint16_t rpm)
{
    if (s_flags.overspeed == FLAG_CLEAR)
    {
        /* Not in fault - check for overspeed onset */
        if (rpm >= MOTOR_OVERSPEED_RPM)
        {
            s_flags.overspeed = FLAG_SET;
        }
    }
    else
    {
        /* In fault - check for recovery with hysteresis */
        if (rpm <= OVERSPEED_HYST_RPM)
        {
            s_flags.overspeed = FLAG_CLEAR;
        }
    }
}

/* ========================================================================
 * Internal Helper: Overcurrent check with hysteresis
 * SET at MOTOR_MAX_MA, CLEAR at OVERCURRENT_HYST_MA
 * ======================================================================== */

static void CheckOvercurrent(uint16_t current_ma)
{
    if (s_flags.overcurrent == FLAG_CLEAR)
    {
        if (current_ma >= MOTOR_MAX_MA)
        {
            s_flags.overcurrent = FLAG_SET;
        }
    }
    else
    {
        if (current_ma <= OVERCURRENT_HYST_MA)
        {
            s_flags.overcurrent = FLAG_CLEAR;
        }
    }
}

/* ========================================================================
 * Internal Helper: Lid open check
 * Direct mapping from debounced lid sensor state
 * ======================================================================== */

static void CheckLid(lid_state_t lid)
{
    if (lid == LID_OPEN)
    {
        s_flags.lid_open = FLAG_SET;
    }
    else
    {
        s_flags.lid_open = FLAG_CLEAR;
    }
}

/* ========================================================================
 * Internal Helper: Voltage window check
 * Must be within VOLTAGE_MIN_MV to VOLTAGE_MAX_MV
 * Hysteresis band: VOLTAGE_HYST_MV inside the window
 * ======================================================================== */

static void CheckVoltage(uint16_t voltage_mv)
{
    if (s_flags.voltage_error == FLAG_CLEAR)
    {
        /* Not in fault - check for window violation */
        if ((voltage_mv < VOLTAGE_MIN_MV) || (voltage_mv > VOLTAGE_MAX_MV))
        {
            s_flags.voltage_error = FLAG_SET;
        }
    }
    else
    {
        /* In fault - check for recovery with hysteresis (must be well within window) */
        if ((voltage_mv >= (VOLTAGE_MIN_MV + VOLTAGE_HYST_MV)) &&
            (voltage_mv <= (VOLTAGE_MAX_MV - VOLTAGE_HYST_MV)))
        {
            s_flags.voltage_error = FLAG_CLEAR;
        }
    }
}

/* ========================================================================
 * Internal Helper: Vibration detection via current variation analysis
 * Computes peak-to-peak variation of recent current samples.
 * If variation exceeds threshold, vibration fault is set.
 * ======================================================================== */

static void CheckVibration(uint16_t current_ma)
{
    uint16_t min_val;
    uint16_t max_val;
    uint16_t peak_to_peak;
    uint8_t  i;
    uint8_t  count;

    /* Store current sample in circular buffer */
    s_current_history[s_current_hist_idx] = current_ma;
    s_current_hist_idx++;

    if (s_current_hist_idx >= SAFETY_VIBRATION_SAMPLES)
    {
        s_current_hist_idx = 0U;
    }

    if (s_current_hist_count < SAFETY_VIBRATION_SAMPLES)
    {
        s_current_hist_count++;
    }

    /* Need full buffer before analysis */
    if (s_current_hist_count < SAFETY_VIBRATION_SAMPLES)
    {
        return;
    }

    /* Find min and max in buffer */
    min_val = 65535U;
    max_val = 0U;
    count = SAFETY_VIBRATION_SAMPLES;

    for (i = 0U; i < count; i++)
    {
        if (s_current_history[i] < min_val)
        {
            min_val = s_current_history[i];
        }
        if (s_current_history[i] > max_val)
        {
            max_val = s_current_history[i];
        }
    }

    peak_to_peak = max_val - min_val;

    if (peak_to_peak >= SAFETY_VIBRATION_THRESH_MA)
    {
        s_flags.vibration = FLAG_SET;
    }
    else
    {
        s_flags.vibration = FLAG_CLEAR;
    }
}

/* ========================================================================
 * Public Functions
 * ======================================================================== */

void SafetyMon_Init(void)
{
    uint8_t i;

    s_flags.overspeed     = FLAG_CLEAR;
    s_flags.overcurrent   = FLAG_CLEAR;
    s_flags.lid_open      = FLAG_CLEAR;
    s_flags.voltage_error = FLAG_CLEAR;
    s_flags.vibration     = FLAG_CLEAR;
    s_flags.diag_fail     = FLAG_CLEAR;

    /* Clear vibration history */
    for (i = 0U; i < SAFETY_VIBRATION_SAMPLES; i++)
    {
        s_current_history[i] = 0U;
    }
    s_current_hist_idx   = 0U;
    s_current_hist_count = 0U;

    /* Compute initial CRC */
    UpdateFlagsCrc();
}

void SafetyMon_Check(void)
{
    uint16_t    rpm;
    uint16_t    current_ma;
    uint16_t    current_ma_raw;
    uint16_t    voltage_mv;
    lid_state_t lid;
    uint8_t     adc_diag;

    /* Read processed sensor values (const access pattern) */
    rpm            = SensorProc_GetRpm();
    current_ma     = SensorProc_GetCurrentMa();
    current_ma_raw = SensorProc_GetRawCurrentMa();
    voltage_mv     = SensorProc_GetVoltageMv();
    lid            = SensorProc_GetLidState();
    adc_diag       = SensorProc_GetAdcDiagFault();

    /* Execute all safety checks */
    CheckOverspeed(rpm);
    CheckOvercurrent(current_ma);
    CheckLid(lid);
    CheckVoltage(voltage_mv);
    CheckVibration(current_ma_raw);  /* Use raw value to detect high-freq vibration */

    /* ADC diagnostic status */
    s_flags.diag_fail = adc_diag;

    /* Update CRC after all flag changes */
    UpdateFlagsCrc();
}

const safety_flags_t *SafetyMon_GetFlags(void)
{
    return &s_flags;
}

uint8_t SafetyMon_VerifyCrc(void)
{
    uint16_t computed;

    computed = CalcFlagsCrc(&s_flags);

    if (computed != s_flags.crc)
    {
        return FLAG_SET;  /* CRC mismatch = fault */
    }

    return FLAG_CLEAR;    /* CRC valid */
}
