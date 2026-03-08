---
name: backup
description: "Creates git tags and backup snapshots of the current project state for safe rollback. Use when users say 'backup', 'create tag', 'save snapshot', or 'create restore point'. Handles: backup, git tag, snapshot, restore point, version archive."
disable-model-invocation: true
argument-hint: "[suffix: タグ名サフィックス (default: なし)]"
---
# /backup — バージョンバックアップ

Git タグによるバックアップを作成し、構成管理記録を更新する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。例: "before-ph01" → "v5-before-ph01"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | Git リポジトリ内で実行 | `git status` | リポジトリ外では実行不可 |
| 2 | 未コミット変更がない | `git status --porcelain` | 先にコミットを促す |

## 手順

### Step 1: 現状確認
1. `git tag` で既存タグを一覧
2. 最新タグ番号を確認（v{N}-backup パターン）
3. `git status` で未コミットの変更がないか確認
4. 未コミットの変更がある場合は先にコミットを促す

### Step 2: タグ作成
1. タグ名を決定:
   - 引数なし: `v{N+1}-backup`
   - 引数あり: `v{N+1}-{suffix}`
2. アノテーション付きタグを作成:
```bash
git tag -a {tag_name} -m "Backup: {現在の状態サマリ}
フェーズ進捗: {completed}/{total}
SPICE Level達成: {summary}
日時: {timestamp}"
```

### Step 3: SUP.8 構成管理記録の更新
1. `process_records/SUP.8_config_management.md` を更新:
   - バックアップ履歴にエントリ追加
   - タグ名、日時、理由、含まれる成果物を記録

### Step 4: 報告
```
=== バックアップ完了 ===
タグ: {tag_name}
コミット: {hash}
含まれるフェーズ: {completed フェーズ一覧}
成果物数: {ファイル数}

復元コマンド: git checkout {tag_name}
```

## リストア手順（参考）
ユーザーが「ロールバックして」と言った場合:
1. `git log --oneline` で履歴表示
2. `git tag` でタグ一覧表示
3. ユーザーに復元先を確認
4. `git revert` で安全に戻す（`git reset --hard` は使わない）

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| 未コミット変更あり | `git status` で検出 | 先にコミットを実施してからリトライ |
| タグ名重複 | `git tag -l` で既存タグと照合 | 連番を自動インクリメントして回避 |
| SUP.8 記録ファイル不在 | Read エラー | 警告を表示してタグ作成のみ続行 |

## 注意事項
- `git push` はしない（ユーザー指示がない限り）
- archive/ ディレクトリへの物理コピーは不要（Git が履歴保持）
- タグ番号は連番を維持
