/**
 * @file    hal_gpio.h
 * @brief   GPIO hardware abstraction layer
 * @module  SA-003 HAL
 * @safety  IEC 60730 Class B
 * @req     SR-007, SR-008
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include "../types/std_types.h"

/**
 * @brief   Initialize GPIO pins (directions and defaults)
 * @module  SA-003 HAL
 */
void HalGpio_Init(void);

/**
 * @brief   Read lid sensor state (with hardware read)
 * @module  SA-003 HAL
 * @req     SR-007
 * @safety  IEC 60730 Class B
 * @return  STD_TRUE if lid is open, STD_FALSE if closed
 */
uint8_t HalGpio_GetLidState(void);

/**
 * @brief   Read hall sensor pattern (U/V/W)
 * @module  SA-003 HAL
 * @req     SR-001
 * @return  3-bit hall pattern (bits 0-2 = U/V/W)
 */
uint8_t HalGpio_GetHallPattern(void);

/**
 * @brief   Set relay output (inverter power control)
 * @module  SA-003 HAL
 * @req     SR-006, SR-008
 * @safety  IEC 60730 Class B
 * @param   state  STD_TRUE=ON (energize), STD_FALSE=OFF (de-energize)
 */
void HalGpio_SetRelay(uint8_t state);

/**
 * @brief   Set red LED (fault indicator)
 * @module  SA-003 HAL
 * @param   state  STD_TRUE=ON, STD_FALSE=OFF
 */
void HalGpio_SetLed(uint8_t state);

#endif /* HAL_GPIO_H */
