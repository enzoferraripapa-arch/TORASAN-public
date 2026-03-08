/**
 * @file   test_common.c
 * @brief  Common setUp/tearDown for all test suites
 */

#include "test_hal_stub.h"
#include "sensor_proc.h"
#include "safety_mon.h"
#include "motor_ctrl.h"
#include "mem_test.h"
#include "dem.h"

void setUp(void)
{
    TestStub_ResetAll();
    SensorProc_Init();
    SafetyMon_Init();
    MotorCtrl_Init();
    MEM_Init();
    DEM_Init();
}

void tearDown(void)
{
}
