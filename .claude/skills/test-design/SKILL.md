---
name: test-design
description: "Designs unit test specifications with test case generation."
argument-hint: "[mode: strategy|cases|boundary|equivalence|negative|all (default: strategy)]"
---

# /test-design — テスト戦略・テストケース設計

PH-11（SWテスト仕様・実施）を支援するテスト設計専門スキル。
テスト戦略策定・テストケース生成・テスト手法選定を行う。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。"cases" は `cases SR-001` のように要件IDを付与。省略時は "strategy"）

> **数値参照ルール**: 本スキル内の数値（rpm、mA、ms 等）は project.json product_spec からの**例示値**です。
> 実行時は必ず `project.json` を Read して最新値を使用してください。ハードコード値をそのまま出力に使用しないこと。

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |

## テスト階層（V-model 右側）

```
                    PH-13 システムテスト
                   ┌─────────────────────┐
                   │ SG/FSR に対する検証   │
                   └──────────┬──────────┘
                              │
                   PH-12 HW統合テスト
                   ┌──────────┴──────────┐
                   │ HW+SW 統合動作検証    │
                   └──────────┬──────────┘
                              │
              PH-11 SWテスト（本スキルの主対象）
         ┌────────────────────┼────────────────────┐
         │                    │                    │
    SWE.6 SW適格性テスト  SWE.5 SW統合テスト  SWE.4 SWユニットテスト
    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
    │ SR全体の検証  │    │ モジュール間IF │    │ 各関数の検証  │
    └─────────────┘    └─────────────┘    └─────────────┘
```

## 手順

### strategy モード — テスト戦略策定
1. project.json の standard を確認（ASIL B → テスト要求レベル決定）
2. テスト戦略を定義:

```
=== テスト戦略 ===
規格: IEC 60730 Class B / ISO 26262 ASIL B

[ユニットテスト (SWE.4)]
  手法: 要求ベース + 構造カバレッジ
  カバレッジ目標:
    - 文カバレッジ: 100%（ASIL B 推奨）
    - 分岐カバレッジ: 100%（ASIL B 必須）
    - MC/DC: 安全関連関数のみ（ASIL B 推奨）
  ツール: Unity テストフレームワーク（組込み向け）
  環境: ホスト PC 上のエミュレーション + ターゲット実機

[統合テスト (SWE.5)]
  手法: インターフェーステスト + 機能テスト
  対象: モジュール間のデータフロー・制御フロー
  重点: 層間IF（APP→BSW→HAL）、割り込みタイミング

[適格性テスト (SWE.6)]
  手法: 要求ベーステスト（SR 全件に対するTC）
  対象: SW安全要求(SR)の全件検証
  カバレッジ: 要件カバレッジ 100%
```

### cases モード — テストケース生成
指定された要件(SR-XXX)に対してテストケースを生成:

```
TC-{XXX}: {テストケース名}
  対象要件: SR-{XXX}
  テストレベル: ユニット / 統合 / 適格性
  前提条件: {setup}
  入力: {input values}
  実行手順:
    1. {step}
    2. {step}
  期待結果: {expected output}
  判定基準: {pass/fail criteria}
  カテゴリ: 正常系 / 境界値 / 異常系
  優先度: 高 / 中 / 低
```

各要件に対して最低3パターン:
- **正常系**: 定格値での動作確認
- **境界値**: 閾値付近（閾値-1, 閾値, 閾値+1）
- **異常系**: 範囲外入力、故障注入

### boundary モード — 境界値分析
project.json product_spec から境界値を自動抽出:

> ※ 以下は例示。実行時は product_spec の実値から動的に生成すること。

| パラメータ | product_spec パス | 下限 | 定格 | 上限 | 単位 |
|-----------|------------------|------|------|------|------|
| モータ速度 | `motor.rated_rpm` / `motor.overspeed_rpm` | 0 | {rated_rpm} | {overspeed_rpm} | rpm |
| モータ電流 | `motor.max_ma` | 0 | {rated_a×1000} | {max_ma} | mA |
| 電源電圧 | `voltage_monitor.min_v` / `max_v` | {min_v×1000} | {vref_v×1000} | {max_v×1000} | mV |
| ADC値 | `adc.bits` | 0 | — | {2^bits - 1} | LSB |
| WDTキック | `wdt.kick_ms` / `wdt.timeout_ms` | — | {kick_ms} | {timeout_ms} | ms |

各境界値に対して TC を自動生成

### equivalence モード — 同値分割
入力ドメインを有効クラス・無効クラスに分割:

> ※ 境界値は product_spec から取得。以下は例示。

| 入力 | 有効クラス | 無効クラス(下) | 無効クラス(上) | product_spec |
|------|----------|-------------|-------------|-------------|
| RPM | [0, {overspeed_rpm}] | [-∞, -1] | [{overspeed_rpm}+1, +∞] | `motor.overspeed_rpm` |
| 電流 mA | [0, {max_ma}] | [-∞, -1] | [{max_ma}+1, +∞] | `motor.max_ma` |
| ADC | [0, {2^bits-1}] | — | [{2^bits}, +∞] | `adc.bits` |
| GPIO | {0, 1} | — | [2, +∞] | — |

各クラスから代表値を1つ選びTCを生成

### negative モード — 異常系テスト
安全関連の異常シナリオ:

> ※ 閾値は product_spec から取得。以下の数値は例示。

| # | 異常シナリオ | 注入方法 | 期待動作 | SG |
|---|------------|---------|---------|-----|
| 1 | 過速度 ({overspeed_rpm}+1) | テスト用速度値設定 | EmergencyStop起動 | SG-001 |
| 2 | 過電流 ({max_ma}+1) | テスト用ADC値注入 | SafetyMgr→FAULT | SG-002 |
| 3 | 蓋開放 | GPIO = Open | モータ停止(≦{lid_response_ms}ms) | SG-003 |
| 4 | RAM bit化け | March Cテスト用パターン破壊 | 診断FAIL→FAULT | SG-005 |
| 5 | ROM CRC不一致 | CRC期待値を書替え | 診断FAIL→FAULT | SG-005 |
| 6 | クロック逸脱 | テスト用カウント値注入 | 診断FAIL→FAULT | SG-005 |
| 7 | 電圧下限割れ | ADC値 < {min_v×1000}mV相当 | FAULT遷移 | SG-006 |
| 8 | WDTタイムアウト | キック停止 | HWリセット | SG-005 |

## 出力
```
=== テスト設計結果 ===
テスト戦略: {策定済/未策定}
テストケース:
  正常系: {count}件
  境界値: {count}件
  異常系: {count}件
  合計: {count}件
要件カバレッジ: {covered_sr}/{total_sr} SR ({pct}%)
出力ファイル: docs/11_test_spec.md
```

## 関連スキル
- /test-coverage — テストカバレッジ分析（カバレッジギャップの特定）
- /sw-design — SW 設計（テスト対象の設計情報）
- /srs-generate — SW安全要求（テスト対象の要件）
- /static-analysis — 静的解析（テストと併用してコード品質確保）

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| 入力データ不足 | 必要ファイル/キーの不在 | 不足項目を報告し、前提スキルの実行を促す |
| 処理中断 | 予期しないエラー | エラー内容を報告。変更途中のファイルがあれば `git checkout` で復元 |

## 注意事項
- TC-ID は §7 の命名規則に従い連番
- 各 TC は必ず SR（またはTSR/FSR/SG）にトレース
- 境界値の数値は project.json product_spec から取得
- 異常系テストは安全目標(SG)に紐付ける
- ASIL B では分岐カバレッジ 100% が必須

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/iso26262_iec60730.md` — カバレッジ要求一覧、ASIL B テスト手法の推奨/必須区分
