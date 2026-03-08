# UML 作図リファレンス — 編集可能な Office 形式

仕様書（PPTX / DOCX）に編集可能な UML 図を埋め込むための技術リファレンス。

---

## 1. 推奨アプローチ

| アプローチ | 編集可能 | 品質 | 推奨度 |
|-----------|---------|------|--------|
| **python-pptx ネイティブ図形** | Yes | 高 | **推奨** |
| PlantUML → PNG 埋め込み | No | 高 | 補助的 |
| Mermaid → SVG 埋め込み | No | 中 | 補助的 |
| DrawingML XML 直接構築 | Yes | 高 | 上級者向け |

**結論**: python-pptx のネイティブ図形（AutoShape + コネクタ）で UML を構築するのが最適。
PowerPoint 上でユーザーが自由に編集・移動・リサイズできる。

**python-docx の制限**: Word は浮動図形（floating shapes）の作成 API を持たない。
仕様書への UML 埋め込みは PPTX で作図し、Word には画像として挿入するのが現実的。

---

## 2. UML 要素 → python-pptx マッピング

### 2.1 クラス図

| UML 要素 | python-pptx 実装 |
|---------|-----------------|
| クラスボックス | RECTANGLE（3区画: 名前/属性/操作をテキストフレームの段落で分離） |
| インターフェース | RECTANGLE + 点線枠 + `<<interface>>` ステレオタイプ |
| 抽象クラス | RECTANGLE + イタリック名 |
| 継承（汎化） | STRAIGHT コネクタ + 三角矢印（tailEnd type="triangle", filled） |
| 実装 | STRAIGHT コネクタ + 点線 + 三角矢印 |
| 関連 | STRAIGHT コネクタ |
| 集約 | STRAIGHT コネクタ + ダイヤモンド（headEnd type="diamond"） |
| コンポジション | STRAIGHT コネクタ + 塗りダイヤモンド |
| 依存 | STRAIGHT コネクタ + 点線 + 矢印 |
| 多重度 | コネクタ端付近に add_textbox で "1", "0..*" |

#### クラスボックス実装例

```python
from pptx.util import Inches, Pt, Emu
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN
from pptx.enum.shapes import MSO_SHAPE

def add_class_box(slide, left, top, width, name, attributes, operations,
                  is_abstract=False, stereotype=None):
    """UMLクラスボックスを追加"""
    # 外枠
    shape = slide.shapes.add_shape(
        MSO_SHAPE.RECTANGLE, left, top, width, Inches(0.1)  # 高さは後で調整
    )
    shape.fill.solid()
    shape.fill.fore_color.rgb = RGBColor(0xFF, 0xFF, 0xFF)
    shape.line.color.rgb = RGBColor(0x00, 0x00, 0x00)
    shape.line.width = Pt(1)

    tf = shape.text_frame
    tf.word_wrap = True
    tf.margin_left = Emu(Inches(0.08))
    tf.margin_right = Emu(Inches(0.08))
    tf.margin_top = Emu(Inches(0.04))

    # ステレオタイプ（あれば）
    if stereotype:
        p = tf.paragraphs[0]
        p.alignment = PP_ALIGN.CENTER
        run = p.add_run()
        run.text = f"<<{stereotype}>>"
        run.font.size = Pt(8)
        run.font.italic = True
        p = tf.add_paragraph()
    else:
        p = tf.paragraphs[0]

    # クラス名
    p.alignment = PP_ALIGN.CENTER
    run = p.add_run()
    run.text = name
    run.font.size = Pt(10)
    run.font.bold = True
    if is_abstract:
        run.font.italic = True

    # 区切り線（Unicode横線で簡易表現）
    sep = tf.add_paragraph()
    sep.alignment = PP_ALIGN.CENTER
    run = sep.add_run()
    run.text = "─" * int(width / Inches(0.08))
    run.font.size = Pt(4)
    run.font.color.rgb = RGBColor(0xCC, 0xCC, 0xCC)

    # 属性
    for attr in attributes:
        p = tf.add_paragraph()
        p.alignment = PP_ALIGN.LEFT
        run = p.add_run()
        run.text = attr  # e.g. "+ name: String"
        run.font.size = Pt(8)
        run.font.name = "Consolas"

    # 区切り線
    sep2 = tf.add_paragraph()
    sep2.alignment = PP_ALIGN.CENTER
    run = sep2.add_run()
    run.text = "─" * int(width / Inches(0.08))
    run.font.size = Pt(4)
    run.font.color.rgb = RGBColor(0xCC, 0xCC, 0xCC)

    # 操作
    for op in operations:
        p = tf.add_paragraph()
        p.alignment = PP_ALIGN.LEFT
        run = p.add_run()
        run.text = op  # e.g. "+ getName(): String"
        run.font.size = Pt(8)
        run.font.name = "Consolas"

    # 高さを自動フィットに設定
    from pptx.enum.text import MSO_AUTO_SIZE
    tf.auto_size = MSO_AUTO_SIZE.SHAPE_TO_FIT_TEXT

    return shape
```

### 2.2 シーケンス図

| UML 要素 | python-pptx 実装 |
|---------|-----------------|
| ライフライン | RECTANGLE（ヘッダ） + 点線（垂直 STRAIGHT コネクタ、dash_style=DASH） |
| メッセージ | STRAIGHT コネクタ + 矢印 + テキストボックス（メッセージ名） |
| 応答メッセージ | STRAIGHT コネクタ + 点線 + 矢印 |
| 活性区間 | 細い RECTANGLE（塗りつぶし薄灰） |
| 自己メッセージ | ELBOW コネクタまたは FreeformBuilder |
| フレーム（opt/alt/loop） | RECTANGLE（枠のみ）+ テキストボックス（ラベル） |

#### シーケンス図の配置戦略

```python
LIFELINE_SPACING = Inches(2.0)   # ライフライン間隔
MESSAGE_Y_STEP = Inches(0.4)     # メッセージ間の垂直間隔
HEADER_HEIGHT = Inches(0.5)
ACTIVATION_WIDTH = Inches(0.15)

def layout_sequence(actors, messages):
    """シーケンス図の座標を計算"""
    positions = {}
    for i, actor in enumerate(actors):
        x = Inches(1) + LIFELINE_SPACING * i
        positions[actor] = x

    y = Inches(1.5)  # メッセージ開始Y
    msg_positions = []
    for msg in messages:
        msg_positions.append({
            'from_x': positions[msg['from']],
            'to_x': positions[msg['to']],
            'y': y,
            'text': msg['text'],
            'is_return': msg.get('is_return', False)
        })
        y += MESSAGE_Y_STEP

    return positions, msg_positions
```

### 2.3 ステートマシン図

| UML 要素 | python-pptx 実装 |
|---------|-----------------|
| 状態 | ROUNDED_RECTANGLE |
| 開始擬似状態 | OVAL（小さい黒丸、塗りつぶし黒） |
| 終了状態 | OVAL（二重丸: 大OVAL + 小OVAL黒） |
| 遷移 | STRAIGHT / ELBOW コネクタ + 矢印 + テキストボックス |
| 複合状態 | ROUNDED_RECTANGLE（大きい）内に小さい状態を配置 |
| フォーク/ジョイン | 細い RECTANGLE（黒塗り、幅大・高さ小） |

### 2.4 アクティビティ図

| UML 要素 | python-pptx 実装 |
|---------|-----------------|
| アクション | ROUNDED_RECTANGLE |
| 決定ノード | DIAMOND |
| 開始/終了 | OVAL（黒丸 / 二重丸） |
| フォーク/ジョイン | 細い RECTANGLE（黒塗り） |
| スイムレーン | 大きい RECTANGLE 枠 + テキストボックス（レーン名） |
| フロー | STRAIGHT コネクタ + 矢印 |

### 2.5 コンポーネント図

| UML 要素 | python-pptx 実装 |
|---------|-----------------|
| コンポーネント | RECTANGLE + 左辺に小さい RECTANGLE 2個（タブ表現） |
| インターフェース（提供） | OVAL（小さい丸）+ 線 |
| インターフェース（要求） | 半円（FreeformBuilder）+ 線 |
| 依存 | 点線 STRAIGHT コネクタ + 矢印 |
| ポート | RECTANGLE（小さい正方形、枠線上） |

---

## 3. レイアウトアルゴリズム

### 3.1 グリッドレイアウト（クラス図・コンポーネント図向け）

```python
def grid_layout(elements, cols=3, x_start=Inches(0.5), y_start=Inches(1),
                x_gap=Inches(2.5), y_gap=Inches(2)):
    """要素をグリッド配置"""
    positions = []
    for i, elem in enumerate(elements):
        row = i // cols
        col = i % cols
        x = x_start + col * x_gap
        y = y_start + row * y_gap
        positions.append((elem, x, y))
    return positions
```

### 3.2 階層レイアウト（継承ツリー向け）

```python
def tree_layout(root, children_map, x_center=Inches(5), y_start=Inches(1),
                level_gap=Inches(1.8), sibling_gap=Inches(2.5)):
    """ツリー構造を階層配置"""
    positions = {}

    def layout_subtree(node, x, y, available_width):
        children = children_map.get(node, [])
        if not children:
            positions[node] = (x, y)
            return

        positions[node] = (x, y)
        child_width = available_width / max(len(children), 1)
        start_x = x - available_width / 2 + child_width / 2

        for i, child in enumerate(children):
            cx = start_x + i * child_width
            layout_subtree(child, cx, y + level_gap, child_width)

    layout_subtree(root, x_center, y_start, Inches(8))
    return positions
```

---

## 4. 矢印タイプ早見表

| UML 関係 | 線種 | 始点 | 終点 | XML headEnd/tailEnd |
|---------|------|------|------|-------------------|
| 関連 | 実線 | なし | なし | — |
| 有向関連 | 実線 | なし | 開矢印 | tailEnd type="arrow" |
| 継承 | 実線 | なし | 白三角 | tailEnd type="triangle" (unfilled) |
| 実装 | 点線 | なし | 白三角 | tailEnd type="triangle" + dash |
| 依存 | 点線 | なし | 開矢印 | tailEnd type="arrow" + dash |
| 集約 | 実線 | 白菱形 | なし | headEnd type="diamond" (unfilled) |
| コンポジション | 実線 | 黒菱形 | なし | headEnd type="diamond" (filled) |
| メッセージ（同期） | 実線 | なし | 黒三角 | tailEnd type="triangle" (filled) |
| メッセージ（非同期） | 実線 | なし | 開矢印 | tailEnd type="arrow" |
| 応答 | 点線 | なし | 開矢印 | tailEnd type="arrow" + dash |

### 白抜き三角の XML 実装

```python
from pptx.oxml.ns import qn

def set_arrow(connector, head_type=None, tail_type=None, filled=True):
    """コネクタに矢印マーカーを設定"""
    line = connector._element.spPr.ln

    if head_type:
        existing = line.find(qn('a:headEnd'))
        if existing is not None:
            line.remove(existing)
        head = line.makeelement(qn('a:headEnd'), {})
        head.set('type', head_type)
        head.set('w', 'med')
        head.set('len', 'med')
        line.append(head)

    if tail_type:
        existing = line.find(qn('a:tailEnd'))
        if existing is not None:
            line.remove(existing)
        tail = line.makeelement(qn('a:tailEnd'), {})
        tail.set('type', tail_type)
        tail.set('w', 'med')
        tail.set('len', 'med')
        line.append(tail)

# 白抜き三角（継承）: fill="hollow" を追加
# → DrawingML では type="triangle" + fill 属性で制御
# ただし python-pptx では fill 属性のサポートが限定的
# 回避策: type="triangle" で三角を配置し、図形の塗り色で白抜き表現
```

---

## 5. DOCX への UML 埋め込み

python-docx は浮動図形をサポートしないため:

### 推奨ワークフロー

1. python-pptx で UML 図を作成（編集可能な図形として）
2. 作成した PPTX から特定スライドを PNG/EMF にエクスポート
3. python-docx で Word 文書に画像として挿入

```python
# Word への画像挿入
from docx import Document
from docx.shared import Inches

doc = Document()
doc.add_heading('システムアーキテクチャ', level=2)
doc.add_picture('architecture_diagram.png', width=Inches(6))
doc.add_paragraph('図 X.X: システムアーキテクチャ図', style='Caption')
```

### 代替: インライン表で簡易構造表現

複雑な図が不要な場合、Word テーブルで構造を表現:

```python
# モジュール構造をテーブルで表現
table = doc.add_table(rows=4, cols=3)
table.style = 'Table Grid'
# ヘッダー行
for i, header in enumerate(['モジュール', '責務', '依存先']):
    table.rows[0].cells[i].text = header
```

---

## 6. スライドサイズと推奨マージン

| 設定 | 値 |
|------|-----|
| スライド幅 | 10 inches (25.4 cm) — ワイドスクリーン |
| スライド高さ | 7.5 inches (19.05 cm) |
| 左右マージン | 0.5 inches |
| 上マージン | 1.0 inches（タイトル領域） |
| 下マージン | 0.5 inches |
| 有効描画エリア | 9.0 x 6.0 inches |

---

## 7. 仕様書テンプレート構成（UML 図を含む場合）

```
スライド 1: 表紙
スライド 2: 目次
スライド 3: システムコンテキスト図（コンポーネント図）
スライド 4: クラス図（主要クラス）
スライド 5-N: シーケンス図（ユースケース毎）
スライド N+1: ステートマシン図（主要状態遷移）
スライド N+2: 用語定義
```

各スライドにタイトル + 図番号 + 図を配置。
編集可能な形式のため、レビュー後にユーザーが PowerPoint 上で直接修正可能。
