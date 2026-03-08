/**
 * @file    hal_timer.c
 * @brief   Timer hardware abstraction layer implementation
 * @module  SA-003 HAL (UT-011)
 * @safety  IEC 60730 Class B
 * @req     SR-001, SR-012
 */

#include "hal_timer.h"
#include "../config/mcu_config.h"
#include "../types/safety_types.h"

/* ============================================================
 * Module-static variables
 * ============================================================ */

/** System tick counter (1ms resolution, wraps at ~49 days) */
static volatile uint32_t s_tick_ms = 0UL;

/** Hall sensor period in timer counts */
static volatile uint16_t s_hall_period_counts = 0U;

/** Previous hall capture value */
static volatile uint16_t s_hall_prev_capture = 0U;

/** Hall period valid flag */
static volatile uint8_t s_hall_valid = STD_FALSE;

/* Timer prescaler: 32MHz / 32 = 1MHz (1us resolution) */
#define TIMER_PRESCALER_US   (1U)

/* ============================================================
 * HalTimer_Init
 * ============================================================ */
void HalTimer_Init(void)
{
    /* Enable TAU0 peripheral (shared with PWM) */
    REG_TAU0EN = (uint8_t)1U;

    /* Channel 00: 1ms interval timer (system tick)
     * fCLK/32 = 1MHz, count 1000 = 1ms */
    REG_TDR00 = (uint16_t)999U;  /* 1000 counts - 1 */

    /* Channel 01: Hall sensor input capture
     * fCLK/32 = 1MHz, free-running counter */
    REG_TDR01 = (uint16_t)0xFFFFU;  /* Free-running */

    /* Start channels 00 and 01 */
    REG_TS0 = (uint8_t)0x03U;  /* Start ch00 + ch01 */

    s_tick_ms = 0UL;
    s_hall_period_counts = 0U;
    s_hall_prev_capture = 0U;
    s_hall_valid = STD_FALSE;
}

/* ============================================================
 * HalTimer_GetHallPeriod
 * ============================================================ */
uint32_t HalTimer_GetHallPeriod(void)
{
    uint32_t period_us;

    ENTER_CRITICAL();
    if (s_hall_valid == STD_TRUE)
    {
        /* Each count = 1us at 1MHz prescaler */
        period_us = (uint32_t)s_hall_period_counts * (uint32_t)TIMER_PRESCALER_US;
    }
    else
    {
        period_us = 0UL;  /* No valid measurement */
    }
    EXIT_CRITICAL();

    return period_us;
}

/* ============================================================
 * HalTimer_GetTick
 * ============================================================ */
uint32_t HalTimer_GetTick(void)
{
    uint32_t tick;

    ENTER_CRITICAL();
    tick = s_tick_ms;
    EXIT_CRITICAL();

    return tick;
}

/* ============================================================
 * HalTimer_TickIncrement (called from Timer CH00 ISR)
 * ============================================================ */
void HalTimer_TickIncrement(void)
{
    s_tick_ms++;
}

/* ============================================================
 * HalTimer_HallCapture (called from Hall edge ISR)
 * ============================================================ */
void HalTimer_HallCapture(uint16_t captured_count)
{
    uint16_t period;

    /* Calculate period as difference from previous capture */
    period = (uint16_t)(captured_count - s_hall_prev_capture);
    s_hall_prev_capture = captured_count;

    if (period > (uint16_t)0U)
    {
        s_hall_period_counts = period;
        s_hall_valid = STD_TRUE;
    }
}
