/**
 * @file   failsafe.h
 * @brief  Fail-Safe Module - emergency motor shutdown and safe state entry
 * @doc    WMC-SUD-001 §4.5 Fail-Safe Specification
 *
 * Provides two levels of motor shutdown:
 *   - FailSafe_StopMotor(): PWM stop only (for lid-open condition)
 *   - FailSafe_Execute():   PWM stop + gate driver disable + DEM notification
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#ifndef FAILSAFE_H
#define FAILSAFE_H

#include "std_types.h"
#include "product_config.h"

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief  Execute full fail-safe sequence
 * @param  fault_code  Fault identifier triggering fail-safe (FAULT_xxx)
 * @detail Execution sequence:
 *         1. Stop PWM output on all phases
 *         2. Disable gate driver (P5.0 = LOW)
 *         3. Report fault to DEM (triggers SAFE/FAULT mode)
 *
 * @note   This function is designed to be called from interrupt context
 *         or normal context. It must complete within SAFE_TRANSITION_MS.
 */
void FailSafe_Execute(uint8_t fault_code);

/**
 * @brief  Stop motor PWM only (soft stop)
 * @detail Stops PWM output but keeps gate driver enabled.
 *         Used for recoverable conditions like lid-open where
 *         motor can be restarted after condition clears.
 */
void FailSafe_StopMotor(void);

#endif /* FAILSAFE_H */
