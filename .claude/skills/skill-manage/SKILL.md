---
name: skill-manage
description: "Lists, audits quality, analyzes coverage, and scaffolds new custom skills."
argument-hint: "[mode: list|coverage|create|audit] [skill-name]"
---
# /skill-manage — スキル管理

スキルの一覧・カバレッジ確認・ナレッジ参照状況・新規スキル雛形生成を行うメタスキル。

引数: $ARGUMENTS（オプション。"list" = 一覧 | "coverage" = カバレッジ分析 | "create {name}" = 雛形生成 | "audit" = 品質監査 → 省略時は "list"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | Git リポジトリ内で実行 | `git status` | リポジトリ外では実行不可 |

## 手順

### list モード — スキル一覧

以下の2箇所を走査し、スキルを分類表示:
- `.claude/skills/` — PJ固有スキル（プロジェクトリポジトリ内）
- `~/.claude/skills/` — 共有スキル（全PJ共通、ホームディレクトリ）

各スキルには **[共有]** または **[PJ固有]** のラベルを付与する。
共有スキルが PJ 側にも存在する場合は **[PJ上書き]** と表示する。

```
=== スキル一覧 ({total}スキル) ===

[P1: コアプロセス]
  /dashboard — プロジェクトダッシュボード表示
  /execute-phase — フェーズ実行（PH-01〜PH-15）
  /assess-spice — SPICE 能力レベル評価
  /validate — 統合検証

[P2: 高頻度操作]
  /trace — トレーサビリティマトリクス
  /commit-change — Git コミット + changeLog
  /update-record — SPICE プロセス記録更新
  /manage-tbd — TBD 管理

[D: ドメイン専門]
  /fmea, /safety-diag, /safety-concept, /safety-verify
  /mcu-config, /memory-map, /driver-gen, /motor-control
  /static-analysis, /hw-review

[E: 設計・テスト]
  /system-design, /sw-design, /srs-generate
  /test-design, /test-coverage, /systest-design

[M: 管理・監査]
  /problem-resolve, /config-audit, /health-check

[O: 運用・環境]
  /env-check, /session, /worktree-cleanup
  /skill-manage, /platform-info

[U: ユーティリティ]
  /switch-standard, /backup, /ingest, /generate-docs
```

### coverage モード — カバレッジ分析

スキルがプロジェクトの全領域をカバーしているか分析:

```
=== スキルカバレッジ ===

[V-model フェーズカバレッジ]
| フェーズ | 専門スキル | 汎用カバー | 判定 |
|---------|----------|----------|------|
| PH-01 | — | /execute-phase | OK |
| PH-03 | /fmea, /safety-concept | /execute-phase | OK |
| PH-10 | /driver-gen, /motor-control, /safety-diag | /execute-phase | OK |

[SPICE プロセスカバレッジ]
| プロセス | 対応スキル | 判定 |
|---------|----------|------|
| MAN.3 | /execute-phase, /health-check | OK |
| SUP.8 | /config-audit, /backup | OK |
| SUP.9 | /problem-resolve | OK |

[ナレッジベース参照状況]
| ナレッジファイル | 参照スキル数 |
|---------------|------------|
| iso26262_iec60730.md | {n} |
| misra_c_2012.md | {n} |
| (参照なし) | {n} |

未参照スキル: {list}
未参照ナレッジ: {list}
```

### create モード — 新規スキル雛形生成

```bash
/skill-manage create {skill-name}
```

以下のテンプレートで `.claude/skills/{skill-name}/SKILL.md` を生成:

```markdown
# /{skill-name} — {タイトル}

{1行説明}

引数: $ARGUMENTS（オプション。"mode1" = ... | "mode2" = ... → 省略時は "mode1"）

## 手順

### mode1 モード — {説明}

#### Step 1: {手順}
{詳細}

## 出力
{出力先の記載}

## 関連スキル
- /{related} — {関連の説明}

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| 入力データ不足 | 必要ファイル/キーの不在 | 不足項目を報告し、前提スキルの実行を促す |
| 処理中断 | 予期しないエラー | エラー内容を報告。変更途中のファイルがあれば `git checkout` で復元 |

## 注意事項
- {注意点}

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/{file}.md` — {参照内容}
```

### audit モード — 品質監査

`.claude/skills/`（PJ固有）と `~/.claude/skills/`（共有）の両方をスキャンし、全スキルの構造的品質をチェック。
各スキルに **[共有]** / **[PJ固有]** ラベルを付与して区別する:

```
=== スキル品質監査 ===

| スキル | 引数定義 | 手順 | 関連スキル | ナレッジ参照 | 注意事項 | 判定 |
|--------|---------|------|----------|-----------|---------|------|
| /fmea | ✓ | ✓ | ✓ | ✓ | ✓ | A |
| /backup | ✓ | ✓ | ✓ | — | ✓ | B |

品質レベル:
  A (完備): 全セクション完備 + ナレッジ参照あり
  B (良好): 主要セクション完備
  C (基本): 最低限の手順のみ
  D (不足): 必須セクション欠落

[サマリ]
A: {n}スキル / B: {n}スキル / C: {n}スキル / D: {n}スキル
```

## スキル命名規則

| カテゴリ | 接頭辞パターン | 例 |
|---------|-------------|-----|
| プロセス実行 | 動詞 | execute-phase, assess-spice |
| 生成 | 名詞-gen / generate | driver-gen, generate-docs |
| 検証・分析 | 名詞-check / verify | env-check, health-check |
| 管理 | manage-名詞 / 名詞-manage | manage-tbd, skill-manage |
| 設計 | 名詞-design | system-design, test-design |
| 情報表示 | 名詞-info | platform-info |

## 関連スキル
- /health-check — プロジェクト全体の健全性（スキル活用状況セクション）
- /env-check — 開発環境側の検証

## 注意事項
- 新規スキル作成後は .gitignore の `!.claude/skills/` が効いているか確認
- スキル名はハイフン区切り小文字英数字（日本語不可）
- $ARGUMENTS のパース規則を統一（引用符不要、スペース区切り）
- 既存スキルとの命名重複を避ける

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `~/.claude/knowledge/claude_code_ops.md` — スキル設計ベストプラクティス、$ARGUMENTS 動作、ファイル構成規則
- `~/.claude/knowledge/skill_lifecycle.md` — スキルライフサイクル管理プロセス、成熟モデル、レビュー基準、品質レベル定義

※ `~/.claude/knowledge/` = 共有ナレッジ（全PJ共通）、`.claude/knowledge/` = PJ固有ナレッジ
