---
name: validate
description: "Runs integrated validation checks for consistency, completeness, and cross-reference integrity."
argument-hint: "[scope: all|phase|docs]"
---

# /validate — プロジェクト一括検証

数値整合性・型ルール・ゲート条件を一括検証し、結果を報告する。

引数: $ARGUMENTS（オプション。"numbers" | "types" | "gates" | "all" → 省略時は "all"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |

## 検証項目

### 1. 数値整合性検証（numbers）
`scripts/num_check.sh` を実行し、以下を検証:
- RAM容量: 5.5KB / 5632B（8KB, 12KB でないこと）
- Flash容量: 64KB（128KB, 256KB でないこと）
- ADC周波数: 100Hz / 10Hz（1kHz, 10kHz でないこと）
- ADC分解能: 10bit（12bit でないこと）
- クロック公差: ±4%（±5% でないこと）
- CRCアルゴリズム: CRC32（CRC-16 でないこと）

追加で、生成済みの docs/ 配下の成果物と project.json の product_spec を突合する。

### 2. 型ルール検証（types）
`scripts/type_check.sh` を実行し、以下を検証（src/ 配下のCソースが対象）:
- T-01: bool/true/false/stdbool.h 禁止
- T-02: 固定幅整数型のみ（uint8_t, int32_t 等）
- T-03a: float の == 比較禁止
- T-03b: double 型禁止
- T-03c: math.h 禁止
- T-04: ブール値は uint8_t + STD_TRUE/STD_FALSE
- T-05: enum は明示的値付与必須
- T-06: union 禁止
- T-07: 動的メモリ確保禁止（malloc, calloc, free）
- T-08: 再帰呼び出し禁止
- T-09: goto 禁止
- T-10: 暗黙型変換禁止

### 3. ゲート条件検証（gates）
project.json の phases を走査し、completed フェーズについて:
- G-01: INPUT ファイルが存在するか
- G-02: OUTPUT ファイルが存在するか
- G-03: 要件IDに重複がないか（SG, FSR, TSR, SR, TC）
- G-04: 双方向トレーサビリティの欠損がないか
- G-05: 数値が project.json と矛盾していないか
- G-06: project.json が最新か
- G-07: process_records が更新されているか

## 出力フォーマット

```
=== TORASAN 検証レポート ===
実行日時: {timestamp}
対象: {検証範囲}

[数値整合性] {PASS|FAIL}
  ✓ RAM容量: 5.5KB / 5632B ... OK
  ✗ ADC周波数: docs/XX に 1kHz の記述あり ... NG
  結果: {pass_count}/{total} PASS

[型ルール] {PASS|FAIL|SKIP}
  ✓ T-01: bool禁止 ... OK (0 violations)
  結果: {pass_count}/{total} PASS

[ゲート条件] {PASS|FAIL|SKIP}
  PH-01: {gate_results}
  結果: {pass_count}/{total} PASS

[総合判定] {ALL PASS | {fail_count}件の問題あり}
```

## 問題検出時の動作
- 問題箇所をファイル名:行番号で具体的に示す
- 修正案を提案する
- 自動修正可能なものは「自動修正しますか？」と確認

## 注意事項
- src/ にCソースがない場合、型ルール検証はスキップ
- completed でないフェーズのゲート検証はスキップ
- スクリプトが実行できない環境では手動チェックにフォールバック

## 関連スキル
- /health-check — プロジェクト健全性の包括的チェック
- /trace — トレーサビリティマトリクスの検証
- /config-audit — 構成管理の監査

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/misra_c_2012.md` — MISRA C:2012 ルール詳細、型ルール根拠
