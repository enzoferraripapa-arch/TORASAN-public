---
name: generate-docs
description: "Generates formatted documents (docx, xlsx) from project artifacts using python-docx. Includes Excalidraw diagram pipeline. Use when users say 'generate documents', 'create report', 'export docs', 'generate docx', 'update manuals', 'update diagrams', or 'create deliverables'. Handles: document generation, docx, xlsx, report, export, deliverables, diagrams, excalidraw."
disable-model-invocation: true
argument-hint: "[doc-type: all|arch|ops|spec|manuals|diagrams|phase|trace-matrix] [format]"
---
# /generate-docs — 成果物ドキュメント生成

V-model フェーズの成果物および管理ドキュメントを Word/Excel 形式で生成する。
Excalidraw ダイアグラムの変換・埋め込みパイプラインを含む。

引数: $ARGUMENTS（生成対象 → 省略時はユーザーに確認）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | 必要な外部ツールがインストール済み | `which {tool}` / `{tool} --version` | エラーメッセージでインストール手順を提示 |
| 2 | 対象ファイルが存在する | Glob/ls で確認 | 対象不在を報告して終了 |

## 生成可能なドキュメント

### フレームワーク管理文書
| 対象 | コマンド | 出力 |
|------|---------|------|
| 全マニュアル | `python scripts/generate_manuals.py` | docs/ 配下 3 文書 |
| アーキテクチャ仕様書 | `python scripts/generate_architecture_doc.py` | docs/TORASAN_Architecture.docx |
| 操作ガイド | `python scripts/generate_ops_manual.py` | docs/TORASAN_操作ガイド.docx |
| フレームワーク仕様書 | `python scripts/generate_spec_doc.py` | docs/TORASAN_フレームワーク仕様書.docx |
| ダイアグラムのみ | 下記「ダイアグラムパイプライン」参照 | docs/diagrams/png/*.png |

### V-model フェーズ成果物
| フェーズ | 成果物 | 形式 | 生成元 |
|---------|--------|------|-------|
| PH-01 | 安全計画書 | docx | docs/01_safety_plan.md |
| PH-02 | アイテム定義書 | docx | docs/02_item_definition.md |
| PH-03 | HARA レポート | docx | docs/03_hara.md |
| PH-04 | FSC 仕様書 | docx | docs/04_fsc.md |
| PH-05〜 | 各フェーズ成果物 | docx | docs/XX_*.md |
| — | トレーサビリティマトリクス | xlsx | 全要件ID |

## ダイアグラムパイプライン

**ナレッジ参照**: `.claude/knowledge/excalidraw_pipeline.md`

Obsidian Excalidraw で作成したダイアグラムを Word 文書に埋め込む 3 ステップ変換。

### Step D1: Excalidraw → SVG
```bash
node scripts/excalidraw2svg.js
```
- `.excalidraw.md` の compressed-json を LZ-string 解凍
- Excalidraw 要素（rect, ellipse, diamond, arrow, text）を SVG に変換
- フォント: Meiryo, Yu Gothic（日本語対応）
- 出力: `docs/diagrams/excalidraw/*.excalidraw.svg`

### Step D2: SVG → PNG
```bash
python scripts/svg2png.py
```
- cairosvg で SVG を 2 倍スケール PNG に変換
- 依存: MSYS2 Cairo DLL（`C:\msys64\ucrt64\bin\libcairo-2.dll`）
- 出力: `docs/diagrams/png/*.png`

### Step D3: PNG → DOCX（自動）
- `generate_architecture_doc.py` 内の `add_diagram_image()` が PNG を検出して埋め込み
- PNG がない場合は ASCII ダイアグラムにフォールバック

### 現行ダイアグラム一覧（5 枚）
| # | 名前 | DOCX セクション |
|---|------|----------------|
| 01 | repo_structure | アーキテクチャ 2.1 |
| 02 | distribution_model | アーキテクチャ 3.3 |
| 03 | memory_architecture | アーキテクチャ 6.0 |
| 04 | vmodel | アーキテクチャ 7.1 |
| 05 | tool_resolution | アーキテクチャ 10.1 |

## 手順

### Step 1: 対象確認
- `$ARGUMENTS` から生成対象を判定
- `diagrams`: ダイアグラムパイプラインのみ（D1→D2）
- `arch`: ダイアグラム + アーキテクチャ文書（D1→D2→D3）
- `manuals`: 全マニュアル（ダイアグラム含む）
- `all`: 全ドキュメント
- `PH-XX`: 特定フェーズ成果物

### Step 2: ダイアグラム更新（manuals / arch / all の場合）
1. `node scripts/excalidraw2svg.js` — SVG 生成
2. `python scripts/svg2png.py` — PNG 変換
3. 生成された PNG の一覧と日本語表示を確認

### Step 3: ドキュメント生成
- マニュアル系: `python scripts/generate_manuals.py` または個別スクリプト
- フェーズ成果物: app/ のジェネレータ経由
- トレーサビリティマトリクス: xlsx 生成

### Step 4: 生成後検証
1. 出力ファイルの存在・サイズ確認
2. 埋め込み画像数の検証（`doc.part.rels` でカウント）
3. 数値整合性チェック（spec_constants.py との照合）

### Step 5: 報告
```
=== ドキュメント生成完了 ===
ダイアグラム: {n}枚 PNG 生成
生成ファイル:
  - docs/TORASAN_Architecture.docx (画像{n}枚埋め込み)
  - docs/TORASAN_操作ガイド.docx
  - docs/TORASAN_フレームワーク仕様書.docx
数値検証: PASS/FAIL
```

## 新規ダイアグラム追加

1. Obsidian で `docs/diagrams/excalidraw/XX_name.excalidraw.md` を作成
2. `excalidraw2svg.js` の `targets` 配列にファイル名を追加
3. `generate_*.py` で `add_diagram_image(doc, "XX_name.png")` を呼び出し
4. `/generate-docs arch` で一括反映

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| ツール未インストール | `which` 失敗 | インストール手順を提示。該当チェックをスキップ |
| ツール実行エラー | 非0終了コード | stderr を報告。対象を分割して再実行を提案 |
| 出力パース失敗 | フォーマット不一致 | 生出力を表示し手動確認を促す |

## 注意事項
- MSYS2 Cairo が必要（`pacman -S mingw-w64-ucrt-x86_64-cairo`）
- SVG のフォントは `Meiryo, Yu Gothic` 指定（Helvetica だと日本語が豆腐化）
- 生成済みファイルは上書き前にユーザー確認
- spec_constants.py のスキル/ナレッジ数と SVG 内テキストの整合性に注意
