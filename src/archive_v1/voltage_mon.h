/**
 * @file   voltage_mon.h
 * @brief  電圧監視モジュール ヘッダ
 * @req    SR-014, SR-015
 * @safety_class  クラスB
 * @project WMC (Washing Machine Motor Control)
 * @standard IEC 60730 Class B
 */

#ifndef VOLTAGE_MON_H
#define VOLTAGE_MON_H

#include <stdint.h>
#include <stdbool.h>

/* ── 定数定義 ─────────────────────────────── */

/** @brief 電圧監視下限 [mV] (TBD: 電源仕様確定後) */
#define VOLTAGE_LOWER_LIMIT_MV    (4500U)

/** @brief 電圧監視上限 [mV] (TBD: 電源仕様確定後) */
#define VOLTAGE_UPPER_LIMIT_MV    (5500U)

/** @brief サンプリング周期 [Hz] */
#define VOLTAGE_SAMPLE_RATE_HZ    (10U)

/* ── 型定義 ───────────────────────────────── */

typedef enum {
    VOLTAGE_NORMAL = 0,       /**< 正常範囲内 */
    VOLTAGE_UNDER,            /**< 低電圧 */
    VOLTAGE_OVER,             /**< 過電圧 */
    VOLTAGE_SENSOR_FAULT      /**< センサ異常 */
} VoltageStatus_t;

/* ── 関数プロトタイプ ─────────────────────── */

void VoltageMon_Init(void);

/**
 * @brief  電圧サンプリング処理
 * @req    SR-014
 * @safety_class  クラスB
 */
void VoltageMon_Sample(void);

/**
 * @brief  ウィンドウコンパレータ（上下限比較）
 * @req    SR-015
 * @safety_class  クラスB
 * @return true: 異常検出 (低電圧または過電圧), false: 正常
 */
bool VoltageMon_CheckVoltageWindow(void);

/**
 * @brief  低電圧検出（個別判定）
 * @req    SR-014
 * @safety_class  クラスB
 * @return true: 低電圧検出, false: 正常
 */
bool VoltageMon_CheckVoltageWindow_Low(void);

/**
 * @brief  過電圧検出（個別判定）
 * @req    SR-015
 * @safety_class  クラスB
 * @return true: 過電圧検出, false: 正常
 */
bool VoltageMon_CheckVoltageWindow_High(void);

float VoltageMon_GetFiltered_mV(void);

float VoltageMon_GetRaw_mV(void);

uint32_t VoltageMon_GetSampleCount(void);

#endif /* VOLTAGE_MON_H */
