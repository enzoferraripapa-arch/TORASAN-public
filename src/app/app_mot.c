/**
 * @file    app_mot.c
 * @brief   Motor control application implementation
 * @module  SA-001 APP_MOT (UT-001 to UT-003)
 * @safety  IEC 60730 Class B
 * @req     SR-001, SR-003
 *
 * BLDC 6-step commutation with hall sensor feedback.
 * RPM measurement via hall period.
 * Simple P-control for speed regulation.
 */

#include "app_mot.h"
#include "../bsw/hal_pwm.h"
#include "../bsw/hal_gpio.h"
#include "../bsw/hal_timer.h"
#include "../config/safety_config.h"

/* ============================================================
 * 6-Step Commutation Table
 * ============================================================
 * Hall pattern (3-bit: W|V|U) maps to active phase pair.
 * Table indexed by hall pattern (1-6 valid, 0 and 7 invalid).
 *
 * Each entry: [phase_high][phase_low] where phase 0=U, 1=V, 2=W
 * 0xFF = invalid pattern
 */

/** Commutation table entry */
typedef struct {
    uint8_t phase_high;  /**< Phase to drive high (PWM) */
    uint8_t phase_low;   /**< Phase to drive low (ground) */
} CommStep_t;

static const CommStep_t s_comm_table[8U] = {
    { 0xFFU, 0xFFU },  /* 0b000: invalid */
    { 0U,    1U    },  /* 0b001: U->V */
    { 2U,    0U    },  /* 0b010: W->U */
    { 2U,    1U    },  /* 0b011: W->V */
    { 1U,    2U    },  /* 0b100: V->W */
    { 0U,    2U    },  /* 0b101: U->W */
    { 1U,    0U    },  /* 0b110: V->U */
    { 0xFFU, 0xFFU },  /* 0b111: invalid */
};

/* ============================================================
 * Module-static variables
 * ============================================================ */

/** Motor RPM (safety-protected) */
static SafetyVar_t s_motor_rpm;

/** Target RPM */
static uint16_t s_target_rpm = 0U;

/** Current PWM duty (0-1000) */
static uint16_t s_current_duty = 0U;

/** RPM moving average buffer (3 samples) */
static uint16_t s_rpm_buffer[3U] = { 0U, 0U, 0U };
static uint8_t  s_rpm_index = 0U;

/** Motor running flag */
static uint8_t s_motor_running = STD_FALSE;

/* ============================================================
 * Internal: Calculate RPM from hall period
 * ============================================================ */
static uint16_t AppMot_CalcRpm(uint32_t hall_period_us)
{
    uint16_t rpm;

    if (hall_period_us == 0UL)
    {
        rpm = 0U;
    }
    else
    {
        /* RPM = 60 * 1000000 / (hall_period_us * pole_pairs * 6)
         * For BLDC with 4 pole pairs:
         * RPM = 60000000 / (hall_period_us * 24)
         *     = 2500000 / hall_period_us */
        if (hall_period_us > (uint32_t)0xFFFFU)
        {
            rpm = 0U;  /* Period too long = very slow or stopped */
        }
        else
        {
            rpm = (uint16_t)(2500000UL / hall_period_us);
        }
    }

    return rpm;
}

/* ============================================================
 * Internal: Moving average filter for RPM
 * ============================================================ */
static uint16_t AppMot_FilterRpm(uint16_t raw_rpm)
{
    uint32_t sum;

    s_rpm_buffer[s_rpm_index] = raw_rpm;
    s_rpm_index++;
    if (s_rpm_index >= (uint8_t)3U)
    {
        s_rpm_index = 0U;
    }

    sum = (uint32_t)s_rpm_buffer[0U]
        + (uint32_t)s_rpm_buffer[1U]
        + (uint32_t)s_rpm_buffer[2U];

    return (uint16_t)(sum / 3UL);
}

/* ============================================================
 * UT-001: AppMot_Init
 * ============================================================ */
void AppMot_Init(void)
{
    HalPwm_Init();

    SAFETYVAR_SET(s_motor_rpm, 0U);
    s_target_rpm  = 0U;
    s_current_duty = 0U;
    s_rpm_index   = 0U;
    s_rpm_buffer[0U] = 0U;
    s_rpm_buffer[1U] = 0U;
    s_rpm_buffer[2U] = 0U;
    s_motor_running = STD_FALSE;
}

/* ============================================================
 * UT-002: AppMot_Cyclic10ms
 * ============================================================ */
void AppMot_Cyclic10ms(void)
{
    uint32_t hall_period;
    uint16_t raw_rpm;
    uint16_t filtered_rpm;
    uint8_t  hall_pattern;
    int32_t  rpm_error;
    int32_t  duty_adjust;

    /* 1. Measure RPM from hall sensor period */
    hall_period = HalTimer_GetHallPeriod();
    raw_rpm = AppMot_CalcRpm(hall_period);
    filtered_rpm = AppMot_FilterRpm(raw_rpm);

    /* Store RPM with safety protection */
    SAFETYVAR_SET(s_motor_rpm, filtered_rpm);

    /* Verify safety variable integrity */
    if (SAFETYVAR_CHECK(s_motor_rpm) != STD_TRUE)
    {
        /* Data corruption detected */
        AppMot_EmergencyStop();
        return;
    }

    /* 2. Speed control (simple P-control) */
    if (s_target_rpm > 0U)
    {
        rpm_error = (int32_t)s_target_rpm - (int32_t)filtered_rpm;

        /* P-gain: 1 duty unit per 2 RPM error */
        duty_adjust = rpm_error / (int32_t)2;

        /* Update duty with clamping */
        int32_t new_duty = (int32_t)s_current_duty + duty_adjust;

        if (new_duty < (int32_t)0)
        {
            s_current_duty = 0U;
        }
        else if (new_duty > (int32_t)PWM_MAX_DUTY)
        {
            s_current_duty = PWM_MAX_DUTY;
        }
        else
        {
            s_current_duty = (uint16_t)new_duty;
        }

        /* 3. 6-step commutation */
        hall_pattern = HalGpio_GetHallPattern();

        if ((hall_pattern >= 1U) && (hall_pattern <= 6U))
        {
            const CommStep_t *p_step = &s_comm_table[hall_pattern];
            uint8_t phase;

            /* 6-step: drive high phase with PWM, low phase grounded,
             * remaining phase floating (duty=0, output disabled) */
            for (phase = 0U; phase < (uint8_t)3U; phase++)
            {
                if (phase == p_step->phase_high)
                {
                    HalPwm_SetDuty(phase, s_current_duty);
                }
                else if (phase == p_step->phase_low)
                {
                    HalPwm_SetDuty(phase, PWM_MAX_DUTY);  /* Low-side ON */
                }
                else
                {
                    HalPwm_SetDuty(phase, 0U);  /* Floating */
                }
            }

            if (s_motor_running == STD_FALSE)
            {
                HalPwm_EnableOutput();
                HalGpio_SetRelay(STD_TRUE);
                s_motor_running = STD_TRUE;
            }
        }
        else
        {
            /* Invalid hall pattern: stop for safety */
            AppMot_EmergencyStop();
        }
    }
    else
    {
        /* Target = 0: stop motor */
        if (s_motor_running == STD_TRUE)
        {
            HalPwm_AllStop();
            HalGpio_SetRelay(STD_FALSE);
            s_motor_running = STD_FALSE;
        }
        s_current_duty = 0U;
    }
}

/* ============================================================
 * UT-003: AppMot_EmergencyStop
 * ============================================================ */
void AppMot_EmergencyStop(void)
{
    /* 1. Immediately stop all PWM output */
    HalPwm_AllStop();

    /* 2. De-energize inverter relay */
    HalGpio_SetRelay(STD_FALSE);

    /* 3. Clear motor state */
    s_current_duty = 0U;
    s_target_rpm   = 0U;
    s_motor_running = STD_FALSE;

    SAFETYVAR_SET(s_motor_rpm, 0U);
}

/* ============================================================
 * AppMot_GetRpm
 * ============================================================ */
uint16_t AppMot_GetRpm(void)
{
    uint16_t rpm;

    /* Verify integrity before returning */
    if (SAFETYVAR_CHECK(s_motor_rpm) == STD_TRUE)
    {
        rpm = SAFETYVAR_GET(s_motor_rpm);
    }
    else
    {
        rpm = 0U;  /* Return safe value on corruption */
    }

    return rpm;
}

/* ============================================================
 * AppMot_SetTargetRpm
 * ============================================================ */
void AppMot_SetTargetRpm(uint16_t target_rpm)
{
    /* Clamp to rated speed */
    if (target_rpm > MOTOR_RATED_RPM)
    {
        s_target_rpm = MOTOR_RATED_RPM;
    }
    else
    {
        s_target_rpm = target_rpm;
    }
}
