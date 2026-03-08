/**
 * @file   hal_wdt.h
 * @brief  WDT HALドライバ ヘッダ
 * @req    —
 * @safety_class  —（HAL層）
 */

#ifndef HAL_WDT_H
#define HAL_WDT_H

#include <stdint.h>

void HAL_WDT_Init(uint16_t timeout_ms);
void HAL_WDT_Kick(void);

/** @brief 共通HW初期化（全HAL統合初期化） */
void HAL_Init(void);

#endif /* HAL_WDT_H */
