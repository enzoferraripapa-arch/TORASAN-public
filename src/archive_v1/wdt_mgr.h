/**
 * @file   wdt_mgr.h
 * @brief  ウォッチドッグタイマ管理モジュール ヘッダ
 * @req    SR-016
 * @safety_class  クラスB
 * @project WMC (Washing Machine Motor Control)
 * @standard IEC 60730 Class B
 */

#ifndef WDT_MGR_H
#define WDT_MGR_H

#include <stdint.h>

/* ── 定数定義 ─────────────────────────────── */

/** @brief WDTタイムアウト [ms] */
#define WDT_TIMEOUT_MS      (100U)

/** @brief WDTキック周期 [ms] */
#define WDT_KICK_PERIOD_MS  (50U)

/* ── 関数プロトタイプ ─────────────────────── */

/**
 * @brief  WDT管理モジュール初期化（WDT開始）
 * @req    SR-016
 * @safety_class  クラスB
 */
void WdtMgr_Init(void);

/**
 * @brief  WDTキック（タイマリフレッシュ）
 * @req    SR-016
 * @safety_class  クラスB
 * @note   メインループ1ms周期で呼び出し
 */
void WdtMgr_Kick(void);

#endif /* WDT_MGR_H */
