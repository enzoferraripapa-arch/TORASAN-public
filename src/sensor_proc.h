/**
 * @file   sensor_proc.h
 * @brief  Sensor Processing Module - ADC filtering, GPIO debounce, RPM calculation
 * @doc    WMC-SUD-001 §4.2 Sensor Processing Specification
 *
 * Reads and processes all sensor inputs:
 *   - Motor current via ADC (100Hz, IIR filtered)
 *   - Supply voltage via ADC (10Hz)
 *   - Lid state via GPIO (6-count debounce)
 *   - Motor RPM from hall sensor capture
 *   - ADC stuck-at diagnostic (DR-003)
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#ifndef SENSOR_PROC_H
#define SENSOR_PROC_H

#include "std_types.h"
#include "product_config.h"

/* ========================================================================
 * ADC Diagnostic Thresholds (DR-003: stuck-at detection)
 * ======================================================================== */

/** Number of consecutive stuck readings before fault */
#define SENSOR_ADC_STUCK_LIMIT  ((uint8_t)5U)

/** ADC stuck-at-low threshold */
#define SENSOR_ADC_STUCK_LOW    ((uint16_t)0U)

/** ADC stuck-at-high threshold */
#define SENSOR_ADC_STUCK_HIGH   ADC_MAX_VALUE

/* ========================================================================
 * IIR Filter Coefficient (fixed-point: alpha = 1/4, shift by 2)
 * New = Old + (Raw - Old) / 4
 * ======================================================================== */

#define SENSOR_IIR_SHIFT        ((uint8_t)2U)

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief  Initialize sensor processing module
 * @detail Clears all internal state, filter buffers, and debounce counters.
 *         Must be called once at startup.
 */
void SensorProc_Init(void);

/**
 * @brief  Update all sensor readings
 * @detail Called every CONTROL_LOOP_MS (10ms) from scheduler.
 *         - Current ADC: read every call (100Hz)
 *         - Voltage ADC: read every 10th call (10Hz)
 *         - Lid GPIO: debounced every call
 *         - RPM: calculated from hall sensor period
 */
void SensorProc_Update(void);

/**
 * @brief  Get filtered motor RPM
 * @return Motor speed in RPM (0 to 65535)
 */
uint16_t SensorProc_GetRpm(void);

/**
 * @brief  Get filtered motor current
 * @return Motor current in milliamps (0 to 65535)
 */
uint16_t SensorProc_GetCurrentMa(void);

/**
 * @brief  Get supply voltage
 * @return Supply voltage in millivolts (0 to 65535)
 */
uint16_t SensorProc_GetVoltageMv(void);

/**
 * @brief  Get debounced lid state
 * @return LID_CLOSED or LID_OPEN
 */
lid_state_t SensorProc_GetLidState(void);

/**
 * @brief  Get ADC diagnostic status
 * @return FLAG_SET if ADC stuck-at fault detected, FLAG_CLEAR otherwise
 */
uint8_t SensorProc_GetAdcDiagFault(void);
/** * @brief  Get unfiltered (raw) motor current * @return Motor current in milliamps before IIR filter (0 to 65535) * @note   Used by SafetyMon for vibration detection to avoid IIR dampening. */uint16_t SensorProc_GetRawCurrentMa(void);

#endif /* SENSOR_PROC_H */
