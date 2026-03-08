/**
 * @file voltage_mon.c
 * @brief Supply Voltage Monitoring Implementation
 * @details VDD monitoring, voltage window validation (4.5V-5.5V)
 * @version 1.0
 * @date 2026-02-27
 * @author TORASAN Motor Control Project
 *
 * @req DR-011: Supply voltage monitoring
 * @req DR-012: Voltage window validation
 *
 * @safety_class IEC 60730-2-1 Class B / ASIL QM
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "voltage_mon.h"
#include "safety_mgr.h"
#include "hal_adc.h"
#include "config.h"

/* ========================================================================
 * CONFIGURATION & CONSTANTS
 * ======================================================================== */

/**
 * @def VOLTAGE_MON_VDD_MIN_MV
 * @brief Minimum acceptable supply voltage (4.5V)
 * @safety [SR-014][クラスB] Lower voltage limit
 * @note Below this: MCU clock may be unstable, ADC inaccurate
 */
#define VOLTAGE_MON_VDD_MIN_MV      (4500U)

/**
 * @def VOLTAGE_MON_VDD_MAX_MV
 * @brief Maximum acceptable supply voltage (5.5V)
 * @safety [SR-015][クラスB] Upper voltage limit
 * @note Above this: Risk of component damage, voltage regulator stress
 */
#define VOLTAGE_MON_VDD_MAX_MV      (5500U)

/**
 * @def VOLTAGE_MON_VDD_NOMINAL_MV
 * @brief Nominal supply voltage (5.0V at nominal load)
 * @safety [SR-014][SR-015][クラスB] Reference for monitoring calculations
 */
#define VOLTAGE_MON_VDD_NOMINAL_MV  (5000U)

/**
 * @def VOLTAGE_MON_ADC_RANGE_MV
 * @brief ADC full-scale input range (millivolts)
 * @note RL78/G14 ADC: 0-5.0V VDD range
 */
#define VOLTAGE_MON_ADC_RANGE_MV    (5000U)

/**
 * @def VOLTAGE_MON_ADC_RESOLUTION_BITS
 * @brief ADC resolution (10-bit on RL78/G14)
 */
#define VOLTAGE_MON_ADC_RESOLUTION_BITS  (10U)

/**
 * @def VOLTAGE_MON_ADC_MAX_VALUE
 * @brief Maximum ADC digital value (2^10 - 1 = 1023)
 */
#define VOLTAGE_MON_ADC_MAX_VALUE   (1023U)

/**
 * @def VOLTAGE_MON_IIR_LPF_ALPHA
 * @brief Low-pass filter coefficient (0.1 = 90% old, 10% new)
 * @safety [SR-014][SR-015][クラスB] Slow filtering to catch gradual droop
 * @details Very slow filter to reject noise but track long-term voltage trends
 */
#define VOLTAGE_MON_IIR_LPF_ALPHA   (0.1f)

/**
 * @def VOLTAGE_MON_LOW_VOLT_DEBOUNCE_COUNT
 * @brief Number of consecutive low voltage samples to trigger fault
 * @safety [SR-014][クラスB] Debouncing to avoid transient faults
 */
#define VOLTAGE_MON_LOW_VOLT_DEBOUNCE_COUNT  (3U)

/**
 * @def VOLTAGE_MON_HIGH_VOLT_DEBOUNCE_COUNT
 * @brief Number of consecutive high voltage samples to trigger fault
 * @safety [SR-015][クラスB] Debouncing to avoid transient faults
 */
#define VOLTAGE_MON_HIGH_VOLT_DEBOUNCE_COUNT  (3U)

/**
 * @def VOLTAGE_MON_ADC_CHANNEL
 * @brief ADC channel number for VDD sense (typically internal VREF on RL78)
 * @note Many MCUs have internal VDD/2 divider on dedicated ADC channel
 */
#define VOLTAGE_MON_ADC_CHANNEL     (0U)  /* Channel 0: VDD measurement */

/* ========================================================================
 * MODULE STATE
 * ======================================================================== */

/**
 * @struct VoltageMon_State_t
 * @brief Voltage monitoring state tracker
 * @safety [SR-014][SR-015][クラスB] Voltage measurement and fault state
 */
typedef struct {
    float filtered_voltage_mv;      /*!< Filtered supply voltage (mV) via IIR LPF */
    float raw_voltage_mv;           /*!< Raw (unfiltered) voltage sample */
    uint16_t adc_raw_value;         /*!< Raw ADC digital value (0-1023) */
    uint32_t adc_sample_count;      /*!< Total samples collected */
    uint8_t low_voltage_counter;    /*!< Debounce counter for low voltage */
    uint8_t high_voltage_counter;   /*!< Debounce counter for high voltage */
    bool low_voltage_flag;          /*!< Low voltage fault flag */
    bool high_voltage_flag;         /*!< High voltage fault flag */
} VoltageMon_State_t;

/**
 * @var voltage_mon_state
 * @brief Module-level voltage monitoring state
 * @safety [SR-014][SR-015][クラスB] Non-volatile across function calls
 */
static VoltageMon_State_t voltage_mon_state = {
    .filtered_voltage_mv = VOLTAGE_MON_VDD_NOMINAL_MV,
    .raw_voltage_mv = VOLTAGE_MON_VDD_NOMINAL_MV,
    .adc_raw_value = 512U,  /* ~50% of 1024 = ~2.5V (mid-range) */
    .adc_sample_count = 0UL,
    .low_voltage_counter = 0U,
    .high_voltage_counter = 0U,
    .low_voltage_flag = false,
    .high_voltage_flag = false
};

/* ========================================================================
 * INITIALIZATION
 * ======================================================================== */

/**
 * @brief Initialize Voltage Monitoring Module
 * @details Configure ADC channel for VDD measurement
 *
 * @safety [SR-014][SR-015][クラスB] Module initialization
 */
void VoltageMon_Init(void)
{
    /* [SR-014][SR-015][クラスB] Voltage monitoring module initialization */

    /* Initialize ADC for VDD sense channel */
    HAL_ADC_InitChannel(VOLTAGE_MON_ADC_CHANNEL, HAL_ADC_RES_10BIT);

    /* Reset state to nominal voltage */
    voltage_mon_state.filtered_voltage_mv = VOLTAGE_MON_VDD_NOMINAL_MV;
    voltage_mon_state.raw_voltage_mv = VOLTAGE_MON_VDD_NOMINAL_MV;
    voltage_mon_state.adc_raw_value = 512U;
    voltage_mon_state.adc_sample_count = 0UL;
    voltage_mon_state.low_voltage_counter = 0U;
    voltage_mon_state.high_voltage_counter = 0U;
    voltage_mon_state.low_voltage_flag = false;
    voltage_mon_state.high_voltage_flag = false;
}

/* ========================================================================
 * VOLTAGE SAMPLING & FILTERING
 * ======================================================================== */

/**
 * @brief Sample VDD from ADC and Update Filtered Value
 * @details Read ADC, convert digital value to millivolts, apply 1st-order IIR LPF
 *
 * @req DR-011: Voltage measurement
 * @safety [SR-014][SR-015][クラスB] Voltage sampling with noise filtering
 */
void VoltageMon_Sample(void)
{
    /* [SR-014][SR-015][クラスB] ADC sample: convert to mV and apply IIR LPF */

    /* Read raw ADC value (0-1023 for 10-bit) */
    voltage_mon_state.adc_raw_value = HAL_ADC_ReadChannel(VOLTAGE_MON_ADC_CHANNEL);
    voltage_mon_state.adc_sample_count++;

    /* Convert ADC value to voltage (mV):
     * voltage_mv = (adc_value / 1024) * 5000 mV
     *
     * Many RL78 designs use internal VDD/2 divider, so:
     * voltage_mv = (adc_value / 1024) * 5000 * 2 = (adc_value * 10000) / 1024
     * (or use direct VDD measurement if external divider present)
     */
    float voltage_mv = (float)voltage_mon_state.adc_raw_value *
                       (VOLTAGE_MON_ADC_RANGE_MV / (float)(VOLTAGE_MON_ADC_MAX_VALUE + 1));

    voltage_mon_state.raw_voltage_mv = voltage_mv;

    /* Apply 1st-order IIR Low-Pass Filter (slow filtering for voltage):
     * filtered = (1-alpha)*old + alpha*new */
    voltage_mon_state.filtered_voltage_mv =
        ((1.0f - VOLTAGE_MON_IIR_LPF_ALPHA) * voltage_mon_state.filtered_voltage_mv) +
        (VOLTAGE_MON_IIR_LPF_ALPHA * voltage_mon_state.raw_voltage_mv);
}

/**
 * @brief Get Filtered Supply Voltage
 * @return Supply voltage in millivolts (filtered via IIR LPF)
 *
 * @safety [SR-014][SR-015][クラスB] Voltage query for monitoring
 */
float VoltageMon_GetFiltered_mV(void)
{
    /* [SR-014][SR-015][クラスB] Query filtered voltage measurement */
    return voltage_mon_state.filtered_voltage_mv;
}

/**
 * @brief Get Raw (Unfiltered) Supply Voltage
 * @return Supply voltage in millivolts (raw sample)
 *
 * @safety [SR-014][SR-015][クラスB] Diagnostic/raw data access
 */
float VoltageMon_GetRaw_mV(void)
{
    /* [SR-014][SR-015][クラスB] Query raw (unfiltered) voltage sample */
    return voltage_mon_state.raw_voltage_mv;
}

/**
 * @brief Get ADC Sample Count (for diagnostics)
 * @return Total number of ADC samples collected
 *
 * @safety [SR-014][SR-015][クラスB] Sampling statistics
 */
uint32_t VoltageMon_GetSampleCount(void)
{
    /* [SR-014][SR-015][クラスB] Query total ADC samples */
    return voltage_mon_state.adc_sample_count;
}

/* ========================================================================
 * VOLTAGE WINDOW VALIDATION [SR-014, SR-015]
 * ======================================================================== */

/**
 * @brief Check Voltage Window: Low Boundary (4.5V)
 * @details Compare voltage vs minimum threshold (4.5V) with debounce.
 * @return true if voltage low condition detected, false otherwise
 *
 * @req DR-012: Low voltage detection
 * @safety [SR-014][クラスB] Low voltage detection with debouncing
 */
bool VoltageMon_CheckVoltageWindow_Low(void)
{
    /* [SR-014][クラスB] Low voltage check: 4.5V threshold, 3-count debounce */

    if (voltage_mon_state.filtered_voltage_mv < (float)VOLTAGE_MON_VDD_MIN_MV) {
        voltage_mon_state.low_voltage_counter++;

        /* [SR-014] Trigger fault after debounce count exceeded */
        if (voltage_mon_state.low_voltage_counter >= VOLTAGE_MON_LOW_VOLT_DEBOUNCE_COUNT) {
            voltage_mon_state.low_voltage_flag = true;
            return true;
        }
    } else {
        /* [SR-014] Clear counter if above threshold */
        voltage_mon_state.low_voltage_counter = 0U;
        voltage_mon_state.low_voltage_flag = false;
    }

    return false;
}

/**
 * @brief Check Voltage Window: High Boundary (5.5V)
 * @details Compare voltage vs maximum threshold (5.5V) with debounce.
 * @return true if voltage high condition detected, false otherwise
 *
 * @req DR-012: High voltage detection
 * @safety [SR-015][クラスB] High voltage detection with debouncing
 */
bool VoltageMon_CheckVoltageWindow_High(void)
{
    /* [SR-015][クラスB] High voltage check: 5.5V threshold, 3-count debounce */

    if (voltage_mon_state.filtered_voltage_mv > (float)VOLTAGE_MON_VDD_MAX_MV) {
        voltage_mon_state.high_voltage_counter++;

        /* [SR-015] Trigger fault after debounce count exceeded */
        if (voltage_mon_state.high_voltage_counter >= VOLTAGE_MON_HIGH_VOLT_DEBOUNCE_COUNT) {
            voltage_mon_state.high_voltage_flag = true;
            return true;
        }
    } else {
        /* [SR-015] Clear counter if below threshold */
        voltage_mon_state.high_voltage_counter = 0U;
        voltage_mon_state.high_voltage_flag = false;
    }

    return false;
}

/**
 * @brief Check Complete Voltage Window (4.5V ≤ VDD ≤ 5.5V)
 * @details Combined check for both low and high voltage conditions
 * @return true if voltage is outside valid window, false if OK
 *
 * @req DR-012: Voltage window validation
 * @safety [SR-014][SR-015][クラスB] Complete voltage window check
 */
bool VoltageMon_CheckVoltageWindow(void)
{
    /* [SR-014][SR-015][クラスB] Complete voltage window validation */

    bool low_fault = VoltageMon_CheckVoltageWindow_Low();
    bool high_fault = VoltageMon_CheckVoltageWindow_High();

    return (low_fault || high_fault);
}

/**
 * @brief Get Low Voltage Flag Status
 * @return true if low voltage fault detected, false otherwise
 *
 * @safety [SR-014][クラスB] Low voltage fault status
 */
bool VoltageMon_IsLowVoltage(void)
{
    /* [SR-014][クラスB] Query low voltage flag */
    return voltage_mon_state.low_voltage_flag;
}

/**
 * @brief Get High Voltage Flag Status
 * @return true if high voltage fault detected, false otherwise
 *
 * @safety [SR-015][クラスB] High voltage fault status
 */
bool VoltageMon_IsHighVoltage(void)
{
    /* [SR-015][クラスB] Query high voltage flag */
    return voltage_mon_state.high_voltage_flag;
}

/**
 * @brief Get Low Voltage Debounce Counter
 * @return Current counter value (0 - VOLTAGE_MON_LOW_VOLT_DEBOUNCE_COUNT)
 *
 * @safety [SR-014][クラスB] Debug support
 */
uint8_t VoltageMon_GetLowVoltCounter(void)
{
    /* [SR-014][クラスB] Query low voltage debounce state */
    return voltage_mon_state.low_voltage_counter;
}

/**
 * @brief Get High Voltage Debounce Counter
 * @return Current counter value (0 - VOLTAGE_MON_HIGH_VOLT_DEBOUNCE_COUNT)
 *
 * @safety [SR-015][クラスB] Debug support
 */
uint8_t VoltageMon_GetHighVoltCounter(void)
{
    /* [SR-015][クラスB] Query high voltage debounce state */
    return voltage_mon_state.high_voltage_counter;
}

/* ========================================================================
 * PERIODIC MONITORING TASK
 * ======================================================================== */

/**
 * @brief Voltage Monitoring Periodic Task (100ms loop)
 * @details Call every 100ms from main superloop.
 *          Samples ADC, updates filters, checks voltage window.
 *
 * @safety [SR-014][SR-015][クラスB] Periodic voltage monitoring execution
 */
void VoltageMon_Task(void)
{
    /* [SR-014][SR-015][クラスB] Periodic voltage monitoring task */

    /* Sample voltage from ADC */
    VoltageMon_Sample();

    /* Check voltage window */
    if (VoltageMon_CheckVoltageWindow_Low()) {
        SafetyMgr_FaultSupplyVoltageLow();
    }

    if (VoltageMon_CheckVoltageWindow_High()) {
        SafetyMgr_FaultSupplyVoltageHigh();
    }
}

/* End of voltage_mon.c */
