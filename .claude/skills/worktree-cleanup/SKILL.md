---
name: worktree-cleanup
description: "Detects and safely cleans stale git worktrees and branches created by Claude Code sessions. Use when users mention 'cleanup', 'worktree cleanup', 'stale branches', 'branch cleanup', or 'git cleanup'. Handles: worktree, branch cleanup, git cleanup, stale branches, disk space."
disable-model-invocation: true
argument-hint: "[mode: scan|clean|branches]"
---
# /worktree-cleanup — ワークツリー・ブランチ整理

不要なワークツリーとブランチを検出・報告し、ユーザー確認のうえ整理する。

引数: $ARGUMENTS（オプション。"scan" = 検出のみ | "clean" = 整理実行 | "branches" = ブランチのみ → 省略時は "scan"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | Git リポジトリ内で実行 | `git status` | リポジトリ外では実行不可 |

## 背景

Claude Code はセッションごとにワークツリーを作成する。
セッション終了後にワークツリーが残り続けると、ディスクとブランチが散乱する。
本スキルで定期的に整理する。

## 手順

### scan モード — 検出のみ

#### Step 1: ワークツリー一覧

```bash
git worktree list
```

#### Step 2: 各ワークツリーの状態を分析

各ワークツリーについて以下を判定:
- master との差分コミット数（`git rev-list --count master..{branch}`）
- master にマージ済みか（差分が0 = マージ済み or 遅れ）
- ワークツリー内に未コミット変更があるか
- 最終コミット日時

#### Step 3: 結果表示

```
=== ワークツリー・ブランチ状態 ===

| ワークツリー | ブランチ | master差分 | 最終コミット | 判定 |
|-------------|---------|-----------|------------|------|
| (main repo) | master | — | {date} | BASE |
| charming-wing | claude/charming-wing | +5 | 今日 | ACTIVE |
| friendly-carson | claude/friendly-carson | 0 | 3日前 | STALE ⚠ |
| naughty-aryabhata | claude/naughty-aryabhata | 0 | 3日前 | STALE ⚠ |

[ブランチのみ（ワークツリーなし）]
| ブランチ | master差分 | 最終コミット | 判定 |
|---------|-----------|------------|------|
| claude/amazing-kare | 0 | 5日前 | STALE ⚠ |

[サマリ]
ワークツリー: {total}個（ACTIVE: {n}, STALE: {n}）
ブランチ: {total}本（ACTIVE: {n}, STALE: {n}, MERGED: {n}）
削除可能: ワークツリー {n}個、ブランチ {n}本
```

### clean モード — 整理実行

#### Step 1: scan を実行して対象を特定

#### Step 2: 削除対象をユーザーに確認

```
以下を削除します:

[ワークツリー削除]
1. .claude/worktrees/friendly-carson (claude/friendly-carson, master同一)
2. .claude/worktrees/naughty-aryabhata (claude/naughty-aryabhata, master同一)

[ブランチ削除]
3. claude/amazing-kare (master同一、ワークツリーなし)

続行しますか？ (y/N)
```

#### Step 3: ユーザー承認後に実行

```bash
# ワークツリー削除
git worktree remove .claude/worktrees/{name}

# ブランチ削除（マージ済みのみ -d で安全削除）
git branch -d claude/{name}

# 未マージブランチは -D が必要 → 追加確認
```

#### Step 4: 整理後の状態を表示

```
=== 整理完了 ===
削除: ワークツリー {n}個、ブランチ {n}本
残存: ワークツリー {n}個、ブランチ {n}本
```

### branches モード — ブランチのみ整理

ワークツリーには触れず、ブランチの整理のみ実行:
- `git branch --merged master` でマージ済みブランチを検出
- master/main 以外のマージ済みブランチを削除提案
- リモート追跡ブランチの不要分も `git remote prune origin` で整理

## 判定基準

| 判定 | 条件 | アクション |
|------|------|----------|
| ACTIVE | 現在のセッションで使用中 | 保持 |
| AHEAD | master より先行（未マージ作業あり） | 保持 or マージ推奨 |
| STALE | master と同一 or 遅れ、未コミット変更なし | 削除推奨 |
| DIRTY | 未コミット変更あり | コミット推奨後に判定 |

## 安全策

- **現在のワークツリーは絶対に削除しない**
- **master / main ブランチは絶対に削除しない**
- **未マージブランチは `-d` で削除試行し、失敗したら報告（`-D` はユーザー確認後のみ）**
- **AHEAD 状態のブランチは削除前にマージ要否を確認**
- **DIRTY 状態のワークツリーは削除しない**

## 関連スキル
- /session — セッション管理（/session end で状態保存後に cleanup 推奨）
- /backup — 重要な状態を保全してから cleanup

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| ワークツリー削除失敗 | `git worktree remove` 非0終了 | `git worktree prune` で不整合修復後リトライ |
| ブランチ削除失敗（未マージ） | `git branch -d` 失敗 | 未マージ内容を報告し `-D` 使用をユーザーに確認 |
| ワークツリーが DIRTY 状態 | 未コミット変更検出 | 削除をスキップし、コミットまたは変更破棄を促す |

## 注意事項
- ワークツリー削除は `git worktree remove` を使用（手動 rm 禁止）
- 削除後に `git worktree prune` で不整合を修復
- リモートブランチの削除は `git push origin --delete` が必要（ユーザー確認必須）
- clean モードは必ずユーザー確認を経てから実行

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `~/.claude/knowledge/git_worktree_branch_management.md` — ワークツリーライフサイクル、安全な削除手順、ブランチ整理戦略、git gc/prune
- `~/.claude/knowledge/claude_code_ops.md` — Claude Code のワークツリー動作、ブランチ命名規則

※ `~/.claude/knowledge/` = 共有ナレッジ（全PJ共通）、`.claude/knowledge/` = PJ固有ナレッジ
