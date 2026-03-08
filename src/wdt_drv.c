/**
 * @file   wdt_drv.c
 * @brief  Watchdog Timer Driver - implementation
 * @doc    WMC-SUD-001 §4.8 WDT Driver Specification
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "wdt_drv.h"
#include "hal.h"
#include "mem_test.h"
#include "safety_mon.h"
#include "sensor_proc.h"

/* ========================================================================
 * Public Functions
 * ======================================================================== */

void WDT_Init(void)
{
    /*
     * Start the hardware watchdog timer.
     * After this call, WDT_Kick() must be called within WDT_TIMEOUT_MS
     * to prevent a hardware reset.
     */
    HAL_WDT_Start();
}

uint8_t WDT_IsAllDiagOk(void)
{
    const diag_status_t *diag;
    uint8_t all_ok = FLAG_SET;

    /* Check MEM diagnostic status */
    diag = MEM_GetStatus();

    if (diag->cpu_ok != FLAG_SET)
    {
        all_ok = FLAG_CLEAR;
    }

    if (diag->ram_ok != FLAG_SET)
    {
        all_ok = FLAG_CLEAR;
    }

    if (diag->rom_ok != FLAG_SET)
    {
        all_ok = FLAG_CLEAR;
    }

    if (diag->clk_ok != FLAG_SET)
    {
        all_ok = FLAG_CLEAR;
    }

    /* Check SafetyMon CRC integrity */
    if (SafetyMon_VerifyCrc() != FLAG_CLEAR)
    {
        /* CRC mismatch - safety data corrupted */
        all_ok = FLAG_CLEAR;
    }

    /* Check ADC diagnostic */
    if (SensorProc_GetAdcDiagFault() != FLAG_CLEAR)
    {
        all_ok = FLAG_CLEAR;
    }

    return all_ok;
}

void WDT_Kick(void)
{
    /*
     * Only kick the watchdog if ALL diagnostics pass.
     * If any diagnostic has failed, deliberately withhold the kick
     * so the WDT will time out and force a hardware reset.
     * This is the ultimate fail-safe mechanism.
     */
    if (WDT_IsAllDiagOk() == FLAG_SET)
    {
        HAL_WDT_Kick();
    }
    else
    {
        /*
         * Diagnostic failure detected - WDT will NOT be kicked.
         * Hardware reset will occur within WDT_TIMEOUT_MS (100ms).
         * This is intentional fail-safe behavior per IEC 60730.
         */
    }
}
