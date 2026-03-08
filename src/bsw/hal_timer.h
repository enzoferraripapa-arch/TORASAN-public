/**
 * @file    hal_timer.h
 * @brief   Timer hardware abstraction layer
 * @module  SA-003 HAL
 * @safety  IEC 60730 Class B
 * @req     SR-001, SR-012
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>
#include "../types/std_types.h"

/**
 * @brief   Initialize timer peripheral (1ms tick + hall capture)
 * @module  SA-003 HAL
 */
void HalTimer_Init(void);

/**
 * @brief   Get hall sensor period (time between edges) in us
 * @module  SA-003 HAL
 * @req     SR-001
 * @safety  IEC 60730 Class B
 * @return  Period in microseconds (0 = no signal)
 */
uint32_t HalTimer_GetHallPeriod(void);

/**
 * @brief   Get system tick counter (1ms resolution)
 * @module  SA-003 HAL
 * @return  Tick count in milliseconds since startup
 */
uint32_t HalTimer_GetTick(void);

/**
 * @brief   Increment tick counter (called from timer ISR)
 * @module  SA-003 HAL
 */
void HalTimer_TickIncrement(void);

/**
 * @brief   Update hall period capture (called from hall edge ISR)
 * @module  SA-003 HAL
 * @param   captured_count  Timer counter value at hall edge
 */
void HalTimer_HallCapture(uint16_t captured_count);

#endif /* HAL_TIMER_H */
