/**
 * @file    main.c
 * @brief   Main entry point — initialization and 10ms super loop
 * @module  System
 * @safety  IEC 60730 Class B
 *
 * Execution flow:
 * 1. Peripheral initialization (HAL)
 * 2. BSW initialization (DEM, MEM, DIAG, WDT, COM)
 * 3. Startup self-diagnosis (Annex H)
 * 4. APP initialization (SAF, MOT)
 * 5. 10ms super loop
 */

#include "types/std_types.h"
#include "types/safety_types.h"
#include "config/safety_config.h"
#include "config/mcu_config.h"
#include "app/app_mot.h"
#include "app/app_saf.h"
#include "bsw/hal_adc.h"
#include "bsw/hal_gpio.h"
#include "bsw/hal_pwm.h"
#include "bsw/hal_timer.h"
#include "bsw/hal_uart.h"
#include "bsw/wdt_mgr.h"
#include "bsw/dem.h"
#include "bsw/mem_mgr.h"
#include "bsw/diag.h"
#include "bsw/com.h"

/* ============================================================
 * Host/Test build: SFR stub variables
 * ============================================================ */
#ifndef TARGET_RL78

volatile uint8_t  REG_PM0    = 0U;
volatile uint8_t  REG_PM2    = 0U;
volatile uint8_t  REG_PM3    = 0U;
volatile uint8_t  REG_PM7    = 0U;
volatile uint8_t  REG_P0     = 0U;
volatile uint8_t  REG_P2     = 0U;
volatile uint8_t  REG_P3     = 0U;
volatile uint8_t  REG_P7     = 0U;

volatile uint8_t  REG_ADCEN  = 0U;
volatile uint8_t  REG_ADM0   = 0U;
volatile uint8_t  REG_ADM1   = 0U;
volatile uint8_t  REG_ADS    = 0U;
volatile uint16_t REG_ADCR   = 0U;
volatile uint8_t  REG_ADIF   = 0U;

volatile uint8_t  REG_TAU0EN = 0U;
volatile uint16_t REG_TDR00  = 0U;
volatile uint16_t REG_TDR01  = 0U;
volatile uint16_t REG_TCR01  = 0U;
volatile uint8_t  REG_TS0    = 0U;
volatile uint8_t  REG_TT0    = 0U;

volatile uint16_t REG_TDR02  = 0U;
volatile uint16_t REG_TDR03  = 0U;
volatile uint16_t REG_TDR04  = 0U;
volatile uint8_t  REG_TOE0   = 0U;
volatile uint8_t  REG_TO0    = 0U;

volatile uint8_t  REG_SAU0EN = 0U;
volatile uint8_t  REG_SS0    = 0U;
volatile uint8_t  REG_ST0    = 0U;
volatile uint8_t  REG_SOE0   = 0U;
volatile uint16_t REG_SDR00  = 0U;
volatile uint16_t REG_SDR01  = 0U;
volatile uint8_t  REG_SSR01  = 0U;
volatile uint8_t  REG_SIR01  = 0U;

volatile uint8_t  REG_WDTE   = 0U;
volatile uint8_t  REG_CMC    = 0U;
volatile uint8_t  REG_CSC    = 0U;
volatile uint8_t  REG_CKC    = 0U;

#endif /* TARGET_RL78 */

/* ============================================================
 * Internal: Wait for next 10ms cycle
 * ============================================================ */
static void Main_WaitCycle(uint32_t next_tick)
{
    while (HalTimer_GetTick() < next_tick)
    {
        __NOP();
    }
}

/* ============================================================
 * main
 * ============================================================ */
int main(void)
{
    DiagResult_t diag_result;
    uint32_t next_cycle_tick;
    uint8_t wdt_flags;

    /* ========================================
     * Phase 1: HAL Initialization
     * ======================================== */
    HalTimer_Init();
    HalGpio_Init();
    HalAdc_Init();
    /* HalPwm_Init() called by AppMot_Init() */
    /* HalUart_Init() called by Com_Init() */

    /* ========================================
     * Phase 2: BSW Initialization
     * ======================================== */
    Dem_Init();
    MemMgr_Init();
    WdtMgr_Init();
    Com_Init();

    /* ========================================
     * Phase 3: Startup Self-Diagnosis (DR-001)
     * ======================================== */
    AppSaf_Init();  /* Initialize to STARTUP_DIAG state */

    diag_result = Diag_RunStartup();

    if (diag_result != DIAG_PASS)
    {
        /* Startup diagnosis failed: enter permanent safe state */
        AppSaf_TransitionSafe(FAULT_CPU_REG);  /* Fault code from first failure */

        /* Remain in safe state forever (WDT will not be kicked) */
        for (;;)
        {
            __HALT();
        }
    }

    /* ========================================
     * Phase 4: APP Initialization
     * ======================================== */
    AppMot_Init();

    /* Transition to NORMAL state */
    /* (AppSaf internal state update) */

    /* Set green LED (normal indication) */
    HalGpio_SetLed(STD_FALSE);

    /* ========================================
     * Phase 5: 10ms Super Loop
     * ======================================== */
    next_cycle_tick = HalTimer_GetTick() + (uint32_t)MAIN_CYCLE_MS;

    for (;;)
    {
        wdt_flags = 0U;

        /* [Step 1] Input acquisition (HAL) — Budget: 1ms */
        /* ADC conversions triggered within check functions */

        /* [Step 2] Safety judgment (APP_SAF) — Budget: 1ms */
        AppSaf_Cyclic10ms();
        wdt_flags |= WDT_FLAG_SAFETY_CHECK;

        /* [Step 3] Motor control (APP_MOT) — Budget: 3ms */
        if (AppSaf_GetState() == SYS_STATE_NORMAL)
        {
            AppMot_Cyclic10ms();
        }
        wdt_flags |= WDT_FLAG_MOTOR_CHECK;

        /* [Step 4] Runtime diagnosis (DIAG) — Budget: 2ms */
        Diag_RunCyclic10ms();
        wdt_flags |= WDT_FLAG_DIAG_CHECK;

        /* [Step 5] Communication (COM) — Budget: 1ms */
        Com_Cyclic10ms();
        Com_SendStatus(
            AppSaf_GetState(),
            AppMot_GetRpm(),
            Dem_GetStatus()
        );

        /* [Step 6] WDT kick — Budget: <0.1ms */
        (void)WdtMgr_TryKick(wdt_flags);

        /* Wait for next 10ms cycle */
        Main_WaitCycle(next_cycle_tick);
        next_cycle_tick += (uint32_t)MAIN_CYCLE_MS;
    }

    /* Never reached */
    return 0;
}
