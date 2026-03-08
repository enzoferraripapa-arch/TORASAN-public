/**
 * @file    safety_types.h
 * @brief   Safety-related type definitions
 * @module  Common
 * @safety  IEC 60730 Class B
 *
 * Defines SafetyVar_t (inverse copy protection), fault codes,
 * system states, and diagnostic result types.
 */

#ifndef SAFETY_TYPES_H
#define SAFETY_TYPES_H

#include <stdint.h>
#include "std_types.h"

/* ============================================================
 * SafetyVar_t: Inverse copy protection for safety variables
 * ============================================================ */

typedef struct {
    uint16_t value;     /**< Actual value */
    uint16_t inverted;  /**< Bit-inverted value (~value) */
} SafetyVar_t;

/** @brief Set safety variable with inverse copy */
#define SAFETYVAR_SET(sv, val) \
    do { \
        (sv).value    = (uint16_t)(val); \
        (sv).inverted = (uint16_t)(~(uint16_t)(val)); \
    } while (0)

/** @brief Get value from safety variable */
#define SAFETYVAR_GET(sv)   ((sv).value)

/** @brief Check integrity of safety variable (STD_TRUE=OK) */
#define SAFETYVAR_CHECK(sv) \
    (((sv).value == (uint16_t)(~(sv).inverted)) ? STD_TRUE : STD_FALSE)

/* ============================================================
 * System State (T-05: all enum values explicitly assigned)
 * ============================================================ */

typedef enum {
    SYS_STATE_STARTUP_DIAG = 0U,  /**< Startup self-diagnosis */
    SYS_STATE_NORMAL       = 1U,  /**< Normal operation */
    SYS_STATE_SAFE         = 2U   /**< Safe state (motor stopped) */
} SystemState_t;

/* ============================================================
 * Fault Code (T-05: explicit values)
 * ============================================================ */

typedef enum {
    FAULT_NONE           = 0x00U,  /**< No fault */
    FAULT_CPU_REG        = 0x01U,  /**< CPU register test fail */
    FAULT_RAM            = 0x02U,  /**< RAM March C test fail */
    FAULT_ROM            = 0x03U,  /**< ROM CRC mismatch */
    FAULT_CLOCK          = 0x04U,  /**< Clock frequency out of range */
    FAULT_OVERSPEED      = 0x05U,  /**< Motor overspeed detected */
    FAULT_OVERCURRENT    = 0x06U,  /**< Overcurrent detected */
    FAULT_VOLTAGE_LOW    = 0x07U,  /**< Supply voltage too low */
    FAULT_VOLTAGE_HIGH   = 0x08U,  /**< Supply voltage too high */
    FAULT_LID_OPEN       = 0x09U,  /**< Lid open during operation */
    FAULT_WDT_TIMEOUT    = 0x0AU,  /**< Watchdog timeout */
    FAULT_ADC            = 0x0BU,  /**< ADC self-test fail */
    FAULT_GPIO           = 0x0CU   /**< GPIO self-test fail */
} FaultCode_t;

/* ============================================================
 * Diagnostic Result (T-05: explicit values)
 * ============================================================ */

typedef enum {
    DIAG_PASS    = 0U,  /**< Diagnosis passed */
    DIAG_FAIL    = 1U,  /**< Diagnosis failed */
    DIAG_RUNNING = 2U   /**< Diagnosis in progress */
} DiagResult_t;

/* ============================================================
 * Safety Status for threshold comparisons
 * ============================================================ */

typedef enum {
    SAFETY_NORMAL   = 0U,  /**< Within limits */
    SAFETY_DETECTED = 1U   /**< Anomaly detected */
} SafetyStatus_t;

/* ============================================================
 * Error Log Entry
 * ============================================================ */

typedef struct {
    FaultCode_t fault_code;   /**< Fault code */
    uint32_t    timestamp_ms; /**< Timestamp in ms since startup */
} LogEntry_t;

/* ============================================================
 * Critical Section Macros (RL78/G14 DI/EI)
 * ============================================================ */

#ifdef TARGET_RL78
  #define ENTER_CRITICAL()  do { __DI()
  #define EXIT_CRITICAL()   __EI(); } while (0)
#else
  /* Host/test stub: no-op */
  #define ENTER_CRITICAL()  do {
  #define EXIT_CRITICAL()   } while (0)
#endif

#endif /* SAFETY_TYPES_H */
