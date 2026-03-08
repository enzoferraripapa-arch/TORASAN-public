/**
 * @file   test_unit.c
 * @brief  PH-11A Unit Tests - White-box testing of individual modules
 * @doc    WMC-TVR-001 Unit Test Specification
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



static void test_TC_001_SensorProc_RpmCalculation(void)
{
    TEST_MESSAGE("TC-001: SensorProc RPM calculation");
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 0U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 512U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    SensorProc_Update();
    TEST_ASSERT_EQUAL_UINT16(0U, SensorProc_GetRpm());
}

static void test_TC_001b_SensorProc_AdcStuckDiag(void)
{
    uint8_t fault, i;
    TEST_MESSAGE("TC-001b: SensorProc ADC stuck-at diagnostic");
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 512U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 10U; i++) { SensorProc_Update(); }
    fault = SensorProc_GetAdcDiagFault();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, fault);

    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 0U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 512U);
    for (i = 0U; i < (SENSOR_ADC_STUCK_LIMIT + 1U); i++) { SensorProc_Update(); }
    fault = SensorProc_GetAdcDiagFault();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, fault);

    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, ADC_MAX_VALUE);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 512U);
    for (i = 0U; i < (SENSOR_ADC_STUCK_LIMIT + 1U); i++) { SensorProc_Update(); }
    fault = SensorProc_GetAdcDiagFault();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, fault);
}

static void test_TC_002_SensorProc_CurrentConversion(void)
{
    uint16_t current_ma;
    uint8_t i;
    TEST_MESSAGE("TC-002: SensorProc current conversion & IIR filter");
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 512U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 40U; i++) { SensorProc_Update(); }
    current_ma = SensorProc_GetCurrentMa();
    TEST_ASSERT_UINT16_WITHIN(100U, 5004U, current_ma);

    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 0U);
    for (i = 0U; i < 60U; i++) { SensorProc_Update(); }
    current_ma = SensorProc_GetCurrentMa();
    TEST_ASSERT_UINT16_WITHIN(200U, 0U, current_ma);

    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 1023U);
    for (i = 0U; i < 60U; i++) { SensorProc_Update(); }
    current_ma = SensorProc_GetCurrentMa();
    TEST_ASSERT_UINT16_WITHIN(200U, 10000U, current_ma);
}

static void test_TC_003_SensorProc_VoltageAndLidDebounce(void)
{
    uint16_t voltage_mv;
    uint8_t i;
    TEST_MESSAGE("TC-003: SensorProc voltage & lid debounce");
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 0U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 512U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 20U; i++) { SensorProc_Update(); }
    voltage_mv = SensorProc_GetVoltageMv();
    TEST_ASSERT_UINT16_WITHIN(50U, 2502U, voltage_mv);

    TEST_ASSERT_EQUAL_UINT8((uint8_t)LID_CLOSED, (uint8_t)SensorProc_GetLidState());
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_HIGH);
    for (i = 0U; i < 5U; i++) { SensorProc_Update(); }
    TEST_ASSERT_EQUAL_UINT8((uint8_t)LID_CLOSED, (uint8_t)SensorProc_GetLidState());
    SensorProc_Update();
    TEST_ASSERT_EQUAL_UINT8((uint8_t)LID_OPEN, (uint8_t)SensorProc_GetLidState());

    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 6U; i++) { SensorProc_Update(); }
    TEST_ASSERT_EQUAL_UINT8((uint8_t)LID_CLOSED, (uint8_t)SensorProc_GetLidState());
}

static void test_TC_004_SafetyMon_OverspeedDetection(void)
{
    const safety_flags_t *flags;
    uint8_t i;
    TEST_MESSAGE("TC-004: SafetyMon overspeed detection");
    SafetyMon_Init();
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 614U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 921U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 20U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, flags->overspeed);
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, SafetyMon_VerifyCrc());
}

static void test_TC_005_SafetyMon_OvercurrentDetection(void)
{
    const safety_flags_t *flags;
    uint8_t i;
    TEST_MESSAGE("TC-005: SafetyMon overcurrent detection");
    SafetyMon_Init();
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 614U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 921U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 40U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, flags->overcurrent);

    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 819U);
    for (i = 0U; i < 40U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, flags->overcurrent);

    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 770U);
    for (i = 0U; i < 60U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, flags->overcurrent);
}

static void test_TC_006_SafetyMon_VoltageAndCrc(void)
{
    const safety_flags_t *flags;
    uint8_t i;
    TEST_MESSAGE("TC-006: SafetyMon voltage & CRC");
    SafetyMon_Init();
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 0U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 20U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, flags->voltage_error);
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, SafetyMon_VerifyCrc());

    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 800U);
    for (i = 0U; i < 20U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, flags->voltage_error);
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, SafetyMon_VerifyCrc());
}

static void test_TC_006b_SafetyMon_VibrationDetection(void)
{
    const safety_flags_t *flags;
    uint8_t i;
    TEST_MESSAGE("TC-006b: SafetyMon vibration detection");
    SafetyMon_Init();
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    for (i = 0U; i < 40U; i++) { SensorProc_Update(); SafetyMon_Check(); }
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, flags->vibration);

    SafetyMon_Init();
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 20U; i++)
    {
        if ((i % 2U) == 0U) { TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 200U); }
        else { TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 900U); }
        SensorProc_Update();
        SafetyMon_Check();
    }
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, flags->vibration);
}

static void test_TC_007_FailSafe_MotorStop(void)
{
    TEST_MESSAGE("TC-007: FailSafe motor stop");
    HAL_Init();
    HAL_PWM_Start();
    HAL_PWM_SetDuty(HAL_PWM_PHASE_U, 500U);
    TEST_ASSERT_EQUAL_UINT8(1U, TestStub_GetPwmRunning());
    TEST_ASSERT_EQUAL_UINT16(500U, TestStub_GetPwmDuty(HAL_PWM_PHASE_U));
    FailSafe_StopMotor();
    TEST_ASSERT_EQUAL_UINT8(0U, TestStub_GetPwmRunning());
    TEST_ASSERT_EQUAL_UINT16(0U, TestStub_GetPwmDuty(HAL_PWM_PHASE_U));
}

static void test_TC_008_FailSafe_GateShutdown(void)
{
    uint8_t gate_state;
    TEST_MESSAGE("TC-008: FailSafe gate shutdown");
    HAL_Init();
    DEM_Init();
    HAL_PWM_Start();
    HAL_PWM_SetDuty(HAL_PWM_PHASE_U, 500U);
    HAL_GPIO_Write(HAL_PORT_GATE_EN, HAL_PIN_GATE_EN, HAL_GPIO_HIGH);
    FailSafe_Execute(FAULT_OVERCURRENT);
    TEST_ASSERT_EQUAL_UINT8(0U, TestStub_GetPwmRunning());
    gate_state = TestStub_GetGpioOutput(HAL_PORT_GATE_EN, HAL_PIN_GATE_EN);
    TEST_ASSERT_EQUAL_UINT8(HAL_GPIO_LOW, gate_state);
    TEST_ASSERT_EQUAL(SYS_MODE_SAFE, DEM_GetMode());
}

static void test_TC_009_MotorCtrl_PwmUpdate(void)
{
    TEST_MESSAGE("TC-009: MotorCtrl PWM update");
    MotorCtrl_Init();
    DEM_Init();
    SensorProc_Init();
    DEM_SetStartup();
    DEM_SetNormal();
    MotorCtrl_SetTargetRpm(600U);
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 0U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 921U);
    MotorCtrl_Update();
    TEST_ASSERT_TRUE(TestStub_GetPwmDuty(HAL_PWM_PHASE_U) > 0U);
    MotorCtrl_SetTargetRpm(0U);
    MotorCtrl_Update();
    TEST_ASSERT_EQUAL_UINT16(0U, TestStub_GetPwmDuty(HAL_PWM_PHASE_U));
}

static void test_TC_010_MotorCtrl_Deceleration(void)
{
    TEST_MESSAGE("TC-010: MotorCtrl deceleration");
    MotorCtrl_Init();
    SensorProc_Init();
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();
    MotorCtrl_Decelerate(500U, 1000U);
    MotorCtrl_SetTargetRpm(800U);
    MotorCtrl_Update();
    TEST_ASSERT_TRUE(TestStub_GetPwmRunning() != 0U);
    DEM_ReportFault(FAULT_OVERCURRENT);
    MotorCtrl_Update();
    TEST_ASSERT_EQUAL_UINT8(0U, TestStub_GetPwmRunning());
}

static void test_TC_011_MEM_CpuRegTest(void)
{
    Std_ReturnType result;
    const diag_status_t *diag;
    TEST_MESSAGE("TC-011: MEM CPU register test");
    result = MEM_CpuRegTest();
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
    diag = MEM_GetStatus();
    TEST_ASSERT_NOT_NULL(diag);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->cpu_ok);
}

static void test_TC_012_MEM_RamMarchC(void)
{
    Std_ReturnType result;
    TEST_MESSAGE("TC-012: MEM RAM March C test");
    result = MEM_RamBlockTest(0U);
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
    result = MEM_RamBlockTest(RAM_TEST_BLOCK_COUNT - 1U);
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
    result = MEM_RamBlockTest(RAM_TEST_BLOCK_COUNT);
    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, result);
}

static void test_TC_012b_MEM_FullStartupTest(void)
{
    Std_ReturnType result;
    const diag_status_t *diag;
    TEST_MESSAGE("TC-012b: MEM full startup test");
    result = MEM_StartupTest();
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
    diag = MEM_GetStatus();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->cpu_ok);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->ram_ok);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->rom_ok);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->clk_ok);
}

static void test_TC_012c_MEM_RuntimeDiag(void)
{
    Std_ReturnType result;
    const diag_status_t *diag;
    uint8_t i;
    TEST_MESSAGE("TC-012c: MEM runtime diagnostic");
    MEM_StartupTest();
    for (i = 0U; i < 5U; i++) { result = MEM_RunDiag(); TEST_ASSERT_EQUAL_UINT8(E_OK, result); }
    diag = MEM_GetStatus();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->ram_ok);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->rom_ok);
}

static void test_TC_013_MEM_RomCrc32(void)
{
    Std_ReturnType result;
    TEST_MESSAGE("TC-013: MEM ROM CRC-32");
    result = MEM_RomBlockCrc(0U);
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
    result = MEM_RomBlockCrc(ROM_CRC_BLOCK_COUNT);
    TEST_ASSERT_EQUAL_UINT8(E_NOT_OK, result);
}

static void test_TC_014_MEM_ClockCheck(void)
{
    Std_ReturnType result;
    const diag_status_t *diag;
    TEST_MESSAGE("TC-014: MEM clock check");
    result = MEM_ClockCheck();
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
    diag = MEM_GetStatus();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, diag->clk_ok);
}

static void test_TC_015_WDT_ConditionalKick(void)
{
    uint8_t all_ok;
    Std_ReturnType res;
    TEST_MESSAGE("TC-015: WDT conditional kick");
    SensorProc_Init();
    SafetyMon_Init();
    res = MEM_StartupTest();
    TEST_ASSERT_EQUAL_UINT8(E_OK, res);
    all_ok = WDT_IsAllDiagOk();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, all_ok);
    WDT_Init();
    WDT_Kick();
}

static void test_TC_015b_WDT_DiagFailure(void)
{
    uint8_t all_ok, i;
    TEST_MESSAGE("TC-015b: WDT diagnostic failure");
    SensorProc_Init();
    SafetyMon_Init();
    all_ok = WDT_IsAllDiagOk();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, all_ok);
    MEM_StartupTest();
    all_ok = WDT_IsAllDiagOk();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, all_ok);
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 0U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 512U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 10U; i++) { SensorProc_Update(); }
    all_ok = WDT_IsAllDiagOk();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, all_ok);
}

static void test_TC_016_DEM_ModeTransitions(void)
{
    TEST_MESSAGE("TC-016: DEM mode transitions");
    DEM_Init();
    TEST_ASSERT_EQUAL(SYS_MODE_INIT, DEM_GetMode());
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, DEM_IsMotorAllowed());
    DEM_SetStartup();
    TEST_ASSERT_EQUAL(SYS_MODE_STARTUP, DEM_GetMode());
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, DEM_IsMotorAllowed());
    DEM_SetNormal();
    TEST_ASSERT_EQUAL(SYS_MODE_NORMAL, DEM_GetMode());
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, DEM_IsMotorAllowed());
    DEM_SetStartup();
    TEST_ASSERT_EQUAL(SYS_MODE_NORMAL, DEM_GetMode());
    DEM_SetNormal();
    TEST_ASSERT_EQUAL(SYS_MODE_NORMAL, DEM_GetMode());
    DEM_ReportFault(FAULT_OVERCURRENT);
    TEST_ASSERT_EQUAL(SYS_MODE_SAFE, DEM_GetMode());
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, DEM_IsMotorAllowed());
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();
    DEM_ReportFault(FAULT_CPU_REG);
    TEST_ASSERT_EQUAL(SYS_MODE_FAULT, DEM_GetMode());
    DEM_ReportFault(FAULT_OVERCURRENT);
    TEST_ASSERT_EQUAL(SYS_MODE_FAULT, DEM_GetMode());
    DEM_Init();
    DEM_ReportFault(FAULT_NONE);
    TEST_ASSERT_EQUAL(SYS_MODE_INIT, DEM_GetMode());
}

static void test_TC_017_DiagMgr_StartupSequence(void)
{
    Std_ReturnType result;
    TEST_MESSAGE("TC-017: DiagMgr startup sequence");
    DEM_Init();
    result = DiagMgr_StartupTest();
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, DiagMgr_AllOK());
    TEST_ASSERT_EQUAL(SYS_MODE_STARTUP, DEM_GetMode());
}

static void test_TC_018_Scheduler_SingleCycle(void)
{
    Std_ReturnType res;
    TEST_MESSAGE("TC-018: Scheduler single cycle");
    HAL_Init();
    DEM_Init();
    res = DiagMgr_StartupTest();
    TEST_ASSERT_EQUAL_UINT8(E_OK, res);
    WDT_Init();
    Scheduler_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    Scheduler_Run();
    TEST_ASSERT_EQUAL(SYS_MODE_NORMAL, DEM_GetMode());
}

uint32_t run_unit_tests(void)
{
    printf("\n====================================\n");
    printf("PH-11A: Unit Verification (SWE.4)\n");
    printf("====================================\n\n");
    UNITY_BEGIN();
    RUN_TEST(test_TC_001_SensorProc_RpmCalculation);
    RUN_TEST(test_TC_001b_SensorProc_AdcStuckDiag);
    RUN_TEST(test_TC_002_SensorProc_CurrentConversion);
    RUN_TEST(test_TC_003_SensorProc_VoltageAndLidDebounce);
    RUN_TEST(test_TC_004_SafetyMon_OverspeedDetection);
    RUN_TEST(test_TC_005_SafetyMon_OvercurrentDetection);
    RUN_TEST(test_TC_006_SafetyMon_VoltageAndCrc);
    RUN_TEST(test_TC_006b_SafetyMon_VibrationDetection);
    RUN_TEST(test_TC_007_FailSafe_MotorStop);
    RUN_TEST(test_TC_008_FailSafe_GateShutdown);
    RUN_TEST(test_TC_009_MotorCtrl_PwmUpdate);
    RUN_TEST(test_TC_010_MotorCtrl_Deceleration);
    RUN_TEST(test_TC_011_MEM_CpuRegTest);
    RUN_TEST(test_TC_012_MEM_RamMarchC);
    RUN_TEST(test_TC_012b_MEM_FullStartupTest);
    RUN_TEST(test_TC_012c_MEM_RuntimeDiag);
    RUN_TEST(test_TC_013_MEM_RomCrc32);
    RUN_TEST(test_TC_014_MEM_ClockCheck);
    RUN_TEST(test_TC_015_WDT_ConditionalKick);
    RUN_TEST(test_TC_015b_WDT_DiagFailure);
    RUN_TEST(test_TC_016_DEM_ModeTransitions);
    RUN_TEST(test_TC_017_DiagMgr_StartupSequence);
    RUN_TEST(test_TC_018_Scheduler_SingleCycle);
    printf("\n--- Unit Test Results ---\n");
    printf("Total: %u  Passed: %u  Failed: %u\n\n",
           (unsigned)Unity.total, (unsigned)Unity.passed, (unsigned)Unity.failed);
    return UNITY_END();
}
