/**
 * @file    safety_config.h
 * @brief   Safety threshold definitions derived from product_spec
 * @module  Config
 * @safety  IEC 60730 Class B
 *
 * All numerical thresholds are defined here as macros.
 * Values are derived from project.json product_spec (Single Source of Truth).
 * Integer-only: voltages in mV, currents in mA to avoid float.
 */

#ifndef SAFETY_CONFIG_H
#define SAFETY_CONFIG_H

/* ============================================================
 * Motor Parameters (product_spec.motor)
 * ============================================================ */
#define MOTOR_RATED_RPM              (1200U)   /**< Rated speed [rpm] */
#define OVERSPEED_THRESHOLD_RPM      (1500U)   /**< product_spec.motor.overspeed_rpm */
#define OVERCURRENT_THRESHOLD_MA     (8000U)   /**< product_spec.motor.max_a * 1000 */
#define MOTOR_RATED_CURRENT_MA       (6000U)   /**< product_spec.motor.rated_a * 1000 */
#define MOTOR_POWER_W                (1500U)   /**< product_spec.motor.power_w */

/* Debounce counts for safety detection */
#define OVERSPEED_DEBOUNCE_COUNT     (3U)      /**< 3 consecutive detections (30ms @10ms) */
#define OVERCURRENT_DEBOUNCE_COUNT   (2U)      /**< 2 consecutive detections (20ms @10ms) */
#define LID_DEBOUNCE_COUNT           (3U)      /**< 3 consecutive detections (30ms @10ms) */
#define VOLTAGE_DEBOUNCE_COUNT       (3U)      /**< 3 consecutive detections */

/* ============================================================
 * ADC Parameters (product_spec.adc)
 * ============================================================ */
#define ADC_BITS                     (10U)     /**< product_spec.adc.bits */
#define ADC_MAX_COUNT                (1023U)   /**< (1 << ADC_BITS) - 1 */
#define ADC_VREF_MV                  (5000U)   /**< product_spec.adc.vref_v * 1000 */
#define ADC_CURRENT_SAMPLE_HZ        (100U)    /**< product_spec.adc.current_hz */
#define ADC_VOLTAGE_SAMPLE_HZ        (10U)     /**< product_spec.adc.voltage_hz */

/* ADC diagnosis */
#define ADC_DIAG_CONSECUTIVE_FAIL    (5U)      /**< 5 consecutive anomalies to confirm */

/* ============================================================
 * Voltage Monitor (product_spec.voltage_monitor)
 * ============================================================ */
#define VOLTAGE_MIN_MV               (4500U)   /**< product_spec.voltage_monitor.min_v * 1000 */
#define VOLTAGE_MAX_MV               (5500U)   /**< product_spec.voltage_monitor.max_v * 1000 */

/* ============================================================
 * WDT Parameters (product_spec.wdt)
 * ============================================================ */
#define WDT_TIMEOUT_MS               (100U)    /**< product_spec.wdt.timeout_ms */
#define WDT_KICK_PERIOD_MS           (50U)     /**< product_spec.wdt.kick_ms */

/* ============================================================
 * Clock Parameters (product_spec.mcu)
 * ============================================================ */
#define MCU_CLOCK_MHZ                (32U)     /**< product_spec.mcu.clock_mhz */
#define MCU_SUB_CLOCK_KHZ            (15U)     /**< product_spec.mcu.sub_clock_khz */
#define CLOCK_TOLERANCE_PCT          (4U)      /**< product_spec.clock_tolerance_pct */

/* ============================================================
 * RAM Test Parameters (product_spec.ram_test)
 * ============================================================ */
#define RAM_TEST_BLOCK_SIZE_B        (256U)    /**< product_spec.ram_test.block_b */
#define RAM_TEST_BLOCK_COUNT         (22U)     /**< product_spec.ram_test.block_count */
#define RAM_TOTAL_B                  (5632U)   /**< product_spec.mcu.ram_b */

/* ============================================================
 * ROM/Flash Parameters (product_spec.mcu)
 * ============================================================ */
#define FLASH_SIZE_KB                (64U)     /**< product_spec.mcu.flash_kb */
#define CRC_POLYNOMIAL               (0x04C11DB7UL)  /**< CRC-32 polynomial */

/* ============================================================
 * Timing Budget
 * ============================================================ */
#define MAIN_CYCLE_MS                (10U)     /**< Main loop cycle period */
#define WDT_KICK_CYCLE_MS            (50U)     /**< WDT kick cycle */
#define STARTUP_DIAG_TIMEOUT_MS      (700U)    /**< DR-001: Max startup time */

/* ============================================================
 * LPF Coefficients (integer fixed-point, Q8 format)
 * ============================================================ */
/** LPF: y = (alpha * x + (256 - alpha) * y_prev) >> 8
 *  alpha=51 corresponds to ~0.2 weighting (51/256 = 0.199) */
#define LPF_ALPHA_CURRENT            (51U)     /**< Current LPF coefficient (Q8) */
#define LPF_ALPHA_VOLTAGE            (26U)     /**< Voltage LPF coefficient (Q8) */

/* ============================================================
 * Error Log
 * ============================================================ */
#define ERROR_LOG_MAX_ENTRIES        (16U)     /**< Max error log entries in NVM */

/* ============================================================
 * BLDC Commutation
 * ============================================================ */
#define HALL_PHASES                  (6U)      /**< 6-step commutation phases */
#define PWM_MAX_DUTY                 (1000U)   /**< PWM duty cycle max (0.1% resolution) */

#endif /* SAFETY_CONFIG_H */
