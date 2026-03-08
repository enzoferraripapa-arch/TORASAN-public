/**
 * @file   safety_mgr.h
 * @brief  安全管理モジュール ヘッダファイル
 * @req    SA-006 (安全管理サブシステム)
 * @safety_class クラスB
 * @project WMC (Washing Machine Motor Control)
 * @standard IEC 60730 Class B
 * @version 1.0
 * @date    2026-02-27
 */

#ifndef SAFETY_MGR_H_
#define SAFETY_MGR_H_

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*/
/* 型定義 — システム状態遷移                                                 */
/*============================================================================*/

/**
 * @brief システム動作状態 (State Machine)
 *
 * 状態遷移図:
 *
 *   [STARTUP_DIAG] ──(診断PASS)──→ [NORMAL]
 *       │                              │
 *       │                              │(故障検出)
 *       │                              ▼
 *       │                          [SAFE_STATE] ←──(診断FAIL)──┐
 *       │                              │                       │
 *       └──────(診断FAIL)──────────────┴───────────────────────┘
 *
 *   [SAFE_STATE] ──(電源再投入)──→ [STARTUP_DIAG]
 *
 */
typedef enum {
    STATE_STARTUP_DIAG    = 0x00U,  /**< 起動時自己診断フェーズ */
    STATE_NORMAL          = 0x01U,  /**< 通常動作状態 */
    STATE_SAFE_STATE      = 0x02U,  /**< 安全状態（故障時） */
} SystemState_t;

/*============================================================================*/
/* 型定義 — 故障コード                                                       */
/*============================================================================*/

/**
 * @brief 故障コード定義
 *
 * 各故障コードは以下の情報を持つ：
 *   - 故障の種類（ハードウェア / ソフトウェア / センサ）
 *   - 対応する SR (Safety Requirement)
 *   - 対応する UT (Unit Test)
 *   - 復旧可能性（一時的 / 永久的）
 *
 */
typedef enum {
    FAULT_NONE                = 0x00U,  /**< エラーなし */

    /* 起動時診断関連 (STARTUP_DIAG) */
    FAULT_CPU_REG             = 0x01U,  /**< CPU Register test fail [SR-009][UT-010] */
    FAULT_RAM                 = 0x02U,  /**< RAM March C test fail [SR-010][UT-011] */
    FAULT_ROM                 = 0x03U,  /**< ROM CRC mismatch [SR-011][UT-012] */
    FAULT_CLOCK               = 0x04U,  /**< Clock freq out of range [SR-012][UT-013] */

    /* モータ制御関連 (NORMAL → SAFE_STATE) */
    FAULT_OVERSPEED           = 0x05U,  /**< Motor overspeed detected [SR-002][UT-003] */

    /* 電流・電力関連 */
    FAULT_OVERCURRENT         = 0x06U,  /**< Overcurrent detected [SR-005][UT-006] */

    /* 電圧監視関連 */
    FAULT_VOLTAGE_LOW         = 0x07U,  /**< Supply voltage too low [SR-015][UT-016] */
    FAULT_VOLTAGE_HIGH        = 0x08U,  /**< Supply voltage too high [SR-015][UT-016] */

    /* センサ関連 */
    FAULT_LID_OPEN            = 0x09U,  /**< Lid open during operation [SR-008][UT-009] */

    /* ハードウェア関連 */
    FAULT_WDT_TIMEOUT         = 0x0AU,  /**< Watchdog timeout (HW) [SR-016][UT-017] */

    /* 予約領域 */
    FAULT_RESERVED_0B         = 0x0BU,
    FAULT_RESERVED_0C         = 0x0CU,
    FAULT_RESERVED_0D         = 0x0DU,
    FAULT_RESERVED_0E         = 0x0EU,
    FAULT_RESERVED_0F         = 0x0FU,

} FaultCode_t;

/*============================================================================*/
/* 定数定義                                                                  */
/*============================================================================*/

/** @brief 安全状態での LED点灯色（警告表示） */
#define SAFE_STATE_LED_COLOR        (LED_RED)

/** @brief 安全状態でのアラート音（有効/無効） */
#define SAFE_STATE_BUZZER_ENABLED   (1U)

/** @brief 安全状態での電源遮断タイムアウト [s]（無限の場合は0） */
#define SAFE_STATE_POWERDOWN_TIMEOUT (0U)  /* 電源再投入まで待機 */

/*============================================================================*/
/* グローバルシステムコンテキスト構造体                                       */
/*============================================================================*/

/**
 * @brief システム全体のコンテキスト情報
 *
 * @description
 *   システムの現在の状態、検出された故障、稼働時間等を
 *   統合的に管理。本モジュールの内部で維持され、
 *   外部モジュールは SafetyMgr_GetStatus() 等を通じてアクセス。
 */
typedef struct {
    SystemState_t   state;           /**< 現在のシステム状態 */
    FaultCode_t     fault_code;      /**< 最後に検出された故障コード */
    uint32_t        runtime_ms;      /**< システム稼働時間 [ms] */
    uint32_t        fault_timestamp; /**< 故障検出時刻 [ms] */
    uint8_t         diag_phase;      /**< 起動時診断フェーズ (0-4) */
    bool            restart_allowed; /**< 再スタート許可フラグ */
} SystemContext_t;

/*============================================================================*/
/* 関数プロトタイプ                                                          */
/*============================================================================*/

/**
 * @brief  システムを安全状態へ遷移させる
 * @req    UT-014
 * @req    SR-013 (故障時の安全状態遷移)
 * @safety_class クラスB
 * @test   TC-014 (TBD)
 *
 * @description
 *   任意の安全関連故障が検出された場合、
 *   以下の統一的な対応を実施する：
 *
 *   1. モータ停止
 *      - MotorCtrl_EmergencyStop() により PWM=0
 *      - CurrentMon_GateShutdown() により通電遮断
 *
 *   2. 外部表示
 *      - LED: 赤色点灯（警告表示）
 *      - BUZZER: 連続音（アラート）
 *
 *   3. 内部フラグ
 *      - restart_allowed = false （再スタート禁止）
 *      - state = STATE_SAFE_STATE
 *      - fault_code = faultCode（故障情報保持）
 *
 *   4. WDT
 *      - キック停止（タイムアウト待機）
 *
 *   スレッドセーフ性:
 *     - 複数の割込みコンテキスト、またはメインループから
 *       並行呼び出し可能。内部で排他制御を実施。
 *
 *   応答時間: <1ms (割込み禁止時間を最小化)
 *
 * @param  faultCode  故障コード (FaultCode_t enum)
 * @return void
 */
void SafetyMgr_TransitionSafe(FaultCode_t faultCode);

/**
 * @brief  現在のシステム状態を取得
 * @req    —
 * @safety_class —
 * @test   —
 *
 * @description
 *   システム状態、故障コード、稼働時間等を
 *   SystemContext_t 構造体で返す。
 *   ロック保護により、読み出し一貫性を確保。
 *
 * @return SystemContext_t  現在のシステムコンテキスト
 */
SystemContext_t SafetyMgr_GetStatus(void);

/**
 * @brief  故障コードから故障説明文字列を取得（デバッグ用）
 * @req    —
 * @safety_class —
 *
 * @description
 *   与えられた故障コードに対応する人間が読める説明文を
 *   返す。シリアル通信ログ出力、Web UI表示等に使用可能。
 *
 * @param  faultCode  故障コード
 * @return const char* 説明文字列（NULLターミネート）
 */
const char* SafetyMgr_FaultCodeToString(FaultCode_t faultCode);

/**
 * @brief  安全状態での LED/BUZZER 制御（内部用）
 * @req    —
 * @safety_class —
 *
 * @description
 *   SafetyMgr_TransitionSafe() 内部から呼び出される。
 *   SAFE_STATE LED点灯、BUZZER鳴動の制御。
 *   外部からは直接呼び出し不可能。
 *
 * @return void
 */
void SafetyMgr_SetAlertOutput(void);

/**
 * @brief  システム状態をシリアルポートに出力（デバッグ用）
 * @req    —
 * @safety_class —
 *
 * @description
 *   現在のシステム状態、故障情報、稼働時間等を
 *   人間が読める形式で UART 経由で出力。
 *   デバッグ・トラブルシューティング用。
 *
 * @return void
 */
void SafetyMgr_PrintStatus(void);

#endif /* SAFETY_MGR_H_ */
