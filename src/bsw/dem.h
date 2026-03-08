/**
 * @file    dem.h
 * @brief   Diagnostic Event Manager
 * @module  SA-005 DEM
 * @safety  IEC 60730 Class B
 * @req     SR-013
 */

#ifndef DEM_H
#define DEM_H

#include <stdint.h>
#include "../types/std_types.h"
#include "../types/safety_types.h"

/**
 * @brief   Initialize DEM (clear all error flags)
 * @module  SA-005 DEM
 */
void Dem_Init(void);

/**
 * @brief   Report an error event
 * @module  SA-005 DEM
 * @req     SR-013
 * @safety  IEC 60730 Class B
 * @param   fault_code  Fault code to report
 */
void Dem_ReportError(FaultCode_t fault_code);

/**
 * @brief   Clear a specific error flag (power cycle only)
 * @module  SA-005 DEM
 * @param   fault_code  Fault code to clear
 */
void Dem_ClearError(FaultCode_t fault_code);

/**
 * @brief   Get current aggregated fault status
 * @module  SA-005 DEM
 * @return  Most recent fault code (FAULT_NONE if no faults)
 */
FaultCode_t Dem_GetStatus(void);

/**
 * @brief   Check if any fault is active
 * @module  SA-005 DEM
 * @return  STD_TRUE if any fault active, STD_FALSE if no faults
 */
uint8_t Dem_HasActiveFault(void);

/**
 * @brief   Get fault count since startup
 * @module  SA-005 DEM
 * @return  Number of faults reported
 */
uint16_t Dem_GetFaultCount(void);

#endif /* DEM_H */
