/**
 * @file lid_mon.c
 * @brief Washing Machine Lid Monitoring
 * @details GPIO polling, debouncing, emergency stop on lid open
 * @version 1.0
 * @date 2026-02-27
 * @author TORASAN Motor Control Project
 *
 * @req DR-013: Lid position monitoring
 * @req DR-014: Emergency stop on lid open during operation
 *
 * @safety_class IEC 60730-2-1 Class B / ASIL QM
 */

#include <stdint.h>
#include <stdbool.h>
#include "lid_mon.h"
#include "motor_ctrl.h"
#include "safety_mgr.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "config.h"

/* ========================================================================
 * CONFIGURATION & CONSTANTS
 * ======================================================================== */

/**
 * @def LID_MON_GPIO_PORT
 * @brief GPIO port number for lid sensor
 * @note RL78/G14: ports P0-P15
 */
#define LID_MON_GPIO_PORT           (LID_SENSOR_PIN_PORT)

/**
 * @def LID_MON_GPIO_PIN
 * @brief GPIO pin number for lid sensor
 */
#define LID_MON_GPIO_PIN            (LID_SENSOR_PIN_BIT)

/**
 * @def LID_MON_CLOSED_STATE
 * @brief GPIO pin state when lid is closed (0=low, 1=high)
 * @note Typically active-low (closed=0, open=1) for switch contact
 */
#define LID_MON_CLOSED_STATE        (0U)

/**
 * @def LID_MON_OPEN_STATE
 * @brief GPIO pin state when lid is open
 */
#define LID_MON_OPEN_STATE          (1U)

/**
 * @def LID_MON_DEBOUNCE_SAMPLES
 * @brief Number of consistent samples needed to confirm state change
 * @safety [SR-007][SR-008][クラスB] Debouncing to filter switch bounce
 * @note 20ms debounce: 3 samples @ 10ms = 30ms total filter time
 */
#define LID_MON_DEBOUNCE_SAMPLES    (3U)

/**
 * @def LID_MON_DEBOUNCE_PERIOD_MS
 * @brief Debounce sample interval (milliseconds)
 * @note ~10ms sampling rate
 */
#define LID_MON_DEBOUNCE_PERIOD_MS  (10U)

/**
 * @def LID_MON_MOTOR_STOP_DELAY_MS
 * @brief Maximum delay from lid open to motor stop (safety requirement)
 * @safety [SR-007][クラスB] Emergency stop latency bound
 */
#define LID_MON_MOTOR_STOP_DELAY_MS (50U)

/* ========================================================================
 * MODULE STATE
 * ======================================================================== */

/**
 * @enum LidState_t
 * @brief Lid position state enumeration
 * @safety [SR-007][SR-008][クラスB] Lid states for safety logic
 */
typedef enum {
    LID_STATE_CLOSED = 0x3C3CU,    /*!< 0x3C3C: Lid closed (safe to run) */
    LID_STATE_OPEN = 0xC3C3U       /*!< 0xC3C3: Lid open (stop motor) */
} LidState_t;

/**
 * @struct LidMon_State_t
 * @brief Lid monitoring state tracker
 * @safety [SR-007][SR-008][クラスB] Lid position and fault state
 */
typedef struct {
    LidState_t current_state;          /*!< Current lid position (CLOSED/OPEN) */
    LidState_t previous_state;         /*!< Previous lid position (for edge detection) */
    uint8_t gpio_read_raw;             /*!< Raw GPIO pin state (0 or 1) */
    uint8_t debounce_sample_count;     /*!< Debounce sample accumulator */
    uint32_t last_edge_timestamp_ms;   /*!< Timestamp of last state change */
    uint32_t samples_collected;        /*!< Total GPIO samples read */
    bool lid_open_during_run_fault;    /*!< Fault flag: lid opened while motor running */
} LidMon_State_t;

/**
 * @var lid_mon_state
 * @brief Module-level lid monitoring state
 * @safety [SR-007][SR-008][クラスB] Non-volatile across function calls
 */
static LidMon_State_t lid_mon_state = {
    .current_state = LID_STATE_CLOSED,
    .previous_state = LID_STATE_CLOSED,
    .gpio_read_raw = LID_MON_CLOSED_STATE,
    .debounce_sample_count = 0U,
    .last_edge_timestamp_ms = 0UL,
    .samples_collected = 0UL,
    .lid_open_during_run_fault = false
};

/* ========================================================================
 * INITIALIZATION
 * ======================================================================== */

/**
 * @brief Initialize Lid Monitoring Module
 * @details Configure GPIO for lid sensor input (typically active-low switch)
 *
 * @safety [SR-007][SR-008][クラスB] Module initialization
 */
void LidMon_Init(void)
{
    /* [SR-007][SR-008][クラスB] Lid monitoring module initialization */

    /* Initialize GPIO as input with pull-up (typical for switch contact) */
    HAL_GPIO_InitInput(LID_MON_GPIO_PORT, LID_MON_GPIO_PIN, HAL_GPIO_PULL_UP);

    /* Reset state */
    lid_mon_state.current_state = LID_STATE_CLOSED;
    lid_mon_state.previous_state = LID_STATE_CLOSED;
    lid_mon_state.gpio_read_raw = LID_MON_CLOSED_STATE;
    lid_mon_state.debounce_sample_count = 0U;
    lid_mon_state.last_edge_timestamp_ms = 0UL;
    lid_mon_state.samples_collected = 0UL;
    lid_mon_state.lid_open_during_run_fault = false;
}

/* ========================================================================
 * GPIO SCANNING & DEBOUNCING
 * ======================================================================== */

/**
 * @brief Scan Lid Sensor with Debouncing
 * @details Read GPIO, apply state machine debouncing (20ms filter)
 *          Called periodically (e.g., every 10ms from main superloop)
 *
 * @req DR-013: Lid position monitoring
 * @safety [SR-007][SR-008][クラスB] Debounced GPIO state tracking
 */
void LidMon_Scan(void)
{
    /* [SR-007][SR-008][クラスB] Lid GPIO scan with debouncing (20ms window) */

    /* Read raw GPIO state */
    uint8_t gpio_state = HAL_GPIO_ReadPin(LID_MON_GPIO_PORT, LID_MON_GPIO_PIN);
    lid_mon_state.gpio_read_raw = gpio_state;
    lid_mon_state.samples_collected++;

    /* Debounce state machine: require N consecutive samples to confirm change */
    if (gpio_state == LID_MON_OPEN_STATE) {
        /* GPIO reads open */
        if (lid_mon_state.debounce_sample_count < LID_MON_DEBOUNCE_SAMPLES) {
            lid_mon_state.debounce_sample_count++;
        } else {
            /* Confirmed open (after 3 samples) */
            if (lid_mon_state.current_state != LID_STATE_OPEN) {
                lid_mon_state.previous_state = lid_mon_state.current_state;
                lid_mon_state.current_state = LID_STATE_OPEN;
                lid_mon_state.last_edge_timestamp_ms = HAL_Timer_GetSystemTicks_ms();

                /* [SR-007] Edge: CLOSED → OPEN detected */
            }
        }
    } else {
        /* GPIO reads closed */
        if (lid_mon_state.debounce_sample_count > 0U) {
            lid_mon_state.debounce_sample_count--;
        } else {
            /* Confirmed closed (after 3 samples) */
            if (lid_mon_state.current_state != LID_STATE_CLOSED) {
                lid_mon_state.previous_state = lid_mon_state.current_state;
                lid_mon_state.current_state = LID_STATE_CLOSED;
                lid_mon_state.last_edge_timestamp_ms = HAL_Timer_GetSystemTicks_ms();

                /* [SR-007] Edge: OPEN → CLOSED detected (motor auto-lock released) */
            }
        }
    }
}

/**
 * @brief Get Current Lid State
 * @return LID_STATE_CLOSED or LID_STATE_OPEN
 *
 * @safety [SR-007][SR-008][クラスB] Lid state query
 */
LidState_t LidMon_GetState(void)
{
    /* [SR-007][SR-008][クラスB] Query current lid position */
    return lid_mon_state.current_state;
}

/**
 * @brief Check if Lid is Closed
 * @return true if lid is confirmed closed, false otherwise
 *
 * @safety [SR-007][SR-008][クラスB] Lid closed status
 */
bool LidMon_IsClosed(void)
{
    /* [SR-007][SR-008][クラスB] Query if lid is closed */
    return (lid_mon_state.current_state == LID_STATE_CLOSED);
}

/**
 * @brief Check if Lid is Open
 * @return true if lid is confirmed open, false otherwise
 *
 * @safety [SR-007][SR-008][クラスB] Lid open status
 */
bool LidMon_IsOpen(void)
{
    /* [SR-007][SR-008][クラスB] Query if lid is open */
    return (lid_mon_state.current_state == LID_STATE_OPEN);
}

/**
 * @brief Get Debounce Sample Count
 * @return Current debounce accumulator (0-3)
 *
 * @safety [SR-007][SR-008][クラスB] Debug support
 */
uint8_t LidMon_GetDebounceCount(void)
{
    /* [SR-007][SR-008][クラスB] Query debounce state */
    return lid_mon_state.debounce_sample_count;
}

/**
 * @brief Get Total GPIO Samples Collected
 * @return Sample count for diagnostics
 *
 * @safety [SR-007][クラスB] Statistics
 */
uint32_t LidMon_GetSampleCount(void)
{
    /* [SR-007][クラスB] Query total GPIO samples */
    return lid_mon_state.samples_collected;
}

/* ========================================================================
 * EMERGENCY STOP & SAFETY LOGIC [SR-007, SR-008]
 * ======================================================================== */

/**
 * @brief Stop Motor Upon Lid Open (Emergency Stop)
 * @details CRITICAL: Immediately halt motor and trigger safety fault
 *
 * @req DR-014: Emergency stop on lid open
 * @safety [SR-007][SR-008][クラスB] Motor emergency stop: lid safety interlock
 */
void LidMon_StopMotor(void)
{
    /* [SR-007][SR-008][クラスB] Emergency stop: Lid open detected while running */

    MotorCtrl_EmergencyStop();  /* [SR-008] Force motor PWM to 0% */
    lid_mon_state.lid_open_during_run_fault = true;
    SafetyMgr_FaultLidOpen();   /* [SR-007] Trigger fault event */
}

/**
 * @brief Check for Lid Open During Run Fault
 * @details Called from main control loop to detect edge: CLOSED→OPEN while motor running
 * @return true if fault condition detected, false otherwise
 *
 * @req DR-014: Lid interlock enforcement
 * @safety [SR-007][SR-008][クラスB] Lid safety interlock check
 */
bool LidMon_CheckLidOpenFault(void)
{
    /* [SR-007][SR-008][クラスB] Detect: Lid opened while motor running */

    /* Edge detection: previous state CLOSED, current state OPEN */
    bool edge_closed_to_open = (lid_mon_state.previous_state == LID_STATE_CLOSED) &&
                               (lid_mon_state.current_state == LID_STATE_OPEN);

    /* Is motor currently running? */
    bool motor_running = MotorCtrl_IsRunning();

    /* Fault: Lid opened while motor is running */
    if (edge_closed_to_open && motor_running) {
        return true;
    }

    return false;
}

/**
 * @brief Get Lid Open During Run Fault Status
 * @return true if fault has been detected, false otherwise
 *
 * @safety [SR-007][SR-008][クラスB] Fault flag status
 */
bool LidMon_HasLidOpenFault(void)
{
    /* [SR-007][SR-008][クラスB] Query lid open fault flag */
    return lid_mon_state.lid_open_during_run_fault;
}

/**
 * @brief Clear Lid Open Fault (After Safety Manager Clear)
 * @details Only called by safety manager when user acknowledges and diagnostics pass
 *
 * @safety [SR-007][SR-008][クラスB] Fault recovery
 */
void LidMon_ClearFault(void)
{
    /* [SR-007][SR-008][クラスB] Clear lid fault flag (guarded by safety manager) */
    lid_mon_state.lid_open_during_run_fault = false;
}

/**
 * @brief Unlock Lid (Release Motor Lock After Spin-Down)
 * @details Called when wash cycle completes and motor has stopped safely
 *
 * @safety [SR-008][クラスB] Lid unlock after safe stop
 */
void LidMon_UnlockLid(void)
{
    /* [SR-008][クラスB] Release lid lock solenoid (motor stopped) */

    /* Placeholder: Control solenoid/latch release pin */
    HAL_GPIO_SetPin(LID_LOCK_SOLENOID_PORT, LID_LOCK_SOLENOID_PIN, 1U);
}

/**
 * @brief Lock Lid (Enable Motor Lock During Operation)
 * @details Called at start of wash cycle to prevent lid opening
 *
 * @safety [SR-008][クラスB] Lid lock during operation
 */
void LidMon_LockLid(void)
{
    /* [SR-008][クラスB] Engage lid lock solenoid (motor running) */

    /* Placeholder: Control solenoid/latch lock pin */
    HAL_GPIO_SetPin(LID_LOCK_SOLENOID_PORT, LID_LOCK_SOLENOID_PIN, 0U);
}

/* ========================================================================
 * PERIODIC MONITORING TASK
 * ======================================================================== */

/**
 * @brief Lid Monitoring Periodic Task (10ms loop)
 * @details Call every 10ms from main superloop.
 *          Scans GPIO, applies debouncing, checks safety interlocks.
 *
 * @safety [SR-007][SR-008][クラスB] Periodic lid monitoring execution
 */
void LidMon_Task(void)
{
    /* [SR-007][SR-008][クラスB] Periodic lid monitoring task (10ms) */

    /* Scan GPIO with debouncing */
    LidMon_Scan();

    /* Check for lid open during operation fault */
    if (LidMon_CheckLidOpenFault()) {
        LidMon_StopMotor();
    }
}

/* End of lid_mon.c */
