/**
 * @file    hal_gpio.c
 * @brief   GPIO hardware abstraction layer implementation
 * @module  SA-003 HAL (UT-009)
 * @safety  IEC 60730 Class B
 * @req     SR-007, SR-008
 */

#include "hal_gpio.h"
#include "../config/mcu_config.h"
#include "../config/pin_config.h"

/* ============================================================
 * HalGpio_Init
 * ============================================================ */
void HalGpio_Init(void)
{
    /* Port 0: ADC analog inputs (configured by HalAdc_Init) */

    /* Port 2: Digital outputs (relay, LEDs) */
    PIN_MODE_OUTPUT(REG_PM2, PIN_RELAY_OUT);
    PIN_MODE_OUTPUT(REG_PM2, PIN_LED_RED);
    PIN_MODE_OUTPUT(REG_PM2, PIN_LED_GREEN);

    /* Default: relay OFF, red LED OFF, green LED OFF */
    REG_P2 &= (uint8_t)(~(GPIO_RELAY_BIT | GPIO_LED_RED_BIT | GPIO_LED_GREEN_BIT));

    /* Port 3: Hall sensor inputs (U/V/W) */
    PIN_MODE_INPUT(REG_PM3, PIN_HALL_U);
    PIN_MODE_INPUT(REG_PM3, PIN_HALL_V);
    PIN_MODE_INPUT(REG_PM3, PIN_HALL_W);

    /* Port 7: Lid sensor input */
    PIN_MODE_INPUT(REG_PM7, PIN_LID_SENSOR);
}

/* ============================================================
 * HalGpio_GetLidState
 * ============================================================ */
uint8_t HalGpio_GetLidState(void)
{
    uint8_t result;

    if ((REG_P7 & GPIO_LID_SENSOR_BIT) != (uint8_t)0U)
    {
        result = STD_TRUE;   /* Lid open (active high) */
    }
    else
    {
        result = STD_FALSE;  /* Lid closed */
    }

    return result;
}

/* ============================================================
 * HalGpio_GetHallPattern
 * ============================================================ */
uint8_t HalGpio_GetHallPattern(void)
{
    uint8_t pattern;

    pattern = (uint8_t)(REG_P3 & (GPIO_HALL_U_BIT | GPIO_HALL_V_BIT | GPIO_HALL_W_BIT));

    return pattern;
}

/* ============================================================
 * HalGpio_SetRelay
 * ============================================================ */
void HalGpio_SetRelay(uint8_t state)
{
    if (state == STD_TRUE)
    {
        REG_P2 |= GPIO_RELAY_BIT;   /* Relay ON */
    }
    else
    {
        REG_P2 &= (uint8_t)(~GPIO_RELAY_BIT);  /* Relay OFF */
    }
}

/* ============================================================
 * HalGpio_SetLed
 * ============================================================ */
void HalGpio_SetLed(uint8_t state)
{
    if (state == STD_TRUE)
    {
        /* Fault: Red ON, Green OFF */
        REG_P2 |= GPIO_LED_RED_BIT;
        REG_P2 &= (uint8_t)(~GPIO_LED_GREEN_BIT);
    }
    else
    {
        /* Normal: Red OFF, Green ON */
        REG_P2 &= (uint8_t)(~GPIO_LED_RED_BIT);
        REG_P2 |= GPIO_LED_GREEN_BIT;
    }
}
