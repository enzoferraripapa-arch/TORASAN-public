/**
 * @file   current_mon.h
 * @brief  電流監視モジュール ヘッダ
 * @req    SR-004, SR-005, SR-006, DR-003
 * @safety_class  クラスB
 * @project WMC (Washing Machine Motor Control)
 * @standard IEC 60730 Class B
 */

#ifndef CURRENT_MON_H
#define CURRENT_MON_H

#include <stdint.h>

/* ── 定数定義 ─────────────────────────────── */

/** @brief 過電流閾値 [mA] (TBD: モータ仕様確定後に設定) */
#define OVERCURRENT_THRESHOLD_MA    (8000U)

/** @brief ADCサンプリングレート [Hz] */
#define CURRENT_SAMPLE_RATE_HZ      (100U)

/** @brief デジタルLPFカットオフ周波数 [Hz] */
#define CURRENT_LPF_CUTOFF_HZ      (100U)

/** @brief 過電流デバウンスカウント (20ms / 10ms = 2回) */
#define OVERCURRENT_DEBOUNCE_CNT   (2U)

/** @brief ADC固着検出用閾値 */
#define ADC_STUCK_LOW_THRESHOLD    (10U)
#define ADC_STUCK_HIGH_THRESHOLD   (4085U)

/* ── 型定義 ───────────────────────────────── */

/** @brief 電流監視ステータス */
typedef enum {
    CURRENT_NORMAL = 0,       /**< 正常 */
    CURRENT_OVERCURRENT,      /**< 過電流検出 */
    CURRENT_SENSOR_FAULT      /**< センサ異常 */
} CurrentStatus_t;

/* ── 関数プロトタイプ ─────────────────────── */

/**
 * @brief  電流監視モジュール初期化
 * @req    SR-004
 * @safety_class  クラスB
 */
void CurrentMon_Init(void);

/**
 * @brief  電流サンプリング＋デジタルLPF処理
 * @req    SR-004
 * @safety_class  クラスB
 * @note   10ms周期でメインループから呼び出し
 */
void CurrentMon_Sample(void);

/**
 * @brief  過電流判定処理
 * @req    SR-005, DR-003
 * @safety_class  クラスB
 * @return CURRENT_NORMAL / CURRENT_OVERCURRENT / CURRENT_SENSOR_FAULT
 */
CurrentStatus_t CurrentMon_CheckOvercurrent(void);

/**
 * @brief  過電流時ゲートドライバ遮断処理
 * @req    SR-006
 * @safety_class  クラスB
 * @note   50ms以内に遮断完了すること
 */
void CurrentMon_GateShutdown(void);

/**
 * @brief  現在のフィルタ済み電流値を取得 [mA]
 * @return 電流値 [mA]
 */
float CurrentMon_GetFiltered_mA(void);

float CurrentMon_GetRaw_mA(void);

uint32_t CurrentMon_GetSampleCount(void);

#endif /* CURRENT_MON_H */
