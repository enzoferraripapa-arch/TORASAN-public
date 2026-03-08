/**
 * @file   wdt_drv.h
 * @brief  Watchdog Timer Driver - conditional kick based on diagnostic status
 * @doc    WMC-SUD-001 §4.8 WDT Driver Specification
 *
 * Wraps the hardware WDT with a safety-aware kick mechanism.
 * The WDT is only kicked if all runtime diagnostics pass.
 * If any diagnostic fails, the WDT intentionally times out
 * and triggers a hardware reset (fail-safe behavior).
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#ifndef WDT_DRV_H
#define WDT_DRV_H

#include "std_types.h"
#include "product_config.h"

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief  Initialize and start the hardware watchdog timer
 * @detail Configures WDT for WDT_TIMEOUT_MS period and starts it.
 *         Once started, WDT cannot be stopped (RL78 hardware constraint).
 *         Must be called after startup diagnostics pass.
 */
void WDT_Init(void);

/**
 * @brief  Conditionally kick the watchdog timer
 * @detail Only kicks the WDT if all diagnostics are OK.
 *         If any diagnostic has failed, the WDT is NOT kicked,
 *         which will cause a hardware reset within WDT_TIMEOUT_MS.
 *         Called from scheduler every control cycle.
 */
void WDT_Kick(void);

/**
 * @brief  Check if all diagnostics allow WDT kick
 * @return FLAG_SET if all diagnostics OK, FLAG_CLEAR if any failed
 * @detail Checks: MEM diagnostic status, SafetyMon CRC integrity,
 *         ADC diagnostic, and safety flags CRC.
 */
uint8_t WDT_IsAllDiagOk(void);

#endif /* WDT_DRV_H */
