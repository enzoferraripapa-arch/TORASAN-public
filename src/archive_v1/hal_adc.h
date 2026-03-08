/**
 * @file   hal_adc.h
 * @brief  ADC HALドライバ ヘッダ
 * @req    —
 * @safety_class  —（HAL層）
 */

#ifndef HAL_ADC_H
#define HAL_ADC_H

#include <stdint.h>

void HAL_ADC_Init(void);
uint16_t HAL_ADC_Read(uint8_t channel);
void HAL_ADC_StartContinuous(uint8_t channel);

#endif /* HAL_ADC_H */
