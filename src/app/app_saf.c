/**
 * @file    app_saf.c
 * @brief   Safety monitoring application implementation
 * @module  SA-002 APP_SAF (UT-004 to UT-007)
 * @safety  IEC 60730 Class B
 * @req     SR-002, SR-005, SR-006, SR-007, SR-008, SR-013, SR-014, SR-015
 *
 * Central safety monitor: threshold judgment, state transition,
 * stop sequence coordination. All safety decisions flow through here.
 */

#include "app_saf.h"
#include "app_mot.h"
#include "../bsw/hal_adc.h"
#include "../bsw/hal_gpio.h"
#include "../bsw/hal_timer.h"
#include "../bsw/dem.h"
#include "../bsw/mem_mgr.h"
#include "../config/safety_config.h"

/* ============================================================
 * Module-static variables
 * ============================================================ */

/** Current system state */
static SystemState_t s_system_state = SYS_STATE_STARTUP_DIAG;

/** Overspeed debounce counter */
static uint8_t s_overspeed_count = 0U;

/** Overcurrent debounce counter */
static uint8_t s_overcurrent_count = 0U;

/** Lid open debounce counter */
static uint8_t s_lid_open_count = 0U;

/** Voltage anomaly debounce counter */
static uint8_t s_voltage_fault_count = 0U;

/** Filtered current value [mA] (integer LPF) */
static uint32_t s_filtered_current_ma = 0UL;

/** Filtered voltage value [mV] (integer LPF) */
static uint32_t s_filtered_voltage_mv = 0UL;

/** Voltage check divider (10Hz from 100Hz main cycle) */
static uint8_t s_voltage_divider = 0U;

/* ============================================================
 * Internal: Integer LPF
 * y_new = (alpha * x + (256 - alpha) * y_prev) / 256
 * ============================================================ */
static uint32_t AppSaf_Lpf(uint32_t prev, uint32_t input, uint8_t alpha)
{
    uint32_t result;

    result = ((uint32_t)alpha * input
              + (uint32_t)(256U - (uint32_t)alpha) * prev) / 256UL;

    return result;
}

/* ============================================================
 * Internal: Check overspeed (SR-002)
 * ============================================================ */
static void AppSaf_CheckOverspeed(void)
{
    uint16_t current_rpm;

    current_rpm = AppMot_GetRpm();

    if (current_rpm > OVERSPEED_THRESHOLD_RPM)
    {
        s_overspeed_count++;
        if (s_overspeed_count >= OVERSPEED_DEBOUNCE_COUNT)
        {
            AppSaf_TransitionSafe(FAULT_OVERSPEED);
            s_overspeed_count = 0U;
        }
    }
    else
    {
        s_overspeed_count = 0U;
    }
}

/* ============================================================
 * Internal: Check overcurrent (SR-005, SR-006)
 * ============================================================ */
static void AppSaf_CheckOvercurrent(void)
{
    uint16_t raw_current_ma;

    raw_current_ma = HalAdc_GetCurrent();

    /* Apply LPF */
    s_filtered_current_ma = AppSaf_Lpf(
        s_filtered_current_ma,
        (uint32_t)raw_current_ma,
        LPF_ALPHA_CURRENT
    );

    if (s_filtered_current_ma > (uint32_t)OVERCURRENT_THRESHOLD_MA)
    {
        s_overcurrent_count++;
        if (s_overcurrent_count >= OVERCURRENT_DEBOUNCE_COUNT)
        {
            AppSaf_TransitionSafe(FAULT_OVERCURRENT);
            s_overcurrent_count = 0U;
        }
    }
    else
    {
        s_overcurrent_count = 0U;
    }
}

/* ============================================================
 * Internal: Check lid sensor (SR-007, SR-008)
 * ============================================================ */
static void AppSaf_CheckLid(void)
{
    uint8_t lid_state;

    lid_state = HalGpio_GetLidState();

    if (lid_state == STD_TRUE)
    {
        /* Lid is open */
        s_lid_open_count++;
        if (s_lid_open_count >= LID_DEBOUNCE_COUNT)
        {
            AppSaf_TransitionSafe(FAULT_LID_OPEN);
            s_lid_open_count = 0U;
        }
    }
    else
    {
        s_lid_open_count = 0U;
    }
}

/* ============================================================
 * Internal: Check voltage window (SR-014, SR-015)
 * ============================================================ */
static void AppSaf_CheckVoltage(void)
{
    uint16_t raw_voltage_mv;

    raw_voltage_mv = HalAdc_GetVoltage();

    /* Apply LPF */
    s_filtered_voltage_mv = AppSaf_Lpf(
        s_filtered_voltage_mv,
        (uint32_t)raw_voltage_mv,
        LPF_ALPHA_VOLTAGE
    );

    /* Window comparison */
    if (s_filtered_voltage_mv < (uint32_t)VOLTAGE_MIN_MV)
    {
        s_voltage_fault_count++;
        if (s_voltage_fault_count >= VOLTAGE_DEBOUNCE_COUNT)
        {
            AppSaf_TransitionSafe(FAULT_VOLTAGE_LOW);
            s_voltage_fault_count = 0U;
        }
    }
    else if (s_filtered_voltage_mv > (uint32_t)VOLTAGE_MAX_MV)
    {
        s_voltage_fault_count++;
        if (s_voltage_fault_count >= VOLTAGE_DEBOUNCE_COUNT)
        {
            AppSaf_TransitionSafe(FAULT_VOLTAGE_HIGH);
            s_voltage_fault_count = 0U;
        }
    }
    else
    {
        s_voltage_fault_count = 0U;
    }
}

/* ============================================================
 * UT-004: AppSaf_Init
 * ============================================================ */
void AppSaf_Init(void)
{
    s_system_state       = SYS_STATE_STARTUP_DIAG;
    s_overspeed_count    = 0U;
    s_overcurrent_count  = 0U;
    s_lid_open_count     = 0U;
    s_voltage_fault_count = 0U;
    s_filtered_current_ma = 0UL;
    s_filtered_voltage_mv = (uint32_t)ADC_VREF_MV;  /* Initialize to nominal */
    s_voltage_divider    = 0U;
}

/* ============================================================
 * UT-005: AppSaf_Cyclic10ms
 * ============================================================ */
void AppSaf_Cyclic10ms(void)
{
    /* Only monitor in NORMAL state */
    if (s_system_state != SYS_STATE_NORMAL)
    {
        return;
    }

    /* 1. Overspeed check (SR-002) — every 10ms */
    AppSaf_CheckOverspeed();

    /* 2. Overcurrent check (SR-005) — every 10ms (100Hz) */
    AppSaf_CheckOvercurrent();

    /* 3. Lid sensor check (SR-007) — every 10ms */
    AppSaf_CheckLid();

    /* 4. Voltage check (SR-014) — every 100ms (10Hz) */
    s_voltage_divider++;
    if (s_voltage_divider >= (uint8_t)10U)
    {
        s_voltage_divider = 0U;
        AppSaf_CheckVoltage();
    }
}

/* ============================================================
 * UT-006: AppSaf_TransitionSafe (SR-013)
 * ============================================================ */
void AppSaf_TransitionSafe(FaultCode_t fault_code)
{
    LogEntry_t log_entry;

    /* 1. Emergency stop motor (SR-003, SR-006, SR-008) */
    AppMot_EmergencyStop();

    /* 2. Set safe state */
    s_system_state = SYS_STATE_SAFE;

    /* 3. Activate fault LED */
    HalGpio_SetLed(STD_TRUE);

    /* 4. Report to DEM */
    Dem_ReportError(fault_code);

    /* 5. Log to NVM */
    log_entry.fault_code   = fault_code;
    log_entry.timestamp_ms = HalTimer_GetTick();
    (void)MemMgr_WriteLog(&log_entry);
}

/* ============================================================
 * UT-007: AppSaf_GetState
 * ============================================================ */
SystemState_t AppSaf_GetState(void)
{
    return s_system_state;
}
