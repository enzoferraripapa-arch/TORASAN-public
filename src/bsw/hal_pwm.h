/**
 * @file    hal_pwm.h
 * @brief   PWM hardware abstraction layer for BLDC 6-step commutation
 * @module  SA-003 HAL
 * @safety  IEC 60730 Class B
 * @req     SR-003, SR-006
 */

#ifndef HAL_PWM_H
#define HAL_PWM_H

#include <stdint.h>
#include "../types/std_types.h"

/**
 * @brief   Initialize PWM peripheral (3-phase, 20kHz carrier)
 * @module  SA-003 HAL
 */
void HalPwm_Init(void);

/**
 * @brief   Set PWM duty cycle for specified phase
 * @module  SA-003 HAL
 * @param   phase  Phase index (0=U, 1=V, 2=W)
 * @param   duty   Duty cycle (0-1000 = 0.0%-100.0%)
 */
void HalPwm_SetDuty(uint8_t phase, uint16_t duty);

/**
 * @brief   Emergency stop: all PWM outputs to 0
 * @module  SA-003 HAL
 * @req     SR-003, SR-006
 * @safety  IEC 60730 Class B
 */
void HalPwm_AllStop(void);

/**
 * @brief   Enable PWM output
 * @module  SA-003 HAL
 */
void HalPwm_EnableOutput(void);

/**
 * @brief   Disable PWM output (safe state)
 * @module  SA-003 HAL
 */
void HalPwm_DisableOutput(void);

#endif /* HAL_PWM_H */
