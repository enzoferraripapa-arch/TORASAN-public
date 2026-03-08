---
name: update-record
description: "Updates SPICE process records with BP status and work products."
argument-hint: "[process-id e.g. SWE.1, MAN.3]"
disable-model-invocation: true
---


# /update-record — SPICE プロセス記録更新

指定プロセスの process_records/ ファイルを更新する。

引数: $ARGUMENTS（プロセスID。例: "SYS.1", "SWE.3", "MAN.3" → 省略時はユーザーに確認）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在し有効な JSON である | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | 更新対象セクションが存在する | キー存在確認 | セクションを初期化して続行（ユーザー確認後） |

## プロセス記録ファイルの対応表

| プロセスID | ファイル |
|-----------|--------|
| SYS.1 | process_records/SYS.1_requirements_elicitation.md |
| SYS.2 | process_records/SYS.2_system_requirements.md |
| SYS.3 | process_records/SYS.3_system_architecture.md |
| SYS.4 | process_records/SYS.4_system_integration.md |
| SYS.5 | process_records/SYS.5_system_qualification.md |
| SWE.1 | process_records/SWE.1_requirements.md |
| SWE.2 | process_records/SWE.2_architecture.md |
| SWE.3 | process_records/SWE.3_detailed_design.md |
| SWE.4 | process_records/SWE.4_unit_verification.md |
| SWE.5 | process_records/SWE.5_integration_test.md |
| SWE.6 | process_records/SWE.6_qualification_test.md |
| MAN.3 | process_records/MAN.3_project_plan.md |
| SUP.1 | process_records/SUP.1_quality_assurance.md |
| SUP.8 | process_records/SUP.8_config_management.md |
| SUP.9 | process_records/SUP.9_problem_resolution.md |
| SUP.10 | process_records/SUP.10_change_request.md |

## 手順

### Step 1: 現状確認
1. 該当プロセス記録ファイルを読む
2. PROCESS.md §2 の該当プロセス定義（BP/WP/Outcomes）を参照

### Step 2: エビデンス収集
1. 関連するフェーズの成果物（docs/ 配下）を確認
2. project.json の phases から該当フェーズの状態を確認
3. Git log から関連コミットを確認

### Step 3: 記録更新

#### §1 プロセス概要（PA 1.1）
- Outcome 達成状況テーブルを更新（O1〜O6: 達成/部分/未着手）

#### §2 BP 実施記録（PA 1.1）
- 各 BP の実施日・エビデンス・備考を記入
- 新たに実施した BP があれば追記

#### §3 成果物管理記録（PA 2.2）
- WP 一覧を更新（ID・成果物名・バージョン・レビュー状況・保管場所）
- 新たに生成した WP があれば追記

#### §4 実施管理記録（PA 2.1）
- 目標・計画・監視結果・調整内容を更新

#### §7 変更履歴
- 今回の更新内容を追記

### Step 4: project.json 更新
1. `spice_assessment.processes.{PROC_ID}` の PA 値を再評価
2. level を再計算

## 出力
```
=== プロセス記録更新完了 ===
プロセス: {PROC_ID} ({プロセス名})
更新箇所:
  - BP: {更新したBP一覧}
  - WP: {更新したWP一覧}
  - Outcome: {変化したOutcome}
PA評価: PA1.1={val} PA2.1={val} PA2.2={val} → Level {lv}
```

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json パース失敗 | JSON decode エラー | 構文エラー箇所を報告。`git checkout -- project.json` で復元 |
| 書き込み失敗 | Write エラー | 変更内容を表示し手動適用を提案 |
| 更新内容の整合性エラー | バリデーション | 更新前の値を表示し `git checkout -- project.json` でロールバック |

## 注意事項
- 既存の記録は上書きせず追記する
- エビデンスのない項目は記録しない（推測で埋めない）
- 変更履歴セクションは必ず更新する

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/automotive_spice.md` — GP エビデンステーブル、プロセス記録の必須要件
