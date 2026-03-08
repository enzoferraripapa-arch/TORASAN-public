/**
 * @file   motor_ctrl.h
 * @brief  モータ制御モジュール ヘッダファイル
 * @req    SA-002 (モータ制御サブシステム)
 * @safety_class —
 * @project WMC (Washing Machine Motor Control)
 * @standard IEC 60730 Class B
 * @version 1.0
 * @date    2026-02-27
 */

#ifndef MOTOR_CTRL_H_
#define MOTOR_CTRL_H_

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*/
/* 型定義                                                                    */
/*============================================================================*/

/**
 * @brief 安全状態ステータス
 */
typedef enum {
    NORMAL               = 0x00U,
    OVERSPEED_DETECTED   = 0x01U,
} SafetyStatus_t;

/*============================================================================*/
/* 定数定義                                                                  */
/*============================================================================*/

/** @brief 過速度判定しきい値 [rpm] */
#define MOTOR_OVERSPEED_THRESHOLD     (1500U)

/** @brief 過速度デバウンス回数 */
#define MOTOR_OVERSPEED_DEBOUNCE_CNT  (3U)

/** @brief モータ最大回転数 [rpm] */
#define MOTOR_MAX_RPM                 (3000U)

/** @brief モータ回転数フィルタ係数 (IIR) */
#define MOTOR_RPM_FILTER_ALPHA        (0.3f)  /* fc ≒ 1Hz @ 10ms */

/*============================================================================*/
/* 関数プロトタイプ                                                          */
/*============================================================================*/

/**
 * @brief  モータ制御モジュール初期化
 * @req    UT-001
 * @safety_class —
 * @test   TC-001 (TBD)
 *
 * @description
 *   PWM出力ピン、パルスエンコーダ入力ピン、割込み設定等
 *   を初期化。リレー出力はデフォルトで OFF に設定。
 *
 * @return void
 */
void MotorCtrl_Init(void);

/**
 * @brief  現在の回転数を取得する
 * @req    UT-002
 * @req    SR-001 (モータ回転数の周期的監視)
 * @safety_class クラスB
 * @test   TC-002 (TBD)
 *
 * @description
 *   パルスエンコーダまたはHallセンサからの入力を集計し、
 *   低速フィルタ(LPF)を適用して回転数を算出する。
 *   安全性：複数の計測周期にわたる移動平均により、
 *   ノイズ耐性および異常検出遅延の最小化を実現。
 *   周期: 10ms
 *
 * @return 現在の回転数 [rpm] (0-3000)
 */
float MotorCtrl_GetRpm(void);

/**
 * @brief  過速度を判定する
 * @req    UT-003
 * @req    SR-002 (過速度の検出と即座の停止)
 * @safety_class クラスB
 * @test   TC-003 (TBD)
 *
 * @description
 *   現在の回転数が設定された上限値（MOTOR_OVERSPEED_THRESHOLD）
 *   を超えた場合、デバウンス処理後に警告/停止を発行する。
 *   デバウンス処理により、一過的なノイズを無視。
 *   応答遅延: 最大 30ms (3回 × 10ms周期)
 *   周期: 10ms
 *
 *   故障検出時の動作:
 *     1. SafetyMgr_TransitionSafe(FAULT_OVERSPEED) 呼び出し
 *     2. 戻り値: true (過速度検出)
 *
 * @return bool: true (過速度検出) / false (正常)
 */
bool MotorCtrl_CheckOverspeed(void);

/**
 * @brief  緊急停止処理を実行する
 * @req    UT-004
 * @req    SR-003 (安全な停止状態への強制遷移)
 * @safety_class クラスB
 * @test   TC-004 (TBD)
 *
 * @description
 *   モータへのPWM出力を0に設定し、同時に再スタート禁止フラグを
 *   セットする。割込みコンテキストからも安全に呼び出し可能。
 *   応答時間: <1ms
 *
 * @return void
 */
void MotorCtrl_EmergencyStop(void);

/**
 * @brief  現在のPWMデューティサイクルを取得する
 * @req    —
 * @safety_class クラスB
 *
 * @description
 *   現在設定されているPWMのデューティサイクル（0-100%）を返す。
 *   モータ制御ループから確認用に使用される。
 *
 * @return PWMデューティサイクル (0-100%)
 */
uint8_t MotorCtrl_GetDuty(void);

#endif /* MOTOR_CTRL_H_ */
