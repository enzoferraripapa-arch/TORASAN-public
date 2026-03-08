---
name: manage-tbd
description: "Manages TBD items tracked in project.json."
argument-hint: "[action: list|add {内容}|resolve TBD-XX (default: list)]"
disable-model-invocation: true
---


# /manage-tbd — TBD 管理

project.json の TBD（To Be Determined）項目を管理する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "list"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在し有効な JSON である | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | 更新対象セクションが存在する | キー存在確認 | セクションを初期化して続行（ユーザー確認後） |

## 手順

### list モード（デフォルト）
1. project.json の `tbd_items` と `tbd_count` を読む
2. 各 TBD について関連フェーズ・影響範囲を分析
3. 以下の形式で表示:

```
=== TBD 管理一覧 ===
モード: {mode}（EXPLORE: TBD許容 / CERTIFY: TBD=0必須）

| # | TBD-ID | 内容 | 関連フェーズ | 影響度 | 解消候補 |
|---|--------|------|-----------|-------|---------|
| 1 | TBD-01 | {内容} | PH-XX | 高/中/低 | {提案} |

合計: {tbd_count}件
CERTIFY移行に必要な解消: {count}件
```

### resolve モード
1. 指定された TBD-XX を確認
2. 解消内容をユーザーに確認
3. project.json 更新:
   - `tbd_items` から該当項目を削除
   - `tbd_count` を -1
   - `changeLog` に解消記録を追加
4. 関連する成果物があれば更新提案

### add モード
1. 新しい TBD を追加
2. project.json 更新:
   - `tbd_items` に追加（"TBD-{連番}: {内容}"）
   - `tbd_count` を +1
3. 関連フェーズへの影響を報告

## 影響度判定基準
- **高**: 安全要求・ASIL判定に関わる / コード生成をブロックする
- **中**: 設計判断に影響する / テスト仕様に影響する
- **低**: ツール選定・運用手順に関わる / 機能に直接影響しない

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json パース失敗 | JSON decode エラー | 構文エラー箇所を報告。`git checkout -- project.json` で復元 |
| 書き込み失敗 | Write エラー | 変更内容を表示し手動適用を提案 |
| 更新内容の整合性エラー | バリデーション | 更新前の値を表示し `git checkout -- project.json` でロールバック |

## 注意事項
- CERTIFY モードでは TBD 追加時に警告を出す
- TBD 解消時は changeLog に記録必須
- TBD-ID は連番を維持（欠番は埋めない）

## 関連スキル
- /dashboard — プロジェクト全体のTBD件数を確認
- /execute-phase — フェーズ実行中にTBDを発見した場合の登録
- /health-check — TBD残存を含むプロジェクト健全性チェック
