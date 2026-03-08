/**
 * @file   failsafe.c
 * @brief  Fail-Safe Module - implementation
 * @doc    WMC-SUD-001 §4.5 Fail-Safe Specification
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "failsafe.h"
#include "hal.h"
#include "dem.h"

/* ========================================================================
 * FailSafe_Execute
 *
 * Full fail-safe sequence:
 *   1. Immediately stop PWM (all phases driven low)
 *   2. Disable gate driver hardware (cut power to motor driver IC)
 *   3. Report fault to DEM for mode transition and UART notification
 *
 * Timing requirement: complete within SAFE_TRANSITION_MS (100ms)
 * ======================================================================== */

void FailSafe_Execute(uint8_t fault_code)
{
    /*
     * Step 1: Stop PWM output immediately
     * This drives all FET gate signals low, stopping current flow
     * through the motor windings.
     */
    HAL_PWM_Stop();

    /*
     * Step 2: Disable gate driver
     * Pull P5.0 (gate driver enable) LOW to hardware-disable
     * the gate driver IC. This provides a second layer of protection
     * independent of PWM control.
     */
    HAL_GPIO_Write(HAL_PORT_GATE_EN, HAL_PIN_GATE_EN, HAL_GPIO_LOW);

    /*
     * Step 3: Report fault to DEM
     * DEM will transition system mode to SAFE or FAULT
     * and send UART error notification.
     */
    DEM_ReportFault(fault_code);
}

/* ========================================================================
 * FailSafe_StopMotor
 *
 * Soft stop: PWM off, gate driver remains enabled.
 * Used for recoverable conditions (e.g., lid open) where the motor
 * should stop but the system can resume normal operation after the
 * condition clears.
 * ======================================================================== */

void FailSafe_StopMotor(void)
{
    /*
     * Stop PWM output only.
     * Gate driver remains enabled so motor can be restarted
     * without re-initialization sequence.
     */
    HAL_PWM_Stop();
}
