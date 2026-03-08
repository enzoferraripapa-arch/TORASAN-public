/**
 * @file    app_mot.h
 * @brief   Motor control application
 * @module  SA-001 APP_MOT
 * @safety  IEC 60730 Class B
 * @req     SR-001, SR-003
 */

#ifndef APP_MOT_H
#define APP_MOT_H

#include <stdint.h>
#include "../types/std_types.h"
#include "../types/safety_types.h"

/**
 * @brief   Initialize motor control module
 * @module  SA-001 APP_MOT (UT-001)
 */
void AppMot_Init(void);

/**
 * @brief   Motor control cyclic task (10ms period)
 * @module  SA-001 APP_MOT (UT-002)
 * @req     SR-001, SR-003
 * @safety  IEC 60730 Class B
 *
 * Performs: RPM measurement, PWM duty calculation,
 * 6-step commutation table lookup.
 */
void AppMot_Cyclic10ms(void);

/**
 * @brief   Emergency stop: all PWM off, relay off
 * @module  SA-001 APP_MOT (UT-003)
 * @req     SR-003
 * @safety  IEC 60730 Class B
 */
void AppMot_EmergencyStop(void);

/**
 * @brief   Get current motor RPM (safety-protected)
 * @module  SA-001 APP_MOT
 * @req     SR-001
 * @return  Current RPM value
 */
uint16_t AppMot_GetRpm(void);

/**
 * @brief   Set target RPM for speed control
 * @module  SA-001 APP_MOT
 * @param   target_rpm  Target speed [rpm]
 */
void AppMot_SetTargetRpm(uint16_t target_rpm);

#endif /* APP_MOT_H */
