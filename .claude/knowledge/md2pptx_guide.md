# md2pptx 詳細ガイド

Markdown → PowerPoint 自動変換ツール。Obsidian でコンテンツを書き、コマンド一発でデザイン済みスライドを生成する。

スクリプト: `scripts/md2pptx.py`
サンプル: `docs/samples/EDIA_roadmap_sample.md`

---

## 1. テーマシステム

テーマは frontmatter の `theme:` で指定。各テーマは 11 色のカラーパレットを持つ。

```yaml
---
theme: edia          # earth | corporate | ocean | mono | edia
font_jp: 游ゴシック   # 日本語フォント
font_en: Segoe UI    # 英語フォント
---
```

### テーマ別カラー

| キー | earth | corporate | ocean | edia |
|------|-------|-----------|-------|------|
| primary | #E07A5F | #0066CC | #0077B6 | #0000FF |
| secondary | #81B29A | #009966 | #00B4D8 | #002060 |
| accent | #F2CC8F | #FF9900 | #90E0EF | #FFFF00 |
| dark3 (警告色) | #5B7553 | #006644 | #0096C7 | #FF0000 |

### edia テーマの特殊機能

edia テーマは roadmap レイアウトで以下を自動有効化:
- **swooshArrow 背景**: グラデーション付き曲線矢印 (上段=primary系, 下段=secondary系)
- **対角レイアウト**: Phase が左下→右上に斜めに配置
- **影効果**: ヘッダーボックスにドロップシャドウ
- **凡例**: 左上に凡例ボックス、右下にバッジ
- **吹き出し**: ギャップ分析セクションを wedgeRectCallout で描画
- **黄色サブタイトル帯**: 引用行を黄色帯 + 赤枠で表示

---

## 2. レイアウト詳細

### roadmap（ロードマップ）

最も高機能なレイアウト。Phase ベースの対角フロー。

#### H2: Phase 定義

```markdown
## Phase 1 (2026-2027) | 上段ヘッダーラベル
```

- `|` の左: Phase ラベル（年代）
- `|` の右: 上段ヘッダーボックスのテキスト

#### H3: セクション振り分け

| H3 タイトル | 配置先 | 備考 |
|------------|--------|------|
| `### 可能` | 上段コンテンツ | 箇条書き |
| `### 限界` | 上段コンテンツ | 箇条書きに続けて配置 |
| `### 施策 \| 下段ヘッダー` | 下段 | `\|` の右が下段ヘッダーテキスト |

#### 特殊セクション

```markdown
## ギャップ分析
### ギャップ要因
- 法的責任
### 解決の方向性
- 段階的導入
```

→ 左下に wedgeRectCallout（吹き出し）で自動配置

#### 文字色自動設定

| プレフィックス | 色 | 用途 |
|--------------|-----|------|
| ◎ | primary (青) | 技術的可能性 |
| △ ▲ | dark3 (赤) | 限界・制約 |
| ◇ ◆ | secondary (紺) | 施策・カテゴリ |
| → | secondary (紺) | まとめ・方向性 |
| その他 | text (黒) | 通常テキスト |

#### 自動レイアウト計算

- **列幅**: `(スライド幅 - マージン) / Phase数` で均等分割
- **対角線**: Phase 1 が最も低く、Phase N が最も高い位置
- **対角範囲**: `min(2.0, 0.55 × (N-1))` インチ
- **縦区切り線**: Phase 間に自動配置
- **swoosh背景**: 上段・下段にそれぞれ配置（edia テーマ時）

### title（表紙）

装飾円 + アクセントバー + タグバッジ。

### content（箇条書き）

ヘッダー帯 + 箇条書き。H2 でセクション分割。

### two-col（2列比較）

2つの H2 セクションを左右に配置。中央に区切り線。

### grid（カードグリッド）

H2 セクションを 2〜4 列のカードに配置。自動折り返し。

### steps（階段）

H2 セクションを下から上へ階段状に配置。

### hub（ハブ＆スポーク）

中央の円 + 周囲にサテライトボックスを放射状配置。

### summary（まとめ）

H2 セクションを円で表示 + 引用をメッセージボックスで表示。

---

## 3. カスタマイズ

### 新テーマ追加

`scripts/md2pptx.py` の `THEMES` 辞書に追加:

```python
"my_theme": {
    "primary": (0xRR, 0xGG, 0xBB),
    "secondary": (0xRR, 0xGG, 0xBB),
    "accent": (0xRR, 0xGG, 0xBB),
    "dark": (0xRR, 0xGG, 0xBB),
    "bg": (0xRR, 0xGG, 0xBB),
    "text": (0xRR, 0xGG, 0xBB),
    "white": (0xFF, 0xFF, 0xFF),
    "light1": (0xRR, 0xGG, 0xBB),
    "light2": (0xRR, 0xGG, 0xBB),
    "dark2": (0xRR, 0xGG, 0xBB),
    "dark3": (0xRR, 0xGG, 0xBB),
},
```

### swoosh 有効テーマを追加

`lay_roadmap` 内の `swoosh = self.theme_name in ("edia",)` にテーマ名を追加。

---

## 4. トラブルシューティング

| 症状 | 原因 | 対策 |
|------|------|------|
| swoosh が見えない | alpha 値が低すぎる | `_add_swoosh` の alpha 値を調整 |
| 矢印がはみ出る | width が大きすぎる | left + width ≤ 13.333 を確認 |
| 文字が切れる | コンテンツ量過多 | フォントサイズ or ボックス高さを調整 |
| テーマが適用されない | frontmatter 未設定 | `---` で囲んだ YAML ブロックを確認 |

---

## 5. 技術メモ

### swooshArrow 実装

python-pptx は swooshArrow を直接サポートしない。実装手順:
1. `MSO_SHAPE.ROUNDED_RECTANGLE` で図形作成
2. XML の `prstGeom prst` を `swooshArrow` に書き換え
3. `avLst` で adj1=25000, adj2=25000 を設定
4. `gradFill` でグラデーション（右→左: ang=10800000）
5. alpha で半透明化（80%/50%）

### スライドサイズ

ワイドスクリーン: 13.333" × 7.5"（デフォルト）

### 単位系

| API | 単位 |
|-----|------|
| Inches(n) | インチ → EMU 変換 |
| Pt(n) | ポイント → EMU 変換 |
| 1 inch | = 914400 EMU |
