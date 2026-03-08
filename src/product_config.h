/**
 * @file product_config.h
 * @brief Product specification macros and common type definitions
 * @doc WMC-SUD-001 §2, WMC-CDG-001 §3.2
 *
 * Single Source of Truth: project.json product_spec
 * All numeric constants derived from product_spec values.
 */

#ifndef PRODUCT_CONFIG_H
#define PRODUCT_CONFIG_H

#include <stdint.h>

/* ========================================================================
 * product_spec.motor
 * ======================================================================== */
#define MOTOR_RATED_RPM         ((uint16_t)1200U)
#define MOTOR_OVERSPEED_RPM     ((uint16_t)1500U)
#define MOTOR_RATED_A           ((uint16_t)6U)
#define MOTOR_MAX_A             ((uint16_t)8U)
#define MOTOR_MAX_MA            ((uint16_t)8000U)
#define MOTOR_POLE_PAIRS        ((uint8_t)4U)
#define MOTOR_VIBRATION_RPM     ((uint16_t)500U)    /* Deceleration target */
#define MOTOR_VIBRATION_MS      ((uint16_t)1000U)   /* Deceleration time limit */

/* Hysteresis */
#define OVERSPEED_HYST_RPM      ((uint16_t)1350U)   /* 1500 * 0.9 */
#define OVERCURRENT_HYST_MA     ((uint16_t)7600U)   /* 8000 * 0.95 */

/* ========================================================================
 * product_spec.adc
 * ======================================================================== */
#define ADC_BITS                ((uint8_t)10U)
#define ADC_MAX_VALUE           ((uint16_t)1023U)
#define ADC_VREF_MV             ((uint16_t)5000U)
#define ADC_CURRENT_HZ          ((uint8_t)100U)
#define ADC_VOLTAGE_HZ          ((uint8_t)10U)

/* ========================================================================
 * product_spec.wdt
 * ======================================================================== */
#define WDT_TIMEOUT_MS          ((uint16_t)100U)
#define WDT_KICK_MS             ((uint16_t)50U)

/* ========================================================================
 * product_spec.voltage_monitor
 * ======================================================================== */
#define VOLTAGE_MIN_MV          ((uint16_t)4500U)
#define VOLTAGE_MAX_MV          ((uint16_t)5500U)
#define VOLTAGE_HYST_MV         ((uint16_t)100U)

/* ========================================================================
 * product_spec.clock
 * ======================================================================== */
#define CLOCK_MAIN_MHZ          ((uint8_t)32U)
#define CLOCK_SUB_KHZ           ((uint16_t)32U)     /* 32.768 kHz crystal */
#define CLOCK_TOLERANCE_PCT     ((uint8_t)4U)

/* ========================================================================
 * product_spec.ram_test
 * ======================================================================== */
#define RAM_TEST_BLOCK_B        ((uint16_t)256U)
#define RAM_TEST_BLOCK_COUNT    ((uint8_t)22U)
#define RAM_TOTAL_B             ((uint16_t)5632U)

/* ========================================================================
 * product_spec.mcu
 * ======================================================================== */
#define MCU_FLASH_KB            ((uint8_t)64U)
#define MCU_RAM_B               ((uint16_t)5632U)

/* ========================================================================
 * product_spec.crc
 * ======================================================================== */
#define CRC32_POLYNOMIAL        ((uint32_t)0x04C11DB7UL)

/* ========================================================================
 * Timing constants
 * ======================================================================== */
#define CONTROL_LOOP_MS         ((uint8_t)10U)
#define DEBOUNCE_COUNT          ((uint8_t)6U)       /* 6 * 10ms = 60ms */
#define LID_STOP_MS             ((uint16_t)140U)
#define SAFE_TRANSITION_MS      ((uint16_t)100U)

/* ========================================================================
 * Common type definitions
 * ======================================================================== */

/** Lid sensor state */
typedef enum {
    LID_CLOSED = 0U,
    LID_OPEN   = 1U
} lid_state_t;

/** System operating mode */
typedef enum {
    SYS_MODE_INIT     = 0U,
    SYS_MODE_STARTUP  = 1U,
    SYS_MODE_NORMAL   = 2U,
    SYS_MODE_SAFE     = 3U,
    SYS_MODE_FAULT    = 4U
} sys_mode_t;

/** Safety flags with CRC-16 protection */
typedef struct {
    uint8_t  overspeed;
    uint8_t  overcurrent;
    uint8_t  lid_open;
    uint8_t  voltage_error;
    uint8_t  vibration;
    uint8_t  diag_fail;
    uint16_t crc;
} safety_flags_t;

/** Diagnostic status */
typedef struct {
    uint8_t cpu_ok;
    uint8_t ram_ok;
    uint8_t rom_ok;
    uint8_t clk_ok;
    uint8_t ram_block_idx;    /* Current RAM test block index */
    uint8_t rom_block_idx;    /* Current ROM test block index */
} diag_status_t;

/** Fault codes */
#define FAULT_NONE              ((uint8_t)0x00U)
#define FAULT_CPU_REG           ((uint8_t)0x01U)
#define FAULT_RAM               ((uint8_t)0x02U)
#define FAULT_ROM               ((uint8_t)0x03U)
#define FAULT_CLOCK             ((uint8_t)0x04U)
#define FAULT_VOLTAGE           ((uint8_t)0x05U)
#define FAULT_OVERCURRENT       ((uint8_t)0x06U)
#define FAULT_OVERSPEED         ((uint8_t)0x07U)
#define FAULT_LID_OPEN          ((uint8_t)0x08U)
#define FAULT_VIBRATION         ((uint8_t)0x09U)

/* Boolean type for safety flags */
#define FLAG_CLEAR              ((uint8_t)0U)
#define FLAG_SET                ((uint8_t)1U)

#endif /* PRODUCT_CONFIG_H */
