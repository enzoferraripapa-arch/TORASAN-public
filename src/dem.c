/**
 * @file   dem.c
 * @brief  Diagnostic Event Manager - implementation
 * @doc    WMC-SUD-001 §4.4 DEM Specification
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "dem.h"
#include "hal.h"

/* ========================================================================
 * [SAFETY VARIABLE REGION]
 * System mode is safety-critical - determines motor operation permission.
 * In linker script: place in .safety_data section.
 * ======================================================================== */

/** Current system operating mode */
static sys_mode_t s_mode;

/** Last reported fault code (for notification) */
static uint8_t s_last_fault;

/* ========================================================================
 * Internal Helper: Send error notification via UART
 * Frame format: [STX][LEN][FAULT_CODE][MODE][ETX]
 * ======================================================================== */

static void SendErrorNotification(uint8_t fault_code, sys_mode_t mode)
{
    uint8_t frame[DEM_UART_FRAME_LEN];

    frame[0] = DEM_UART_STX;
    frame[1] = DEM_UART_FRAME_LEN;
    frame[2] = fault_code;
    frame[3] = (uint8_t)mode;
    frame[4] = DEM_UART_ETX;

    HAL_UART_Send(frame, DEM_UART_FRAME_LEN);
}

/* ========================================================================
 * Internal Helper: Determine if fault is non-recoverable
 * Hardware diagnostic failures (CPU, RAM, ROM, Clock) are non-recoverable
 * and require system restart.
 * ======================================================================== */

static uint8_t IsNonRecoverableFault(uint8_t fault_code)
{
    uint8_t result = FLAG_CLEAR;

    switch (fault_code)
    {
        case FAULT_CPU_REG:
            /* Fall through */
        case FAULT_RAM:
            /* Fall through */
        case FAULT_ROM:
            /* Fall through */
        case FAULT_CLOCK:
            result = FLAG_SET;
            break;

        default:
            result = FLAG_CLEAR;
            break;
    }

    return result;
}

/* ========================================================================
 * Public Functions
 * ======================================================================== */

void DEM_Init(void)
{
    s_mode       = SYS_MODE_INIT;
    s_last_fault = FAULT_NONE;
}

void DEM_ReportFault(uint8_t fault_code)
{
    s_last_fault = fault_code;

    /* Determine target mode based on fault severity */
    if (fault_code == FAULT_NONE)
    {
        /* No fault - no action */
        return;
    }

    if (IsNonRecoverableFault(fault_code) == FLAG_SET)
    {
        /*
         * Non-recoverable fault: CPU/RAM/ROM/Clock failure
         * Transition to FAULT mode - system requires restart
         */
        s_mode = SYS_MODE_FAULT;
    }
    else
    {
        /*
         * Recoverable fault: overcurrent, overspeed, lid open, voltage, vibration
         * Transition to SAFE mode - motor stopped but system alive
         * Recovery possible after fault condition clears
         */
        if (s_mode != SYS_MODE_FAULT)
        {
            /* Never transition from FAULT to SAFE (FAULT is terminal) */
            s_mode = SYS_MODE_SAFE;
        }
    }

    /* Send error notification */
    SendErrorNotification(fault_code, s_mode);
}

sys_mode_t DEM_GetMode(void)
{
    return s_mode;
}

void DEM_SetStartup(void)
{
    /* Only transition from INIT to STARTUP */
    if (s_mode == SYS_MODE_INIT)
    {
        s_mode = SYS_MODE_STARTUP;
    }
}

void DEM_SetNormal(void)
{
    /* Only transition from STARTUP to NORMAL */
    if (s_mode == SYS_MODE_STARTUP)
    {
        s_mode = SYS_MODE_NORMAL;
    }
}

uint8_t DEM_IsMotorAllowed(void)
{
    uint8_t result;

    if (s_mode == SYS_MODE_NORMAL)
    {
        result = FLAG_SET;
    }
    else
    {
        result = FLAG_CLEAR;
    }

    return result;
}
