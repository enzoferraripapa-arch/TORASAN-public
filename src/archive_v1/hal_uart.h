/**
 * @file   hal_uart.h
 * @brief  UART HALドライバ ヘッダ
 * @req    —
 * @safety_class  —（HAL層）
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>

void HAL_UART_Init(uint32_t baudrate);
void HAL_UART_Send(const uint8_t *data, uint16_t len);
uint16_t HAL_UART_Receive(uint8_t *buf, uint16_t maxLen);

#endif /* HAL_UART_H */
