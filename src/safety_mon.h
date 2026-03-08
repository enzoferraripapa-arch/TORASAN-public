/**
 * @file   safety_mon.h
 * @brief  Safety Monitor Module - runtime safety condition checking
 * @doc    WMC-SUD-001 §4.3 Safety Monitor Specification
 *
 * Monitors safety-critical conditions each control cycle:
 *   - Overspeed detection with hysteresis
 *   - Overcurrent detection with hysteresis
 *   - Lid open detection
 *   - Voltage window monitoring
 *   - Vibration detection via current variation analysis
 *
 * Safety flags are CRC-16 protected against memory corruption.
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#ifndef SAFETY_MON_H
#define SAFETY_MON_H

#include "std_types.h"
#include "product_config.h"

/* ========================================================================
 * Vibration Detection Parameters
 * ======================================================================== */

/** Current variation buffer depth for vibration analysis */
#define SAFETY_VIBRATION_SAMPLES   ((uint8_t)10U)

/** Current variation threshold (mA peak-to-peak) for vibration fault */
#define SAFETY_VIBRATION_THRESH_MA ((uint16_t)2000U)

/* ========================================================================
 * CRC-16 Parameters (CRC-CCITT)
 * ======================================================================== */

#define CRC16_INIT                 ((uint16_t)0xFFFFU)
#define CRC16_POLY                 ((uint16_t)0x1021U)

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief  Initialize safety monitor module
 * @detail Clears all safety flags and computes initial CRC.
 */
void SafetyMon_Init(void);

/**
 * @brief  Execute all safety checks
 * @detail Called every CONTROL_LOOP_MS from scheduler.
 *         Reads processed sensor values and sets/clears safety flags.
 *         Updates CRC-16 after any flag change.
 */
void SafetyMon_Check(void);

/**
 * @brief  Get read-only pointer to safety flags
 * @return Const pointer to safety_flags_t structure
 * @note   Caller must verify CRC before using flag values.
 */
const safety_flags_t *SafetyMon_GetFlags(void);

/**
 * @brief  Verify CRC-16 integrity of safety flags
 * @return FLAG_CLEAR if CRC valid, FLAG_SET if CRC mismatch
 */
uint8_t SafetyMon_VerifyCrc(void);

#endif /* SAFETY_MON_H */
