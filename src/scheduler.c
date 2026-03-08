/**
 * @file   scheduler.c
 * @brief  Scheduler Module - implementation
 * @doc    WMC-SUD-001 §4.10 Scheduler Specification
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "scheduler.h"
#include "sensor_proc.h"
#include "safety_mon.h"
#include "motor_ctrl.h"
#include "mem_test.h"
#include "wdt_drv.h"
#include "dem.h"
#include "failsafe.h"

/* ========================================================================
 * [CONTROL VARIABLE REGION]
 * Scheduler state variables.
 * ======================================================================== */

/** Tick flag - set by timer interrupt, cleared by scheduler */
static volatile uint8_t s_tick_flag;

/** Scheduler running flag (for host-environment test control) */
static volatile uint8_t s_running;

/* ========================================================================
 * Internal Helper: Wait for 10ms tick
 *
 * On target RL78/G14:
 *   Uses interval timer (IT) interrupt at 10ms period.
 *   The ISR sets s_tick_flag; main loop polls and clears it.
 *
 * In host environment:
 *   Tick is simulated (always ready) for test execution.
 * ======================================================================== */

static void WaitForTick(void)
{
#ifdef TARGET_RL78_G14
    /*
     * Target: Wait for interval timer interrupt flag
     * The IT ISR (in hal.c) sets s_tick_flag = 1
     * We wait here with HALT instruction for power saving
     *
     *   while (s_tick_flag == 0U)
     *   {
     *       __halt();   / * Enter HALT until next interrupt * /
     *   }
     *   s_tick_flag = 0U;
     */
    while (s_tick_flag == 0U)
    {
        /* Wait for timer tick */
    }
    s_tick_flag = 0U;
#else
    /*
     * Host environment: immediate return for test execution.
     * Each call to Scheduler_Run processes exactly one cycle.
     */
    s_tick_flag = 0U;
#endif
}

/* ========================================================================
 * Internal Helper: Evaluate safety flags and trigger fail-safe if needed
 *
 * Called after SafetyMon_Check() and before MotorCtrl_Update().
 * Processes safety flags in priority order:
 *   1. Diagnostic failure (highest - non-recoverable)
 *   2. Overspeed
 *   3. Overcurrent
 *   4. Voltage error
 *   5. Vibration
 *   6. Lid open (lowest - recoverable, PWM stop only)
 * ======================================================================== */

static void EvaluateSafetyFlags(void)
{
    const safety_flags_t *flags;
    uint8_t crc_err;

    /* Verify CRC integrity of safety flags first */
    crc_err = SafetyMon_VerifyCrc();
    if (crc_err != FLAG_CLEAR)
    {
        /* Safety data corrupted - execute full fail-safe */
        FailSafe_Execute(FAULT_RAM);
        return;
    }

    /* Get read-only pointer to safety flags (const access pattern) */
    flags = SafetyMon_GetFlags();

    /* Check diagnostic failure (non-recoverable) */
    if (flags->diag_fail != FLAG_CLEAR)
    {
        FailSafe_Execute(FAULT_VOLTAGE);
        return;
    }

    /* Check overspeed */
    if (flags->overspeed != FLAG_CLEAR)
    {
        FailSafe_Execute(FAULT_OVERSPEED);
        return;
    }

    /* Check overcurrent */
    if (flags->overcurrent != FLAG_CLEAR)
    {
        FailSafe_Execute(FAULT_OVERCURRENT);
        return;
    }

    /* Check voltage window */
    if (flags->voltage_error != FLAG_CLEAR)
    {
        FailSafe_Execute(FAULT_VOLTAGE);
        return;
    }

    /* Check vibration */
    if (flags->vibration != FLAG_CLEAR)
    {
        /*
         * Vibration detected: initiate controlled deceleration
         * to vibration-safe RPM before stopping.
         */
        MotorCtrl_Decelerate(MOTOR_VIBRATION_RPM, MOTOR_VIBRATION_MS);
        DEM_ReportFault(FAULT_VIBRATION);
        return;
    }

    /* Check lid open (lowest priority - recoverable) */
    if (flags->lid_open != FLAG_CLEAR)
    {
        FailSafe_StopMotor();
        DEM_ReportFault(FAULT_LID_OPEN);
        return;
    }
}

/* ========================================================================
 * Public Functions
 * ======================================================================== */

void Scheduler_Init(void)
{
    s_tick_flag = 0U;
    s_running   = FLAG_SET;

    /*
     * Initialize all subsystem modules in dependency order.
     * HAL must be initialized before this function is called.
     */
    SensorProc_Init();
    SafetyMon_Init();
    MotorCtrl_Init();
    /* DEM already initialized by startup sequence - do not re-init */

    /*
     * Note: MEM and WDT initialization is handled by DiagMgr_StartupTest()
     * and WDT_Init() in main.c before Scheduler_Run() is called.
     */
}

void Scheduler_Run(void)
{
    Std_ReturnType mem_result;

    /*
     * Transition to NORMAL mode - startup sequence is complete.
     * This enables motor operation via DEM_IsMotorAllowed().
     */
    DEM_SetNormal();

    /*
     * Main super-loop: executes at CONTROL_LOOP_MS (10ms) intervals.
     * Task execution order is fixed and deterministic.
     *
     * On target RL78/G14: infinite loop (never returns)
     * In host environment: can be exited by clearing s_running
     */
    while (s_running == FLAG_SET)
    {
        /* Wait for 10ms timer tick */
        WaitForTick();

        /* ---- Task 1: Sensor Processing ---- */
        SensorProc_Update();

        /* ---- Task 2: Safety Monitoring ---- */
        SafetyMon_Check();

        /* ---- Evaluate safety flags (may trigger fail-safe) ---- */
        EvaluateSafetyFlags();

        /* ---- Task 3: Motor Control ---- */
        MotorCtrl_Update();

        /* ---- Task 4: Runtime Memory Diagnostic (1 block) ---- */
        mem_result = MEM_RunDiag();
        if (mem_result != E_OK)
        {
            /* Runtime diagnostic failure detected */
            FailSafe_Execute(FAULT_RAM);
        }

        /* ---- Task 5: Watchdog Kick (conditional) ---- */
        WDT_Kick();

#ifndef TARGET_RL78_G14
        /*
         * Host environment: exit after one cycle for testing.
         * Remove this for continuous simulation.
         */
        s_running = FLAG_CLEAR;
#endif
    }
}
