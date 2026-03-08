/**
 * @file   safety_diag.h
 * @brief  安全関連自己診断モジュール ヘッダファイル
 * @req    SA-005 (安全診断サブシステム)
 * @safety_class クラスB
 * @project WMC (Washing Machine Motor Control)
 * @standard IEC 60730 Class B / Annex H
 * @version 1.0
 * @date    2026-02-27
 */

#ifndef SAFETY_DIAG_H_
#define SAFETY_DIAG_H_

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*/
/* 型定義                                                                    */
/*============================================================================*/

/**
 * @brief 診断結果ステータス
 */
typedef enum {
    DIAG_PASS         = 0x00U,  /* テスト成功 */
    DIAG_FAIL         = 0x01U,  /* テスト失敗 */
    DIAG_NOT_READY    = 0x02U,  /* テスト不可（リソース使用中） */
} DiagStatus_t;

/*============================================================================*/
/* 定数定義                                                                  */
/*============================================================================*/

/** @brief CPU Register テストパターン個数 */
#define CPU_REG_TEST_PATTERNS       (4U)

/** @brief RAM March C テストの段数 */
#define RAM_MARCH_C_PASSES          (6U)

/** @brief ROM CRC32 期待値格納位置（アドレス） */
#define ROM_CRC32_EXPECTED_ADDR     (0xFFFFF000U)  /* TBD: 実機設定 */

/** @brief RL78/G14 SRAM 開始アドレス */
#define SRAM_START_ADDR             (0xFF700U)  /* RL78/G14 internal RAM start */

/** @brief RL78/G14 SRAM サイズ [bytes] (5.5KB = 5632B) */
#define SRAM_SIZE_BYTES             (5632U)

/** @brief RAM自己テスト ブロック数 (5632B / 256B = 22 blocks) */
#define RAM_TEST_BLOCK_COUNT        (22U)

/** @brief クロック監視の許容偏差 [%] */
#define CLOCK_TOLERANCE_PERCENT     (4U)

/** @brief クロック監視の参照周波数 [Hz] */
#define CLOCK_REF_FREQUENCY         (15000U)  /* 15kHz LOCO内蔵オシレータ */

/** @brief クロック監視の評価時間 [ms] */
#define CLOCK_EVAL_PERIOD_MS        (100U)

/*============================================================================*/
/* 関数プロトタイプ — 起動時診断                                             */
/*============================================================================*/

/**
 * @brief  CPUレジスタの機能テストを実行
 * @req    UT-010
 * @req    SR-009 (CPU制御フローの障害検出)
 * @safety_class クラスB
 * @test   TC-010 (TBD)
 *
 * @description
 *   IEC 60730 Annex H.3.1: CPU Register Test
 *   全般用途レジスタ(R0-R31等)に対して、
 *   パターンテスト（0x55AA, 0xAA55, 0xFFFF, 0x0000 など）を実施。
 *   アセンブリで実装し、テスト中の割込み禁止を確保。
 *   起動時1回実行 + ランタイム定期実行可能。
 *
 *   動作時間: 典型値 <1ms
 *   応答遅延: N/A (起動直後実行)
 *
 * @return DiagStatus_t: DIAG_PASS / DIAG_FAIL
 */
DiagStatus_t SafeDiag_CpuRegTest(void);

/**
 * @brief  RAM全体を March C アルゴリズムでテストする
 * @req    UT-011
 * @req    SR-010 (RAM障害の検出)
 * @safety_class クラスB
 * @test   TC-011 (TBD)
 *
 * @description
 *   IEC 60730 Annex H.3.2: Static Memory Test (March C)
 *   March C: 6N+4のメモリアクセスで以下を検出：
 *     - stuck-at fault (位が常に0または1)
 *     - transition fault (ビット遷移の失敗)
 *     - coupling fault (ビット間の相互作用)
 *
 *   テスト対象: スタック領域および静的RAM領域。
 *   システムRAM全体の必要割合（>90%）を満たす設計。
 *
 *   March C のパス:
 *     1. Ascending  Write 0, Read 0
 *     2. Ascending  Write 1, Read 1, Write 0, Read 0
 *     3. Descending Write 1, Read 1, Write 0, Read 0
 *     4. Descending Write 1, Read 1
 *
 *   動作時間: 例：32KB RAM → 典型値 <10ms
 *
 * @return DiagStatus_t: DIAG_PASS / DIAG_FAIL
 */
DiagStatus_t SafeDiag_RamTest(void);

/**
 * @brief  ROM内プログラム領域の CRC32 チェックサムを検証
 * @req    UT-012
 * @req    SR-011 (ROM障害の検出)
 * @safety_class クラスB
 * @test   TC-012 (TBD)
 *
 * @description
 *   IEC 60730 Annex H.3.3: ROM Check
 *   プログラムROM領域に対して、CRC32チェックサムを計算し、
 *   起動時に保存された期待値（ROM_CRC32_EXPECTED_ADDR 位置）と比較。
 *   不一致時は即座に SafetyMgr_TransitionSafe(FAULT_ROM) を呼び出す。
 *
 *   チェックサム格納位置: ROM末尾固定セクション（TBD）
 *   アルゴリズム: CRC-32-MPEG-2 (polynomial: 0x04C11DB7)
 *   初期値: 0xFFFFFFFF
 *   最終XOR: 0xFFFFFFFF
 *
 *   動作時間: 例：64KB ROM → 典型値 <20ms
 *   起動時1回実行 + ランタイム定期実行可能（100ms周期）
 *
 * @return DiagStatus_t: DIAG_PASS / DIAG_FAIL
 */
DiagStatus_t SafeDiag_RomCrcTest(void);

/**
 * @brief  システムクロックの周波数を監視する
 * @req    UT-013
 * @req    SR-012 (クロック異常の検出)
 * @safety_class クラスB
 * @test   TC-013 (TBD)
 *
 * @description
 *   IEC 60730 Annex H.3.5: Clock Monitoring
 *   外部リファレンスクロック(またはwatch dog timer)を用いて、
 *   システムクロック周波数が想定範囲内であることを確認。
 *   許容偏差: ±4% (CLOCK_TOLERANCE_PERCENT)
 *
 *   実装方法（選択肢）:
 *     A. 外部クリスタルを使用し、內蔵PLL周波数のカウント比較
 *     B. WDT周波数（通常は安定した低速OSC）との同期
 *
 *   評価時間: 100ms (CLOCK_EVAL_PERIOD_MS)
 *   起動時1回 + 100ms周期での定期実行
 *
 * @return DiagStatus_t: DIAG_PASS / DIAG_FAIL
 */
DiagStatus_t SafeDiag_ClockTest(void);

/*============================================================================*/
/* 関数プロトタイプ — 分割実行版（Startup / Runtime）                        */
/*============================================================================*/

/** @brief RAM起動時テスト（全5632Bフルスキャン） */
DiagStatus_t SafeDiag_RamTest_Startup(void);

/** @brief RAMランタイムテスト（256B/ブロック分割） */
DiagStatus_t SafeDiag_RamTest_Runtime(void);

/** @brief ROM起動時CRC32テスト（全64KBフルスキャン） */
DiagStatus_t SafeDiag_RomCrcTest_Startup(void);

/** @brief ROMランタイムCRC32テスト（4KB/ブロック分割） */
DiagStatus_t SafeDiag_RomCrcTest_Runtime(void);

/*============================================================================*/
/* 関数プロトタイプ — ランタイム診断                                         */
/*============================================================================*/

/**
 * @brief  ランタイム自己診断を実行（分割実行用）
 * @req    —
 * @safety_class クラスB
 *
 * @description
 *   起動時テストの後、周期的（100ms）に実行可能。
 *   内部で診断フェーズを管理し、毎回異なるテストを実行して
 *   メインループの負荷を分散。
 *
 *   例: Phase 0→CPU, Phase 1→RAM(分割), Phase 2→Clock, Phase 3→ROM(分割)
 *
 *   故障検出時は即座に SafetyMgr_TransitionSafe() を呼び出す。
 *
 * @return DiagStatus_t: DIAG_PASS / DIAG_FAIL
 */
DiagStatus_t SafeDiag_RuntimeTest(void);

/*============================================================================*/
/* ユーティリティ関数                                                        */
/*============================================================================*/

/** @brief 診断初期化 */
void SafeDiag_Init(void);

/** @brief 起動時テスト完了確認 */
DiagStatus_t SafeDiag_StartupTestsComplete(void);

/** @brief 起動時テスト完了マーク */
void SafeDiag_MarkStartupComplete(void);

/** @brief 診断結果のシリアル出力（デバッグ用） */
void SafeDiag_PrintStatus(void);

#endif /* SAFETY_DIAG_H_ */
