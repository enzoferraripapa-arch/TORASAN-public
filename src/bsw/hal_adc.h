/**
 * @file    hal_adc.h
 * @brief   ADC hardware abstraction layer
 * @module  SA-003 HAL
 * @safety  IEC 60730 Class B
 * @req     SR-004, SR-014
 */

#ifndef HAL_ADC_H
#define HAL_ADC_H

#include <stdint.h>
#include "../types/std_types.h"

/**
 * @brief   Initialize ADC peripheral
 * @module  SA-003 HAL
 */
void HalAdc_Init(void);

/**
 * @brief   Start ADC conversion on specified channel
 * @module  SA-003 HAL
 * @param   channel  ADC channel number (0-2)
 */
void HalAdc_StartConversion(uint8_t channel);

/**
 * @brief   Check if ADC conversion is complete
 * @module  SA-003 HAL
 * @return  STD_TRUE if conversion complete, STD_FALSE otherwise
 */
uint8_t HalAdc_IsConversionDone(void);

/**
 * @brief   Read ADC result (10-bit raw value)
 * @module  SA-003 HAL
 * @return  ADC conversion result (0-1023)
 */
uint16_t HalAdc_ReadResult(void);

/**
 * @brief   Get motor current in mA (ADC channel 0)
 * @module  SA-003 HAL
 * @req     SR-004
 * @safety  IEC 60730 Class B
 * @return  Motor current [mA]
 */
uint16_t HalAdc_GetCurrent(void);

/**
 * @brief   Get supply voltage in mV (ADC channel 1)
 * @module  SA-003 HAL
 * @req     SR-014
 * @safety  IEC 60730 Class B
 * @return  Supply voltage [mV]
 */
uint16_t HalAdc_GetVoltage(void);

#endif /* HAL_ADC_H */
