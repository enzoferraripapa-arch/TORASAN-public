---
name: assess-spice
description: "Evaluates Automotive SPICE capability levels for 16 processes."
argument-hint: "[process: SWE.1|SWE.2|...|all (default: all)]"
disable-model-invocation: true
---


# /assess-spice — SPICE 能力レベル評価

Automotive SPICE 能力レベルの評価を実施し、結果を報告する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "all"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在し有効な JSON である | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | 更新対象セクションが存在する | キー存在確認 | セクションを初期化して続行（ユーザー確認後） |

## 評価手順

### Step 1: 現状把握
1. `project.json` の `spice_assessment` を読む
2. 対象プロセスを特定（引数指定 or 全15プロセス）

### Step 2: エビデンス収集
各プロセスについて:
1. `process_records/{PROC_ID}_*.md` を読む
2. 以下を確認:
   - BP（Base Practice）の実施記録（日付・エビデンス有無）
   - WP（Work Product）の存在・バージョン・レビュー状況
   - Outcome の達成状況

### Step 3: PA 評価

#### PA 1.1（プロセス実行）
- BP の実施率を算出: 実施済BP数 / 全BP数
- WP の存在率を算出: 作成済WP数 / 全WP数
- Outcome 達成率を確認

#### PA 2.1（実施管理）
process_records の §4「実施管理記録」を確認:
- 目標が定義されているか
- 計画が策定されているか
- 監視結果が記録されているか
- 調整・是正が実施されているか

#### PA 2.2（成果物管理）
process_records の §3「成果物管理記録」を確認:
- 成果物にバージョンが付与されているか
- レビューが実施されているか
- 保管場所が明確か
- 変更管理されているか

### Step 4: レーティング判定

| 達成率 | レーティング | 記号 |
|-------|-----------|------|
| 0-15% | 未達成 | N |
| 16-50% | 部分達成 | P |
| 51-85% | 概ね達成 | L |
| 86-100% | 完全達成 | F |

### Step 5: 能力レベル判定

| レベル | 条件 |
|-------|------|
| Level 0 | PA 1.1 < L |
| Level 1 | PA 1.1 >= L |
| Level 2 | PA 1.1 = F かつ PA 2.1 >= L かつ PA 2.2 >= L |

### Step 6: 結果更新
1. `project.json` の `spice_assessment` を更新（PA値、level、assessment_date）
2. 結果を以下の形式で報告:

```
=== SPICE 能力レベル評価結果 ===
評価日: {date}
目標: Level {target_level}

[プロセス別評価]
| プロセス | PA 1.1 | PA 2.1 | PA 2.2 | Level | 目標との差 |
|---------|--------|--------|--------|-------|----------|
| SYS.1   | {val}  | {val}  | {val}  | {lv}  | {gap}    |
...

[サマリ]
- Level 2 達成: {count}/{total} プロセス
- Level 1 達成: {count}/{total} プロセス
- 重点改善プロセス: {最も level が低いプロセスを列挙}

[改善推奨アクション]
1. {具体的な改善提案}
2. {具体的な改善提案}
```

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json パース失敗 | JSON decode エラー | 構文エラー箇所を報告。`git checkout -- project.json` で復元 |
| 書き込み失敗 | Write エラー | 変更内容を表示し手動適用を提案 |
| 更新内容の整合性エラー | バリデーション | 更新前の値を表示し `git checkout -- project.json` でロールバック |

## 注意事項
- エビデンスがないものは N（未達成）とする
- 推測で評価を上げない（保守的に判定）
- 前回評価との差分がある場合は明示する

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/automotive_spice.md` — PA レーティングスケール、GP エビデンス要件、SWE BP サマリ
