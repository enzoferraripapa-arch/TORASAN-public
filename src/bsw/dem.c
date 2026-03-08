/**
 * @file    dem.c
 * @brief   Diagnostic Event Manager implementation
 * @module  SA-005 DEM (UT-014)
 * @safety  IEC 60730 Class B
 * @req     SR-013
 */

#include "dem.h"
#include "../config/safety_config.h"

/* ============================================================
 * Module-static variables
 * ============================================================ */

/** Fault status bitmask (1 bit per FaultCode_t value) */
static uint16_t s_fault_mask = 0U;

/** Most recent fault code */
static FaultCode_t s_last_fault = FAULT_NONE;

/** Total fault count since startup */
static uint16_t s_fault_count = 0U;

/* ============================================================
 * Dem_Init
 * ============================================================ */
void Dem_Init(void)
{
    s_fault_mask  = 0U;
    s_last_fault  = FAULT_NONE;
    s_fault_count = 0U;
}

/* ============================================================
 * Dem_ReportError
 * ============================================================ */
void Dem_ReportError(FaultCode_t fault_code)
{
    if (fault_code != FAULT_NONE)
    {
        /* Set fault bit in aggregated mask */
        s_fault_mask |= (uint16_t)(1U << (uint8_t)fault_code);

        /* Update last fault */
        s_last_fault = fault_code;

        /* Increment fault counter (saturate at max) */
        if (s_fault_count < (uint16_t)0xFFFFU)
        {
            s_fault_count++;
        }
    }
}

/* ============================================================
 * Dem_ClearError
 * ============================================================ */
void Dem_ClearError(FaultCode_t fault_code)
{
    if (fault_code != FAULT_NONE)
    {
        s_fault_mask &= (uint16_t)(~(uint16_t)(1U << (uint8_t)fault_code));

        /* If cleared fault was last fault, reset to NONE */
        if (s_last_fault == fault_code)
        {
            s_last_fault = FAULT_NONE;
        }
    }
}

/* ============================================================
 * Dem_GetStatus
 * ============================================================ */
FaultCode_t Dem_GetStatus(void)
{
    return s_last_fault;
}

/* ============================================================
 * Dem_HasActiveFault
 * ============================================================ */
uint8_t Dem_HasActiveFault(void)
{
    uint8_t result;

    if (s_fault_mask != 0U)
    {
        result = STD_TRUE;
    }
    else
    {
        result = STD_FALSE;
    }

    return result;
}

/* ============================================================
 * Dem_GetFaultCount
 * ============================================================ */
uint16_t Dem_GetFaultCount(void)
{
    return s_fault_count;
}
