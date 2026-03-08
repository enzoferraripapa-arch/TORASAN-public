/**
 * @file   diag_mgr.c
 * @brief  Diagnostic Manager - implementation
 * @doc    WMC-SUD-001 §4.9 Diagnostic Manager Specification
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "diag_mgr.h"
#include "mem_test.h"
#include "dem.h"

/* ========================================================================
 * [SAFETY VARIABLE REGION]
 * Startup test result is safety-critical.
 * ======================================================================== */

/** Startup diagnostic result */
static uint8_t s_all_ok;

/* ========================================================================
 * Public Functions
 * ======================================================================== */

Std_ReturnType DiagMgr_StartupTest(void)
{
    Std_ReturnType result;
    const diag_status_t *diag;

    s_all_ok = FLAG_CLEAR;

    /*
     * Execute full startup diagnostic sequence:
     * CPU register test -> RAM March C -> ROM CRC-32 -> Clock check
     *
     * MEM_StartupTest() runs all tests in order and stops on first failure.
     */
    result = MEM_StartupTest();

    if (result != E_OK)
    {
        /*
         * Determine which test failed by examining diagnostic status.
         * Report the specific fault to DEM for appropriate mode transition.
         */
        diag = MEM_GetStatus();

        if (diag->cpu_ok != FLAG_SET)
        {
            DEM_ReportFault(FAULT_CPU_REG);
        }
        else if (diag->ram_ok != FLAG_SET)
        {
            DEM_ReportFault(FAULT_RAM);
        }
        else if (diag->rom_ok != FLAG_SET)
        {
            DEM_ReportFault(FAULT_ROM);
        }
        else if (diag->clk_ok != FLAG_SET)
        {
            DEM_ReportFault(FAULT_CLOCK);
        }
        else
        {
            /* Unknown failure - report CPU fault as default */
            DEM_ReportFault(FAULT_CPU_REG);
        }

        return E_NOT_OK;
    }

    /*
     * All startup tests passed.
     * Transition DEM from INIT to STARTUP mode.
     */
    s_all_ok = FLAG_SET;
    DEM_SetStartup();

    return E_OK;
}

uint8_t DiagMgr_AllOK(void)
{
    return s_all_ok;
}
