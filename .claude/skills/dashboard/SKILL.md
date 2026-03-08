---
name: dashboard
description: "Displays project dashboard with phase progress, SPICE capability levels, and TBD items."
disable-model-invocation: true
---

# /dashboard — プロジェクト ダッシュボード表示

project.json を読み込み、以下のフォーマットでダッシュボードを表示せよ。

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |

## 手順

1. `project.json` を読み込む
2. 以下のフォーマットで表示する

```
=== {projectName} ダッシュボード ===
プロジェクト: {projectName} ({category})
モード: {mode} / 規格: {standard.base} ASIL {standard.asil}
オーバーライド: {standard.product_override} ({standard.product_override_class})
TBD: {tbd_count}件

[フェーズ進捗]
PH-01〜PH-15 の name, spice, status を一覧表示
※ completed は [DONE]、in_progress は [>>>>]、not_started は [----] で表記

[SPICE 能力レベル]
spice_assessment.processes から各プロセスの PA値と level を表示
目標: Level {spice_assessment.target_level}

[トレーサビリティ]
traceability の各カウントを表示

[TBD一覧]
tbd_items を番号付きで表示

[最近の変更 (直近3件)]
changeLog の末尾3件を表示
```

3. 表示後、次に推奨されるアクションを1〜2個提案する（未完了の最初のフェーズ、TBD解消など）

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| データソース読み取り失敗 | Read / Glob エラー | 対象不在を報告。該当項目を「N/A」として表示 |
| 分析対象データ不足 | 必要キーの不在 | 不足項目を明示し、データ投入を促す |

## 注意事項
- project.json のみ読めば完結する（PROCESS.md は不要）
- 簡潔に、見やすく表示する

## 関連スキル
- /execute-phase — フェーズ実行でプロジェクトを進める
- /health-check — プロジェクトの健全性を検証
- /assess-spice — SPICE能力レベルの詳細評価
