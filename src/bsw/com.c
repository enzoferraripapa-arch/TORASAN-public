/**
 * @file    com.c
 * @brief   UART communication module implementation
 * @module  SA-008 COM (UT-020)
 *
 * Non-safety module. Exchanges data with UI panel via UART.
 * Protocol: simple binary frame [STX][LEN][CMD][DATA...][CRC8]
 */

#include "com.h"
#include "hal_uart.h"

/* Protocol constants */
#define COM_STX              ((uint8_t)0x02U)  /**< Start of frame */
#define COM_CMD_STATUS       ((uint8_t)0x01U)  /**< Status report command */
#define COM_STATUS_FRAME_LEN ((uint8_t)7U)     /**< Status frame total length */

/* ============================================================
 * Module-static variables
 * ============================================================ */

/** Transmit buffer */
static uint8_t s_tx_buffer[16U];

/** Receive state */
static uint8_t s_rx_state = 0U;

/* ============================================================
 * Internal: Calculate CRC8 (simple XOR checksum)
 * ============================================================ */
static uint8_t Com_CalcCrc8(const uint8_t *p_data, uint8_t length)
{
    uint8_t crc = 0U;
    uint8_t i;

    for (i = 0U; i < length; i++)
    {
        crc ^= p_data[i];
    }

    return crc;
}

/* ============================================================
 * Com_Init
 * ============================================================ */
void Com_Init(void)
{
    uint8_t i;

    HalUart_Init();

    for (i = 0U; i < (uint8_t)16U; i++)
    {
        s_tx_buffer[i] = 0U;
    }

    s_rx_state = 0U;
}

/* ============================================================
 * Com_Cyclic10ms
 * ============================================================ */
void Com_Cyclic10ms(void)
{
    /* Process any received data (command parsing) */
    while (HalUart_IsRxReady() == STD_TRUE)
    {
        uint8_t rx_byte = HalUart_ReceiveByte();
        (void)rx_byte;  /* Command processing TBD */
    }
}

/* ============================================================
 * Com_SendStatus
 * ============================================================ */
void Com_SendStatus(SystemState_t state, uint16_t rpm, FaultCode_t fault_code)
{
    /* Build status frame: [STX][LEN][CMD][STATE][RPM_H][RPM_L][FAULT][CRC8] */
    s_tx_buffer[0U] = COM_STX;
    s_tx_buffer[1U] = COM_STATUS_FRAME_LEN;
    s_tx_buffer[2U] = COM_CMD_STATUS;
    s_tx_buffer[3U] = (uint8_t)state;
    s_tx_buffer[4U] = (uint8_t)(rpm >> 8U);
    s_tx_buffer[5U] = (uint8_t)(rpm & (uint16_t)0x00FFU);
    s_tx_buffer[6U] = (uint8_t)fault_code;

    /* Calculate CRC over payload (bytes 2-6) */
    s_tx_buffer[7U] = Com_CalcCrc8(&s_tx_buffer[2U], (uint8_t)5U);

    /* Send frame */
    HalUart_Send(s_tx_buffer, (uint16_t)8U);
}
