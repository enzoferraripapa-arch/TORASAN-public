/**
 * @file   test_main.c
 * @brief  Test runner - executes all PH-11 test suites
 * @doc    WMC-TVR-001, WMC-TIR-001, WMC-TQR-001
 */

#include <stdio.h>
#include <stdint.h>

/* External test suite runners */
extern uint32_t run_unit_tests(void);
extern uint32_t run_integration_tests(void);
extern uint32_t run_qualification_tests(void);

int main(void)
{
    uint32_t unit_fail;
    uint32_t integ_fail;
    uint32_t qual_fail;
    uint32_t total_fail;

    printf("================================================\n");
    printf("TORASAN WMC PH-11 Test Suite\n");
    printf("IEC 60730 Class B / MISRA-C:2012\n");
    printf("================================================\n");

    unit_fail  = run_unit_tests();
    integ_fail = run_integration_tests();
    qual_fail  = run_qualification_tests();

    total_fail = unit_fail + integ_fail + qual_fail;

    printf("================================================\n");
    printf("OVERALL RESULTS\n");
    printf("================================================\n");
    printf("  Unit tests:          %s\n", (unit_fail == 0U) ? "PASS" : "FAIL");
    printf("  Integration tests:   %s\n", (integ_fail == 0U) ? "PASS" : "FAIL");
    printf("  Qualification tests: %s\n", (qual_fail == 0U) ? "PASS" : "FAIL");
    printf("================================================\n");

    if (total_fail == 0U)
    {
        printf("ALL TESTS PASSED\n");
    }
    else
    {
        printf("FAILURES DETECTED: %u\n", (unsigned)total_fail);
    }
    printf("================================================\n");

    return (total_fail == 0U) ? 0 : 1;
}
