/**
 * @file    hal_uart.c
 * @brief   UART hardware abstraction layer implementation
 * @module  SA-003 HAL (UT-012)
 */

#include "hal_uart.h"
#include "../config/mcu_config.h"

/* UART baud rate: 9600bps at 32MHz */
#define UART_SDR_VALUE   (0xCE00U)  /* fCLK / (2 * 9600) - 1, upper 9 bits */

/* ============================================================
 * HalUart_Init
 * ============================================================ */
void HalUart_Init(void)
{
    /* Enable SAU0 peripheral clock */
    REG_SAU0EN = (uint8_t)1U;

    /* Configure UART0: 9600bps, 8-bit, no parity, 1 stop bit */
    REG_SDR00 = UART_SDR_VALUE;  /* TX baud rate */
    REG_SDR01 = UART_SDR_VALUE;  /* RX baud rate */

    /* Enable serial output */
    REG_SOE0 = (uint8_t)0x01U;

    /* Start serial channels (TX=ch00, RX=ch01) */
    REG_SS0 = (uint8_t)0x03U;
}

/* ============================================================
 * HalUart_SendByte
 * ============================================================ */
void HalUart_SendByte(uint8_t data)
{
    /* Write data to transmit register */
    REG_SDR00 = (uint16_t)data;

    /* Wait for transmission complete (blocking) */
    /* In production, use interrupt-driven TX */
    __NOP();
}

/* ============================================================
 * HalUart_Send
 * ============================================================ */
void HalUart_Send(const uint8_t *p_data, uint16_t length)
{
    uint16_t i;

    if (p_data != NULL_PTR)
    {
        for (i = 0U; i < length; i++)
        {
            HalUart_SendByte(p_data[i]);
        }
    }
}

/* ============================================================
 * HalUart_IsRxReady
 * ============================================================ */
uint8_t HalUart_IsRxReady(void)
{
    uint8_t result;

    if ((REG_SSR01 & (uint8_t)0x20U) != (uint8_t)0U)
    {
        result = STD_TRUE;
    }
    else
    {
        result = STD_FALSE;
    }

    return result;
}

/* ============================================================
 * HalUart_ReceiveByte
 * ============================================================ */
uint8_t HalUart_ReceiveByte(void)
{
    uint8_t data;

    data = (uint8_t)(REG_SDR01 & (uint16_t)0x00FFU);

    /* Clear error flags */
    REG_SIR01 = (uint8_t)0x07U;

    return data;
}
