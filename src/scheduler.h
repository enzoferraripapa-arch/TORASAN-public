/**
 * @file   scheduler.h
 * @brief  Scheduler Module - super-loop task execution with 10ms tick
 * @doc    WMC-SUD-001 §4.10 Scheduler Specification
 *
 * Implements a cooperative super-loop scheduler with fixed 10ms tick.
 * Tasks are executed in a deterministic order each cycle:
 *   1. SensorProc_Update()  - Read and filter all sensors
 *   2. SafetyMon_Check()    - Evaluate safety conditions
 *   3. MotorCtrl_Update()   - Compute and apply motor control
 *   4. MEM_RunDiag()        - Runtime memory diagnostic (1 block)
 *   5. WDT_Kick()           - Conditional watchdog refresh
 *
 * Safety monitor results are evaluated after SafetyMon_Check()
 * to trigger FailSafe actions before motor control runs.
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "std_types.h"
#include "product_config.h"

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief  Initialize scheduler module
 * @detail Initializes all subsystem modules in dependency order.
 *         Must be called once after HAL_Init() and before Scheduler_Run().
 */
void Scheduler_Init(void);

/**
 * @brief  Enter main scheduler loop (never returns)
 * @detail Executes the super-loop with CONTROL_LOOP_MS (10ms) tick.
 *         On target RL78/G14, uses timer interrupt flag for tick timing.
 *         In host environment, the loop can be exited via a stop flag.
 *
 * @note   This function does not return (infinite loop on target).
 */
void Scheduler_Run(void);

#endif /* SCHEDULER_H */
