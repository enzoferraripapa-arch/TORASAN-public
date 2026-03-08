/**
 * @file   test_hal_stub.c
 * @brief  HAL test stub - replaces hal.c for testing
 * @doc    PH-11 Test Infrastructure
 *
 * Complete replacement of hal.c providing test control functions.
 * Links as a substitute for hal.c in the test build.
 *
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "hal.h"
#include "test_hal_stub.h"
#include <string.h>

/* ========================================================================
 * Simulated Hardware Registers
 * ======================================================================== */

static volatile uint8_t  g_port_out[15];
static volatile uint8_t  g_port_in[15];
static volatile uint16_t g_adc_result[2];
static volatile uint16_t g_pwm_duty[3];
static volatile uint8_t  g_pwm_running;

/** UART capture buffer */
static uint8_t  g_uart_buf[32];
static uint8_t  g_uart_len;

/* ========================================================================
 * HAL Implementation (stub)
 * ======================================================================== */

void HAL_Init(void)
{
    uint8_t i;
    for (i = 0U; i < 15U; i++)
    {
        g_port_out[i] = 0U;
        g_port_in[i]  = 0U;
    }
    g_adc_result[0] = 0U;
    g_adc_result[1] = 0U;
    g_pwm_duty[0] = 0U;
    g_pwm_duty[1] = 0U;
    g_pwm_duty[2] = 0U;
    g_pwm_running = 0U;
    g_uart_len = 0U;
    memset(g_uart_buf, 0, sizeof(g_uart_buf));

    /* Gate driver initially disabled */
    g_port_out[HAL_PORT_GATE_EN] &= (uint8_t)(~(1U << HAL_PIN_GATE_EN));
}

void HAL_PWM_Start(void)
{
    g_pwm_running = 1U;
}

void HAL_PWM_Stop(void)
{
    g_pwm_duty[0] = 0U;
    g_pwm_duty[1] = 0U;
    g_pwm_duty[2] = 0U;
    g_pwm_running = 0U;
}

void HAL_PWM_SetDuty(uint8_t phase, uint16_t duty)
{
    uint16_t clamped = duty;
    if (clamped > HAL_PWM_DUTY_MAX)
    {
        clamped = HAL_PWM_DUTY_MAX;
    }
    if (phase <= HAL_PWM_PHASE_W)
    {
        g_pwm_duty[phase] = clamped;
    }
}

uint16_t HAL_ADC_Read(uint8_t ch)
{
    if (ch <= HAL_ADC_CH_VOLTAGE)
    {
        uint16_t val = g_adc_result[ch];
        if (val > ADC_MAX_VALUE)
        {
            val = ADC_MAX_VALUE;
        }
        return val;
    }
    return 0U;
}

uint8_t HAL_GPIO_Read(uint8_t port, uint8_t pin)
{
    if ((port < 15U) && (pin < 8U))
    {
        if ((g_port_in[port] & (uint8_t)(1U << pin)) != 0U)
        {
            return HAL_GPIO_HIGH;
        }
    }
    return HAL_GPIO_LOW;
}

void HAL_GPIO_Write(uint8_t port, uint8_t pin, uint8_t val)
{
    if ((port < 15U) && (pin < 8U))
    {
        if (val != HAL_GPIO_LOW)
        {
            g_port_out[port] |= (uint8_t)(1U << pin);
        }
        else
        {
            g_port_out[port] &= (uint8_t)(~(1U << pin));
        }
    }
}

void HAL_WDT_Start(void)
{
    /* Stub: no action */
}

void HAL_WDT_Kick(void)
{
    /* Stub: no action */
}

void HAL_UART_Send(const uint8_t *data, uint8_t len)
{
    if ((data != NULL_PTR) && (len > 0U) && (len <= sizeof(g_uart_buf)))
    {
        memcpy(g_uart_buf, data, len);
        g_uart_len = len;
    }
}

/* ========================================================================
 * Test Stub Accessor Functions
 * ======================================================================== */

void TestStub_SetAdcValue(uint8_t ch, uint16_t value)
{
    if (ch <= HAL_ADC_CH_VOLTAGE)
    {
        g_adc_result[ch] = value;
    }
}

void TestStub_SetGpioInput(uint8_t port, uint8_t pin, uint8_t val)
{
    if ((port < 15U) && (pin < 8U))
    {
        if (val != 0U)
        {
            g_port_in[port] |= (uint8_t)(1U << pin);
        }
        else
        {
            g_port_in[port] &= (uint8_t)(~(1U << pin));
        }
    }
}

uint8_t TestStub_GetGpioOutput(uint8_t port, uint8_t pin)
{
    if ((port < 15U) && (pin < 8U))
    {
        if ((g_port_out[port] & (uint8_t)(1U << pin)) != 0U)
        {
            return HAL_GPIO_HIGH;
        }
    }
    return HAL_GPIO_LOW;
}

uint16_t TestStub_GetPwmDuty(uint8_t phase)
{
    if (phase <= HAL_PWM_PHASE_W)
    {
        return g_pwm_duty[phase];
    }
    return 0U;
}

uint8_t TestStub_GetPwmRunning(void)
{
    return g_pwm_running;
}

void TestStub_GetLastUartFrame(uint8_t *buf, uint8_t *out_len)
{
    if ((buf != NULL) && (out_len != NULL))
    {
        memcpy(buf, g_uart_buf, g_uart_len);
        *out_len = g_uart_len;
    }
}

void TestStub_ResetAll(void)
{
    HAL_Init();
}
