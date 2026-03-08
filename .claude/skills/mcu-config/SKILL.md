---
name: mcu-config
description: "Generates MCU peripheral configuration code for Renesas RL78/G14."
argument-hint: "[peripheral: adc|wdt|timer|gpio|uart|clock|all (default: all)]"
disable-model-invocation: true
---

# /mcu-config — MCU ペリフェラル初期化コード生成

project.json の product_spec から RL78/G14 のペリフェラル設定コード（Cマクロ・初期化関数）を生成する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "all"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | product_spec の必要フィールドが定義済み | project.json 内を確認 | 必要フィールドを設定してから再実行 |
| 3 | 前段フェーズが完了済み | phases[] の status 確認 | 該当フェーズを先に `/execute-phase` で実行 |
| 4 | 出力先ディレクトリが存在する | `ls` で確認 | 自動作成（mkdir -p） |

## 対象 MCU
- **型番**: Renesas RL78/G14 (R5F104BG)
- **Flash**: 64KB / **RAM**: 5.5KB (5,632B)
- **メインクロック**: 32MHz / **サブクロック**: 15kHz

## 手順

### Step 1: パラメータ抽出
project.json の product_spec から以下を読み取り:

```c
/* === MCU Configuration (auto-generated from project.json) === */
/* Clock */
#define MCU_MAIN_CLOCK_MHZ      (32U)
#define MCU_SUB_CLOCK_KHZ       (15U)
#define MCU_CLOCK_TOLERANCE_PCT (4U)

/* Memory */
#define MCU_FLASH_KB            (64U)
#define MCU_RAM_KB_X10          (55U)  /* 5.5KB → 整数表現 */
#define MCU_RAM_BYTES           (5632U)

/* ADC */
#define ADC_RESOLUTION_BITS     (10U)
#define ADC_MAX_VALUE           (1023U)
#define ADC_VREF_MV             (5000U)
#define ADC_CURRENT_SAMPLE_HZ   (100U)
#define ADC_VOLTAGE_SAMPLE_HZ   (10U)

/* WDT */
#define WDT_TIMEOUT_MS          (100U)
#define WDT_KICK_PERIOD_MS      (50U)

/* Voltage Monitor */
#define VOLTAGE_LOWER_LIMIT_MV  (4500U)
#define VOLTAGE_UPPER_LIMIT_MV  (5500U)

/* Motor */
#define MOTOR_RATED_RPM         (1200U)
#define MOTOR_OVERSPEED_RPM     (1500U)
#define MOTOR_RATED_MA          (6000U)
#define MOTOR_MAX_MA            (8000U)
```

### Step 2: 初期化関数生成
対象ペリフェラルごとに MISRA C 準拠の初期化コードを生成:

- **clock**: クロック設定（メイン32MHz、サブ15kHz、PLL構成）
- **adc**: ADC 初期化（10bit、5V基準、サンプリング周期設定）
- **wdt**: WDT 設定（100msタイムアウト、キック関数）
- **timer**: タイマ設定（1ms ティック、PWM 用タイマ）
- **gpio**: GPIO ピンアサイン（モータ出力、センサ入力、LED出力）
- **uart**: UART 初期化（デバッグ用、ボーレート設定）

### Step 3: §5 型ルール準拠チェック
生成コードが以下を満たすことを確認:
- T-01: bool 未使用 → uint8_t + STD_TRUE/STD_FALSE
- T-02: 固定幅整数型のみ
- T-03: float の == 比較なし、double 未使用
- T-07: malloc/calloc/free 未使用
- 全リテラルに U/UL サフィックス付与

### Step 3.5: 既存ファイル確認・バックアップ

出力先 `src/config/` に既存ファイル（mcu_config.h, mcu_init.c）がある場合:
1. 既存ファイルの内容を Read で確認
2. バックアップ作成（`cp {file} {file}.bak`）
3. diff を生成してユーザーに提示
4. ユーザーの明示承認後に Write で上書き

> 参照: error_prevention.md SS6「成果物作成時チェックリスト」EP-B

### Step 4: 出力
```
=== MCU コンフィグ生成完了 ===
対象: {peripheral_list}
出力ファイル:
  - src/config/mcu_config.h（マクロ定義）
  - src/config/mcu_init.c（初期化関数）
数値ソース: project.json product_spec
型ルール: §5 準拠確認済
```

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json 読み取り失敗 | Read エラー | パスと構文を確認。`git checkout -- project.json` で復元 |
| product_spec フィールド欠落 | キー不在 / null | 必要フィールドを報告し設定を促す。TBD マーカー付きで続行可 |
| 既存ファイル上書き競合 | Read で既存ファイル検出 | `cp {file} {file}.bak` → diff 表示 → ユーザー承認後に上書き |
| TBD 未解消（安全クリティカル） | TBD チェック | **生成を中止**。未解消 TBD を報告 |
| WDT 設定値矛盾（timeout_ms ≦ kick_period_ms） | product_spec.wdt の値比較 | 設定値を表示し**生成を中止**。WDT タイムアウトがキック周期より大きいことを確認後に再実行 |
| クロック分周計算オーバーフロー | 分周比が uint16_t 範囲超 | 計算式と結果値を表示し**生成を中止**。clock_mhz / 目標周波数の組合せを見直し |
| ADC 変換値が 10bit 上限超過 | `(voltage_mv × 1023) / vref_mv > 1023` | 1023 にクランプし**警告**出力。電圧範囲が ADC 基準電圧を超えている旨を報告 |

## 注意事項
- 数値は全て project.json から取得（ハードコーディング禁止）
- RL78/G14 固有のレジスタ名は Renesas 命名規則に従う
- 既存の src/archive_v1/ のヘッダと互換性を保つ
- 生成先ディレクトリがない場合は作成

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/safety_diagnostics.md` — ペリフェラル自己テスト要件、WDT コンフィグレーション設計

## 関連スキル
- `/driver-gen` — BSW/HAL ドライバコード生成
- `/motor-control` — BLDC モータ制御コード生成
- `/memory-map` — メモリマップ管理・リソース計算
- `/safety-diag` — 安全診断機能実装
