/**
 * @file    wdt_mgr.c
 * @brief   Watchdog timer management implementation
 * @module  SA-004 WDT_MGR (UT-013)
 * @safety  IEC 60730 Class B
 * @req     SR-016
 */

#include "wdt_mgr.h"
#include "../config/mcu_config.h"
#include "../config/safety_config.h"

/* ============================================================
 * Module-static variables
 * ============================================================ */

/** Last kick timestamp for period enforcement */
static uint32_t s_last_kick_ms = 0UL;

/* ============================================================
 * WdtMgr_Init
 * ============================================================ */
void WdtMgr_Init(void)
{
    /* Configure WDT: timeout = WDT_TIMEOUT_MS (100ms)
     * RL78/G14 WDT uses internal oscillator.
     * Write WDTE register to start WDT. */
    REG_WDTE = WDT_KICK_PATTERN;

    s_last_kick_ms = 0UL;
}

/* ============================================================
 * WdtMgr_TryKick
 * ============================================================ */
uint8_t WdtMgr_TryKick(uint8_t safety_flag)
{
    uint8_t result;

    /* Only kick if all safety checks have completed */
    if ((safety_flag & WDT_FLAG_ALL_CHECKS) == WDT_FLAG_ALL_CHECKS)
    {
        /* Kick the watchdog */
        REG_WDTE = WDT_KICK_PATTERN;
        result = STD_TRUE;
    }
    else
    {
        /* Deny kick: not all checks completed.
         * WDT will timeout and reset the MCU. */
        result = STD_FALSE;
    }

    return result;
}
