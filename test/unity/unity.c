/**
 * @file   unity.c
 * @brief  Minimal Unity-compatible test framework - global state
 * @doc    PH-11 Test Framework
 *
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "unity.h"

/** Global Unity state instance */
UnityState_t Unity = {0U, 0U, 0U, NULL, NULL, 0U, 0U};
