/**
 * @file   lid_mon.h
 * @brief  蓋監視モジュール ヘッダ
 * @req    SR-007, SR-008, DR-004
 * @safety_class  クラスB
 * @project WMC (Washing Machine Motor Control)
 * @standard IEC 60730 Class B
 */

#ifndef LID_MON_H
#define LID_MON_H

#include <stdint.h>

/* ── 定数定義 ─────────────────────────────── */

/** @brief 蓋センサポーリング周期 [ms] */
#define LID_POLL_PERIOD_MS      (10U)

/** @brief デバウンス時間 [ms] (20ms = 2回@10ms) */
#define LID_DEBOUNCE_MS         (20U)
#define LID_DEBOUNCE_CNT        (2U)

/** @brief 蓋開放検出後のモータ停止許容時間 [ms] */
#define LID_STOP_TIMEOUT_MS     (200U)

/* ── 型定義 ───────────────────────────────── */

typedef enum {
    LID_CLOSED = 0,       /**< 蓋閉 */
    LID_OPEN,             /**< 蓋開放 */
    LID_SENSOR_FAULT      /**< センサ異常 */
} LidStatus_t;

/* ── 関数プロトタイプ ─────────────────────── */

/**
 * @brief  蓋監視モジュール初期化
 * @req    SR-007
 * @safety_class  クラスB
 */
void LidMon_Init(void);

/**
 * @brief  蓋センサスキャン＋デバウンス処理
 * @req    SR-007, DR-004
 * @safety_class  クラスB
 * @note   10ms周期でメインループから呼び出し
 */
void LidMon_Scan(void);

/**
 * @brief  蓋開放時モータ停止処理
 * @req    SR-008
 * @safety_class  クラスB
 * @note   200ms以内に停止完了すること
 */
void LidMon_StopMotor(void);

/**
 * @brief  現在の蓋状態を取得
 * @return LID_CLOSED / LID_OPEN / LID_SENSOR_FAULT
 */
LidStatus_t LidMon_GetStatus(void);

#endif /* LID_MON_H */
