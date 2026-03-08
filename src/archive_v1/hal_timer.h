/**
 * @file   hal_timer.h
 * @brief  タイマHALドライバ ヘッダ
 * @req    —
 * @safety_class  —（HAL層）
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>

void HAL_Timer_Init(void);
void HAL_Timer_StartPwm(uint8_t channel, uint16_t period, uint16_t duty);
void HAL_Timer_StopPwm(uint8_t channel);
void HAL_Timer_StopAllPwm(void);
uint32_t HAL_Timer_GetCapture(uint8_t channel);

#endif /* HAL_TIMER_H */
