---
name: commit-change
description: "Creates git commits following TORASAN conventions with automatic changelog updates in project.json. Use when users say 'commit', 'save changes', or 'record changes'. Handles: git commit, changelog, CHG record, version control."
disable-model-invocation: true
argument-hint: "[message: type+要約 (default: git diffから自動判定)]"
---

# /commit-change — Git コミット + changeLog 自動記録

変更内容を Git コミットし、project.json の changeLog に記録する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。例: "feat PH-01 完了"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在し有効な JSON である | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | 更新対象セクションが存在する | キー存在確認 | セクションを初期化して続行（ユーザー確認後） |

## 手順

### Step 1: 変更内容の分析
1. `git status` で変更ファイルを確認
2. `git diff --staged` と `git diff` で差分を確認
3. 変更ファイルなしの場合は「コミット対象がありません」と報告して終了

### Step 2: コミットタイプの判定
引数がない場合、以下のルールで自動判定:
- docs/ の変更のみ → `docs`
- process_records/ の変更のみ → `docs`
- src/ の新規追加 → `feat`
- src/ の修正 → `fix`
- project.json / CLAUDE.md / PROCESS.md → `chore`
- テストファイル → `test`
- 複合変更 → 主要な変更に基づき判定

### Step 3: コミットメッセージ生成
```
{type}: {日本語の要約}

{変更理由・影響範囲の詳細}
変更ファイル: {ファイル一覧}

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
```

### Step 4: changeLog エントリ生成
1. project.json の changeLog を読み、最新の CHG-{N} を確認
2. 新しいエントリを追加:
```json
{
  "id": "CHG-{N+1}",
  "date": "{today YYYY-MM-DD}",
  "type": "{変更タイプ}",
  "item": "{変更対象}",
  "from": "{変更前の状態}",
  "to": "{変更後の状態}",
  "reason": "{変更理由}",
  "affectedPhases": ["{影響フェーズ}"],
  "approvedBy": "",
  "status": "完了"
}
```

### Step 5: コミット実行
1. 変更ファイルをステージング（`git add` — 対象ファイルを明示指定）
2. project.json の更新もステージに含める
3. コミット実行
4. 結果を報告

## 出力
```
=== コミット完了 ===
コミット: {hash} {type}: {要約}
changeLog: {CHG-ID} 追加
ファイル: {count}件
```

## 関連スキル
- /backup — 重要フェーズ完了時やリリース前にはコミット後に /backup でタグ作成を推奨
- /problem-resolve — バグ修正コミット時は関連する PRB-ID を resolution に記録
- /config-audit — 定期的な構成管理監査で changeLog の整合性を検証

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json パース失敗 | JSON decode エラー | 構文エラー箇所を報告。`git checkout -- project.json` で復元 |
| 書き込み失敗 | Write エラー | 変更内容を表示し手動適用を提案 |
| 更新内容の整合性エラー | バリデーション | 更新前の値を表示し `git checkout -- project.json` でロールバック |

## 注意事項
- .env, credentials 等のセンシティブファイルはコミットしない
- コミット前にユーザーにメッセージを確認してもらう
- changeLog の id は既存の最大値 +1 で採番

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `~/.claude/knowledge/claude_code_ops.md` — Git操作のベストプラクティス、コミット規約詳細

※ `~/.claude/knowledge/` = 共有ナレッジ（全PJ共通）、`.claude/knowledge/` = PJ固有ナレッジ
