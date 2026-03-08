---
name: repo-manage
description: "Manages project repositories: registry, domain skill sync, and directory migration."
argument-hint: "[mode: list|register|sync|move|remove|status|discover]"
---

# /repo-manage --- プロジェクトリポジトリ管理

TORASAN フレームワークベースのプロジェクトリポジトリを一元管理する。
レジストリ登録・ドメインスキル配布・ディレクトリ移動時のメモリパス移行・同期状態追跡を提供。

引数: $ARGUMENTS（"list" = 登録PJ一覧 | "register {path}" = PJ登録 | "sync {name|all}" = ドメインスキル同期 | "move {name} {new-path}" = 移動+メモリ移行 | "remove {name}" = 登録解除 | "status" = 同期状態詳細 | "discover" = ファイルシステム走査 → 省略時は "list"）

---

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | Git リポジトリ内で実行 | `git status` | リポジトリ外では実行不可 |

## レジストリ仕様

中央レジストリ: `~/.claude/project_registry.json`（実行時に自動作成、Git 管理外）

```json
{
  "_schema_version": "1.0",
  "_updated_at": "ISO8601",
  "torasan_source": "絶対パス",
  "torasan_commit": "short_hash",
  "projects": [
    {
      "name": "projectName",
      "path": "絶対パス",
      "category": "カテゴリ",
      "framework_commit": "同期時コミット or null",
      "domain_skills_synced_at": "ISO8601 or null",
      "domain_skills_list": [],
      "domain_knowledge_list": [],
      "memory_slug": "C--Users-xxx",
      "registered_at": "ISO8601"
    }
  ]
}
```

---

## 手順

### list モード --- 登録PJ一覧（デフォルト）

#### Step 1: レジストリ読み込み

`~/.claude/project_registry.json` を読む。不在時は案内:
```
レジストリが未初期化です。
→ /repo-manage discover で自動検出
→ /repo-manage register {path} で個別登録
```

#### Step 2: 一覧表示

```
=== TORASAN プロジェクトレジストリ ({count}件) ===
TORASAN: {torasan_source} ({torasan_commit})

| # | プロジェクト | カテゴリ | 同期状態 | 最終同期 |
|---|------------|---------|---------|---------|
| 1 | {name} | {category} | {SYNCED/STALE/UNSYNC} | {date or "---"} |

同期状態凡例:
  SYNCED — 最新の TORASAN コミットと一致
  STALE  — TORASAN 側に新しいコミットあり
  UNSYNC — 未同期（sync 未実行）
```

---

### register モード --- PJ登録

#### Step 1: パス解決と検証

1. `$ARGUMENTS` からパスを抽出（省略時は `$PWD`）
2. 指定パスに `project.json` が存在するか確認
3. 不在時:
   ```
   {path} に project.json が見つかりません。
   (a) このディレクトリをそのまま登録する
   (b) 別のパスを指定する
   (c) キャンセル
   ```

#### Step 2: 重複チェック

レジストリに同一パスが既にあれば報告して終了。

#### Step 3: 情報抽出

`project.json` から `projectName`, `category`, `_schema_version` を読み取り。

#### Step 4: メモリスラグ算出

パスからスラグを算出:
- `\` → `-` に変換（連続で `--`）
- ドライブレター `C:` → `C-`
- 例: `C:\Users\<USERNAME>\Documents\MyProject` → `C--Users-<USERNAME>-Documents-MyProject`

#### Step 5: レジストリ追加

エントリを追加し `_updated_at` を更新。

#### Step 6: 同期提案

```
=== 登録完了 ===
プロジェクト: {name}
パス: {path}

ドメインスキルを同期しますか？ → /repo-manage sync {name}
```

---

### sync モード --- ドメインスキル・ナレッジ同期

#### Step 1: 対象の特定

- `sync {name}` → 指定プロジェクト1件
- `sync all` → 全登録プロジェクト
- ユーザーに確認

#### Step 2: TORASAN ソースの特定

1. `~/.claude/.shared-skills-manifest.json` の `source` から TORASAN パスを取得
2. パスの有効性確認
3. 現在のコミットハッシュを取得: `git -C {torasan_path} rev-parse --short HEAD`

#### Step 3: ドメインスキルの判定

TORASAN の `install.sh` 内 `SHARED_SKILLS` 配列を解析し、`.claude/skills/` 全体から差し引いてドメインスキルを特定。

```
ドメインスキル: {count}本
ドメインナレッジ: {count}本
```

#### Step 4: 同期範囲の選択

```
全ドメインスキル({total}本)を同期しますか？
(a) 全て同期（推奨）
(b) カテゴリで選択
(c) 個別選択
```

#### Step 5: コピー実行

各対象プロジェクトに対して:
1. `{torasan}/.claude/skills/{skill}/` → `{project}/.claude/skills/{skill}/`
2. `{torasan}/.claude/knowledge/{file}` → `{project}/.claude/knowledge/{file}`
3. 既存ファイルは上書き（バージョン管理は Git に委任）

#### Step 6: framework_version 記録

対象プロジェクトの `project.json` に追加/更新:

```json
{
  "framework_version": {
    "source": "TORASAN",
    "commit": "{short_hash}",
    "synced_at": "{ISO8601}",
    "domain_skills_count": 26,
    "domain_knowledge_count": 9
  }
}
```

#### Step 7: レジストリ更新

`framework_commit`, `domain_skills_synced_at`, `domain_skills_list`, `domain_knowledge_list` を更新。

#### Step 8: 完了報告

```
=== 同期完了 ===
対象: {name} ({path})
TORASAN: {commit}
スキル: {n}本 / ナレッジ: {n}本
framework_version: project.json 記録済み
```

---

### move モード --- 移動 + メモリパス移行

#### Step 1: 移動元の確認

レジストリから `{name}` を検索。不在時はエラー。

#### Step 2: 事前チェック

1. 移動先が存在しないことを確認
2. 親ディレクトリの存在確認
3. 対象ディレクトリの Git 状態確認（未コミット変更があれば警告）

#### Step 3: 移動プラン提示

```
=== 移動プラン ===
プロジェクト: {name}
移動元: {old_path}
移動先: {new_path}

[メモリパス移行]
  ~/.claude/projects/{old_slug}/  →  {new_slug}/
  (WT用メモリがあれば追加表示)

続行しますか？
```

ユーザー確認必須。

#### Step 4: ディレクトリ移動

`mv "{old_path}" "{new_path}"`

#### Step 5: メモリパス移行

1. `~/.claude/projects/` 配下で旧スラグに一致するディレクトリを検索
2. メインリポ用: `{old_slug}` → `{new_slug}` にリネーム
3. WT用: `{old_slug}--claude-worktrees-*` パターンも移行

#### Step 6: レジストリ更新

`path`, `memory_slug` を新しい値に更新。

#### Step 7: 完了報告

```
=== 移動完了 ===
新パス: {new_path}
メモリ移行: {n}ディレクトリ
レジストリ: 更新済み
```

---

### remove モード --- 登録解除

#### Step 1: 対象の確認

レジストリから `{name}` を検索。

#### Step 2: 削除オプション提示

```
=== 登録解除: {name} ===
パス: {path}

(a) レジストリから登録解除のみ（ディレクトリ・メモリ保持）
(b) 登録解除 + メモリパスも削除
(c) 登録解除 + メモリ + ディレクトリ完全削除
(d) キャンセル
```

#### Step 3: (b)(c) の場合 — 追加確認

(b): メモリパス削除を確認。
(c): **二段階確認** — プロジェクト名の再入力を要求:
```
⚠ 警告: {path} を完全削除します（取消不可）。
プロジェクト名を入力して確認: ___
```

#### Step 4: 実行

選択に応じて:
- (a): レジストリからエントリ削除のみ
- (b): + `~/.claude/projects/{slug}/` と WT 用メモリを削除
- (c): + プロジェクトディレクトリ自体を削除

#### Step 5: 完了報告

```
=== 登録解除完了 ===
レジストリ: 解除済み
メモリ: {削除済み/保持}
ディレクトリ: {削除済み/保持}
```

---

### status モード --- 同期状態詳細

#### Step 1: TORASAN の現在状態を取得

`torasan_source` の最新コミットとドメインスキル一覧を取得。

#### Step 2: 各PJの同期差分を分析・表示

```
=== プロジェクト同期状態 ===
TORASAN: {commit} ({date})

[{name}] {category} — {SYNCED/STALE/UNSYNC}
  パス: {path} ({存在する/⚠ パス不在})
  同期コミット: {framework_commit or "---"}
  スキル差分: 追加 {n} / 更新 {n}
  ナレッジ差分: 追加 {n} / 更新 {n}

[サマリ]
  SYNCED {n} / STALE {n} / UNSYNC {n}
  推奨: /repo-manage sync {name}
```

#### Step 3: パス不在の検出

```
⚠ {name} のパスが存在しません: {path}
→ /repo-manage move {name} {new_path} で更新
→ /repo-manage remove {name} で登録解除
```

---

### discover モード --- ファイルシステム走査

#### Step 1: 走査対象の決定

```
=== プロジェクト自動検出 ===
走査ディレクトリを選択:
(a) ~/Documents/ 配下（推奨）
(b) 特定のディレクトリを指定
```

#### Step 2: project.json の検索

指定ディレクトリ配下で `project.json` を含むディレクトリを検索。
除外: `node_modules/`, `.claude/worktrees/`, `archive/`, `.git/`

#### Step 3: 発見結果の表示

```
=== 検出されたプロジェクト ===
| # | プロジェクト | カテゴリ | パス | 登録状態 |
|---|------------|---------|------|---------|
| 1 | {name} | {cat} | {path} | 登録済み/未登録 |

登録するプロジェクトを選択（番号をカンマ区切り、"all" で全件）:
```

#### Step 4: 登録実行

選択されたプロジェクトを register モード Step 3〜6 で処理。

---

## 関連スキル

| スキル | 連携内容 |
|--------|---------|
| /session | start Phase A で `multi_candidate` 時にレジストリ参照。新規PJ後に register 提案 |
| /skill-manage | list モードでドメインスキル配布状況を補完表示 |
| /health-check | フレームワーク同期チェックセクション追加 |
| /backup | move/remove 前にスナップショット推奨 |
| /worktree-cleanup | move 前に WT 整理を推奨 |
| /commit-change | sync 後の project.json 変更コミット |

---

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| 入力データ不足 | 必要ファイル/キーの不在 | 不足項目を報告し、前提スキルの実行を促す |
| 処理中断 | 予期しないエラー | エラー内容を報告。変更途中のファイルがあれば `git checkout` で復元 |

## 注意事項

### TORASAN パスの自動検出

`~/.claude/.shared-skills-manifest.json` の `source` フィールドから取得。マニフェスト不在時は `install.sh` 実行を案内。

### ドメインスキル判定

install.sh の `SHARED_SKILLS` 配列を唯一の真実源として使用。配列に含まれないスキル = ドメインスキル。

### メモリスラグ算出の一貫性

`/session` Phase B と同一の算出方式を使用（`memory_paths.md` 参照）。

### ワークツリーメモリパスの対応

`move` 時に `{slug}--claude-worktrees-{wt-name}` パターンのディレクトリも移行対象。

### 破壊的操作の安全策

- `move`: 未コミット変更チェック + 移動プラン表示 + ユーザー確認
- `remove` (c): プロジェクト名再入力の二段階確認
- 全破壊的操作はユーザー確認必須

---

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:

| ファイル | 参照内容 | 参照方法 |
|---------|---------|---------|
| `~/.claude/knowledge/memory_paths.md` | メモリパス構成、スラグ算出方式（move モード） | 直接参照 |
| `~/.claude/knowledge/claude_code_ops.md` | メモリシステム、WT 動作（move/register モード） | 直接参照 |
| `~/.claude/knowledge/cross_platform_dev.md` | Windows パス処理 | 直接参照 |
| `~/.claude/knowledge/skill_lifecycle.md` | スキル分類体系（ドメインスキル判定） | 必要時 |

※ `~/.claude/knowledge/` = 共有ナレッジ（全PJ共通）、`.claude/knowledge/` = PJ固有ナレッジ
