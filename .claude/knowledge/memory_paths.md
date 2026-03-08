# メモリ・状態ファイルのパス構成

Claude Code のメモリシステムは複数の場所に分散している。本ドキュメントはその構造を整理する。

## 1. 自動メモリディレクトリ（Claude Code が管理）

各セッションの auto memory は以下のパスに自動割当される:

```
~/.claude/projects/{project-path-slug}/memory/
```

| コンテキスト | パス例 |
|------------|--------|
| メインリポジトリ | `~/.claude/projects/C--Users-<USERNAME>-Documents-TORASAN/memory/` |
| ワークツリー | `~/.claude/projects/C--Users-<USERNAME>-Documents-TORASAN--claude-worktrees-{name}/memory/` |

- `MEMORY.md` はセッション開始時に自動読み込み（200行まで）
- ワークツリーごとにパスが異なる点に注意

### slug 算出ルール（パス正規化）

```
0. ls ~/.claude/projects/ | grep {PJ名} で実在確認（推測禁止・最重要）
1. ドライブレター C: → C-
2. バックスラッシュ \ → ハイフン -
3. アンダースコア _ はそのまま保持（置換しない）
例: C:\Users\<USERNAME>\Documents\TORASAN → C--Users-<USERNAME>-Documents-TORASAN
```

## 2. セッション状態ファイル

`/session end` 実行時に以下の **両方** に保存:

| ファイル | パス | 用途 |
|---------|------|------|
| `session_state.md` | ワークツリーのメモリディレクトリ | 同一ワークツリーでの再開用 |
| `session_state.md` | メインリポジトリのメモリディレクトリ | 新規ワークツリーでの引継ぎ用 |

## 3. プロジェクト内ファイル（Git管理）

| ファイル | パス | 用途 |
|---------|------|------|
| `skill_feedback_log.md` | `.claude/knowledge/` | スキルフィードバック履歴 |
| `memory_paths.md` | `.claude/knowledge/` | 本ファイル（パス構成ドキュメント） |
| `*.md` ナレッジ群 | `.claude/knowledge/` | ドメイン・運用知識 |
| `SKILL.md` | `.claude/skills/{name}/` | スキル定義 |
| 反省会議事録 | `process_records/retrospective/` | セッション反省会の記録 |

## 4. クロスPJ共有ファイル

| ファイル | パス | 用途 |
|---------|------|------|
| `env_state.md` | `~/.claude/knowledge/env_state.md` | TORASAN 環境スナップショット（全PJ参照） |
| `project_registry.json` | `~/.claude/project_registry.json` | PJ一覧・sync 状態 |
| `.shared-skills-manifest.json` | `~/.claude/.shared-skills-manifest.json` | install.sh 配布状態（hash比較用） |

- `env_state.md` は TORASAN セッション終了時（Step 1.6）+ 週次 schedule で更新
- 子PJの `/session start` Phase B サブエージェントが参照し、Phase C で更新通知を表示

## 5. 注意事項

- ワークツリー名が変わるとメモリパスも変わる
- 重要な教訓は `MEMORY.md`（自動読込）に記載し、詳細はトピック別ファイルへ
- セッション状態は揮発性が高いため、Git管理対象外の auto memory に保存
- ナレッジベースは永続的なため、Git管理対象の `.claude/knowledge/` に保存
