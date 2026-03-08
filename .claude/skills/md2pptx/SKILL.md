---
name: md2pptx
description: "Converts Markdown to PowerPoint slides with professional themes. Use when users say 'create slides', 'make presentation', 'markdown to pptx', 'generate pptx', 'スライド作成', 'プレゼン作成', 'パワポ作成'. Handles: pptx, powerpoint, slides, presentation, roadmap."
disable-model-invocation: true
argument-hint: "[input.md] [-o output.pptx] [-t theme]"
---
# /md2pptx — Markdown → PowerPoint 変換

Obsidian 等で書いた Markdown を、デザイン済み PowerPoint に自動変換する。

引数: $ARGUMENTS（入力ファイルパス。省略時はユーザーに確認）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | 必要な外部ツールがインストール済み | `which {tool}` / `{tool} --version` | エラーメッセージでインストール手順を提示 |
| 2 | 対象ファイルが存在する | Glob/ls で確認 | 対象不在を報告して終了 |

## クイックスタート

```bash
python scripts/md2pptx.py input.md                    # → input.pptx
python scripts/md2pptx.py input.md -o output.pptx     # 出力先指定
python scripts/md2pptx.py input.md -t edia            # テーマ指定
```

## テーマ一覧

| テーマ | 特徴 | 向いている用途 |
|--------|------|---------------|
| earth | 暖色系（テラコッタ＋セージ） | 一般プレゼン |
| corporate | ブルー系ビジネス | 社内報告 |
| ocean | 海洋ブルー | 技術発表 |
| mono | モノクロ＋アクセント赤 | シンプル資料 |
| **edia** | swoosh背景＋対角レイアウト＋グラデーション | **ロードマップ・戦略資料** |

## レイアウト一覧

Markdown の H1 に `[layout]` を指定:

| レイアウト | 記法 | 用途 |
|-----------|------|------|
| title | `# [title] タイトル` | 表紙 |
| content | `# [content] タイトル` | 箇条書き |
| two-col | `# [two-col] タイトル` | 2列比較 |
| grid | `# [grid] タイトル` | カード型グリッド |
| **roadmap** | `# [roadmap] タイトル` | **ロードマップ（対角フロー）** |
| steps | `# [steps] タイトル` | ステップ階段 |
| hub | `# [hub] タイトル` | ハブ＆スポーク |
| summary | `# [summary] タイトル` | まとめ（円＋メッセージ） |

## Markdown 記法

### 基本構造

```markdown
---
theme: edia
font_jp: 游ゴシック
---

# [layout] スライドタイトル
> サブタイトル（引用）
`タグ1` `タグ2`

## セクション1
- 箇条書き
  - ネスト

---              ← スライド区切り

# [layout] 次のスライド
```

### roadmap レイアウト（拡張記法）

```markdown
# [roadmap] ロードマップタイトル
> サブタイトル行1
> サブタイトル行2

## Phase 1 (2026-2027) | ヘッダーラベル
### 可能
- ◎可能：技術的に可能な項目
### 限界
- △限界：制約事項
### 施策 | 下段ヘッダーラベル
- ◇施策カテゴリ
- ◆具体的施策

## ギャップ分析
### ギャップ要因
- 要因1
### 解決の方向性
- 方向性1
```

**roadmap 自動機能（edia テーマ時）:**
- swoosh 曲線矢印の背景を自動配置
- Phase 数に応じた対角レイアウト自動計算
- ◎/△/◇/◆ プレフィックスで文字色を自動設定
- `### 可能`/`### 限界` → 上段、`### 施策` → 下段に自動振り分け
- `## ギャップ分析` → 左下に吹き出しボックスを自動生成
- ヘッダーに影効果を自動付与

## 手順

### Step 1: Markdown 作成
1. Obsidian またはテキストエディタでスライド内容を Markdown で記述
2. frontmatter でテーマとフォントを指定
3. サンプル: `docs/samples/EDIA_roadmap_sample.md`

### Step 2: 変換実行
```bash
python scripts/md2pptx.py <input.md> [-o <output.pptx>] [-t <theme>]
```

### Step 3: 確認・微調整
1. 生成された PPTX を PowerPoint で開いて確認
2. 必要に応じて PowerPoint 上で位置・サイズを微調整
3. 大幅な変更が必要なら Markdown を修正して再生成

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| ツール未インストール | `which` 失敗 | インストール手順を提示。該当チェックをスキップ |
| ツール実行エラー | 非0終了コード | stderr を報告。対象を分割して再実行を提案 |
| 出力パース失敗 | フォーマット不一致 | 生出力を表示し手動確認を促す |

## ナレッジ参照
- `.claude/knowledge/md2pptx_guide.md` — 詳細ガイド・カスタマイズ方法
- `.claude/knowledge/pptx_advanced_shapes.md` — python-pptx 高度図形操作
