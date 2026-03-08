/**
 * @file    hal_uart.h
 * @brief   UART hardware abstraction layer
 * @module  SA-003 HAL
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include "../types/std_types.h"

/**
 * @brief   Initialize UART (9600bps, 8N1)
 * @module  SA-003 HAL
 */
void HalUart_Init(void);

/**
 * @brief   Send a single byte via UART
 * @module  SA-003 HAL
 * @param   data  Byte to send
 */
void HalUart_SendByte(uint8_t data);

/**
 * @brief   Send data buffer via UART
 * @module  SA-003 HAL
 * @param   p_data  Pointer to data buffer
 * @param   length  Number of bytes to send
 */
void HalUart_Send(const uint8_t *p_data, uint16_t length);

/**
 * @brief   Check if receive data is available
 * @module  SA-003 HAL
 * @return  STD_TRUE if data available, STD_FALSE otherwise
 */
uint8_t HalUart_IsRxReady(void);

/**
 * @brief   Receive a single byte from UART
 * @module  SA-003 HAL
 * @return  Received byte
 */
uint8_t HalUart_ReceiveByte(void);

#endif /* HAL_UART_H */
