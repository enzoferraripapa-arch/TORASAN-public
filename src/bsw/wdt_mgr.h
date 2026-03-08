/**
 * @file    wdt_mgr.h
 * @brief   Watchdog timer management
 * @module  SA-004 WDT_MGR
 * @safety  IEC 60730 Class B
 * @req     SR-016
 */

#ifndef WDT_MGR_H
#define WDT_MGR_H

#include <stdint.h>
#include "../types/std_types.h"

/**
 * @brief   Initialize WDT manager (set timeout, start WDT)
 * @module  SA-004 WDT_MGR
 * @req     SR-016
 * @safety  IEC 60730 Class B
 */
void WdtMgr_Init(void);

/**
 * @brief   Attempt WDT kick with condition checking
 * @module  SA-004 WDT_MGR
 * @req     SR-016
 * @safety  IEC 60730 Class B
 *
 * Only kicks WDT if all safety monitors have executed
 * within the current cycle. This ensures that a stuck
 * module will cause WDT timeout and system reset.
 *
 * @param   safety_flag  Bitmask of completed safety checks
 * @return  STD_TRUE if kick performed, STD_FALSE if denied
 */
uint8_t WdtMgr_TryKick(uint8_t safety_flag);

/** Safety check completion bits for WdtMgr_TryKick */
#define WDT_FLAG_MOTOR_CHECK    ((uint8_t)0x01U)
#define WDT_FLAG_SAFETY_CHECK   ((uint8_t)0x02U)
#define WDT_FLAG_DIAG_CHECK     ((uint8_t)0x04U)
#define WDT_FLAG_ALL_CHECKS     ((uint8_t)0x07U)

#endif /* WDT_MGR_H */
