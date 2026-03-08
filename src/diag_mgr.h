/**
 * @file   diag_mgr.h
 * @brief  Diagnostic Manager - startup test orchestration and status reporting
 * @doc    WMC-SUD-001 §4.9 Diagnostic Manager Specification
 *
 * Orchestrates the startup diagnostic sequence in the correct order:
 *   1. CPU register test
 *   2. RAM March C test (all blocks)
 *   3. ROM CRC-32 test (all blocks)
 *   4. Clock cross-check
 *
 * If any test fails, the system enters FAULT mode and motor is disabled.
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#ifndef DIAG_MGR_H
#define DIAG_MGR_H

#include "std_types.h"
#include "product_config.h"

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief  Execute startup diagnostic sequence
 * @return E_OK if all tests pass, E_NOT_OK if any test fails
 * @detail Calls MEM_StartupTest() which runs CPU->RAM->ROM->Clock tests.
 *         On failure, reports appropriate fault to DEM.
 *         On success, transitions DEM to STARTUP mode.
 *         This function blocks until all tests complete.
 */
Std_ReturnType DiagMgr_StartupTest(void);

/**
 * @brief  Check if all startup diagnostics passed
 * @return FLAG_SET if all OK, FLAG_CLEAR if any failed
 */
uint8_t DiagMgr_AllOK(void);

#endif /* DIAG_MGR_H */
