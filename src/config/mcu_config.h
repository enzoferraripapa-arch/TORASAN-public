/**
 * @file    mcu_config.h
 * @brief   RL78/G14 MCU register and SFR abstraction
 * @module  Config
 * @safety  IEC 60730 Class B
 *
 * Provides portable register access macros for Renesas RL78/G14.
 * On host/test builds, registers are mapped to RAM variables.
 */

#ifndef MCU_CONFIG_H
#define MCU_CONFIG_H

#include <stdint.h>

#ifdef TARGET_RL78
  /* RL78/G14 hardware register includes */
  #include "iodefine.h"
#else
  /* ============================================================
   * Host/Test Stub: Registers as volatile RAM variables
   * ============================================================ */

  /* General SFR stubs */
  extern volatile uint8_t  REG_PM0;    /**< Port mode register 0 */
  extern volatile uint8_t  REG_PM2;    /**< Port mode register 2 */
  extern volatile uint8_t  REG_PM3;    /**< Port mode register 3 */
  extern volatile uint8_t  REG_PM7;    /**< Port mode register 7 */
  extern volatile uint8_t  REG_P0;     /**< Port output register 0 */
  extern volatile uint8_t  REG_P2;     /**< Port output register 2 */
  extern volatile uint8_t  REG_P3;     /**< Port output register 3 */
  extern volatile uint8_t  REG_P7;     /**< Port output register 7 */

  /* ADC registers */
  extern volatile uint8_t  REG_ADCEN;  /**< ADC enable */
  extern volatile uint8_t  REG_ADM0;   /**< ADC mode register 0 */
  extern volatile uint8_t  REG_ADM1;   /**< ADC mode register 1 */
  extern volatile uint8_t  REG_ADS;    /**< ADC channel select */
  extern volatile uint16_t REG_ADCR;   /**< ADC result register */
  extern volatile uint8_t  REG_ADIF;   /**< ADC interrupt flag */

  /* Timer registers */
  extern volatile uint8_t  REG_TAU0EN; /**< TAU0 enable */
  extern volatile uint16_t REG_TDR00;  /**< Timer data register 00 */
  extern volatile uint16_t REG_TDR01;  /**< Timer data register 01 */
  extern volatile uint16_t REG_TCR01;  /**< Timer counter register 01 */
  extern volatile uint8_t  REG_TS0;    /**< Timer start */
  extern volatile uint8_t  REG_TT0;    /**< Timer stop */

  /* PWM registers */
  extern volatile uint16_t REG_TDR02;  /**< PWM duty U-phase */
  extern volatile uint16_t REG_TDR03;  /**< PWM duty V-phase */
  extern volatile uint16_t REG_TDR04;  /**< PWM duty W-phase */
  extern volatile uint8_t  REG_TOE0;   /**< Timer output enable */
  extern volatile uint8_t  REG_TO0;    /**< Timer output */

  /* UART registers */
  extern volatile uint8_t  REG_SAU0EN; /**< SAU0 enable */
  extern volatile uint8_t  REG_SS0;    /**< Serial start */
  extern volatile uint8_t  REG_ST0;    /**< Serial stop */
  extern volatile uint8_t  REG_SOE0;   /**< Serial output enable */
  extern volatile uint16_t REG_SDR00;  /**< Serial data register 00 (TX) */
  extern volatile uint16_t REG_SDR01;  /**< Serial data register 01 (RX) */
  extern volatile uint8_t  REG_SSR01;  /**< Serial status register */
  extern volatile uint8_t  REG_SIR01;  /**< Serial interrupt flag */

  /* WDT register */
  extern volatile uint8_t  REG_WDTE;   /**< WDT enable/kick register */

  /* Clock registers */
  extern volatile uint8_t  REG_CMC;    /**< Clock mode control */
  extern volatile uint8_t  REG_CSC;    /**< Clock control */
  extern volatile uint8_t  REG_CKC;    /**< System clock control */

  /* Interrupt control */
  #define __DI()   ((void)0)
  #define __EI()   ((void)0)
  #define __NOP()  ((void)0)
  #define __HALT() ((void)0)
#endif

/* ============================================================
 * Register Bit Definitions
 * ============================================================ */

/* WDT */
#define WDT_KICK_PATTERN     (0xACU)   /**< WDT kick pattern for RL78 */

/* ADC */
#define ADC_START_BIT        (0x80U)   /**< ADC conversion start bit */
#define ADC_COMPLETE_BIT     (0x01U)   /**< ADC conversion complete flag */

/* ADC Channel Assignments */
#define ADC_CH_CURRENT       (0U)      /**< ANI0: Motor current sense */
#define ADC_CH_VOLTAGE       (1U)      /**< ANI1: Supply voltage sense */
#define ADC_CH_DIAG_REF      (2U)      /**< ANI2: Diagnostic reference */

/* GPIO Bit Masks */
#define GPIO_LID_SENSOR_BIT  (0x01U)   /**< P7.0: Lid sensor input */
#define GPIO_RELAY_BIT       (0x02U)   /**< P2.1: Relay output */
#define GPIO_LED_RED_BIT     (0x04U)   /**< P2.2: Red LED output */
#define GPIO_LED_GREEN_BIT   (0x08U)   /**< P2.3: Green LED output */
#define GPIO_HALL_U_BIT      (0x01U)   /**< P3.0: Hall sensor U */
#define GPIO_HALL_V_BIT      (0x02U)   /**< P3.1: Hall sensor V */
#define GPIO_HALL_W_BIT      (0x04U)   /**< P3.2: Hall sensor W */

#endif /* MCU_CONFIG_H */
