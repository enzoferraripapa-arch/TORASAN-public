/**
 * @file    pin_config.h
 * @brief   Pin assignment definitions for RL78/G14 R5F104BG
 * @module  Config
 * @safety  IEC 60730 Class B
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

/* ============================================================
 * Port 0: ADC Inputs (Analog)
 * ============================================================ */
#define PIN_ANI0_CURRENT     (0U)   /**< P0.0/ANI0: Motor current sense */
#define PIN_ANI1_VOLTAGE     (1U)   /**< P0.1/ANI1: Supply voltage sense */
#define PIN_ANI2_DIAG_REF    (2U)   /**< P0.2/ANI2: Diagnostic reference */

/* ============================================================
 * Port 2: Digital Outputs
 * ============================================================ */
#define PIN_RELAY_OUT        (1U)   /**< P2.1: Inverter relay control */
#define PIN_LED_RED          (2U)   /**< P2.2: Fault indicator (red) */
#define PIN_LED_GREEN        (3U)   /**< P2.3: Normal indicator (green) */

/* ============================================================
 * Port 3: Hall Sensor Inputs + PWM Outputs
 * ============================================================ */
#define PIN_HALL_U           (0U)   /**< P3.0: Hall sensor U-phase */
#define PIN_HALL_V           (1U)   /**< P3.1: Hall sensor V-phase */
#define PIN_HALL_W           (2U)   /**< P3.2: Hall sensor W-phase */
#define PIN_PWM_U            (3U)   /**< P3.3/TO02: PWM U-phase output */
#define PIN_PWM_V            (4U)   /**< P3.4/TO03: PWM V-phase output */
#define PIN_PWM_W            (5U)   /**< P3.5/TO04: PWM W-phase output */

/* ============================================================
 * Port 7: Sensor / Communication
 * ============================================================ */
#define PIN_LID_SENSOR       (0U)   /**< P7.0: Lid open sensor input */
#define PIN_UART_TX          (2U)   /**< P7.2/TxD0: UART transmit */
#define PIN_UART_RX          (3U)   /**< P7.3/RxD0: UART receive */

/* ============================================================
 * Port Configuration Macros
 * ============================================================ */
/** Set pin as input (PM bit = 1) */
#define PIN_MODE_INPUT(port_reg, bit)   ((port_reg) |= (uint8_t)(1U << (bit)))
/** Set pin as output (PM bit = 0) */
#define PIN_MODE_OUTPUT(port_reg, bit)  ((port_reg) &= (uint8_t)(~(uint8_t)(1U << (bit))))

#endif /* PIN_CONFIG_H */
