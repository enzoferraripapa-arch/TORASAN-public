/**
 * @file   unity.h
 * @brief  Minimal Unity-compatible test framework for IEC 60730 Class B testing
 * @doc    PH-11 Test Framework
 *
 * Provides assertion macros, test runner, and result tracking.
 * No dynamic memory allocation. No float/double.
 *
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#ifndef UNITY_H
#define UNITY_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ========================================================================
 * Test Result Tracking
 * ======================================================================== */

typedef struct {
    uint32_t total;
    uint32_t passed;
    uint32_t failed;
    const char *current_test;
    const char *current_file;
    uint32_t current_line;
    uint8_t  test_failed;
} UnityState_t;

extern UnityState_t Unity;

/* ========================================================================
 * setUp / tearDown prototypes (user must define)
 * ======================================================================== */

extern void setUp(void);
extern void tearDown(void);

/* ========================================================================
 * Test Runner Macros
 * ======================================================================== */

#define RUN_TEST(func) \
    do { \
        Unity.current_test = #func; \
        Unity.test_failed = 0U; \
        setUp(); \
        func(); \
        tearDown(); \
        Unity.total++; \
        if (Unity.test_failed == 0U) { \
            Unity.passed++; \
            printf("  [PASS] %s\n", #func); \
        } else { \
            Unity.failed++; \
            printf("  [FAIL] %s\n", #func); \
        } \
    } while (0)

/* ========================================================================
 * Unity Begin / End
 * ======================================================================== */

#define UNITY_BEGIN() \
    do { \
        Unity.total = 0U; \
        Unity.passed = 0U; \
        Unity.failed = 0U; \
        Unity.test_failed = 0U; \
    } while (0)

#define UNITY_END() (Unity.failed)

/* ========================================================================
 * Internal Failure Handler
 * ======================================================================== */

#define UNITY_FAIL(msg) \
    do { \
        printf("    ASSERT FAIL @ %s:%u: %s\n", __FILE__, (unsigned)__LINE__, (msg)); \
        Unity.test_failed = 1U; \
    } while (0)

/* ========================================================================
 * Assertion Macros
 * ======================================================================== */

#define TEST_ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            UNITY_FAIL("Expected TRUE, got FALSE"); \
        } \
    } while (0)

#define TEST_ASSERT_FALSE(condition) \
    do { \
        if ((condition)) { \
            UNITY_FAIL("Expected FALSE, got TRUE"); \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), "Expected %d, got %d", \
                     (int)(expected), (int)(actual)); \
            UNITY_FAIL(_msg); \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_UINT8(expected, actual) \
    do { \
        if ((uint8_t)(expected) != (uint8_t)(actual)) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), "Expected 0x%02X, got 0x%02X", \
                     (unsigned)(uint8_t)(expected), (unsigned)(uint8_t)(actual)); \
            UNITY_FAIL(_msg); \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_UINT16(expected, actual) \
    do { \
        if ((uint16_t)(expected) != (uint16_t)(actual)) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), "Expected %u, got %u", \
                     (unsigned)(uint16_t)(expected), (unsigned)(uint16_t)(actual)); \
            UNITY_FAIL(_msg); \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_UINT32(expected, actual) \
    do { \
        if ((uint32_t)(expected) != (uint32_t)(actual)) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), "Expected %u, got %u", \
                     (unsigned)(uint32_t)(expected), (unsigned)(uint32_t)(actual)); \
            UNITY_FAIL(_msg); \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(pointer) \
    do { \
        if ((pointer) == NULL) { \
            UNITY_FAIL("Expected non-NULL pointer"); \
        } \
    } while (0)

#define TEST_ASSERT_UINT16_WITHIN(delta, expected, actual) \
    do { \
        uint16_t _exp = (uint16_t)(expected); \
        uint16_t _act = (uint16_t)(actual); \
        uint16_t _dlt = (uint16_t)(delta); \
        uint16_t _diff = (_act >= _exp) ? (_act - _exp) : (_exp - _act); \
        if (_diff > _dlt) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), \
                     "Expected %u +/-%u, got %u (diff=%u)", \
                     (unsigned)_exp, (unsigned)_dlt, \
                     (unsigned)_act, (unsigned)_diff); \
            UNITY_FAIL(_msg); \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    TEST_ASSERT_EQUAL((expected), (actual))

#define TEST_ASSERT_NOT_EQUAL(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), "Expected NOT %d, but got %d", \
                     (int)(expected), (int)(actual)); \
            UNITY_FAIL(_msg); \
        } \
    } while (0)

#define TEST_ASSERT_GREATER_OR_EQUAL(threshold, actual) \
    do { \
        if ((int)(actual) < (int)(threshold)) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), "Expected >= %d, got %d", \
                     (int)(threshold), (int)(actual)); \
            UNITY_FAIL(_msg); \
        } \
    } while (0)

#define TEST_ASSERT_LESS_OR_EQUAL(threshold, actual) \
    do { \
        if ((int)(actual) > (int)(threshold)) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), "Expected <= %d, got %d", \
                     (int)(threshold), (int)(actual)); \
            UNITY_FAIL(_msg); \
        } \
    } while (0)

#define TEST_MESSAGE(msg) \
    printf("    [INFO] %s\n", (msg))

#endif /* UNITY_H */
