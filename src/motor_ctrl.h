/**
 * @file   motor_ctrl.h
 * @brief  Motor Control Module - BLDC speed control with proportional PWM
 * @doc    WMC-SUD-001 §4.6 Motor Control Specification
 *
 * Provides closed-loop proportional speed control for 3-phase BLDC motor.
 *   - Target RPM setting with clamp to rated maximum
 *   - Proportional control: duty = Kp * (target - actual)
 *   - Controlled deceleration for vibration mitigation
 *   - Motor enable/disable based on DEM permission
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#ifndef MOTOR_CTRL_H
#define MOTOR_CTRL_H

#include "std_types.h"
#include "product_config.h"

/* ========================================================================
 * Proportional Controller Parameters
 * ======================================================================== */

/**
 * Proportional gain (fixed-point: Kp = numerator / denominator)
 * Kp = 1/2 => duty_increment = error_rpm / 2
 * This maps RPM error to PWM duty adjustment
 */
#define MOTOR_KP_NUM            ((uint16_t)1U)
#define MOTOR_KP_DEN            ((uint16_t)2U)

/** Minimum PWM duty for motor to start spinning */
#define MOTOR_MIN_DUTY          ((uint16_t)50U)

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief  Initialize motor control module
 * @detail Clears target RPM and internal state. Motor starts stopped.
 */
void MotorCtrl_Init(void);

/**
 * @brief  Update motor control loop
 * @detail Called every CONTROL_LOOP_MS from scheduler.
 *         Reads current RPM from SensorProc, computes proportional
 *         duty correction, and applies PWM duty.
 *         Checks DEM_IsMotorAllowed() before enabling output.
 */
void MotorCtrl_Update(void);

/**
 * @brief  Set target motor speed
 * @param  rpm  Target speed in RPM (clamped to MOTOR_RATED_RPM)
 */
void MotorCtrl_SetTargetRpm(uint16_t rpm);

/**
 * @brief  Execute controlled deceleration
 * @param  target_rpm  Target RPM to decelerate to
 * @param  time_ms     Maximum deceleration time in milliseconds
 * @detail Initiates a controlled ramp-down from current speed to target_rpm.
 *         The deceleration completes over approximately time_ms milliseconds.
 *         Used for vibration band avoidance during spin-up/down.
 */
void MotorCtrl_Decelerate(uint16_t target_rpm, uint16_t time_ms);

#endif /* MOTOR_CTRL_H */
