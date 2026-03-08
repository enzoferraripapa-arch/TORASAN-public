/**
 * @file current_mon.c
 * @brief Motor Current Monitoring Implementation
 * @details ADC sampling, low-pass filtering, overcurrent detection
 * @version 1.0
 * @date 2026-02-27
 * @author TORASAN Motor Control Project
 *
 * @req DR-009: Motor current monitoring
 * @req DR-010: Overcurrent fault detection
 *
 * @safety_class IEC 60730-2-1 Class B / ASIL QM
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "current_mon.h"
#include "safety_mgr.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "config.h"

/* ========================================================================
 * CONFIGURATION & CONSTANTS
 * ======================================================================== */

/**
 * @def CURRENT_MON_OVERCURRENT_THRESHOLD_MA
 * @brief Maximum allowable motor current (8000 mA = 8A)
 * @safety [SR-004][クラスB] Overcurrent protection threshold
 * @note Motor rated current at 1200 RPM: ~5A nominal; 8A is 160% overate threshold
 */
#define CURRENT_MON_OVERCURRENT_THRESHOLD_MA  (8000U)

/**
 * @def CURRENT_MON_ADC_RANGE_MV
 * @brief ADC full-scale input range (millivolts)
 * @note RL78/G14 ADC: 0-5.0V VDD range
 */
#define CURRENT_MON_ADC_RANGE_MV  (5000U)

/**
 * @def CURRENT_MON_ADC_RESOLUTION_BITS
 * @brief ADC resolution (10-bit on RL78/G14)
 */
#define CURRENT_MON_ADC_RESOLUTION_BITS  (10U)

/**
 * @def CURRENT_MON_ADC_MAX_VALUE
 * @brief Maximum ADC digital value (2^10 - 1 = 1023)
 */
#define CURRENT_MON_ADC_MAX_VALUE  (1023U)

/**
 * @def CURRENT_MON_SHUNT_RESISTANCE_MOHM
 * @brief Current sense shunt resistance (milliohms)
 * @note 0.01Ω = 10mΩ; 8A → 80mV drop (within ADC range)
 */
#define CURRENT_MON_SHUNT_RESISTANCE_MOHM  (10U)

/**
 * @def CURRENT_MON_IIR_LPF_ALPHA
 * @brief Low-pass filter coefficient (0.125 = 87.5% old, 12.5% new)
 * @safety [SR-005][クラスB] Noise rejection for current measurement
 * @details Stronger filtering than motor speed to reject switching noise
 */
#define CURRENT_MON_IIR_LPF_ALPHA  (0.125f)

/**
 * @def CURRENT_MON_OVERCURRENT_DEBOUNCE_COUNT
 * @brief Number of consecutive overcurrent samples to trigger fault
 * @safety [SR-004][クラスB] Debouncing to avoid transient trips
 */
#define CURRENT_MON_OVERCURRENT_DEBOUNCE_COUNT  (2U)

/**
 * @def CURRENT_MON_ADC_CHANNEL
 * @brief ADC channel number for current sense (channel 1 on RL78/G14)
 */
#define CURRENT_MON_ADC_CHANNEL  (1U)

/**
 * @def CURRENT_MON_SAMPLE_PERIOD_MS
 * @brief Current sampling period (milliseconds)
 * @note 10ms = 100Hz sampling rate
 */
#define CURRENT_MON_SAMPLE_PERIOD_MS  (10U)

/* ========================================================================
 * MODULE STATE
 * ======================================================================== */

/**
 * @struct CurrentMon_State_t
 * @brief Current monitoring state tracker
 * @safety [SR-004][SR-005][SR-006][クラスB] Current measurement and fault state
 */
typedef struct {
    float filtered_current_ma;      /*!< Filtered motor current (mA) via IIR LPF */
    float raw_current_ma;           /*!< Raw (unfiltered) current sample */
    uint16_t adc_raw_value;         /*!< Raw ADC digital value (0-1023) */
    uint32_t adc_sample_count;      /*!< Total samples collected */
    uint8_t overcurrent_counter;    /*!< Debounce counter for overcurrent */
    bool overcurrent_flag;          /*!< Persistent overcurrent flag */
    bool gate_shutdown;             /*!< MOSFET gate shutdown active */
} CurrentMon_State_t;

/**
 * @var current_mon_state
 * @brief Module-level current monitoring state
 * @safety [SR-004][SR-005][SR-006][クラスB] Non-volatile across function calls
 */
static CurrentMon_State_t current_mon_state = {
    .filtered_current_ma = 0.0f,
    .raw_current_ma = 0.0f,
    .adc_raw_value = 0U,
    .adc_sample_count = 0UL,
    .overcurrent_counter = 0U,
    .overcurrent_flag = false,
    .gate_shutdown = false
};

/* ========================================================================
 * INITIALIZATION
 * ======================================================================== */

/**
 * @brief Initialize Current Monitoring Module
 * @details Configure ADC channel for current sense shunt
 *
 * @safety [SR-004][SR-005][SR-006][クラスB] Module initialization
 */
void CurrentMon_Init(void)
{
    /* [SR-004][SR-005][SR-006][クラスB] Current monitoring module initialization */

    /* Initialize ADC for current sense channel */
    HAL_ADC_InitChannel(CURRENT_MON_ADC_CHANNEL, HAL_ADC_RES_10BIT);

    /* Reset state */
    current_mon_state.filtered_current_ma = 0.0f;
    current_mon_state.raw_current_ma = 0.0f;
    current_mon_state.adc_raw_value = 0U;
    current_mon_state.adc_sample_count = 0UL;
    current_mon_state.overcurrent_counter = 0U;
    current_mon_state.overcurrent_flag = false;
    current_mon_state.gate_shutdown = false;
}

/* ========================================================================
 * CURRENT SAMPLING & FILTERING
 * ======================================================================== */

/**
 * @brief Sample Current from ADC and Update Filtered Value
 * @details Read ADC, convert digital value to mA, apply 1st-order IIR LPF
 *
 * @req DR-009: Current measurement and filtering
 * @safety [SR-005][クラスB] Current sampling with noise filtering
 */
void CurrentMon_Sample(void)
{
    /* [SR-005][クラスB] ADC sample: convert to mA and apply IIR LPF */

    /* Read raw ADC value (0-1023 for 10-bit) */
    current_mon_state.adc_raw_value = HAL_ADC_ReadChannel(CURRENT_MON_ADC_CHANNEL);
    current_mon_state.adc_sample_count++;

    /* Convert ADC value to voltage (mV):
     * voltage_mv = (adc_value / 1024) * 5000 mV */
    float voltage_mv = (float)current_mon_state.adc_raw_value *
                       (CURRENT_MON_ADC_RANGE_MV / (float)(CURRENT_MON_ADC_MAX_VALUE + 1));

    /* Convert voltage to current (mA):
     * current_ma = voltage_mv / shunt_resistance_mohm */
    current_mon_state.raw_current_ma = voltage_mv /
                                       (float)CURRENT_MON_SHUNT_RESISTANCE_MOHM;

    /* [SR-005] Clamp to valid range (0-10A = 0-10000mA) */
    if (current_mon_state.raw_current_ma < 0.0f) {
        current_mon_state.raw_current_ma = 0.0f;
    }
    if (current_mon_state.raw_current_ma > 10000.0f) {
        current_mon_state.raw_current_ma = 10000.0f;
    }

    /* Apply 1st-order IIR Low-Pass Filter:
     * filtered = (1-alpha)*old + alpha*new */
    current_mon_state.filtered_current_ma =
        ((1.0f - CURRENT_MON_IIR_LPF_ALPHA) * current_mon_state.filtered_current_ma) +
        (CURRENT_MON_IIR_LPF_ALPHA * current_mon_state.raw_current_ma);
}

/**
 * @brief Get Filtered Current Reading
 * @return Motor current in milliamps (filtered via IIR LPF)
 *
 * @safety [SR-005][クラスB] Current query for monitoring
 */
float CurrentMon_GetFiltered_mA(void)
{
    /* [SR-005][クラスB] Query filtered current measurement */
    return current_mon_state.filtered_current_ma;
}

/**
 * @brief Get Raw (Unfiltered) Current Reading
 * @return Motor current in milliamps (raw sample)
 *
 * @safety [SR-006][クラスB] Diagnostic/raw data access
 */
float CurrentMon_GetRaw_mA(void)
{
    /* [SR-006][クラスB] Query raw (unfiltered) current sample */
    return current_mon_state.raw_current_ma;
}

/**
 * @brief Get ADC Sample Count (for diagnostics)
 * @return Total number of ADC samples collected
 *
 * @safety [SR-006][クラスB] Sampling statistics
 */
uint32_t CurrentMon_GetSampleCount(void)
{
    /* [SR-006][クラスB] Query total ADC samples */
    return current_mon_state.adc_sample_count;
}

/* ========================================================================
 * OVERCURRENT DETECTION & PROTECTION [SR-004]
 * ======================================================================== */

/**
 * @brief Check Motor Overcurrent Condition
 * @details Compare filtered current vs 8A threshold with 2-count debounce.
 *          Triggers fault only if overcurrent is sustained.
 * @return true if overcurrent is detected and debounced, false otherwise
 *
 * @req DR-010: Overcurrent fault detection
 * @safety [SR-004][クラスB] Overcurrent detection with debouncing
 */
bool CurrentMon_CheckOvercurrent(void)
{
    /* [SR-004][クラスB] Overcurrent check: 8A threshold, 2-count debounce */

    if (current_mon_state.filtered_current_ma > (float)CURRENT_MON_OVERCURRENT_THRESHOLD_MA) {
        current_mon_state.overcurrent_counter++;

        /* [SR-004] Trigger fault after debounce count exceeded */
        if (current_mon_state.overcurrent_counter >= CURRENT_MON_OVERCURRENT_DEBOUNCE_COUNT) {
            current_mon_state.overcurrent_flag = true;
            return true;
        }
    } else {
        /* [SR-004] Clear counter if below threshold */
        current_mon_state.overcurrent_counter = 0U;
        current_mon_state.overcurrent_flag = false;
    }

    return false;
}

/**
 * @brief Get Overcurrent Flag Status
 * @return true if overcurrent has been detected (persistent flag)
 *
 * @safety [SR-004][クラスB] Overcurrent event history
 */
bool CurrentMon_IsOvercurrent(void)
{
    /* [SR-004][クラスB] Query overcurrent flag */
    return current_mon_state.overcurrent_flag;
}

/**
 * @brief Get Overcurrent Debounce Counter
 * @return Current counter value (0 - CURRENT_MON_OVERCURRENT_DEBOUNCE_COUNT)
 *
 * @safety [SR-004][クラスB] Debug support
 */
uint8_t CurrentMon_GetOvercurrentCounter(void)
{
    /* [SR-004][クラスB] Query overcurrent debounce state */
    return current_mon_state.overcurrent_counter;
}

/* ========================================================================
 * GATE SHUTDOWN / EMERGENCY DISABLE [SR-006]
 * ======================================================================== */

/**
 * @brief Emergency Gate Shutdown (Stop Motor Drive)
 * @details Disable MOSFET gate drivers immediately, force motor offline
 *
 * @req DR-010: Emergency motor disable
 * @safety [SR-006][クラスB] Gate driver shutdown: motor forced offline
 */
void CurrentMon_GateShutdown(void)
{
    /* [SR-006][クラスB] Emergency gate shutdown: Motor offline immediately */

    HAL_GPIO_SetPin(GATE_DRIVER_ENABLE_PIN, 0U);  /* Pull gate enable low */
    current_mon_state.gate_shutdown = true;
}

/**
 * @brief Emergency Gate Shutdown Check
 * @return true if gate shutdown is active, false otherwise
 *
 * @safety [SR-006][クラスB] Shutdown status verification
 */
bool CurrentMon_IsGateShutdown(void)
{
    /* [SR-006][クラスB] Query gate shutdown status */
    return current_mon_state.gate_shutdown;
}

/**
 * @brief Soft Gate Enable (Resume Motor Drive After Fault Clear)
 * @details Only allowed after fault has been cleared and diagnostics pass
 *
 * @safety [SR-006][クラスB] Gate re-enable (guarded operation)
 */
void CurrentMon_GateEnable(void)
{
    /* [SR-006][クラスB] Soft gate enable: Resume motor control */

    HAL_GPIO_SetPin(GATE_DRIVER_ENABLE_PIN, 1U);  /* Pull gate enable high */
    current_mon_state.gate_shutdown = false;
}

/* ========================================================================
 * PERIODIC MONITORING TASK
 * ======================================================================== */

/**
 * @brief Current Monitoring Periodic Task (100ms loop)
 * @details Call every 10-100ms from main superloop.
 *          Samples ADC, updates filters, checks overcurrent.
 *
 * @safety [SR-004][SR-005][SR-006][クラスB] Periodic monitoring execution
 */
void CurrentMon_Task(void)
{
    /* [SR-004][SR-005][SR-006][クラスB] Periodic current monitoring task */

    /* Sample current from ADC */
    CurrentMon_Sample();

    /* Check for overcurrent condition */
    if (CurrentMon_CheckOvercurrent()) {
        SafetyMgr_FaultOvercurrent();
        CurrentMon_GateShutdown();
    }
}

/* End of current_mon.c */
