---
name: safety-verify
description: "Verifies safety case completeness using GSN and checks evidence coverage."
argument-hint: "[mode: verify|case|report|gap|all (default: all)]"
disable-model-invocation: true
---

# /safety-verify — 機能安全検証

PH-14（機能安全検証）を支援し、安全論証（Safety Case）の構築を含む安全検証レポートを生成する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "all"）

> **数値参照ルール**: 製品名は `project.json:description`、ASIL は `standard.asil` / `standard.product_override_class` から取得。
> DC 目標値は規格由来（ナレッジ `iso26262_iec60730.md` 参照）。本スキル内の数値は例示値。

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |

## 安全検証の位置づけ

```
PH-03 HARA → SG定義
PH-04 FSC  → FSR定義        ──→ PH-14 安全検証 ★本スキル
PH-05 TSC  → TSR定義        ──→   全要件が検証されたか確認
PH-08 SRS  → SR/DR定義      ──→   全テストがPASSしたか確認
PH-11 SWテスト → TC実施     ──→   安全論証が成立するか判定
PH-13 システムテスト → ST実施
```

## 手順

### verify モード — 安全要件検証
全安全要件チェーンを検証:

#### Step 1: 要件完備性
```
[要件チェーン完備性]
SG → FSR: 全SG が FSR に展開されているか
FSR → TSR: 全FSR が TSR に展開されているか
TSR → SR/DR: 全TSR が SR/DR に展開されているか
SR/DR → TC/ST: 全SR/DR がテストケースでカバーされているか
```

#### Step 2: テスト結果検証
```
[テスト結果]
| テストレベル | 総数 | PASS | FAIL | 未実施 | 合格率 |
|------------|------|------|------|-------|-------|
| ユニットテスト (SWE.4) | {n} | {n} | {n} | {n} | {pct}% |
| 統合テスト (SWE.5) | {n} | {n} | {n} | {n} | {pct}% |
| SW適格性テスト (SWE.6) | {n} | {n} | {n} | {n} | {pct}% |
| システムテスト (SYS.5) | {n} | {n} | {n} | {n} | {pct}% |
```

#### Step 3: 安全メカニズム検証
```
[安全メカニズム有効性]
| メカニズム | DC目標 | DC実測/分析 | 判定 |
|-----------|-------|-----------|------|
| WDT | 90% | {val}% | PASS/FAIL |
| RAM March C | 95% | {val}% | PASS/FAIL |
| ROM CRC32 | 99% | {val}% | PASS/FAIL |
| クロックモニタ | 90% | {val}% | PASS/FAIL |
| 電圧モニタ | 60% | {val}% | PASS/FAIL |
```

#### Step 4: ASIL 適合判定
```
[ASIL B 適合判定]
| 項目 | 要求 | 実績 | 判定 |
|------|------|------|------|
| 要件カバレッジ | 100% | {val}% | {P/F} |
| 分岐カバレッジ | 100% | {val}% | {P/F} |
| 安全メカニズムDC | ≧60% | {val}% | {P/F} |
| テスト合格率(安全) | 100% | {val}% | {P/F} |
| FTTI余裕 | >0 | {val}ms | {P/F} |
| TBD残数(CERTIFY時) | 0 | {val} | {P/F} |
```

### case モード — Safety Case 構築
GSN（Goal Structuring Notation）的な安全論証を構築:

```
=== Safety Case 構造 ===

[G1] {project.json:description} は {standard.product_override} {standard.product_override_class} に適合する
  │
  ├─[S1] 戦略: ハザード分析に基づく安全目標の網羅的検証
  │  │
  │  ├─[G1.1] 全安全目標(SG)が特定され、FSRに展開されている
  │  │  └─[E1.1] docs/03_hara.md, docs/04_fsc.md（SG→FSR マトリクス）
  │  │
  │  ├─[G1.2] 全安全要求が SW/HW に割り当てられ実装されている
  │  │  ├─[E1.2a] docs/05_tsc.md（TSR一覧）
  │  │  ├─[E1.2b] docs/08_srs.md（SR/DR一覧）
  │  │  └─[E1.2c] src/（実装コード）
  │  │
  │  ├─[G1.3] 全安全要求がテストで検証されている
  │  │  ├─[E1.3a] docs/11_test_spec.md（TC一覧 + 結果）
  │  │  ├─[E1.3b] docs/13_system_test.md（ST一覧 + 結果）
  │  │  └─[E1.3c] テストカバレッジレポート
  │  │
  │  └─[G1.4] 安全メカニズムが十分な検出カバレッジを提供している
  │     ├─[E1.4a] docs/FMEA.md（DC分析）
  │     └─[E1.4b] 診断テスト結果
  │
  ├─[S2] 戦略: プロセス準拠による品質保証
  │  ├─[G2.1] ISO 26262 / IEC 60730 プロセスに準拠して開発された
  │  │  └─[E2.1] process_records/（全15プロセスの実施記録）
  │  │
  │  └─[G2.2] Automotive SPICE Level 2 を達成している
  │     └─[E2.2] spice_assessment（PA評価結果）
  │
  └─[A1] 前提: product_spec の仕様が正確である
     └─[J1] 正当化: 数値整合性検証（/validate）で確認済
```

### report モード — 安全検証レポート生成
docs/14_safety_verification.md を生成:

```markdown
# 機能安全検証レポート

## 1. 検証概要
## 2. 安全要件トレーサビリティ検証結果
## 3. テスト結果サマリ
## 4. 安全メカニズム有効性評価
## 5. ASIL B 適合判定
## 6. Safety Case（安全論証）
## 7. 残課題・制約事項
## 8. 検証結論
## 9. 変更履歴
```

### gap モード — ギャップ分析
安全検証に不足している項目を特定:

```
=== 安全検証ギャップ ===
[Critical — 検証未完了]
  ⚠ SG-005: MCU異常テスト未実施（実機必要）
  ⚠ DR-003: クロックモニタの DC 未確定

[High — エビデンス不足]
  ⚠ 分岐カバレッジ実測値なし（gcov 未実行）
  ⚠ FMEA-010 の DC=60% が Class B 下限

[推奨アクション]
  1. /systest-design でシステムテスト実施
  2. /test-coverage code で実測カバレッジ取得
  3. /fmea dc で DC 再評価
```

## 出力
```
=== 安全検証結果 ===
要件チェーン完備: {YES/NO}
テスト合格率: {pct}%
安全メカニズムDC: {all_pass/fail_count}
ASIL B 適合: {YES/NO/PENDING}
Safety Case: {構築済/未構築}
ギャップ: Critical {n} / High {n} / Low {n}
出力ファイル: docs/14_safety_verification.md
```

## 関連スキル
- /assess-spice — SPICE プロセスレベルの評価（プロセス面の検証）
- /test-coverage — テストカバレッジの詳細分析
- /systest-design — システムテスト仕様（本スキルの入力）
- /fmea dc — DC 算出（安全メカニズムの根拠）
- /trace — トレーサビリティの詳細検証
- /health-check — プロジェクト全体の健全性（本スキルの上位俯瞰）

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| 入力データ不足 | 必要ファイル/キーの不在 | 不足項目を報告し、前提スキルの実行を促す |
| 処理中断 | 予期しないエラー | エラー内容を報告。変更途中のファイルがあれば `git checkout` で復元 |

## 注意事項
- 安全検証は独立性が求められる（開発者≠検証者が理想）
- エビデンスのない項目は PENDING とし PASS にしない
- Safety Case は論証構造を明示（根拠なき主張は不可）
- CERTIFY モードでは全項目 PASS + TBD=0 が必須

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/safety_case_gsn.md` — GSN パターン、安全論証テンプレート、Goal/Strategy/Evidence 構造化手法
- `.claude/knowledge/iso26262_iec60730.md` — ASIL B メトリクス要求値、SPFM/LFM/PMHF 算出基準
