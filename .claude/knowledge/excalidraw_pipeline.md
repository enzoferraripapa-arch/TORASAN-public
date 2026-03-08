# Excalidraw → DOCX ダイアグラムパイプライン

Obsidian Excalidraw で作成したダイアグラムを Word 文書に埋め込むための変換パイプライン。

---

## 1. パイプライン概要

```
.excalidraw.md  →  excalidraw2svg.js  →  .excalidraw.svg
                                              ↓
                                         svg2png.py  →  .png
                                                          ↓
                                                    generate_*.py  →  .docx
```

| ステップ | スクリプト | 入力 | 出力 |
|---------|-----------|------|------|
| 1. SVG 生成 | `scripts/excalidraw2svg.js` | `docs/diagrams/excalidraw/*.excalidraw.md` | `*.excalidraw.svg` |
| 2. PNG 変換 | `scripts/svg2png.py` | `*.excalidraw.svg` | `docs/diagrams/png/*.png` |
| 3. DOCX 埋め込み | `scripts/generate_architecture_doc.py` 等 | `*.png` | `docs/*.docx` |

## 2. コマンド（一括実行）

```bash
node scripts/excalidraw2svg.js      # Step 1: Excalidraw → SVG
python scripts/svg2png.py           # Step 2: SVG → PNG (scale=2.0)
python scripts/generate_architecture_doc.py  # Step 3: PNG → DOCX
```

## 3. ダイアグラム一覧

| # | ファイル名 | 内容 | 使用先 |
|---|----------|------|-------|
| 01 | `01_repo_structure` | リポジトリ構成図 | アーキテクチャ 2.1 |
| 02 | `02_distribution_model` | 二層配布モデル | アーキテクチャ 3.3 |
| 03 | `03_memory_architecture` | メモリ・状態管理 | アーキテクチャ 6.0 |
| 04 | `04_vmodel` | SPICE V字モデル | アーキテクチャ 7.1 |
| 05 | `05_tool_resolution` | ツールパス解決フロー | アーキテクチャ 10.1 |

## 4. 新規ダイアグラム追加手順

1. Obsidian で `docs/diagrams/excalidraw/XX_name.excalidraw.md` を作成
2. `excalidraw2svg.js` の `targets` 配列に `'XX_name'` を追加
3. `generate_*.py` で `add_diagram_image(doc, "XX_name.png", caption="...")` を呼び出し
4. パイプライン一括実行

## 5. 画像埋め込み API

```python
# generate_architecture_doc.py 内のヘルパー
add_diagram_image(doc, "01_repo_structure.png",
                  width=Inches(6.0),
                  caption="図 2-1: リポジトリ構成")
# 戻り値: True（成功）/ False（PNG なし → ASCII フォールバック推奨）
```

## 6. SVG の微調整方法

| 調整内容 | 方法 |
|---------|------|
| テキスト・数値修正 | SVG ファイルをテキスト編集（`<tspan>` 要素） |
| レイアウト変更 | Obsidian Excalidraw で GUI 編集 → 再エクスポート |
| フォント変更 | SVG 内 `font-family` 属性を一括置換 |
| 色・スタイル全体 | `excalidraw2svg.js` の `renderElement()` を修正 |
| スケール変更 | `svg2png.py` の `scale` パラメータ（デフォルト 2.0） |

## 7. トラブルシューティング

### 日本語が文字化け（豆腐）する
- **原因**: SVG のフォントに日本語グリフがない
- **対処**: SVG 内の `font-family` を `"Meiryo, Yu Gothic, sans-serif"` に変更
- **根本対策**: fontconfig に Windows フォントを登録済み
  - `C:\msys64\ucrt64\etc\fonts\local.conf` → `<dir>C:/Windows/Fonts</dir>`
  - `fc-cache -fv` で 960 フォントをキャッシュ

### cairosvg / cairocffi が DLL を見つけられない
- **原因**: Windows では Cairo C ライブラリが標準 PATH にない
- **対処**: `cairocffi/__init__.py` の filenames にフルパスをパッチ済み
  - `r'C:\msys64\ucrt64\bin\libcairo-2.dll'` を先頭に追加
- **依存**: MSYS2 `mingw-w64-ucrt-x86_64-cairo` パッケージ

### excalidraw2svg.js で decompression failed
- **原因**: `.excalidraw.md` が非圧縮形式（Obsidian 設定による）
- **対処**: Obsidian Excalidraw プラグイン設定で「Compress」を有効にするか、
  非圧縮 JSON を直接パースするよう JS を拡張

### PNG のファイルサイズが大きい
- `svg2png.py` の `scale` を 1.5 や 1.0 に下げる
- `add_diagram_image()` の `width` を小さくすれば DOCX 上の表示は縮小

## 8. 前提環境

| 依存 | バージョン | インストール |
|------|----------|------------|
| Node.js | 24+ | — |
| Python | 3.14+ | — |
| cairosvg | 2.8+ | `pip install cairosvg` |
| cairocffi | 1.7+ | `pip install cairocffi`（cairosvg 依存） |
| MSYS2 Cairo | — | `pacman -S mingw-w64-ucrt-x86_64-cairo` |
| python-docx | 1.1+ | `pip install python-docx` |
