/**
 * @file   motor_ctrl.c
 * @brief  Motor Control Module - implementation
 * @doc    WMC-SUD-001 §4.6 Motor Control Specification
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "motor_ctrl.h"
#include "hal.h"
#include "sensor_proc.h"
#include "dem.h"

/* ========================================================================
 * [CONTROL VARIABLE REGION]
 * Motor control state variables.
 * In linker script: place in .control_data section.
 * ======================================================================== */

/** Target motor speed (RPM) */
static uint16_t s_target_rpm;

/** Current PWM duty (0 to HAL_PWM_DUTY_MAX) */
static uint16_t s_current_duty;

/** Deceleration active flag */
static uint8_t s_decel_active;

/** Deceleration target RPM */
static uint16_t s_decel_target_rpm;

/** Deceleration step per control cycle (RPM decrement per 10ms) */
static uint16_t s_decel_step;

/** Deceleration intermediate target RPM (ramps down each cycle) */
static uint16_t s_decel_ramp_rpm;

/* ========================================================================
 * Internal Helper: Compute proportional duty from RPM error
 * duty = Kp * (target - actual)
 * Clamped to [0, HAL_PWM_DUTY_MAX]
 * ======================================================================== */

static uint16_t ComputeDuty(uint16_t target, uint16_t actual)
{
    uint16_t duty;
    uint16_t error;

    if (target == 0U)
    {
        /* Target is zero - stop motor */
        return 0U;
    }

    if (actual >= target)
    {
        /*
         * At or above target speed.
         * Reduce duty slightly to allow coast-down.
         * Maintain a base duty proportional to target.
         */
        duty = (uint16_t)(((uint32_t)target * (uint32_t)HAL_PWM_DUTY_MAX) /
                          (uint32_t)MOTOR_RATED_RPM);
    }
    else
    {
        /* Below target - compute proportional correction */
        error = target - actual;

        /* P-controller: duty_increment = error * Kp_num / Kp_den */
        duty = (uint16_t)(((uint32_t)error * (uint32_t)MOTOR_KP_NUM) /
                          (uint32_t)MOTOR_KP_DEN);

        /* Add base duty proportional to target */
        duty += (uint16_t)(((uint32_t)target * (uint32_t)HAL_PWM_DUTY_MAX) /
                           (uint32_t)MOTOR_RATED_RPM);
    }

    /* Apply minimum duty threshold for startup torque */
    if ((duty > 0U) && (duty < MOTOR_MIN_DUTY))
    {
        duty = MOTOR_MIN_DUTY;
    }

    /* Clamp to maximum */
    if (duty > HAL_PWM_DUTY_MAX)
    {
        duty = HAL_PWM_DUTY_MAX;
    }

    return duty;
}

/* ========================================================================
 * Public Functions
 * ======================================================================== */

void MotorCtrl_Init(void)
{
    s_target_rpm      = 0U;
    s_current_duty    = 0U;
    s_decel_active    = FLAG_CLEAR;
    s_decel_target_rpm = 0U;
    s_decel_step      = 0U;
    s_decel_ramp_rpm  = 0U;
}

void MotorCtrl_Update(void)
{
    uint16_t actual_rpm;
    uint16_t effective_target;
    uint16_t duty;

    /* Check if motor operation is permitted */
    if (DEM_IsMotorAllowed() == FLAG_CLEAR)
    {
        /* Motor not allowed - ensure PWM is stopped */
        if (s_current_duty != 0U)
        {
            HAL_PWM_Stop();
            s_current_duty = 0U;
        }
        return;
    }

    /* Get current speed */
    actual_rpm = SensorProc_GetRpm();

    /* Handle deceleration ramp if active */
    if (s_decel_active == FLAG_SET)
    {
        if (s_decel_ramp_rpm > s_decel_target_rpm)
        {
            /* Ramp down intermediate target */
            if (s_decel_ramp_rpm > s_decel_step)
            {
                s_decel_ramp_rpm -= s_decel_step;
            }
            else
            {
                s_decel_ramp_rpm = s_decel_target_rpm;
            }

            if (s_decel_ramp_rpm < s_decel_target_rpm)
            {
                s_decel_ramp_rpm = s_decel_target_rpm;
            }
        }
        else
        {
            /* Deceleration complete */
            s_decel_active = FLAG_CLEAR;
            s_target_rpm = s_decel_target_rpm;
        }

        effective_target = s_decel_ramp_rpm;
    }
    else
    {
        effective_target = s_target_rpm;
    }

    /* Compute duty from proportional controller */
    duty = ComputeDuty(effective_target, actual_rpm);

    /* Apply PWM duty to all phases (simplified: same duty for all) */
    if (duty > 0U)
    {
        HAL_PWM_SetDuty(HAL_PWM_PHASE_U, duty);
        HAL_PWM_SetDuty(HAL_PWM_PHASE_V, duty);
        HAL_PWM_SetDuty(HAL_PWM_PHASE_W, duty);

        if (s_current_duty == 0U)
        {
            /* Motor was stopped - start PWM */
            HAL_PWM_Start();

            /* Enable gate driver */
            HAL_GPIO_Write(HAL_PORT_GATE_EN, HAL_PIN_GATE_EN, HAL_GPIO_HIGH);
        }
    }
    else
    {
        if (s_current_duty != 0U)
        {
            /* Motor running - stop PWM */
            HAL_PWM_Stop();
        }
    }

    s_current_duty = duty;
}

void MotorCtrl_SetTargetRpm(uint16_t rpm)
{
    /* Clamp to rated maximum */
    if (rpm > MOTOR_RATED_RPM)
    {
        s_target_rpm = MOTOR_RATED_RPM;
    }
    else
    {
        s_target_rpm = rpm;
    }

    /* Cancel any active deceleration */
    s_decel_active = FLAG_CLEAR;
}

void MotorCtrl_Decelerate(uint16_t target_rpm, uint16_t time_ms)
{
    uint16_t current_rpm;
    uint16_t rpm_range;
    uint16_t num_steps;

    current_rpm = SensorProc_GetRpm();

    /* Only decelerate if currently above target */
    if (current_rpm <= target_rpm)
    {
        s_target_rpm = target_rpm;
        return;
    }

    rpm_range = current_rpm - target_rpm;

    /* Calculate number of control cycles for deceleration */
    num_steps = time_ms / CONTROL_LOOP_MS;
    if (num_steps == 0U)
    {
        num_steps = 1U;
    }

    /* RPM decrement per control cycle */
    s_decel_step = (rpm_range + (num_steps - 1U)) / num_steps;
    if (s_decel_step == 0U)
    {
        s_decel_step = 1U;
    }

    s_decel_target_rpm = target_rpm;
    s_decel_ramp_rpm   = current_rpm;
    s_decel_active     = FLAG_SET;
}
