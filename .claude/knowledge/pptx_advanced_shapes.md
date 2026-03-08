# python-pptx 高度な図形操作リファレンス

python-pptx で複雑な図形・配置・コネクタ・グループ化を実現するための技術リファレンス。

---

## 1. AutoShape 一覧（主要カテゴリ）

python-pptx は MSO_AUTO_SHAPE_TYPE 経由で 182 種類の AutoShape をサポート。

| カテゴリ | 代表的な図形 | Enum 値 |
|---------|------------|---------|
| 基本図形 | RECTANGLE, ROUNDED_RECTANGLE, OVAL, DIAMOND | 1, 5, 9, 4 |
| 矢印 | RIGHT_ARROW, LEFT_ARROW, UP_ARROW, DOWN_ARROW, CHEVRON | 33-36, 52 |
| フローチャート | FLOWCHART_PROCESS, FLOWCHART_DECISION, FLOWCHART_TERMINATOR | 109-137 |
| ブロック | ACTION_BUTTON_*, BEVEL, CAN | 189-202, 15, 13 |
| 星・バナー | STAR_4_POINT 〜 STAR_32_POINT, RIBBON, WAVE | 187, 59, 60, 64 |
| 吹き出し | CALLOUT_*, OVAL_CALLOUT, CLOUD_CALLOUT | 41-44, 107, 108 |
| 数式記号 | MATH_PLUS, MATH_MINUS, MATH_MULTIPLY, MATH_DIVIDE | 161-168 |

### 使用方法

```python
from pptx.enum.shapes import MSO_SHAPE
from pptx.util import Inches, Pt, Emu

slide = prs.slides.add_slide(prs.slide_layouts[6])  # blank layout
shape = slide.shapes.add_shape(
    MSO_SHAPE.ROUNDED_RECTANGLE,
    Inches(1), Inches(1),   # left, top
    Inches(3), Inches(1.5)  # width, height
)
shape.text = "テキスト"
```

---

## 2. コネクタ（接続線）

### 基本コネクタ

```python
from pptx.enum.shapes import MSO_CONNECTOR_TYPE

connector = slide.shapes.add_connector(
    MSO_CONNECTOR_TYPE.STRAIGHT,  # STRAIGHT / ELBOW / CURVE
    Inches(1), Inches(1),  # begin_x, begin_y
    Inches(4), Inches(3),  # end_x, end_y
)
```

### 図形への接続（begin_connect / end_connect）

```python
connector.begin_connect(shape_a, connection_idx=0)  # 0=top, 1=right, 2=bottom, 3=left
connector.end_connect(shape_b, connection_idx=2)
```

connection_idx は図形の接続ポイント番号。通常:
- 0: 上中央
- 1: 右中央
- 2: 下中央
- 3: 左中央

### 矢印マーカー（XML 直接操作）

python-pptx は矢印ヘッド/テールの高レベル API を持たないため、lxml で直接操作:

```python
from pptx.oxml.ns import qn

line = connector._element.spPr.ln
# 矢印ヘッド追加
head = line.makeelement(qn('a:headEnd'), {})
head.set('type', 'triangle')   # triangle, stealth, diamond, oval, arrow, none
head.set('w', 'med')           # sm, med, lg
head.set('len', 'med')         # sm, med, lg
line.append(head)

# 矢印テール追加
tail = line.makeelement(qn('a:tailEnd'), {})
tail.set('type', 'triangle')
tail.set('w', 'med')
tail.set('len', 'med')
line.append(tail)
```

### 線スタイル

```python
from pptx.util import Pt
from pptx.dml.color import RGBColor
from pptx.enum.dml import MSO_LINE_DASH_STYLE

connector.line.width = Pt(1.5)
connector.line.color.rgb = RGBColor(0, 0, 0)
connector.line.dash_style = MSO_LINE_DASH_STYLE.DASH  # SOLID, DASH, DOT, DASH_DOT
```

---

## 3. FreeformBuilder（自由図形）

任意の多角形・曲線を描画:

```python
builder = slide.shapes.build_freeform(Inches(1), Inches(1))
builder.add_line_segments([
    (Inches(3), Inches(1)),    # 右へ線
    (Inches(3), Inches(2.5)),  # 下へ線
    (Inches(1), Inches(2.5)),  # 左へ線
])
shape = builder.convert_to_shape()  # 自動で閉じる
shape.fill.solid()
shape.fill.fore_color.rgb = RGBColor(0xCC, 0xE5, 0xFF)
```

曲線セグメント（ベジェ）:

```python
builder.add_line_segments([(x1, y1)])
# ベジェ曲線は DrawingML XML で cubicBezTo を直接挿入
```

---

## 4. グループ化（GroupShapes）

```python
from pptx.util import Inches, Emu
from lxml import etree
from pptx.oxml.ns import qn

# グループ図形の XML を構築
grpSp = slide.shapes._spTree.makeelement(qn('p:grpSp'), {})
grpSpPr = grpSp.makeelement(qn('p:grpSpPr'), {})
xfrm = grpSpPr.makeelement(qn('a:xfrm'), {})
off = xfrm.makeelement(qn('a:off'), {'x': str(Emu(Inches(1))), 'y': str(Emu(Inches(1)))})
ext = xfrm.makeelement(qn('a:ext'), {'cx': str(Emu(Inches(4))), 'cy': str(Emu(Inches(3)))})
chOff = xfrm.makeelement(qn('a:chOff'), {'x': '0', 'y': '0'})
chExt = xfrm.makeelement(qn('a:chExt'), {'cx': str(Emu(Inches(4))), 'cy': str(Emu(Inches(3)))})
xfrm.extend([off, ext, chOff, chExt])
grpSpPr.append(xfrm)
grpSp.append(grpSpPr)

# 子図形を grpSp に追加（sp 要素を移動）
# grpSp.append(shape1._element)
# grpSp.append(shape2._element)
slide.shapes._spTree.append(grpSp)
```

**注意**: python-pptx にはグループ作成の高レベル API がない。XML 直接操作が必要。

---

## 5. テーブル

```python
table_shape = slide.shapes.add_table(
    rows=4, cols=3,
    left=Inches(0.5), top=Inches(1.5),
    width=Inches(9), height=Inches(3)
)
table = table_shape.table

# セル操作
cell = table.cell(0, 0)
cell.text = "ヘッダー"

# セルの書式設定
from pptx.util import Pt
from pptx.dml.color import RGBColor

para = cell.text_frame.paragraphs[0]
para.font.size = Pt(11)
para.font.bold = True
para.font.color.rgb = RGBColor(0xFF, 0xFF, 0xFF)

# セルの背景色
from pptx.oxml.ns import qn
tcPr = cell._tc.get_or_add_tcPr()
solidFill = tcPr.makeelement(qn('a:solidFill'), {})
srgb = solidFill.makeelement(qn('a:srgbClr'), {'val': '003366'})
solidFill.append(srgb)
tcPr.append(solidFill)

# セル結合
table.cell(0, 0).merge(table.cell(0, 2))  # 行0の列0-2を結合

# 列幅設定
table.columns[0].width = Inches(2)
table.columns[1].width = Inches(4)
table.columns[2].width = Inches(3)
```

---

## 6. テキスト書式の詳細

```python
from pptx.util import Pt, Emu
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN, MSO_ANCHOR, MSO_AUTO_SIZE

tf = shape.text_frame
tf.word_wrap = True
tf.auto_size = MSO_AUTO_SIZE.SHAPE_TO_FIT_TEXT
tf.margin_left = Emu(Inches(0.1))
tf.margin_top = Emu(Inches(0.05))

# 段落
para = tf.paragraphs[0]
para.alignment = PP_ALIGN.CENTER
para.space_before = Pt(6)
para.space_after = Pt(6)

# ラン（テキスト装飾単位）
run = para.add_run()
run.text = "太字テキスト"
run.font.size = Pt(12)
run.font.bold = True
run.font.italic = False
run.font.color.rgb = RGBColor(0x00, 0x00, 0x00)
run.font.name = "Meiryo UI"

# 複数段落（改行）
new_para = tf.add_paragraph()
new_para.text = "2行目"
new_para.alignment = PP_ALIGN.LEFT
```

---

## 7. 塗り・線スタイル

```python
from pptx.dml.color import RGBColor
from pptx.enum.dml import MSO_THEME_COLOR

# 塗りつぶし
shape.fill.solid()
shape.fill.fore_color.rgb = RGBColor(0xCC, 0xE5, 0xFF)

# グラデーション
shape.fill.gradient()
shape.fill.gradient_stops[0].color.rgb = RGBColor(0x00, 0x66, 0xCC)
shape.fill.gradient_stops[1].color.rgb = RGBColor(0xCC, 0xE5, 0xFF)

# パターン（XML直接操作が必要）

# 線
shape.line.color.rgb = RGBColor(0x00, 0x33, 0x66)
shape.line.width = Pt(1)
shape.line.dash_style = MSO_LINE_DASH_STYLE.SOLID

# 影効果（XML直接操作）
from pptx.oxml.ns import qn
spPr = shape._element.spPr
effectLst = spPr.makeelement(qn('a:effectLst'), {})
outerShdw = effectLst.makeelement(qn('a:outerShdw'), {
    'blurRad': '50800', 'dist': '38100', 'dir': '2700000', 'algn': 'tl'
})
srgb = outerShdw.makeelement(qn('a:srgbClr'), {'val': '000000'})
alpha = srgb.makeelement(qn('a:alpha'), {'val': '40000'})
srgb.append(alpha)
outerShdw.append(srgb)
effectLst.append(outerShdw)
spPr.append(effectLst)
```

---

## 8. Z オーダー（重なり順）

```python
from pptx.oxml.ns import qn

spTree = slide.shapes._spTree
# shape を最前面に移動
sp = shape._element
spTree.remove(sp)
spTree.append(sp)

# shape を最背面に移動
spTree.remove(sp)
spTree.insert(2, sp)  # index 0=cNvGrpSpPr, 1=grpSpPr, 2以降=図形
```

---

## 9. スライドレイアウトとマスター

```python
# レイアウト一覧
for i, layout in enumerate(prs.slide_layouts):
    print(f"{i}: {layout.name}")

# 一般的なレイアウト
# 0: Title Slide
# 1: Title and Content
# 5: Title Only
# 6: Blank

# プレースホルダー操作
for ph in slide.placeholders:
    print(f"idx={ph.placeholder_format.idx}, type={ph.placeholder_format.type}")

# タイトルプレースホルダー
slide.placeholders[0].text = "スライドタイトル"
```

---

## 10. 画像

```python
# 画像追加
pic = slide.shapes.add_picture(
    'image.png',
    left=Inches(1), top=Inches(1),
    width=Inches(4)  # height は自動計算（アスペクト比維持）
)

# 画像のトリミング（XML直接操作）
from pptx.oxml.ns import qn
blipFill = pic._element.blipFill
srcRect = blipFill.makeelement(qn('a:srcRect'), {
    'l': '10000', 't': '10000', 'r': '10000', 'b': '10000'  # 10% crop each side
})
blipFill.insert(1, srcRect)
```

---

## 11. 単位系

| 単位 | 変換 | 用途 |
|------|------|------|
| Inches(n) | → EMU | 配置・サイズ |
| Pt(n) | → EMU | フォントサイズ・線幅 |
| Cm(n) | → EMU | 配置（メトリック） |
| Emu(n) | 生EMU値 | 精密位置指定 |

1 inch = 914400 EMU, 1 pt = 12700 EMU, 1 cm = 360000 EMU

---

## 12. 制限事項と回避策

| 制限 | 回避策 |
|------|--------|
| 矢印マーカー API なし | XML 直接操作（§2 参照） |
| グループ作成 API なし | XML 直接操作（§4 参照） |
| 影・3D 効果 API なし | effectLst XML 操作 |
| SmartArt 作成不可 | 個別図形+コネクタで再現 |
| アニメーション操作不可 | PowerPoint で後から追加 |
| OLE オブジェクト不可 | 画像として埋め込み |
| 既存コネクタの接続先変更不可 | 削除→再作成 |
| swooshArrow 非サポート | prstGeom XML 書き換え（§14 参照） |
| グラデーション+alpha 非サポート | gradFill XML 直接構築（§14 参照） |
| wedgeRectCallout 非サポート | RECTANGULAR_CALLOUT → XML 書き換え（§14 参照） |

---

## 13. 定番カラーパレット（仕様書向け）

```python
COLORS = {
    'header_bg':    RGBColor(0x00, 0x33, 0x66),  # 濃紺
    'header_text':  RGBColor(0xFF, 0xFF, 0xFF),  # 白
    'accent':       RGBColor(0x00, 0x66, 0xCC),  # 青
    'light_bg':     RGBColor(0xCC, 0xE5, 0xFF),  # 薄青
    'success':      RGBColor(0x00, 0x80, 0x00),  # 緑
    'warning':      RGBColor(0xFF, 0x99, 0x00),  # 橙
    'danger':       RGBColor(0xCC, 0x00, 0x00),  # 赤
    'gray_bg':      RGBColor(0xF2, 0xF2, 0xF2),  # 薄灰
    'text':         RGBColor(0x33, 0x33, 0x33),  # 黒寄り灰
    'border':       RGBColor(0xCC, 0xCC, 0xCC),  # 灰線
}
```

---

## 14. 非サポート図形の XML 書き換えパターン

### swooshArrow（曲線矢印）

python-pptx は swooshArrow プリセットをサポートしない。ROUNDED_RECTANGLE で作成後に XML を書き換える:

```python
from pptx.oxml.ns import qn

s = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, left, top, width, height)
spPr = s._element.spPr

# 回転
xfrm = spPr.find(qn("a:xfrm"))
xfrm.set("rot", str(int(355 * 60000)))  # 355度

# prstGeom を swooshArrow に変更
prstGeom = spPr.find(qn("a:prstGeom"))
prstGeom.set("prst", "swooshArrow")
avLst = prstGeom.find(qn("a:avLst"))
avLst.clear()
avLst.append(avLst.makeelement(qn("a:gd"), {"name": "adj1", "fmla": "val 25000"}))
avLst.append(avLst.makeelement(qn("a:gd"), {"name": "adj2", "fmla": "val 25000"}))
```

### グラデーション + 半透明

```python
# 既存の fill を削除
for tag in ["a:solidFill", "a:gradFill", "a:noFill"]:
    old = spPr.find(qn(tag))
    if old is not None:
        spPr.remove(old)

gradFill = spPr.makeelement(qn("a:gradFill"), {"rotWithShape": "1"})
gsLst = gradFill.makeelement(qn("a:gsLst"), {})

# Stop 1: 濃い色 (0%) + 80%不透明
gs1 = gsLst.makeelement(qn("a:gs"), {"pos": "0"})
clr1 = gs1.makeelement(qn("a:srgbClr"), {"val": "4F81BD"})
clr1.append(clr1.makeelement(qn("a:alpha"), {"val": "80000"}))
gs1.append(clr1)
gsLst.append(gs1)

# Stop 2: 薄い色 (100%) + 50%不透明
gs2 = gsLst.makeelement(qn("a:gs"), {"pos": "100000"})
clr2 = gs2.makeelement(qn("a:srgbClr"), {"val": "DBE5F1"})
clr2.append(clr2.makeelement(qn("a:alpha"), {"val": "50000"}))
gs2.append(clr2)
gsLst.append(gs2)

gradFill.append(gsLst)
# 右→左: ang=10800000 (180度 × 60000)
gradFill.append(gradFill.makeelement(qn("a:lin"), {"ang": "10800000", "scaled": "1"}))

# prstGeom の直後に挿入
prstGeom.addnext(gradFill)
```

### wedgeRectCallout（吹き出し + ポインタ方向指定）

```python
s = slide.shapes.add_shape(MSO_SHAPE.RECTANGULAR_CALLOUT, left, top, width, height)
prstGeom = s._element.spPr.find(qn("a:prstGeom"))
prstGeom.set("prst", "wedgeRectCallout")
avLst = prstGeom.find(qn("a:avLst"))
avLst.clear()
# adj1: ポインタ X 位置 (0=中央, 正=右, 負=左)
# adj2: ポインタ Y 位置 (正=下, 負=上 → -95818 で上方向に大きく伸びる)
avLst.append(avLst.makeelement(qn("a:gd"), {"name": "adj1", "fmla": "val 60180"}))
avLst.append(avLst.makeelement(qn("a:gd"), {"name": "adj2", "fmla": "val -95818"}))
```

### alpha 値の目安

| val | 不透明度 | 見え方 |
|-----|---------|--------|
| 100000 | 100% | 完全に不透明 |
| 80000 | 80% | ほぼ不透明 |
| 50000 | 50% | 半透明 |
| 25000 | 25% | かなり透明 |
| 0 | 0% | 完全に透明 |
```
