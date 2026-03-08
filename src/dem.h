/**
 * @file   dem.h
 * @brief  Diagnostic Event Manager - fault reporting and system mode management
 * @doc    WMC-SUD-001 §4.4 DEM Specification
 *
 * Manages system operating mode state machine:
 *   INIT -> STARTUP -> NORMAL -> SAFE / FAULT
 *
 * Receives fault reports from safety monitor and diagnostic modules,
 * determines appropriate system mode transition, and sends error
 * notifications via UART.
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#ifndef DEM_H
#define DEM_H

#include "std_types.h"
#include "product_config.h"

/* ========================================================================
 * UART Error Notification Frame Format
 * [STX][LEN][FAULT_CODE][MODE][ETX]
 * ======================================================================== */

#define DEM_UART_STX            ((uint8_t)0x02U)
#define DEM_UART_ETX            ((uint8_t)0x03U)
#define DEM_UART_FRAME_LEN      ((uint8_t)5U)

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief  Initialize DEM module
 * @detail Sets system mode to SYS_MODE_INIT and clears fault history.
 */
void DEM_Init(void);

/**
 * @brief  Report a fault condition to DEM
 * @param  fault_code  Fault identifier (FAULT_xxx from product_config.h)
 * @detail Triggers appropriate mode transition and sends UART notification.
 *         Faults FAULT_OVERCURRENT, FAULT_OVERSPEED, FAULT_LID_OPEN
 *         transition to SYS_MODE_SAFE.
 *         Faults FAULT_CPU_REG, FAULT_RAM, FAULT_ROM, FAULT_CLOCK
 *         transition to SYS_MODE_FAULT (non-recoverable).
 */
void DEM_ReportFault(uint8_t fault_code);

/**
 * @brief  Get current system operating mode
 * @return Current sys_mode_t value
 */
sys_mode_t DEM_GetMode(void);

/**
 * @brief  Set system mode to STARTUP (called after startup diagnostics pass)
 */
void DEM_SetStartup(void);

/**
 * @brief  Set system mode to NORMAL (called when startup sequence completes)
 */
void DEM_SetNormal(void);

/**
 * @brief  Check if motor operation is allowed in current mode
 * @return FLAG_SET if motor allowed, FLAG_CLEAR if not
 * @detail Motor is only allowed in SYS_MODE_NORMAL.
 */
uint8_t DEM_IsMotorAllowed(void);

#endif /* DEM_H */
