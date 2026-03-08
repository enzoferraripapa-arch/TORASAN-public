---
name: safety-diag
description: "Designs safety diagnostic coverage for IEC 60730 / ISO 26262 with DC and SFF calculations."
argument-hint: "[target: ram|rom|cpu|clock|voltage|wdt|all (default: all)]"
disable-model-invocation: true
---

# /safety-diag — IEC 60730 Annex H 自己診断コード生成

IEC 60730 Class B 準拠の自己診断ルーチンを生成する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "all"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | product_spec の必要フィールドが定義済み | project.json 内を確認 | 必要フィールドを設定してから再実行 |
| 3 | 前段フェーズが完了済み | phases[] の status 確認 | 該当フェーズを先に `/execute-phase` で実行 |
| 4 | 出力先ディレクトリが存在する | `ls` で確認 | 自動作成（mkdir -p） |

## 診断項目と仕様（TSR-008〜015 / FMEA-005〜010）

### 1. CPU レジスタテスト（TSR-008）
- **目的**: レジスタ固着・制御フロー異常の検出
- **方式**: テストパターン書込→読戻→比較
- **パターン**: 0x55AA, 0xAA55, 0xFFFF, 0x0000
- **タイミング**: 起動時（全レジスタ）+ 実行時（100ms 周期、分割）
- **FMEA**: FMEA-005, DC=95%
- **生成コード**: `SafetyDiag_CpuRegTest()`

### 2. RAM March C テスト（TSR-009）
- **目的**: RAM の stuck-at / transition / coupling fault 検出
- **アルゴリズム**: March C（6パス: ⇑w0, ⇑r0w1, ⇑r1w0, ⇓r0w1, ⇓r1w0, ⇑r0）
- **パラメータ**（project.json product_spec.ram_test）:
  - ブロックサイズ: 256B
  - ブロック数: 22（= 5,632B / 256B）
  - 全周期: 2.2秒
- **タイミング**: 起動時（全域）+ 実行時（100ms 周期、1ブロックずつ巡回）
- **FMEA**: FMEA-006, DC=95%
- **生成コード**: `SafetyDiag_RamMarchC(block_index)`

```c
/* March C テスト — 1ブロック処理 */
typedef struct {
    volatile uint8_t* base_addr;
    uint16_t          block_size;   /* 256U */
    uint8_t           block_count;  /* 22U */
    uint8_t           current_block;
    uint8_t           backup[256];  /* 退避バッファ */
} MarchC_Context_t;
```

### 3. ROM CRC32 検証（TSR-010）
- **目的**: Flash メモリ内容の改ざん・劣化検出
- **アルゴリズム**: CRC-32（多項式: 0x04C11DB7）
- **期待値格納**: Flash 末尾（ROM_CRC32_EXPECTED_ADDR = 0xFFFFF000U, TBD）
- **タイミング**: 起動時（全域スキャン）+ 実行時（100ms 周期、4KBチャンク）
- **FMEA**: FMEA-007, DC=99%
- **生成コード**: `SafetyDiag_RomCrc32(chunk_index)`

### 4. クロックモニタ（TSR-011）
- **目的**: メインクロック異常（停止・周波数逸脱）の検出
- **方式**: サブクロック(15kHz)を基準にメインクロック(32MHz)のカウント比較
- **許容公差**: ±4%（project.json clock_tolerance_pct）
- **タイミング**: 100ms 周期
- **FMEA**: FMEA-008
- **生成コード**: `SafetyDiag_ClockMonitor()`

### 5. 電圧モニタ（TSR-013/014）
- **目的**: 電源電圧の異常（過電圧/不足電圧）検出
- **方式**: ADC サンプリング（10Hz）+ ウィンドウ比較
- **範囲**: 4.5V〜5.5V（project.json voltage_monitor）
- **FMEA**: FMEA-010, DC=60%
- **生成コード**: `SafetyDiag_VoltageMonitor()`

```c
/* 電圧判定 — ADC値変換 */
/* adc_value = (voltage_mv * ADC_MAX_VALUE) / ADC_VREF_MV */
#define VOLTAGE_LOWER_ADC  ((uint16_t)((4500UL * 1023UL) / 5000UL))  /* 921 */
#define VOLTAGE_UPPER_ADC  ((uint16_t)((5500UL * 1023UL) / 5000UL))  /* 1125 → clamp 1023 */
```

### 6. WDT 管理（TSR-015）
- **目的**: メインループ停止の検出→強制リセット
- **タイムアウト**: 100ms / **キック周期**: 50ms
- **生成コード**: `WdtMgr_Init()`, `WdtMgr_Kick()`

## 生成手順

### Step 1: パラメータ確認
project.json の product_spec から全数値を取得

### Step 1.5: TBD ブロッカー確認

以下の TBD 項目が全て解消されているか確認。**未解消の場合は生成を中止**し、
ユーザーに報告すること。

- [ ] `ROM_CRC32_EXPECTED_ADDR` — リンカスクリプトで確定済みか
- [ ] `SafetyMgr_TransitionSafe()` 遷移パスの実装が存在するか

TBD が残存する場合、生成コード内に `#error "TBD: ..."` を使用しコンパイルエラーを強制する。
仮アドレスを埋め込んだまま生成することは禁止。

### Step 2: コード生成
1. ヘッダファイル: `src/safety/safety_diag.h`（関数プロトタイプ、マクロ定義）
2. ソースファイル: `src/safety/safety_diag.c`（実装）
3. §5 型ルール厳守（固定幅整数、明示キャスト、Uサフィックス）

### Step 2.5: 既存ファイル確認・バックアップ

出力先 `src/safety/` に既存ファイル（safety_diag.h, safety_diag.c）がある場合:
1. 既存ファイルの内容を Read で確認
2. バックアップ作成（`cp {file} {file}.bak`）
3. diff を生成してユーザーに提示
4. ユーザーの明示承認後に Write で上書き

> 参照: error_prevention.md SS6「成果物作成時チェックリスト」EP-B

### Step 3: テストケース生成
各診断に対して:
- 正常系: 診断 PASS を確認
- 異常系: 故障注入シミュレーション（意図的にパターン不一致を発生させ検出を確認）

### Step 4: 出力
```
=== 自己診断コード生成完了 ===
対象: {diagnostic_list}
出力:
  - src/safety/safety_diag.h
  - src/safety/safety_diag.c
TSRカバレッジ: TSR-008〜015
FMEAマッピング: FMEA-005〜010
```

## 関連スキル
- /fmea — FMEA で特定した故障モードに対する診断設計の根拠
- /safety-concept — 安全メカニズムの上流設計（FSR/TSR）
- /mcu-config — MCU ペリフェラル設定（WDT、ADC、タイマ）との整合
- /motor-control — モータ制御系の診断（過速度・過電流検出）との連携

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json 読み取り失敗 | Read エラー | パスと構文を確認。`git checkout -- project.json` で復元 |
| product_spec フィールド欠落 | キー不在 / null | 必要フィールドを報告し設定を促す。TBD マーカー付きで続行可 |
| 既存ファイル上書き競合 | Read で既存ファイル検出 | `cp {file} {file}.bak` → diff 表示 → ユーザー承認後に上書き |
| TBD 未解消（安全クリティカル） | TBD チェック | **生成を中止**。未解消 TBD を報告 |
| ROM_CRC32_EXPECTED_ADDR 未確定 | product_spec に TBD / リンカスクリプト未確定 | **生成を中止**。`#error` 付き仮コード出力も禁止。リンカ確定後に再実行 |
| SafetyMgr_TransitionSafe() 未実装 | src/ 内に関数定義が存在しない | **生成を中止**。先に `/driver-gen dem` で DEM + SafetyMgr を生成 |
| DC 合計が ASIL B 要求未達 | 各診断の DC 値合算で判定 | 不足メカニズムと現在 DC を報告し**生成を中止**。`/safety-concept mechanism` で追加対策を検討 |
| March C ブロック数と RAM 容量の不整合 | `block_size × block_count ≠ ram_b` | 正しい値を計算して提示。product_spec.ram_test の修正を促す |

## 注意事項
- March C テスト中は割り込みを禁止（テスト対象RAMへのアクセス防止）
- CRC32 の期待値アドレスは TBD — リンカスクリプトで確定後に更新
- 全診断は SafetyMgr_TransitionSafe() への遷移パスを持つこと

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/safety_diagnostics.md` — Annex H Table H.1 診断項目、March C-Minus アルゴリズム、CRC-32 多項式、テストタイミング設計
- `.claude/knowledge/iso26262_iec60730.md` — DC カテゴリ定義と診断カバレッジ目標値
