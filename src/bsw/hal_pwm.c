/**
 * @file    hal_pwm.c
 * @brief   PWM hardware abstraction layer implementation
 * @module  SA-003 HAL (UT-010)
 * @safety  IEC 60730 Class B
 * @req     SR-003, SR-006
 */

#include "hal_pwm.h"
#include "../config/mcu_config.h"
#include "../config/safety_config.h"

/* PWM period for 20kHz at 32MHz: 32000000/20000 = 1600 counts */
#define PWM_PERIOD_COUNTS    (1600U)

/* ============================================================
 * HalPwm_Init
 * ============================================================ */
void HalPwm_Init(void)
{
    /* Enable TAU0 peripheral clock */
    REG_TAU0EN = (uint8_t)1U;

    /* Configure timer channels 02-04 as PWM output
     * Period = PWM_PERIOD_COUNTS at fCLK */
    REG_TDR02 = (uint16_t)0U;  /* U-phase: duty 0% */
    REG_TDR03 = (uint16_t)0U;  /* V-phase: duty 0% */
    REG_TDR04 = (uint16_t)0U;  /* W-phase: duty 0% */

    /* Disable output initially (safe default) */
    REG_TOE0 &= (uint8_t)(~(uint8_t)0x1CU);  /* Clear bits 2-4 */
    REG_TO0  &= (uint8_t)(~(uint8_t)0x1CU);   /* Output low */
}

/* ============================================================
 * HalPwm_SetDuty
 * ============================================================ */
void HalPwm_SetDuty(uint8_t phase, uint16_t duty)
{
    uint16_t compare_val;

    /* Clamp duty to max */
    uint16_t clamped_duty = duty;
    if (clamped_duty > PWM_MAX_DUTY)
    {
        clamped_duty = PWM_MAX_DUTY;
    }

    /* Convert duty (0-1000) to compare value (0-PWM_PERIOD_COUNTS)
     * compare_val = (duty * PWM_PERIOD_COUNTS) / PWM_MAX_DUTY */
    compare_val = (uint16_t)(((uint32_t)clamped_duty * (uint32_t)PWM_PERIOD_COUNTS)
                             / (uint32_t)PWM_MAX_DUTY);

    switch (phase)
    {
        case 0U:  /* U-phase */
            REG_TDR02 = compare_val;
            break;
        case 1U:  /* V-phase */
            REG_TDR03 = compare_val;
            break;
        case 2U:  /* W-phase */
            REG_TDR04 = compare_val;
            break;
        default:
            /* Invalid phase: no action */
            break;
    }
}

/* ============================================================
 * HalPwm_AllStop
 * ============================================================ */
void HalPwm_AllStop(void)
{
    /* Set all duty to 0 */
    REG_TDR02 = (uint16_t)0U;
    REG_TDR03 = (uint16_t)0U;
    REG_TDR04 = (uint16_t)0U;

    /* Disable output */
    REG_TOE0 &= (uint8_t)(~(uint8_t)0x1CU);
    REG_TO0  &= (uint8_t)(~(uint8_t)0x1CU);
}

/* ============================================================
 * HalPwm_EnableOutput
 * ============================================================ */
void HalPwm_EnableOutput(void)
{
    REG_TOE0 |= (uint8_t)0x1CU;  /* Enable channels 2-4 output */
}

/* ============================================================
 * HalPwm_DisableOutput
 * ============================================================ */
void HalPwm_DisableOutput(void)
{
    REG_TOE0 &= (uint8_t)(~(uint8_t)0x1CU);
    REG_TO0  &= (uint8_t)(~(uint8_t)0x1CU);
}
