/**
 * @file    app_saf.h
 * @brief   Safety monitoring application
 * @module  SA-002 APP_SAF
 * @safety  IEC 60730 Class B
 * @req     SR-002, SR-005, SR-006, SR-007, SR-008, SR-013, SR-014, SR-015
 */

#ifndef APP_SAF_H
#define APP_SAF_H

#include <stdint.h>
#include "../types/std_types.h"
#include "../types/safety_types.h"

/**
 * @brief   Initialize safety monitoring module
 * @module  SA-002 APP_SAF (UT-004)
 */
void AppSaf_Init(void);

/**
 * @brief   Safety monitoring cyclic task (10ms period)
 * @module  SA-002 APP_SAF (UT-005)
 * @req     SR-002, SR-005, SR-007, SR-008, SR-014, SR-015
 * @safety  IEC 60730 Class B
 *
 * Checks: overspeed, overcurrent, lid open, voltage window.
 * Triggers safe state transition on any anomaly detection.
 */
void AppSaf_Cyclic10ms(void);

/**
 * @brief   Transition system to safe state
 * @module  SA-002 APP_SAF (UT-006)
 * @req     SR-013
 * @safety  IEC 60730 Class B
 * @param   fault_code  Fault code triggering the transition
 */
void AppSaf_TransitionSafe(FaultCode_t fault_code);

/**
 * @brief   Get current system state
 * @module  SA-002 APP_SAF (UT-007)
 * @req     SR-013
 * @return  Current system state
 */
SystemState_t AppSaf_GetState(void);

#endif /* APP_SAF_H */
