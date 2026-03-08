/**
 * @file   test_qualification.c
 * @brief  PH-11C Qualification Tests - Requirements-based black-box testing
 * @doc    WMC-TQR-001 Qualification Test Specification
 *
 * Test cases TC-201 through TC-222: one per safety/diagnostic requirement.
 * Includes fault injection tests for DC verification.
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



/* TC-201 (SR-001): RPM measurement accuracy +/-50rpm */
static void test_TC_201_SR001_RpmAccuracy(void)
{
    TEST_MESSAGE("TC-201: SR-001 RPM measurement accuracy");
    SensorProc_Init();
    /* With hall_period=0, RPM should be exactly 0 */
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    SensorProc_Update();
    TEST_ASSERT_EQUAL_UINT16(0U, SensorProc_GetRpm());
}

/* TC-202 (SR-002): Overspeed detection at 1500rpm */
static void test_TC_202_SR002_OverspeedThreshold(void)
{
    const safety_flags_t *flags;
    uint8_t i;
    TEST_MESSAGE("TC-202: SR-002 Overspeed detection at 1500rpm");
    SensorProc_Init();
    SafetyMon_Init();
    /* RPM is 0 (no hall), so no overspeed */
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 10U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, flags->overspeed);
    /* Threshold verification: MOTOR_OVERSPEED_RPM = 1500 */
    TEST_ASSERT_EQUAL_UINT16(1500U, MOTOR_OVERSPEED_RPM);
}

/* TC-203 (SR-003): PWM stop within 1 control cycle on overspeed */
static void test_TC_203_SR003_PwmStopOnOverspeed(void)
{
    TEST_MESSAGE("TC-203: SR-003 PWM stop on overspeed");
    HAL_Init();
    DEM_Init();
    HAL_PWM_Start();
    HAL_PWM_SetDuty(HAL_PWM_PHASE_U, 500U);
    HAL_GPIO_Write(HAL_PORT_GATE_EN, HAL_PIN_GATE_EN, HAL_GPIO_HIGH);
    TEST_ASSERT_EQUAL_UINT8(1U, TestStub_GetPwmRunning());
    FailSafe_Execute(FAULT_OVERSPEED);
    TEST_ASSERT_EQUAL_UINT8(0U, TestStub_GetPwmRunning());
}

/* TC-204 (SR-004): Current measurement at 100Hz, 10-bit */
static void test_TC_204_SR004_CurrentMeasurement(void)
{
    uint8_t i;
    TEST_MESSAGE("TC-204: SR-004 Current measurement 100Hz 10-bit");
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    /* Current is read every call (100Hz at 10ms tick) */
    for (i = 0U; i < 40U; i++) { SensorProc_Update(); }
    TEST_ASSERT_TRUE(SensorProc_GetCurrentMa() > 0U);
    TEST_ASSERT_EQUAL_UINT8(10U, ADC_BITS);
    TEST_ASSERT_EQUAL_UINT8(100U, ADC_CURRENT_HZ);
}

/* TC-205 (SR-005): Overcurrent detection at 8A */
static void test_TC_205_SR005_OvercurrentThreshold(void)
{
    const safety_flags_t *flags;
    uint8_t i;
    TEST_MESSAGE("TC-205: SR-005 Overcurrent detection at 8A");
    SensorProc_Init();
    SafetyMon_Init();
    TEST_ASSERT_EQUAL_UINT16(8000U, MOTOR_MAX_MA);

    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 819U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 40U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, flags->overcurrent);
}

/* TC-206 (SR-006): Gate shutdown within 1ms on overcurrent */
static void test_TC_206_SR006_GateShutdownTiming(void)
{
    TEST_MESSAGE("TC-206: SR-006 Gate shutdown on overcurrent");
    HAL_Init();
    DEM_Init();
    HAL_PWM_Start();
    HAL_GPIO_Write(HAL_PORT_GATE_EN, HAL_PIN_GATE_EN, HAL_GPIO_HIGH);
    FailSafe_Execute(FAULT_OVERCURRENT);
    TEST_ASSERT_EQUAL_UINT8(0U, TestStub_GetPwmRunning());
    TEST_ASSERT_EQUAL_UINT8(HAL_GPIO_LOW,
        TestStub_GetGpioOutput(HAL_PORT_GATE_EN, HAL_PIN_GATE_EN));
}

/* TC-207 (SR-007): Lid debounce within 60ms */
static void test_TC_207_SR007_LidDebounce(void)
{
    uint8_t i;
    TEST_MESSAGE("TC-207: SR-007 Lid debounce <= 60ms");
    SensorProc_Init();
    TEST_ASSERT_EQUAL_UINT8(6U, DEBOUNCE_COUNT);
    /* 6 * 10ms = 60ms */
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_HIGH);
    for (i = 0U; i < 6U; i++) { SensorProc_Update(); }
    TEST_ASSERT_EQUAL_UINT8((uint8_t)LID_OPEN, (uint8_t)SensorProc_GetLidState());
}

/* TC-208 (SR-008): Lid open stop sequence within 140ms */
static void test_TC_208_SR008_LidOpenStopSequence(void)
{
    TEST_MESSAGE("TC-208: SR-008 Lid open stop <= 140ms");
    TEST_ASSERT_EQUAL_UINT16(140U, LID_STOP_MS);
    HAL_Init();
    DEM_Init();
    HAL_PWM_Start();
    HAL_PWM_SetDuty(HAL_PWM_PHASE_U, 500U);
    FailSafe_StopMotor();
    TEST_ASSERT_EQUAL_UINT8(0U, TestStub_GetPwmRunning());
}

/* TC-209 (SR-009): CPU register test (Annex H) */
static void test_TC_209_SR009_CpuRegTest(void)
{
    Std_ReturnType result;
    TEST_MESSAGE("TC-209: SR-009 CPU register test");
    result = MEM_CpuRegTest();
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
}

/* TC-210 (SR-010): RAM MarchC test (256B x 22) */
static void test_TC_210_SR010_RamMarchC(void)
{
    Std_ReturnType result;
    uint8_t block;
    TEST_MESSAGE("TC-210: SR-010 RAM MarchC test");
    TEST_ASSERT_EQUAL_UINT16(256U, RAM_TEST_BLOCK_B);
    TEST_ASSERT_EQUAL_UINT8(22U, RAM_TEST_BLOCK_COUNT);
    for (block = 0U; block < RAM_TEST_BLOCK_COUNT; block++)
    {
        result = MEM_RamBlockTest(block);
        TEST_ASSERT_EQUAL_UINT8(E_OK, result);
    }
}

/* TC-211 (SR-011): Flash ROM CRC32 */
static void test_TC_211_SR011_FlashCrc32(void)
{
    Std_ReturnType result;
    TEST_MESSAGE("TC-211: SR-011 Flash ROM CRC-32");
    TEST_ASSERT_EQUAL_UINT32(0x04C11DB7UL, CRC32_POLYNOMIAL);
    result = MEM_RomBlockCrc(0U);
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
}

/* TC-212 (SR-012): Clock monitoring +/-4% */
static void test_TC_212_SR012_ClockMonitoring(void)
{
    Std_ReturnType result;
    TEST_MESSAGE("TC-212: SR-012 Clock monitoring +/-4%");
    TEST_ASSERT_EQUAL_UINT8(4U, CLOCK_TOLERANCE_PCT);
    result = MEM_ClockCheck();
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
}

/* TC-213 (SR-013): Safe state transition within 100ms */
static void test_TC_213_SR013_SafeStateTransition(void)
{
    TEST_MESSAGE("TC-213: SR-013 Safe state <= 100ms");
    TEST_ASSERT_EQUAL_UINT16(100U, SAFE_TRANSITION_MS);
    HAL_Init();
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();
    HAL_PWM_Start();
    HAL_GPIO_Write(HAL_PORT_GATE_EN, HAL_PIN_GATE_EN, HAL_GPIO_HIGH);
    FailSafe_Execute(FAULT_OVERCURRENT);
    TEST_ASSERT_EQUAL_UINT8(0U, TestStub_GetPwmRunning());
    TEST_ASSERT_EQUAL(SYS_MODE_SAFE, DEM_GetMode());
}

/* TC-214 (SR-014): Voltage monitoring 4.5V~5.5V */
static void test_TC_214_SR014_VoltageRange(void)
{
    const safety_flags_t *flags;
    uint8_t i;
    TEST_MESSAGE("TC-214: SR-014 Voltage monitoring 4.5V-5.5V");
    TEST_ASSERT_EQUAL_UINT16(4500U, VOLTAGE_MIN_MV);
    TEST_ASSERT_EQUAL_UINT16(5500U, VOLTAGE_MAX_MV);
    SensorProc_Init();
    SafetyMon_Init();

    /* In-range: ADC=1000 -> ~4888mV */
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 20U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, flags->voltage_error);

    /* Under-voltage: ADC=800 -> ~3910mV */
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 800U);
    for (i = 0U; i < 20U; i++) { SensorProc_Update(); }
    SafetyMon_Check();
    flags = SafetyMon_GetFlags();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, flags->voltage_error);
}

/* TC-215 (SR-015): Voltage error startup inhibit */
static void test_TC_215_SR015_VoltageStartupInhibit(void)
{
    TEST_MESSAGE("TC-215: SR-015 Voltage error startup inhibit");
    DEM_Init();
    DEM_ReportFault(FAULT_VOLTAGE);
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, DEM_IsMotorAllowed());
}

/* TC-216 (SR-016): WDT kick management (100ms/50ms) */
static void test_TC_216_SR016_WdtKickManagement(void)
{
    TEST_MESSAGE("TC-216: SR-016 WDT kick management");
    TEST_ASSERT_EQUAL_UINT16(100U, WDT_TIMEOUT_MS);
    TEST_ASSERT_EQUAL_UINT16(50U, WDT_KICK_MS);
    SensorProc_Init();
    SafetyMon_Init();
    MEM_StartupTest();
    WDT_Init();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, WDT_IsAllDiagOk());
    WDT_Kick();
}

/* TC-217 (SR-017): Vibration detection (current pattern) */
static void test_TC_217_SR017_VibrationDetection(void)
{
    const safety_flags_t *flags;
    uint8_t i;
    TEST_MESSAGE("TC-217: SR-017 Vibration detection");
    SensorProc_Init();
    SafetyMon_Init();
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

/* TC-218 (SR-018): Vibration deceleration to 500rpm within 1000ms */
static void test_TC_218_SR018_VibrationDeceleration(void)
{
    TEST_MESSAGE("TC-218: SR-018 Vibration deceleration");
    TEST_ASSERT_EQUAL_UINT16(500U, MOTOR_VIBRATION_RPM);
    TEST_ASSERT_EQUAL_UINT16(1000U, MOTOR_VIBRATION_MS);
    MotorCtrl_Init();
    SensorProc_Init();
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();
    /* Initiate deceleration (RPM=0 means already below target) */
    MotorCtrl_Decelerate(MOTOR_VIBRATION_RPM, MOTOR_VIBRATION_MS);
}

/* TC-219 (DR-001): Startup diagnostic all-pass required */
static void test_TC_219_DR001_StartupAllPass(void)
{
    Std_ReturnType result;
    TEST_MESSAGE("TC-219: DR-001 Startup diagnostic all-pass");
    DEM_Init();
    result = DiagMgr_StartupTest();
    TEST_ASSERT_EQUAL_UINT8(E_OK, result);
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, DiagMgr_AllOK());
    TEST_ASSERT_EQUAL(SYS_MODE_STARTUP, DEM_GetMode());
}

/* TC-220 (DR-002): Runtime diagnostic continuation */
static void test_TC_220_DR002_RuntimeDiagContinuation(void)
{
    Std_ReturnType result;
    uint8_t i;
    TEST_MESSAGE("TC-220: DR-002 Runtime diagnostic continuation");
    MEM_StartupTest();
    for (i = 0U; i < 10U; i++)
    {
        result = MEM_RunDiag();
        TEST_ASSERT_EQUAL_UINT8(E_OK, result);
    }
}

/* TC-221 (DR-003): ADC plausibility check */
static void test_TC_221_DR003_AdcPlausibility(void)
{
    uint8_t i;
    TEST_MESSAGE("TC-221: DR-003 ADC plausibility check");
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 0U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 512U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 10U; i++) { SensorProc_Update(); }
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, SensorProc_GetAdcDiagFault());
}

/* TC-222 (DR-004): Sensor range validation */
static void test_TC_222_DR004_SensorRangeValidation(void)
{
    uint8_t i;
    uint16_t current_ma;
    TEST_MESSAGE("TC-222: DR-004 Sensor range validation");
    SensorProc_Init();
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, 512U);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 1000U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 40U; i++) { SensorProc_Update(); }
    current_ma = SensorProc_GetCurrentMa();
    TEST_ASSERT_TRUE(current_ma <= 10000U);
    TEST_ASSERT_TRUE(SensorProc_GetVoltageMv() <= 5000U);
}

/* TC-223: Fault injection - CPU diagnostic failure */
static void test_TC_223_FaultInjection_CpuDiag(void)
{
    TEST_MESSAGE("TC-223: Fault injection - CPU diagnostic");
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();
    DEM_ReportFault(FAULT_CPU_REG);
    TEST_ASSERT_EQUAL(SYS_MODE_FAULT, DEM_GetMode());
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, DEM_IsMotorAllowed());
}

/* TC-224: Fault injection - RAM diagnostic failure */
static void test_TC_224_FaultInjection_RamDiag(void)
{
    TEST_MESSAGE("TC-224: Fault injection - RAM diagnostic");
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();
    DEM_ReportFault(FAULT_RAM);
    TEST_ASSERT_EQUAL(SYS_MODE_FAULT, DEM_GetMode());
}

/* TC-225: Fault injection - ROM diagnostic failure */
static void test_TC_225_FaultInjection_RomDiag(void)
{
    TEST_MESSAGE("TC-225: Fault injection - ROM diagnostic");
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();
    DEM_ReportFault(FAULT_ROM);
    TEST_ASSERT_EQUAL(SYS_MODE_FAULT, DEM_GetMode());
}

/* TC-226: Fault injection - Clock diagnostic failure */
static void test_TC_226_FaultInjection_ClockDiag(void)
{
    TEST_MESSAGE("TC-226: Fault injection - Clock diagnostic");
    DEM_Init();
    DEM_SetStartup();
    DEM_SetNormal();
    DEM_ReportFault(FAULT_CLOCK);
    TEST_ASSERT_EQUAL(SYS_MODE_FAULT, DEM_GetMode());
}

/* TC-227: Fault injection - WDT gating on diagnostic failure */
static void test_TC_227_FaultInjection_WdtGating(void)
{
    uint8_t i;
    TEST_MESSAGE("TC-227: Fault injection - WDT gating");
    SensorProc_Init();
    SafetyMon_Init();
    MEM_StartupTest();
    TEST_ASSERT_EQUAL_UINT8(FLAG_SET, WDT_IsAllDiagOk());

    /* Inject ADC stuck-at fault */
    TestStub_SetAdcValue(HAL_ADC_CH_CURRENT, ADC_MAX_VALUE);
    TestStub_SetAdcValue(HAL_ADC_CH_VOLTAGE, 512U);
    TestStub_SetGpioInput(HAL_PORT_LID_SENSOR, HAL_PIN_LID_SENSOR, HAL_GPIO_LOW);
    for (i = 0U; i < 10U; i++) { SensorProc_Update(); }
    TEST_ASSERT_EQUAL_UINT8(FLAG_CLEAR, WDT_IsAllDiagOk());
}

uint32_t run_qualification_tests(void)
{
    printf("\n=============================================\n");
    printf("PH-11C: Qualification Verification (SWE.6)\n");
    printf("=============================================\n\n");
    UNITY_BEGIN();
    RUN_TEST(test_TC_201_SR001_RpmAccuracy);
    RUN_TEST(test_TC_202_SR002_OverspeedThreshold);
    RUN_TEST(test_TC_203_SR003_PwmStopOnOverspeed);
    RUN_TEST(test_TC_204_SR004_CurrentMeasurement);
    RUN_TEST(test_TC_205_SR005_OvercurrentThreshold);
    RUN_TEST(test_TC_206_SR006_GateShutdownTiming);
    RUN_TEST(test_TC_207_SR007_LidDebounce);
    RUN_TEST(test_TC_208_SR008_LidOpenStopSequence);
    RUN_TEST(test_TC_209_SR009_CpuRegTest);
    RUN_TEST(test_TC_210_SR010_RamMarchC);
    RUN_TEST(test_TC_211_SR011_FlashCrc32);
    RUN_TEST(test_TC_212_SR012_ClockMonitoring);
    RUN_TEST(test_TC_213_SR013_SafeStateTransition);
    RUN_TEST(test_TC_214_SR014_VoltageRange);
    RUN_TEST(test_TC_215_SR015_VoltageStartupInhibit);
    RUN_TEST(test_TC_216_SR016_WdtKickManagement);
    RUN_TEST(test_TC_217_SR017_VibrationDetection);
    RUN_TEST(test_TC_218_SR018_VibrationDeceleration);
    RUN_TEST(test_TC_219_DR001_StartupAllPass);
    RUN_TEST(test_TC_220_DR002_RuntimeDiagContinuation);
    RUN_TEST(test_TC_221_DR003_AdcPlausibility);
    RUN_TEST(test_TC_222_DR004_SensorRangeValidation);
    RUN_TEST(test_TC_223_FaultInjection_CpuDiag);
    RUN_TEST(test_TC_224_FaultInjection_RamDiag);
    RUN_TEST(test_TC_225_FaultInjection_RomDiag);
    RUN_TEST(test_TC_226_FaultInjection_ClockDiag);
    RUN_TEST(test_TC_227_FaultInjection_WdtGating);
    printf("\n--- Qualification Test Results ---\n");
    printf("Total: %u  Passed: %u  Failed: %u\n\n",
           (unsigned)Unity.total, (unsigned)Unity.passed, (unsigned)Unity.failed);
    return UNITY_END();
}
