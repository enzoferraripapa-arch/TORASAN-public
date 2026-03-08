/**
 * @file   test_hal_stub.h
 * @brief  HAL test stub - external access to simulated hardware registers
 * @doc    PH-11 Test Infrastructure
 *
 * Provides functions to inject test values into HAL's simulated registers.
 * Used by unit/integration/qualification tests to control sensor inputs,
 * verify PWM outputs, and check GPIO states.
 *
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.1
 * @date    2026-03-01
 */

#ifndef TEST_HAL_STUB_H
#define TEST_HAL_STUB_H

#include <stdint.h>
#include "hal.h"

/* ========================================================================
 * HAL Stub Accessor Functions
 *
 * These are implemented in test_hal_stub.c and provide external access
 * to the simulated hardware registers in hal.c's static storage.
 *
 * Since hal.c uses static globals, we re-declare matching extern variables
 * here and link them from a separate stub implementation that accesses
 * HAL functions to set/get values.
 * ======================================================================== */

/**
 * @brief  Set simulated ADC result for a channel
 * @param  ch     ADC channel (0=current, 1=voltage)
 * @param  value  10-bit ADC value (0-1023)
 */
void TestStub_SetAdcValue(uint8_t ch, uint16_t value);

/**
 * @brief  Set simulated GPIO input pin state
 * @param  port   Port number
 * @param  pin    Pin number
 * @param  val    HAL_GPIO_LOW or HAL_GPIO_HIGH
 */
void TestStub_SetGpioInput(uint8_t port, uint8_t pin, uint8_t val);

/**
 * @brief  Get simulated GPIO output pin state
 * @param  port   Port number
 * @param  pin    Pin number
 * @return HAL_GPIO_LOW or HAL_GPIO_HIGH
 */
uint8_t TestStub_GetGpioOutput(uint8_t port, uint8_t pin);

/**
 * @brief  Get simulated PWM duty for a phase
 * @param  phase  Phase index (0=U, 1=V, 2=W)
 * @return PWM duty value
 */
uint16_t TestStub_GetPwmDuty(uint8_t phase);

/**
 * @brief  Get PWM running state
 * @return 1 if running, 0 if stopped
 */
uint8_t TestStub_GetPwmRunning(void);

/**
 * @brief  Get last UART frame sent by DEM
 * @param  buf     Output buffer (at least 16 bytes)
 * @param  out_len Pointer to receive actual length
 */
void TestStub_GetLastUartFrame(uint8_t *buf, uint8_t *out_len);

/**
 * @brief  Reset all HAL stub state
 */
void TestStub_ResetAll(void);

#endif /* TEST_HAL_STUB_H */
