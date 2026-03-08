/**
 * @file   test_integration.c
 * @brief  PH-11B Integration Tests - SA component interface testing
 * @doc    WMC-TIR-001 Integration Test Specification
 *
 * Test cases TC-101 through TC-108: inter-module data flow verification.
 */

#include "unity.h"
#include "test_hal_stub.h"
#include "sensor_proc.h"
#include "safety_mon.h"
#include "failsafe.h"
#include "motor_ctrl.h"
#include "mem_test.h"
#include "wdt_drv.h"
#include "dem.h"
#include "diag_mgr.h"
#include "scheduler.h"



/* TC-101: SensorProc -> SafetyMon data flow */
static void test_TC_101_SensorProc_SafetyMon_DataFlow(void)
{
    const safety_flags_t *flags;
    uint8_t i;
    TEST_MESSAGE("TC-101: SensorProc -> SafetyMon data flow");
    SensorProc_Init();
    SafetyMon_Init();

    /* Set current above overcurrent threshold */
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 819U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);

    for (i = 0U; i < 40U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, flags->overcurrent);

    /* Verify sensor values propagated correctly */
    TEST_ASSERT_TRUE(SensorProc_GetCurrentMa() > 7000U);
    TEST_ASSERT_TRUE(SensorProc_GetVoltageMv() > 4000U);
}

/* TC-102: SafetyMon -> DEM fault reporting */
static void test_TC_102_SafetyMon_DEM_FaultReporting(void)
{
    const safety_flags_t *flags;
    uint8_t i;
    TEST_MESSAGE("TC-102: SafetyMon -> DEM fault reporting flow");
    SensorProc_Init();
    SafetyMon_Init();
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();

    /* Trigger overcurrent via sensor */
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 850U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 40U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, flags->overcurrent);

    /* Report fault to DEM */
    DEM_ReportFault(FAULT_OVERCURRENT);
    TEST_ASSERT_EQUAL(SYS_MODE_SAFE, DEM_GetMode());
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, DEM_IsMotorAllowed());
}

/* TC-103: DEM -> FailSafe safety action trigger */
static void test_TC_103_DEM_FailSafe_SafetyAction(void)
{
    TEST_MESSAGE("TC-103: DEM -> FailSafe safety action trigger");
    HAL_Init();
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();

    HAL_PWM_Start();
    HAL_PWM_SetDuty(HAL_PWM_PHASE_U, 500U);
    HAL_GPIO_Write(HAL_PORT_GATE_EN, HAL_PIN_GATE_EN, HAL_GPIO_HIGH);

    /* DEM reports fault -> FailSafe executes */
    FailSafe_Execute(FAULT_OVERSPEED);

    TEST_ASSERT_EQUAL_UINT8(0U, TestStub_GetPwmRunning());
    TEST_ASSERT_EQUAL_UINT8(HAL_GPIO_LOW,
        TestStub_GetGpioOutput(HAL_PORT_GATE_EN, HAL_PIN_GATE_EN));
    TEST_ASSERT_EQUAL(SYS_MODE_SAFE, DEM_GetMode());
}

/* TC-104: SensorProc -> MotorCtrl control data flow */
static void test_TC_104_SensorProc_MotorCtrl_DataFlow(void)
{
    TEST_MESSAGE("TC-104: SensorProc -> MotorCtrl control data flow");
    SensorProc_Init();
    MotorCtrl_Init();
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();

    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);

    MotorCtrl_SetTargetRpm(600U);

    /* SensorProc provides RPM=0 (motor starting) */
    SensorProc_Update();
    MotorCtrl_Update();

    /* Motor should be running with non-zero duty */
    TEST_ASSERT_TRUE(TestStub_GetPwmDuty(HAL_PWM_PHASE_U) > 0U);
    TEST_ASSERT_EQUAL_UINT8(1U, TestStub_GetPwmRunning());
}

/* TC-105: DiagMgr -> MEM diagnostic sequencing */
static void test_TC_105_DiagMgr_MEM_Sequencing(void)
{
    Std_ReturnType result;
    const diag_status_t *diag;
    TEST_MESSAGE("TC-105: DiagMgr -> MEM diagnostic sequencing");
    DEM_Init();

    result = DiagMgr_StartupTest();
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);

    diag = MEM_GetStatus();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->cpu_ok);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->ram_ok);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->rom_ok);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->clk_ok);

    TEST_ASSERT_EQUAL(SYS_MODE_STARTUP, DEM_GetMode());
}

/* TC-106: WDT_Drv diagnostic gating with DiagMgr */
static void test_TC_106_WDT_DiagMgr_Gating(void)
{
    uint8_t all_ok;
    TEST_MESSAGE("TC-106: WDT_Drv diagnostic gating with DiagMgr");
    SensorProc_Init();
    SafetyMon_Init();
    DEM_Init();

    /* Before startup: WDT should NOT kick */
    all_ok = WDT_IsAllDiagOk();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, all_ok);

    /* After successful startup: WDT should kick */
    DiagMgr_StartupTest();
    all_ok = WDT_IsAllDiagOk();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, all_ok);

    WDT_Init();
    WDT_Kick();
}

/* TC-107: Safety/control path independence */
static void test_TC_107_SafetyControlIndependence(void)
{
    uint8_t i;
    const safety_flags_t *flags;
    TEST_MESSAGE("TC-107: Safety/control path independence");
    SensorProc_Init();
    SafetyMon_Init();
    MotorCtrl_Init();
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();

    /* Normal operation: motor running, no safety faults */
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    MotorCtrl_SetTargetRpm(600U);

    for (i = 0U; i < 10U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    MotorCtrl_Update();

    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, flags->overcurrent);
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, flags->overspeed);
    TEST_ASSERT_TRUE(TestStub_GetPwmRunning() != 0U);

    /* Safety monitor independently detects overcurrent */
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 819U);
    for (i = 0U; i < 40U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, flags->overcurrent);

    /* FailSafe stops motor regardless of control path */
    FailSafe_Execute(FAULT_OVERCURRENT);
    TEST_ASSERT_EQUAL_UINT8(0U, TestStub_GetPwmRunning());
    TEST_ASSERT_EQUAL(SYS_MODE_SAFE, DEM_GetMode());
}

/* TC-108: Shared memory access isolation */
static void test_TC_108_SharedMemoryIsolation(void)
{
    const safety_flags_t *flags;
    const diag_status_t *diag;
    uint8_t i;
    TEST_MESSAGE("TC-108: Shared memory access isolation");
    SensorProc_Init();
    SafetyMon_Init();
    DEM_Init();

    /* Verify sensor data is read-only from safety monitor perspective */
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 20U; i++) { SensorProc_Update(); }
    SafetyMon_Check();

    /* Safety flags accessible via const pointer */
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_NOT_NULL(flags);
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, SafetyMon_VerifyCrc());

    /* Diagnostic status accessible via const pointer */
    MEM_StartupTest();
    diag = MEM_GetStatus();
    TEST_ASSERT_NOT_NULL(diag);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->cpu_ok);

    /* Sensor values accessible via getter functions */
    TEST_ASSERT_TRUE(SensorProc_GetCurrentMa() > 0U);
    TEST_ASSERT_TRUE(SensorProc_GetVoltageMv() > 0U);
}

uint32_t run_integration_tests(void)
{
    printf("\n==========================================\n");
    printf("PH-11B: Integration Verification (SWE.5)\n");
    printf("==========================================\n\n");
    UNITY_BEGIN();
    RUN_TEST(test_TC_101_SensorProc_SafetyMon_DataFlow);
    RUN_TEST(test_TC_102_SafetyMon_DEM_FaultReporting);
    RUN_TEST(test_TC_103_DEM_FailSafe_SafetyAction);
    RUN_TEST(test_TC_104_SensorProc_MotorCtrl_DataFlow);
    RUN_TEST(test_TC_105_DiagMgr_MEM_Sequencing);
    RUN_TEST(test_TC_106_WDT_DiagMgr_Gating);
    RUN_TEST(test_TC_107_SafetyControlIndependence);
    RUN_TEST(test_TC_108_SharedMemoryIsolation);
    printf("\n--- Integration Test Results ---\n");
    printf("Total: %u  Passed: %u  Failed: %u\n\n",
           (unsigned)Unity.total, (unsigned)Unity.passed, (unsigned)Unity.failed);
    return UNITY_END();
}
