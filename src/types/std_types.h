/**
 * @file    std_types.h
 * @brief   Standard type definitions (T-01/T-02/T-04 compliant)
 * @module  Common
 * @safety  IEC 60730 Class B
 *
 * Type rules:
 *   T-01: bool/true/false/stdbool.h prohibited
 *   T-02: Fixed-width types only
 *   T-04: Logic values as uint8_t + STD_TRUE/STD_FALSE
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <stdint.h>

/* T-01/T-04: Logic value definitions (bool prohibited) */
#define STD_TRUE    ((uint8_t)1U)
#define STD_FALSE   ((uint8_t)0U)

/* Return type for general operations */
#define STD_OK      ((uint8_t)0U)
#define STD_NOT_OK  ((uint8_t)1U)

/* NULL pointer definition */
#ifndef NULL_PTR
#define NULL_PTR    ((void *)0)
#endif

#endif /* STD_TYPES_H */
