/**
 * @file    com.h
 * @brief   UART communication module (UI panel data exchange)
 * @module  SA-008 COM
 */

#ifndef COM_H
#define COM_H

#include <stdint.h>
#include "../types/std_types.h"
#include "../types/safety_types.h"

/**
 * @brief   Initialize communication module
 * @module  SA-008 COM
 */
void Com_Init(void);

/**
 * @brief   Cyclic communication handler (10ms period)
 * @module  SA-008 COM
 *
 * Processes received commands and sends status updates.
 */
void Com_Cyclic10ms(void);

/**
 * @brief   Send system status to UI panel
 * @module  SA-008 COM
 * @param   state       Current system state
 * @param   rpm         Current motor RPM
 * @param   fault_code  Current fault code (FAULT_NONE if normal)
 */
void Com_SendStatus(SystemState_t state, uint16_t rpm, FaultCode_t fault_code);

#endif /* COM_H */
