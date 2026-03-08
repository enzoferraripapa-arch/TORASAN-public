/**
 * @file   hal_gpio.h
 * @brief  GPIO HALドライバ ヘッダ
 * @req    —
 * @safety_class  —（HAL層）
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

typedef enum { GPIO_LOW = 0, GPIO_HIGH = 1 } GpioLevel_t;

void HAL_GPIO_Init(void);
GpioLevel_t HAL_GPIO_Read(uint8_t pin);
void HAL_GPIO_Write(uint8_t pin, GpioLevel_t level);

#endif /* HAL_GPIO_H */
