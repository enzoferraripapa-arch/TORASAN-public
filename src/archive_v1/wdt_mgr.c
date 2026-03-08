/**
 * @file wdt_mgr.c
 * @brief Watchdog Timer Management
 * @details WDT initialization, periodic kick, timeout handling
 * @version 1.0
 * @date 2026-02-27
 * @author TORASAN Motor Control Project
 *
 * @req DR-015: Watchdog timer initialization and refresh
 * @req DR-016: Watchdog timeout detection and fault handling
 *
 * @safety_class IEC 60730-2-1 Class B / ASIL QM
 */

#include <stdint.h>
#include <stdbool.h>
#include "wdt_mgr.h"
#include "safety_mgr.h"
#include "hal_timer.h"
#include "config.h"

/* ========================================================================
 * CONFIGURATION & CONSTANTS
 * ======================================================================== */

/**
 * @def WDT_MGR_TIMEOUT_MS
 * @brief Watchdog timeout interval (100 milliseconds)
 * @safety [SR-016][クラスB] WDT timeout = maximum allowed superloop period
 * @note If superloop doesn't kick within 100ms, WDT triggers reset
 */
#define WDT_MGR_TIMEOUT_MS          (100U)

/**
 * @def WDT_MGR_KICK_PERIOD_MS
 * @brief Desired watchdog kick period (50 milliseconds)
 * @safety [SR-016][クラスB] Kick periodicity (safety margin: 50% of timeout)
 * @note Must kick at least once every 100ms; 50ms provides 2x safety margin
 */
#define WDT_MGR_KICK_PERIOD_MS      (50U)

/**
 * @def WDT_MGR_MARGIN_MS
 * @brief Safety margin below timeout (20ms = 20% safety factor)
 * @safety [SR-016][クラスB] Reserved margin to detect latency overruns
 * @details If superloop takes >80ms, WDT warning triggered (before actual timeout)
 */
#define WDT_MGR_MARGIN_MS           (20U)

/**
 * @def WDT_MGR_KICK_THRESHOLD_MS
 * @brief Time since last kick before next kick allowed (anti-aliasing)
 * @safety [SR-016][クラスB] Prevents spurious dual-kicks in same cycle
 */
#define WDT_MGR_KICK_THRESHOLD_MS   (5U)

/**
 * @def WDT_MGR_TIMEOUT_COUNT_LIMIT
 * @brief Maximum number of WDT timeouts before system lockdown
 * @safety [SR-016][クラスB] Persistent fault after repeated timeouts
 */
#define WDT_MGR_TIMEOUT_COUNT_LIMIT (3U)

/* ========================================================================
 * MODULE STATE
 * ======================================================================== */

/**
 * @enum WdtState_t
 * @brief Watchdog timer state enumeration
 * @safety [SR-016][クラスB] WDT state tracking
 */
typedef enum {
    WDT_STATE_DISABLED = 0x0000U,   /*!< 0x0000: WDT not yet initialized */
    WDT_STATE_ENABLED = 0xA5A5U,    /*!< 0xA5A5: WDT active and monitoring */
    WDT_STATE_FAULTED = 0x5A5AU     /*!< 0x5A5A: WDT timeout detected */
} WdtState_t;

/**
 * @struct WdtMgr_State_t
 * @brief Watchdog manager state tracker
 * @safety [SR-016][クラスB] WDT operation state
 */
typedef struct {
    WdtState_t wdt_state;               /*!< Current WDT state (DISABLED/ENABLED/FAULTED) */
    uint32_t last_kick_timestamp_ms;    /*!< System time of last successful kick */
    uint32_t last_timeout_timestamp_ms; /*!< System time of last timeout event */
    uint16_t timeout_count;             /*!< Total number of WDT timeouts */
    uint16_t successful_kick_count;     /*!< Total number of successful kicks */
    bool kick_pending;                  /*!< Flag: kick pending but not yet executed */
    bool timeout_occurred;              /*!< Flag: timeout detected (persistent until cleared) */
} WdtMgr_State_t;

/**
 * @var wdt_mgr_state
 * @brief Module-level watchdog manager state
 * @safety [SR-016][クラスB] Non-volatile across function calls
 */
static WdtMgr_State_t wdt_mgr_state = {
    .wdt_state = WDT_STATE_DISABLED,
    .last_kick_timestamp_ms = 0UL,
    .last_timeout_timestamp_ms = 0UL,
    .timeout_count = 0U,
    .successful_kick_count = 0U,
    .kick_pending = false,
    .timeout_occurred = false
};

/* ========================================================================
 * WATCHDOG INITIALIZATION
 * ======================================================================== */

/**
 * @brief Initialize Watchdog Timer
 * @details Configure WDT with 100ms timeout, start monitoring
 *          CRITICAL: Must be called during startup before control loop begins
 *
 * @req DR-015: Watchdog timer initialization
 * @safety [SR-016][クラスB] WDT initialization and configuration
 */
void WdtMgr_Init(void)
{
    /* [SR-016][クラスB] Watchdog timer initialization: 100ms timeout */

    /* Reset state */
    wdt_mgr_state.wdt_state = WDT_STATE_DISABLED;
    wdt_mgr_state.last_kick_timestamp_ms = 0UL;
    wdt_mgr_state.last_timeout_timestamp_ms = 0UL;
    wdt_mgr_state.timeout_count = 0U;
    wdt_mgr_state.successful_kick_count = 0U;
    wdt_mgr_state.kick_pending = false;
    wdt_mgr_state.timeout_occurred = false;

    /* Configure and start hardware WDT */
    HAL_Timer_WdtInit(WDT_MGR_TIMEOUT_MS);

    /* [SR-016] Mark as enabled */
    wdt_mgr_state.wdt_state = WDT_STATE_ENABLED;
}

/**
 * @brief Kick Watchdog Timer (Refresh)
 * @details Reset WDT counter, acknowledge superloop execution
 *          CRITICAL: Must be called at least once every 100ms
 *          IDEAL: Call every 50ms for 2x safety margin
 *
 * @req DR-015: Watchdog timer refresh
 * @safety [SR-016][クラスB] WDT kick: acknowledge superloop progress
 */
void WdtMgr_Kick(void)
{
    /* [SR-016][クラスB] Watchdog kick: reset timeout counter */

    uint32_t now_ms = HAL_Timer_GetSystemTicks_ms();

    /* Anti-aliasing: prevent dual-kick in same cycle */
    if ((now_ms - wdt_mgr_state.last_kick_timestamp_ms) < WDT_MGR_KICK_THRESHOLD_MS) {
        return;  /* Too soon; skip this kick to avoid double-refresh artifacts */
    }

    /* Clear WDT counter (kick the dog) */
    HAL_Timer_WdtKick();

    /* Update state */
    wdt_mgr_state.last_kick_timestamp_ms = now_ms;
    wdt_mgr_state.successful_kick_count++;
    wdt_mgr_state.timeout_occurred = false;

    /* [SR-016] Maintain ENABLED state */
    if (wdt_mgr_state.wdt_state != WDT_STATE_FAULTED) {
        wdt_mgr_state.wdt_state = WDT_STATE_ENABLED;
    }
}

/* ========================================================================
 * WATCHDOG TIMEOUT DETECTION & HANDLING
 * ======================================================================== */

/**
 * @brief Watchdog Timeout Interrupt Handler
 * @details Called by hardware WDT ISR on timeout.
 *          CRITICAL: Stop motor immediately, set fault state, disable further kicks
 *
 * @req DR-016: Watchdog timeout handling
 * @safety [SR-016][クラスB] WDT timeout ISR: emergency stop
 */
void WdtMgr_TimeoutISR(void)
{
    /* [SR-016][クラスB] Watchdog timeout ISR: emergency stop */

    uint32_t now_ms = HAL_Timer_GetSystemTicks_ms();

    /* Increment timeout counter */
    wdt_mgr_state.timeout_count++;
    wdt_mgr_state.last_timeout_timestamp_ms = now_ms;

    /* Set timeout flag */
    wdt_mgr_state.timeout_occurred = true;
    wdt_mgr_state.wdt_state = WDT_STATE_FAULTED;

    /* Trigger safety manager fault */
    SafetyMgr_FaultWdtTimeout();
}

/**
 * @brief Check Watchdog Timeout Status
 * @return true if timeout has been detected, false otherwise
 *
 * @safety [SR-016][クラスB] Timeout status query
 */
bool WdtMgr_IsTimeout(void)
{
    /* [SR-016][クラスB] Query WDT timeout flag */
    return wdt_mgr_state.timeout_occurred;
}

/**
 * @brief Get Watchdog State
 * @return Current WDT state (DISABLED/ENABLED/FAULTED)
 *
 * @safety [SR-016][クラスB] WDT state query
 */
WdtState_t WdtMgr_GetState(void)
{
    /* [SR-016][クラスB] Query WDT state */
    return wdt_mgr_state.wdt_state;
}

/**
 * @brief Get Total WDT Timeout Count
 * @return Number of timeouts detected since power-on
 *
 * @safety [SR-016][クラスB] Timeout statistics
 */
uint16_t WdtMgr_GetTimeoutCount(void)
{
    /* [SR-016][クラスB] Query total timeout count */
    return wdt_mgr_state.timeout_count;
}

/**
 * @brief Get Total Successful Kick Count
 * @return Number of successful kicks since initialization
 *
 * @safety [SR-016][クラスB] Kick statistics
 */
uint16_t WdtMgr_GetKickCount(void)
{
    /* [SR-016][クラスB] Query total successful kicks */
    return wdt_mgr_state.successful_kick_count;
}

/**
 * @brief Get Time Since Last Kick
 * @return Milliseconds elapsed since last successful kick
 *
 * @safety [SR-016][クラスB] Time since last kick (diagnostic)
 */
uint32_t WdtMgr_GetTimeSinceLastKick(void)
{
    /* [SR-016][クラスB] Query time since last kick */
    uint32_t now_ms = HAL_Timer_GetSystemTicks_ms();
    return (now_ms - wdt_mgr_state.last_kick_timestamp_ms);
}

/**
 * @brief Get Time Until Timeout
 * @return Estimated milliseconds until WDT timeout (if no kick occurs)
 *
 * @safety [SR-016][クラスB] Time to timeout (diagnostic)
 */
uint32_t WdtMgr_GetTimeUntilTimeout(void)
{
    /* [SR-016][クラスB] Estimate time remaining before timeout */
    uint32_t time_since_kick = WdtMgr_GetTimeSinceLastKick();

    if (time_since_kick >= WDT_MGR_TIMEOUT_MS) {
        return 0UL;  /* Already timed out */
    }

    return (WDT_MGR_TIMEOUT_MS - time_since_kick);
}

/**
 * @brief Check Superloop Latency Warning
 * @details Alert if superloop is approaching timeout without a kick
 * @return true if latency is dangerously high (near timeout), false otherwise
 *
 * @safety [SR-016][クラスB] Superloop latency watchdog
 */
bool WdtMgr_IsLatencyHigh(void)
{
    /* [SR-016][クラスB] Check if superloop latency is dangerously high */
    uint32_t time_since_kick = WdtMgr_GetTimeSinceLastKick();
    uint32_t margin_threshold = WDT_MGR_TIMEOUT_MS - WDT_MGR_MARGIN_MS;

    return (time_since_kick > margin_threshold);
}

/* ========================================================================
 * WATCHDOG PERIODICITY CHECK (Optional Diagnostic)
 * ======================================================================== */

/**
 * @brief Check if Watchdog Kick is Due
 * @details Used by control loop to determine if kick should be issued
 * @return true if time since last kick > kick period, false otherwise
 *
 * @safety [SR-016][クラスB] Kick scheduling helper
 */
bool WdtMgr_IsKickDue(void)
{
    /* [SR-016][クラスB] Check if WDT kick is due */
    uint32_t time_since_kick = WdtMgr_GetTimeSinceLastKick();
    return (time_since_kick >= WDT_MGR_KICK_PERIOD_MS);
}

/**
 * @brief Clear Watchdog Fault (After Safety Manager Clear)
 * @details Only called when safety manager recovers from WDT fault
 *
 * @safety [SR-016][クラスB] Fault recovery
 */
void WdtMgr_ClearFault(void)
{
    /* [SR-016][クラスB] Clear WDT fault flag (guarded by safety manager) */
    wdt_mgr_state.timeout_occurred = false;
    wdt_mgr_state.wdt_state = WDT_STATE_ENABLED;
}

/* ========================================================================
 * WATCHDOG STATISTICS & DIAGNOSTICS
 * ========================================================================== */

/**
 * @struct WdtStatistics_t
 * @brief Watchdog statistics snapshot
 * @safety [SR-016][クラスB] Diagnostic statistics
 */
typedef struct {
    WdtState_t state;
    uint16_t timeout_count;
    uint16_t kick_count;
    uint32_t time_since_last_kick_ms;
    uint32_t time_until_timeout_ms;
    bool is_latency_high;
} WdtStatistics_t;

/**
 * @brief Get Watchdog Statistics
 * @param stats [out] Pointer to statistics structure
 *
 * @safety [SR-016][クラスB] Diagnostic statistics snapshot
 */
void WdtMgr_GetStatistics(WdtStatistics_t *stats)
{
    /* [SR-016][クラスB] Snapshot WDT statistics for diagnostics */
    if (stats == NULL) {
        return;
    }

    stats->state = wdt_mgr_state.wdt_state;
    stats->timeout_count = wdt_mgr_state.timeout_count;
    stats->kick_count = wdt_mgr_state.successful_kick_count;
    stats->time_since_last_kick_ms = WdtMgr_GetTimeSinceLastKick();
    stats->time_until_timeout_ms = WdtMgr_GetTimeUntilTimeout();
    stats->is_latency_high = WdtMgr_IsLatencyHigh();
}

/* End of wdt_mgr.c */
