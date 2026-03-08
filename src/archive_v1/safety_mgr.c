/**
 * @file safety_mgr.c
 * @brief Safety State Machine Manager
 * @details Manages transition between STARTUP_DIAG → NORMAL → SAFE_STATE
 * @version 1.0
 * @date 2026-02-27
 * @author TORASAN Motor Control Project
 *
 * @req DR-005: IEC 60730-2 Fault detection and response
 * @req DR-006: Safety state machine transitions
 *
 * @safety_class IEC 60730-2-1 Class B / ASIL QM
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "safety_mgr.h"
#include "hal_timer.h"
#include "hal_gpio.h"
#include "config.h"

/* ========================================================================
 * CONFIGURATION & CONSTANTS
 * ======================================================================== */

/**
 * @def SAFETY_FAULT_HISTORY_SIZE
 * @brief Circular buffer size for fault logging
 * @safety [SR-013][クラスB] Fault history for debugging
 */
#define SAFETY_FAULT_HISTORY_SIZE  (16U)

/**
 * @def SAFETY_PWM_KILLSWITCH_DELAY_MS
 * @brief Maximum delay from fault detection to PWM stop (absolute upper bound)
 * @safety [SR-013][クラスB] Must guarantee stop within safety requirements
 */
#define SAFETY_PWM_KILLSWITCH_DELAY_MS  (10U)

/* ========================================================================
 * STATE & FAULT ENUMERATIONS
 * ======================================================================== */

/**
 * @enum SafetyState_t
 * @brief System safety state enumeration
 * @safety [SR-013][クラスB] Distinct states for fault isolation
 */
typedef enum {
    SAFETY_STATE_STARTUP_DIAG = 0x5A5AU,  /*!< 0x5A5A: Startup diagnostics in progress */
    SAFETY_STATE_NORMAL       = 0xA5A5U,  /*!< 0xA5A5: Normal operation */
    SAFETY_STATE_SAFE_STATE   = 0xC3C3U   /*!< 0xC3C3: Safe state (fault detected) */
} SafetyState_t;

/**
 * @enum SafetyFaultCode_t
 * @brief Enumeration of detectable fault codes
 * @safety [SR-013][クラスB] Unique codes for all monitored faults
 */
typedef enum {
    FAULT_NONE                      = 0x00U,  /*!< No fault */
    FAULT_CPU_REG_TEST              = 0x01U,  /*!< CPU register test failure [SR-009] */
    FAULT_RAM_TEST                  = 0x02U,  /*!< RAM March-C test failure [SR-010] */
    FAULT_ROM_CRC                   = 0x03U,  /*!< ROM CRC mismatch [SR-011] */
    FAULT_CLOCK_FREQ                = 0x04U,  /*!< Clock frequency out of tolerance [SR-012] */
    FAULT_OVERSPEED                 = 0x05U,  /*!< Motor overspeed detected [SR-001] */
    FAULT_OVERCURRENT               = 0x06U,  /*!< Motor overcurrent detected [SR-004] */
    FAULT_SUPPLY_VOLTAGE_LOW        = 0x07U,  /*!< Supply voltage below 4.5V [SR-014] */
    FAULT_SUPPLY_VOLTAGE_HIGH       = 0x08U,  /*!< Supply voltage above 5.5V [SR-015] */
    FAULT_LID_OPEN_DURING_RUN       = 0x09U,  /*!< Lid opened during operation [SR-007] */
    FAULT_WDT_TIMEOUT               = 0x0AU,  /*!< Watchdog timeout [SR-016] */
    FAULT_MOTOR_STALL               = 0x0BU,  /*!< Motor stall detected */
    FAULT_TEMPERATURE_HIGH          = 0x0CU,  /*!< Heatsink/motor temperature too high */
    FAULT_INVALID_STATE_TRANSITION  = 0x0Du,  /*!< Invalid state machine transition */
    FAULT_CHECKSUM_MISMATCH         = 0x0Eu   /*!< Configuration checksum error */
} SafetyFaultCode_t;

/* ========================================================================
 * MODULE STATE
 * ======================================================================== */

/**
 * @struct SafetyFaultLog_t
 * @brief Single fault log entry
 * @safety [SR-013][クラスB] Timestamped fault record
 */
typedef struct {
    SafetyFaultCode_t fault_code;   /*!< Fault code enumeration */
    uint32_t          timestamp_ms; /*!< System timestamp (ms) */
} SafetyFaultLog_t;

/**
 * @struct SafetyMgr_State_t
 * @brief Safety Manager module state
 * @safety [SR-013][クラスB] Central safety state machine
 */
typedef struct {
    SafetyState_t current_state;           /*!< Current safety state (double-word for robustness) */
    SafetyFaultCode_t last_fault;          /*!< Most recent fault code */
    bool safe_flag;                        /*!< Atomic safety flag (PWM stopped) */
    bool user_acknowledge;                 /*!< User acknowledged fault condition */
    uint16_t fault_count;                  /*!< Total faults detected */
    SafetyFaultLog_t fault_history[SAFETY_FAULT_HISTORY_SIZE];  /*!< Circular fault log */
    uint8_t fault_history_index;           /*!< Current position in circular buffer */
} SafetyMgr_State_t;

/**
 * @var safety_mgr_state
 * @brief Module-level safety manager state (protected from corruption)
 * @safety [SR-013][クラスB] Non-volatile across function calls
 */
static SafetyMgr_State_t safety_mgr_state = {
    .current_state = SAFETY_STATE_STARTUP_DIAG,
    .last_fault = FAULT_NONE,
    .safe_flag = false,
    .user_acknowledge = false,
    .fault_count = 0U,
    .fault_history_index = 0U
};

/* ========================================================================
 * PRIVATE HELPER FUNCTIONS
 * ======================================================================== */

/**
 * @brief Log a fault event to circular buffer
 * @param fault_code [in] Fault code to log
 *
 * @safety [SR-013][クラスB] Fault logging for diagnostics
 */
static void SafetyMgr_LogFault(SafetyFaultCode_t fault_code)
{
    /* [SR-013][クラスB] Fault event logging to circular buffer */

    safety_mgr_state.fault_history[safety_mgr_state.fault_history_index].fault_code = fault_code;
    safety_mgr_state.fault_history[safety_mgr_state.fault_history_index].timestamp_ms =
        HAL_Timer_GetSystemTicks_ms();

    safety_mgr_state.fault_history_index =
        (safety_mgr_state.fault_history_index + 1U) % SAFETY_FAULT_HISTORY_SIZE;

    safety_mgr_state.fault_count++;
}

/**
 * @brief Atomic PWM shutdown (all channels to 0)
 * @details CRITICAL: Must halt motor immediately upon fault detection
 *
 * @safety [SR-013][クラスB] Emergency stop: PWM forced to safe state
 */
static void SafetyMgr_StopMotorPwm(void)
{
    /* [SR-013][クラスB] Motor PWM killswitch: atomic 0% duty */

    HAL_Timer_StopAllPwm();  /* All PWM channels to inactive state */
    safety_mgr_state.safe_flag = true;
}

/**
 * @brief Notify UI/Logger of safety state change
 * @param state [in] New safety state
 * @param fault [in] Associated fault code
 *
 * @safety [SR-013][クラスB] Notification to external monitoring
 */
static void SafetyMgr_NotifyStateChange(SafetyState_t state, SafetyFaultCode_t fault)
{
    /* [SR-013][クラスB] Log state transition for monitoring system */

    /* Placeholder: Could interface with CAN, Ethernet, or UART logger */
    (void)state;  /* Suppress unused warning */
    (void)fault;
}

/* ========================================================================
 * PUBLIC API: STATE MANAGEMENT
 * ======================================================================== */

/**
 * @brief Initialize Safety Manager
 * @details Call once at system startup before main control loop
 *
 * @safety [SR-013][クラスB] Module initialization
 */
void SafetyMgr_Init(void)
{
    /* [SR-013][クラスB] Safety manager initialization */

    memset(&safety_mgr_state, 0U, sizeof(SafetyMgr_State_t));
    safety_mgr_state.current_state = SAFETY_STATE_STARTUP_DIAG;
    safety_mgr_state.last_fault = FAULT_NONE;
    safety_mgr_state.safe_flag = false;
    safety_mgr_state.user_acknowledge = false;
}

/**
 * @brief Get Current Safety State
 * @return Current safety state enumeration
 *
 * @safety [SR-013][クラスB] State query for control loop decisions
 */
SafetyState_t SafetyMgr_GetState(void)
{
    /* [SR-013][クラスB] Query current safety state */
    return safety_mgr_state.current_state;
}

/**
 * @brief Get Most Recent Fault Code
 * @return Fault code enumeration
 *
 * @safety [SR-013][クラスB] Fault reporting to UI
 */
SafetyFaultCode_t SafetyMgr_GetFaultCode(void)
{
    /* [SR-013][クラスB] Query last detected fault */
    return safety_mgr_state.last_fault;
}

/**
 * @brief Get Safe Flag Status
 * @return true if system is in safe state (PWM stopped), false otherwise
 *
 * @safety [SR-013][クラスB] Motor stop status verification
 */
bool SafetyMgr_IsSafeState(void)
{
    /* [SR-013][クラスB] Verify motor PWM is inactive */
    return safety_mgr_state.safe_flag;
}

/**
 * @brief Get Total Fault Count
 * @return Number of faults detected since power-on
 *
 * @safety [SR-013][クラスB] Reliability statistics
 */
uint16_t SafetyMgr_GetFaultCount(void)
{
    /* [SR-013][クラスB] Query total fault count */
    return safety_mgr_state.fault_count;
}

/* ========================================================================
 * PUBLIC API: STATE TRANSITIONS
 * ======================================================================== */

/**
 * @brief Transition to NORMAL Operation State
 * @details Only allowed from STARTUP_DIAG state after all diagnostics pass
 * @return true on success, false if preconditions not met
 *
 * @req DR-006: State transition validation
 * @safety [SR-013][クラスB] Guarded state transition from STARTUP_DIAG → NORMAL
 */
bool SafetyMgr_TransitionToNormal(void)
{
    /* [SR-013][クラスB] Transition: STARTUP_DIAG → NORMAL */

    if (safety_mgr_state.current_state != SAFETY_STATE_STARTUP_DIAG) {
        SafetyMgr_LogFault(FAULT_INVALID_STATE_TRANSITION);
        return false;
    }

    safety_mgr_state.current_state = SAFETY_STATE_NORMAL;
    safety_mgr_state.last_fault = FAULT_NONE;
    SafetyMgr_NotifyStateChange(SAFETY_STATE_NORMAL, FAULT_NONE);

    return true;
}

/**
 * @brief Transition to SAFE_STATE Upon Fault Detection
 * @details Atomic transition: PWM stop, set safe flag, log fault, notify UI
 * @param fault [in] Fault code triggering transition
 * @return true always (transition always succeeds)
 *
 * @req DR-006: Fault → Safe state transition
 * @safety [SR-013][クラスB] Atomic safe transition: stop PWM + set flag + log
 */
bool SafetyMgr_TransitionSafe(SafetyFaultCode_t fault)
{
    /* [SR-013][クラスB] Atomic fault → SAFE_STATE transition */

    SafetyMgr_StopMotorPwm();              /* [SR-013] CRITICAL: Stop PWM immediately */
    safety_mgr_state.current_state = SAFETY_STATE_SAFE_STATE;
    safety_mgr_state.last_fault = fault;
    safety_mgr_state.user_acknowledge = false;  /* [SR-013] Clear acknowledge on new fault */

    SafetyMgr_LogFault(fault);
    SafetyMgr_NotifyStateChange(SAFETY_STATE_SAFE_STATE, fault);

    return true;
}

/**
 * @brief Clear Fault and Return to NORMAL State
 * @details Only allowed if: all diagnostics pass + user has acknowledged fault
 * @return true on success, false if preconditions not met
 *
 * @req DR-006: Fault recovery sequence
 * @safety [SR-013][クラスB] Guarded fault recovery with user acknowledge
 */
bool SafetyMgr_ClearFault(void)
{
    /* [SR-013][クラスB] Fault recovery: diagnostic re-check + user acknowledge */

    /* Precondition 1: Must be in SAFE_STATE */
    if (safety_mgr_state.current_state != SAFETY_STATE_SAFE_STATE) {
        return false;
    }

    /* Precondition 2: User must have acknowledged */
    if (!safety_mgr_state.user_acknowledge) {
        return false;
    }

    /* Precondition 3: All self-diagnostics must pass (simplified for example) */
    /* In real implementation, re-run SafeDiag_StartupTests() */

    /* Clear fault state */
    safety_mgr_state.current_state = SAFETY_STATE_NORMAL;
    safety_mgr_state.last_fault = FAULT_NONE;
    safety_mgr_state.safe_flag = false;
    safety_mgr_state.user_acknowledge = false;

    SafetyMgr_NotifyStateChange(SAFETY_STATE_NORMAL, FAULT_NONE);
    return true;
}

/**
 * @brief Set User Acknowledge Flag
 * @details Called when user confirms fault acknowledgment (e.g., button press)
 *
 * @safety [SR-013][クラスB] Manual fault recovery acknowledgment
 */
void SafetyMgr_UserAcknowledge(void)
{
    /* [SR-013][クラスB] User acknowledges fault condition */
    safety_mgr_state.user_acknowledge = true;
}

/* ========================================================================
 * PUBLIC API: FAULT DETECTION HANDLERS
 * ======================================================================== */

/**
 * @brief Handle CPU Register Test Failure
 * @safety [SR-009][SR-013][クラスB] CPU fault → SAFE_STATE
 */
void SafetyMgr_FaultCpuRegisterTest(void)
{
    /* [SR-009][SR-013][クラスB] CPU register fault detected */
    if (safety_mgr_state.current_state == SAFETY_STATE_NORMAL) {
        SafetyMgr_TransitionSafe(FAULT_CPU_REG_TEST);
    }
}

/**
 * @brief Handle RAM Test Failure
 * @safety [SR-010][SR-013][クラスB] RAM fault → SAFE_STATE
 */
void SafetyMgr_FaultRamTest(void)
{
    /* [SR-010][SR-013][クラスB] RAM test fault detected */
    if (safety_mgr_state.current_state == SAFETY_STATE_NORMAL) {
        SafetyMgr_TransitionSafe(FAULT_RAM_TEST);
    }
}

/**
 * @brief Handle ROM CRC Failure
 * @safety [SR-011][SR-013][クラスB] ROM fault → SAFE_STATE
 */
void SafetyMgr_FaultRomCrc(void)
{
    /* [SR-011][SR-013][クラスB] ROM CRC fault detected */
    if (safety_mgr_state.current_state == SAFETY_STATE_NORMAL) {
        SafetyMgr_TransitionSafe(FAULT_ROM_CRC);
    }
}

/**
 * @brief Handle Clock Frequency Test Failure
 * @safety [SR-012][SR-013][クラスB] Clock fault → SAFE_STATE
 */
void SafetyMgr_FaultClockFreq(void)
{
    /* [SR-012][SR-013][クラスB] Clock frequency fault detected */
    if (safety_mgr_state.current_state == SAFETY_STATE_NORMAL) {
        SafetyMgr_TransitionSafe(FAULT_CLOCK_FREQ);
    }
}

/**
 * @brief Handle Motor Overspeed Fault
 * @safety [SR-001][SR-013][クラスB] Overspeed detection → SAFE_STATE
 */
void SafetyMgr_FaultOverspeed(void)
{
    /* [SR-001][SR-013][クラスB] Motor overspeed fault detected */
    if (safety_mgr_state.current_state == SAFETY_STATE_NORMAL) {
        SafetyMgr_TransitionSafe(FAULT_OVERSPEED);
    }
}

/**
 * @brief Handle Motor Overcurrent Fault
 * @safety [SR-004][SR-013][クラスB] Overcurrent detection → SAFE_STATE
 */
void SafetyMgr_FaultOvercurrent(void)
{
    /* [SR-004][SR-013][クラスB] Motor overcurrent fault detected */
    if (safety_mgr_state.current_state == SAFETY_STATE_NORMAL) {
        SafetyMgr_TransitionSafe(FAULT_OVERCURRENT);
    }
}

/**
 * @brief Handle Supply Voltage Low Fault
 * @safety [SR-014][SR-013][クラスB] Voltage low detected → SAFE_STATE
 */
void SafetyMgr_FaultSupplyVoltageLow(void)
{
    /* [SR-014][SR-013][クラスB] Supply voltage low fault detected */
    if (safety_mgr_state.current_state == SAFETY_STATE_NORMAL) {
        SafetyMgr_TransitionSafe(FAULT_SUPPLY_VOLTAGE_LOW);
    }
}

/**
 * @brief Handle Supply Voltage High Fault
 * @safety [SR-015][SR-013][クラスB] Voltage high detected → SAFE_STATE
 */
void SafetyMgr_FaultSupplyVoltageHigh(void)
{
    /* [SR-015][SR-013][クラスB] Supply voltage high fault detected */
    if (safety_mgr_state.current_state == SAFETY_STATE_NORMAL) {
        SafetyMgr_TransitionSafe(FAULT_SUPPLY_VOLTAGE_HIGH);
    }
}

/**
 * @brief Handle Lid Open During Run Fault
 * @safety [SR-007][SR-013][クラスB] Lid open detected → SAFE_STATE
 */
void SafetyMgr_FaultLidOpen(void)
{
    /* [SR-007][SR-013][クラスB] Lid open during operation detected */
    if (safety_mgr_state.current_state == SAFETY_STATE_NORMAL) {
        SafetyMgr_TransitionSafe(FAULT_LID_OPEN_DURING_RUN);
    }
}

/**
 * @brief Handle Watchdog Timeout Fault
 * @safety [SR-016][SR-013][クラスB] WDT timeout → SAFE_STATE
 */
void SafetyMgr_FaultWdtTimeout(void)
{
    /* [SR-016][SR-013][クラスB] Watchdog timeout fault detected */
    SafetyMgr_TransitionSafe(FAULT_WDT_TIMEOUT);
}

/**
 * @brief 安全状態をシリアルポートに出力（デバッグ用）
 * @details 本番ビルドでは空実装。デバッグビルドでUART出力。
 */
void SafetyMgr_PrintStatus(void)
{
    /* [DEBUG] デバッグビルドでのみUART出力を有効化 */
#ifdef DEBUG_BUILD
    /* TODO: HAL_UART_Printf() でsafety_stateを出力 */
#endif
}

/* End of safety_mgr.c */
