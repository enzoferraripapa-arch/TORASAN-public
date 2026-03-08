/**
 * @file   hal.h
 * @brief  Hardware Abstraction Layer - RL78/G14 peripheral access
 * @doc    WMC-SUD-001 §4.1 HAL Interface Specification
 *
 * Provides unified API for PWM, ADC, GPIO, WDT, and UART peripherals.
 * All functions are non-blocking unless stated otherwise.
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#ifndef HAL_H
#define HAL_H

#include "std_types.h"
#include "product_config.h"

/* ========================================================================
 * Port / Pin Definitions
 * RL78/G14 port mapping for washing machine motor control
 * ======================================================================== */

/** Gate driver enable output (active high) - Port 5, Pin 0 */
#define HAL_PORT_GATE_EN        ((uint8_t)5U)
#define HAL_PIN_GATE_EN         ((uint8_t)0U)

/** Lid sensor input (active low = closed) - Port 4, Pin 3 */
#define HAL_PORT_LID_SENSOR     ((uint8_t)4U)
#define HAL_PIN_LID_SENSOR      ((uint8_t)3U)

/** Lid lock output (active high = locked) - Port 4, Pin 4 */
#define HAL_PORT_LID_LOCK       ((uint8_t)4U)
#define HAL_PIN_LID_LOCK        ((uint8_t)4U)

/* ========================================================================
 * ADC Channel Definitions
 * ======================================================================== */

/** Motor current sense - ANI0 */
#define HAL_ADC_CH_CURRENT      ((uint8_t)0U)

/** Supply voltage sense - ANI1 */
#define HAL_ADC_CH_VOLTAGE      ((uint8_t)1U)

/* ========================================================================
 * PWM Phase Definitions
 * ======================================================================== */

/** PWM output phases for 3-phase BLDC */
#define HAL_PWM_PHASE_U         ((uint8_t)0U)
#define HAL_PWM_PHASE_V         ((uint8_t)1U)
#define HAL_PWM_PHASE_W         ((uint8_t)2U)

/** Maximum PWM duty value (10-bit) */
#define HAL_PWM_DUTY_MAX        ((uint16_t)1023U)

/* ========================================================================
 * GPIO Logic Levels
 * ======================================================================== */

#define HAL_GPIO_LOW            ((uint8_t)0U)
#define HAL_GPIO_HIGH           ((uint8_t)1U)

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief  Initialize all hardware peripherals
 * @detail Configures PWM timer, ADC, GPIO ports, WDT, and UART.
 *         Must be called once at startup before any other HAL function.
 */
void HAL_Init(void);

/* --- PWM functions --- */

/**
 * @brief  Start PWM output on all three phases
 */
void HAL_PWM_Start(void);

/**
 * @brief  Stop PWM output on all three phases (outputs driven low)
 */
void HAL_PWM_Stop(void);

/**
 * @brief  Set PWM duty cycle for a specific phase
 * @param  phase  Phase index (HAL_PWM_PHASE_U/V/W)
 * @param  duty   Duty cycle value (0 to HAL_PWM_DUTY_MAX)
 */
void HAL_PWM_SetDuty(uint8_t phase, uint16_t duty);

/* --- ADC functions --- */

/**
 * @brief  Read ADC value from specified channel
 * @param  ch  ADC channel number (HAL_ADC_CH_CURRENT or HAL_ADC_CH_VOLTAGE)
 * @return 10-bit ADC result (0 to ADC_MAX_VALUE)
 */
uint16_t HAL_ADC_Read(uint8_t ch);

/* --- GPIO functions --- */

/**
 * @brief  Read GPIO pin state
 * @param  port  Port number (0-14)
 * @param  pin   Pin number (0-7)
 * @return HAL_GPIO_LOW or HAL_GPIO_HIGH
 */
uint8_t HAL_GPIO_Read(uint8_t port, uint8_t pin);

/**
 * @brief  Write GPIO pin state
 * @param  port  Port number (0-14)
 * @param  pin   Pin number (0-7)
 * @param  val   HAL_GPIO_LOW or HAL_GPIO_HIGH
 */
void HAL_GPIO_Write(uint8_t port, uint8_t pin, uint8_t val);

/* --- WDT functions --- */

/**
 * @brief  Start hardware watchdog timer
 * @detail Configures WDT with WDT_TIMEOUT_MS period.
 *         Once started, WDT cannot be stopped (hardware limitation).
 */
void HAL_WDT_Start(void);

/**
 * @brief  Kick (refresh) hardware watchdog timer
 * @detail Must be called within WDT_TIMEOUT_MS to prevent reset.
 */
void HAL_WDT_Kick(void);

/* --- UART functions --- */

/**
 * @brief  Send data via UART (blocking)
 * @param  data  Pointer to data buffer (must not be NULL)
 * @param  len   Number of bytes to send (1 to 255)
 */
void HAL_UART_Send(const uint8_t *data, uint8_t len);

#endif /* HAL_H */
