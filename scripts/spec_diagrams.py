"""
TORASAN 仕様書ダイアグラム生成モジュール

Pillow (300DPI PNG) で5種のダイアグラムを生成し、BytesIO で返す。
generate_spec_doc.py から呼び出され、python-docx で DOCX に埋め込まれる。

依存: Pillow (python-pptx の既存依存)
"""

import math
from io import BytesIO
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

from spec_constants import (
    SKILL_COUNT_UNIVERSAL,
    SKILL_COUNT_DOMAIN,
    SKILL_COUNT_TOTAL,
    KNOWLEDGE_COUNT_UNIVERSAL,
    KNOWLEDGE_COUNT_DOMAIN,
    KNOWLEDGE_COUNT_TOTAL,
    PHASE_COUNT,
)

# ============================================================
# カラーパレット (RGB tuples — generate_pptx.py と統一)
# ============================================================
TERRACOTTA = (0xE0, 0x7A, 0x5F)
SAGE       = (0x81, 0xB2, 0x9A)
SAND       = (0xF4, 0xF1, 0xDE)
CHARCOAL   = (0x3D, 0x40, 0x5B)
MUSTARD    = (0xF2, 0xCC, 0x8F)
OLIVE      = (0x5B, 0x75, 0x53)

LIGHT_SAGE  = (0xC8, 0xE0, 0xD2)
LIGHT_TERRA = (0xF0, 0xAA, 0x8F)
LIGHT_MUST  = (0xF9, 0xE4, 0xC0)
LIGHT_OLIVE = (0xA8, 0xBF, 0xA0)
LIGHT_CHAR  = (0x8A, 0x8D, 0xA0)

WHITE  = (0xFF, 0xFF, 0xFF)
BG     = (0xFA, 0xF9, 0xF6)  # オフホワイト背景
GRAY   = (0xBB, 0xBB, 0xBB)

DPI = 300

# ============================================================
# フォント読み込み
# ============================================================
_font_cache: dict[tuple[str, int], ImageFont.FreeTypeFont] = {}

FONT_PATHS_M = [
    'C:/Windows/Fonts/YuGothM.ttc',
    'C:/Windows/Fonts/yugothm.ttc',
    'C:/Windows/Fonts/meiryo.ttc',
    'C:/Windows/Fonts/msgothic.ttc',
]
FONT_PATHS_B = [
    'C:/Windows/Fonts/YuGothB.ttc',
    'C:/Windows/Fonts/yugothb.ttc',
    'C:/Windows/Fonts/meiryob.ttc',
    'C:/Windows/Fonts/msgothic.ttc',
]


def _find_font_path(candidates: list[str]) -> str | None:
    for p in candidates:
        if Path(p).exists():
            return p
    return None


def _font(size: int, bold: bool = False) -> ImageFont.FreeTypeFont:
    key = ('bold' if bold else 'medium', size)
    if key in _font_cache:
        return _font_cache[key]
    paths = FONT_PATHS_B if bold else FONT_PATHS_M
    fpath = _find_font_path(paths)
    if fpath:
        f = ImageFont.truetype(fpath, size)
    else:
        f = ImageFont.load_default()
    _font_cache[key] = f
    return f


# ============================================================
# 描画ヘルパー
# ============================================================

def _rounded_rect(draw: ImageDraw.ImageDraw, xy, radius, fill, outline=None, width=1):
    """角丸四角形を描画"""
    draw.rounded_rectangle(xy, radius=radius, fill=fill, outline=outline, width=width)


def _draw_arrow(draw: ImageDraw.ImageDraw, start, end, fill, width=3, head_size=14):
    """矢印（直線＋三角ヘッド）を描画"""
    draw.line([start, end], fill=fill, width=width)
    angle = math.atan2(end[1] - start[1], end[0] - start[0])
    x, y = end
    pts = [
        (x, y),
        (x - head_size * math.cos(angle - 0.35), y - head_size * math.sin(angle - 0.35)),
        (x - head_size * math.cos(angle + 0.35), y - head_size * math.sin(angle + 0.35)),
    ]
    draw.polygon(pts, fill=fill)


def _draw_dashed_line(draw: ImageDraw.ImageDraw, start, end, fill,
                      width=2, dash_len=12, gap_len=8):
    """点線を描画"""
    dx = end[0] - start[0]
    dy = end[1] - start[1]
    length = math.sqrt(dx * dx + dy * dy)
    if length == 0:
        return
    ux, uy = dx / length, dy / length
    pos = 0.0
    while pos < length:
        seg_end = min(pos + dash_len, length)
        draw.line([
            (start[0] + ux * pos, start[1] + uy * pos),
            (start[0] + ux * seg_end, start[1] + uy * seg_end),
        ], fill=fill, width=width)
        pos += dash_len + gap_len


def _draw_diamond(draw: ImageDraw.ImageDraw, center, w, h, fill, outline=None, width=2):
    """ひし形を描画"""
    cx, cy = center
    pts = [(cx, cy - h // 2), (cx + w // 2, cy), (cx, cy + h // 2), (cx - w // 2, cy)]
    draw.polygon(pts, fill=fill, outline=outline, width=width)


def _text_center(draw: ImageDraw.ImageDraw, center, text, font, fill):
    """テキストを中央寄せで描画"""
    bbox = draw.textbbox((0, 0), text, font=font)
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]
    x = center[0] - tw / 2
    y = center[1] - th / 2
    draw.text((x, y), text, font=font, fill=fill)


def _text_left(draw: ImageDraw.ImageDraw, pos, text, font, fill):
    """テキストを左寄せで描画"""
    draw.text(pos, text, font=font, fill=fill)


def _to_bytesio(img: Image.Image) -> BytesIO:
    """Image を PNG BytesIO に変換"""
    buf = BytesIO()
    img.save(buf, format='PNG', dpi=(DPI, DPI))
    buf.seek(0)
    return buf


# ============================================================
# 図1: リポジトリ構成図 (§2.1)
# ============================================================
def draw_repo_structure() -> BytesIO:
    """TORASAN リポジトリのツリー構成図"""
    W, H = 2400, 1200
    img = Image.new('RGB', (W, H), BG)
    draw = ImageDraw.Draw(img)

    title_font = _font(32, bold=True)
    node_font = _font(22, bold=True)
    desc_font = _font(18)

    # タイトル
    _text_center(draw, (W // 2, 35), 'TORASAN リポジトリ構成', title_font, CHARCOAL)

    # ルートノード
    root_x, root_y = W // 2, 110
    root_w, root_h = 340, 55
    _rounded_rect(draw, (root_x - root_w // 2, root_y - root_h // 2,
                          root_x + root_w // 2, root_y + root_h // 2),
                  radius=10, fill=CHARCOAL, outline=CHARCOAL)
    _text_center(draw, (root_x, root_y), 'TORASAN/', node_font, WHITE)

    # 子ノード定義: (label, description, color, fill_light)
    children = [
        ('.claude/skills/', f'{SKILL_COUNT_TOTAL} skills', SAGE, LIGHT_SAGE),
        ('.claude/knowledge/', f'{KNOWLEDGE_COUNT_TOTAL} files', TERRACOTTA, LIGHT_TERRA),
        ('app/', 'React + Fastify', MUSTARD, LIGHT_MUST),
        ('scripts/', 'ツール・生成', OLIVE, LIGHT_OLIVE),
        ('PROCESS.md', 'V-model 定義', CHARCOAL, LIGHT_CHAR),
        ('project.json', 'SoT', CHARCOAL, LIGHT_CHAR),
        ('packages/', '規格テンプレート', TERRACOTTA, LIGHT_TERRA),
        ('docs/', '成果物文書', SAGE, LIGHT_SAGE),
    ]

    n = len(children)
    start_y = 240
    row_h = 105
    box_w = 340
    box_h = 70
    desc_w = 240

    # 2 カラムレイアウト
    col_x = [W // 4 + 50, 3 * W // 4 - 50]

    for i, (label, desc, color, light) in enumerate(children):
        col = i % 2
        row = i // 2
        cx = col_x[col]
        cy = start_y + row * row_h

        # 接続線（ルートから）
        draw.line([(root_x, root_y + root_h // 2),
                   (root_x, start_y - 30)], fill=GRAY, width=2)
        draw.line([(col_x[0], start_y - 30),
                   (col_x[1], start_y - 30)], fill=GRAY, width=2)
        draw.line([(cx, start_y - 30), (cx, cy - box_h // 2)], fill=GRAY, width=2)

        # ボックス
        _rounded_rect(draw, (cx - box_w // 2, cy - box_h // 2,
                              cx + box_w // 2, cy + box_h // 2),
                      radius=8, fill=light, outline=color, width=2)

        # ラベル（左寄せ）
        _text_center(draw, (cx - 40, cy - 12), label, node_font, color)
        # 説明（右寄せ小文字）
        _text_center(draw, (cx - 40, cy + 16), desc, desc_font, CHARCOAL)

    # 凡例
    legend_y = H - 60
    legend_items = [
        ('Skills', SAGE), ('Knowledge', TERRACOTTA),
        ('App', MUSTARD), ('Tools', OLIVE), ('Config', CHARCOAL),
    ]
    lx = 200
    for label, color in legend_items:
        draw.rounded_rectangle((lx, legend_y, lx + 20, legend_y + 20),
                               radius=3, fill=color)
        _text_left(draw, (lx + 28, legend_y - 2), label, desc_font, CHARCOAL)
        lx += 180

    return _to_bytesio(img)


# ============================================================
# 図2: 二層配布モデル (§2.2)
# ============================================================
def draw_distribution_model() -> BytesIO:
    """スキル配布の二層構造図"""
    W, H = 2400, 1400
    img = Image.new('RGB', (W, H), BG)
    draw = ImageDraw.Draw(img)

    title_font = _font(32, bold=True)
    box_font = _font(22, bold=True)
    detail_font = _font(18)
    label_font = _font(20, bold=True)

    _text_center(draw, (W // 2, 35), '二層配布モデル', title_font, CHARCOAL)

    # ── 中央ハブ: TORASAN ──
    hub_x, hub_y = W // 2, 200
    hub_w, hub_h = 500, 80
    _rounded_rect(draw, (hub_x - hub_w // 2, hub_y - hub_h // 2,
                          hub_x + hub_w // 2, hub_y + hub_h // 2),
                  radius=12, fill=TERRACOTTA, outline=TERRACOTTA)
    _text_center(draw, (hub_x, hub_y - 10), 'TORASAN リポジトリ', box_font, WHITE)
    _text_center(draw, (hub_x, hub_y + 15),
                 f'Skills: {SKILL_COUNT_TOTAL} / Knowledge: {KNOWLEDGE_COUNT_TOTAL}',
                 detail_font, (255, 230, 220))

    # ── Tier 1 (左側) ──
    t1_x = W // 4
    t1_cmd_y = 380
    t1_dest_y = 560
    t1_targets_y = 740

    # install.sh ボックス
    bw, bh = 320, 60
    _rounded_rect(draw, (t1_x - bw // 2, t1_cmd_y - bh // 2,
                          t1_x + bw // 2, t1_cmd_y + bh // 2),
                  radius=8, fill=SAGE, outline=SAGE)
    _text_center(draw, (t1_x, t1_cmd_y), 'install.sh', box_font, WHITE)

    # 矢印: hub → install.sh
    _draw_arrow(draw, (hub_x - 100, hub_y + hub_h // 2),
                (t1_x, t1_cmd_y - bh // 2), SAGE, width=3)

    # 配布先
    _rounded_rect(draw, (t1_x - bw // 2, t1_dest_y - bh // 2,
                          t1_x + bw // 2, t1_dest_y + bh // 2),
                  radius=8, fill=LIGHT_SAGE, outline=SAGE, width=2)
    _text_center(draw, (t1_x, t1_dest_y - 10), '~/.claude/', box_font, SAGE)
    _text_center(draw, (t1_x, t1_dest_y + 14),
                 f'skills/ ({SKILL_COUNT_UNIVERSAL}) + knowledge/ ({KNOWLEDGE_COUNT_UNIVERSAL})',
                 detail_font, CHARCOAL)

    _draw_arrow(draw, (t1_x, t1_cmd_y + bh // 2),
                (t1_x, t1_dest_y - bh // 2), SAGE, width=3)

    # ターゲット PJ
    for i, pj in enumerate(['全プロジェクト共通']):
        py = t1_targets_y + i * 70
        _rounded_rect(draw, (t1_x - 140, py - 25, t1_x + 140, py + 25),
                      radius=6, fill=WHITE, outline=SAGE, width=2)
        _text_center(draw, (t1_x, py), pj, detail_font, CHARCOAL)
        _draw_dashed_line(draw, (t1_x, t1_dest_y + bh // 2),
                          (t1_x, py - 25), SAGE, width=2)

    # Tier ラベル
    _text_center(draw, (t1_x, 310), 'Tier 1: 汎用', label_font, SAGE)

    # ── Tier 2 (右側) ──
    t2_x = 3 * W // 4
    t2_cmd_y = 380
    t2_dest_y = 560
    t2_targets_y = 720

    # /repo-manage sync ボックス
    _rounded_rect(draw, (t2_x - bw // 2, t2_cmd_y - bh // 2,
                          t2_x + bw // 2, t2_cmd_y + bh // 2),
                  radius=8, fill=TERRACOTTA, outline=TERRACOTTA)
    _text_center(draw, (t2_x, t2_cmd_y), '/repo-manage sync', box_font, WHITE)

    _draw_arrow(draw, (hub_x + 100, hub_y + hub_h // 2),
                (t2_x, t2_cmd_y - bh // 2), TERRACOTTA, width=3)

    # 配布先
    _rounded_rect(draw, (t2_x - bw // 2, t2_dest_y - bh // 2,
                          t2_x + bw // 2, t2_dest_y + bh // 2),
                  radius=8, fill=LIGHT_TERRA, outline=TERRACOTTA, width=2)
    _text_center(draw, (t2_x, t2_dest_y - 10), 'PJ/.claude/skills/', box_font, TERRACOTTA)
    _text_center(draw, (t2_x, t2_dest_y + 14),
                 f'{SKILL_COUNT_DOMAIN} ドメインスキル（選択配布）',
                 detail_font, CHARCOAL)

    _draw_arrow(draw, (t2_x, t2_cmd_y + bh // 2),
                (t2_x, t2_dest_y - bh // 2), TERRACOTTA, width=3)

    # 個別PJ
    pjs = ['PJ-A (自動車)', 'PJ-B (家電)', 'PJ-C (産業)']
    for i, pj in enumerate(pjs):
        py = t2_targets_y + i * 65
        _rounded_rect(draw, (t2_x - 140, py - 25, t2_x + 140, py + 25),
                      radius=6, fill=WHITE, outline=TERRACOTTA, width=2)
        _text_center(draw, (t2_x, py), pj, detail_font, CHARCOAL)
        _draw_dashed_line(draw, (t2_x, t2_dest_y + bh // 2),
                          (t2_x, py - 25), TERRACOTTA, width=2)

    _text_center(draw, (t2_x, 310), 'Tier 2: ドメイン', label_font, TERRACOTTA)

    # ── 中央仕切り線 ──
    _draw_dashed_line(draw, (W // 2, 270), (W // 2, H - 80), GRAY, width=1,
                      dash_len=8, gap_len=12)

    # ── 凡例 (下部) ──
    ly = H - 55
    _text_left(draw, (100, ly), '図 2-1: 二層配布モデル（v1.0）', detail_font, GRAY)

    return _to_bytesio(img)


# ============================================================
# 図3: メモリ・状態管理アーキテクチャ (§2.3)
# ============================================================
def draw_memory_architecture() -> BytesIO:
    """3ゾーン（Session / Project / Global）のメモリアーキテクチャ図"""
    W, H = 2400, 1400
    img = Image.new('RGB', (W, H), BG)
    draw = ImageDraw.Draw(img)

    title_font = _font(32, bold=True)
    zone_font = _font(24, bold=True)
    box_font = _font(20, bold=True)
    detail_font = _font(17)

    _text_center(draw, (W // 2, 35), 'メモリ・状態管理アーキテクチャ', title_font, CHARCOAL)

    # 3ゾーンの枠
    zones = [
        ('Session スコープ（揮発性）', TERRACOTTA, LIGHT_TERRA,
         [('Auto Memory', 'MEMORY.md', '自動ロード (200行上限)'),
          ('Session State', 'session_state.md', '/session end で上書き')]),
        ('Project スコープ（Git 永続）', SAGE, LIGHT_SAGE,
         [('Project Knowledge', '.claude/knowledge/*.md', 'ドメイン知識'),
          ('Process Records', 'process_records/*.md', 'SPICE エビデンス'),
          ('project.json', 'SoT', '構成・進捗・変更ログ')]),
        ('Global スコープ（永続）', MUSTARD, LIGHT_MUST,
         [('Project Registry', 'project_registry.json', 'PJ 一覧'),
          ('Skill Manifest', '.shared-skills-manifest.json', '配布履歴')]),
    ]

    margin_x = 60
    zone_w = W - 2 * margin_x
    y_cursor = 90

    for zone_label, zone_color, zone_bg, items in zones:
        zone_h = 70 + len(items) * 85
        # 外枠
        _rounded_rect(draw, (margin_x, y_cursor, margin_x + zone_w, y_cursor + zone_h),
                      radius=14, fill=zone_bg, outline=zone_color, width=3)
        # ゾーンラベル
        _text_left(draw, (margin_x + 20, y_cursor + 10), zone_label, zone_font, zone_color)

        # 内部ボックス
        item_y = y_cursor + 60
        item_w = 650
        item_h = 65
        cols = min(len(items), 3)
        gap = 30
        total_items_w = cols * item_w + (cols - 1) * gap
        start_x = margin_x + (zone_w - total_items_w) // 2

        for j, (name, path, desc) in enumerate(items):
            ix = start_x + j * (item_w + gap)
            iy = item_y
            _rounded_rect(draw, (ix, iy, ix + item_w, iy + item_h),
                          radius=8, fill=WHITE, outline=zone_color, width=2)
            _text_left(draw, (ix + 12, iy + 8), name, box_font, zone_color)
            _text_left(draw, (ix + 12, iy + 34), f'{path} — {desc}', detail_font, CHARCOAL)

        y_cursor += zone_h + 25

    # 矢印: Session → Project → Global (右端に縦矢印)
    arrow_x = W - 45
    _draw_arrow(draw, (arrow_x, 130), (arrow_x, y_cursor - 40), CHARCOAL, width=2, head_size=12)
    # ラベル
    draw.text((arrow_x - 18, 100), '揮', font=_font(16), fill=CHARCOAL)
    draw.text((arrow_x - 18, y_cursor - 60), '永', font=_font(16), fill=CHARCOAL)

    # 凡例
    _text_left(draw, (100, H - 50), '図 2-2: メモリ・状態管理アーキテクチャ（v1.0）',
               detail_font, GRAY)

    return _to_bytesio(img)


# ============================================================
# 図4: V モデル (§5.1)  — 最重要・最高難度
# ============================================================
def draw_vmodel() -> BytesIO:
    """ISO 26262 V-model 15 フェーズ図"""
    W, H = 3600, 2400
    img = Image.new('RGB', (W, H), BG)
    draw = ImageDraw.Draw(img)

    title_font = _font(36, bold=True)
    phase_font = _font(20, bold=True)
    id_font = _font(18, bold=True)
    small_font = _font(16)
    legend_font = _font(18)

    _text_center(draw, (W // 2, 35), f'V モデル — {PHASE_COUNT} フェーズ構成',
                 title_font, CHARCOAL)

    # SPICE カテゴリ → 色
    CAT_COLORS = {
        'MAN': (CHARCOAL, LIGHT_CHAR),
        'SYS': (SAGE, LIGHT_SAGE),
        'SWE': (TERRACOTTA, LIGHT_TERRA),
        'SUP': (OLIVE, LIGHT_OLIVE),
    }

    # 左腕フェーズ定義: (id, short_name, category)
    left_phases = [
        ('PH-01', '計画・安全計画', 'MAN'),
        ('PH-02', 'アイテム定義', 'SYS'),
        ('PH-03', 'HARA', 'SYS'),
        ('PH-04', 'FSC', 'SYS'),
        ('PH-05', 'TSC', 'SYS'),
        ('PH-06', 'システム設計', 'SYS'),
        ('PH-07', 'HW 設計', 'SYS'),
        ('PH-08', 'SRS（SW安全要求）', 'SWE'),
    ]
    # 右腕フェーズ（上から下、Row 0〜6 に対応）
    right_phases = [
        ('PH-15', '安全アセスメント', 'SUP'),
        ('PH-14', '安全検証', 'SYS'),
        ('PH-13', 'システムテスト', 'SYS'),
        ('PH-12', 'HW 統合テスト', 'SYS'),
        ('PH-11', 'SW テスト', 'SWE'),
        ('PH-10', 'SW ユニット', 'SWE'),
        ('PH-09', 'SW アーキ設計', 'SWE'),
    ]

    # V 座標計算
    n_rows = 8  # 8レベル (Row 0=top ~ Row 7=bottom)
    y_top = 120
    y_bottom = 2200
    y_step = (y_bottom - y_top) / (n_rows - 1)

    # X 座標: 左腕は左→中央、右腕は中央→右
    x_left_start = 350     # Row 0 左側の X center
    x_right_start = 3250   # Row 0 右側の X center
    x_center = W // 2      # Row 7 底部の X center
    x_indent = (x_center - x_left_start) / (n_rows - 1)

    box_w = 380
    box_h = 66

    def _phase_box(cx, cy, pid, name, cat):
        """フェーズボックスを描画して中心座標を返す"""
        color, light = CAT_COLORS[cat]
        x0 = cx - box_w // 2
        y0 = cy - box_h // 2
        x1 = cx + box_w // 2
        y1 = cy + box_h // 2
        _rounded_rect(draw, (x0, y0, x1, y1), radius=10, fill=light, outline=color, width=2)
        # ID (左上)
        _text_left(draw, (x0 + 10, y0 + 6), pid, id_font, color)
        # 名前 (中央下)
        _text_center(draw, (cx, cy + 10), name, phase_font, CHARCOAL)
        return (cx, cy)

    # 左腕描画
    left_centers = []
    for i, (pid, name, cat) in enumerate(left_phases):
        if i < 7:
            cx = x_left_start + i * x_indent
        else:
            cx = x_center  # PH-08 は中央
        cy = y_top + i * y_step
        center = _phase_box(cx, cy, pid, name, cat)
        left_centers.append(center)

    # 右腕描画
    right_centers = []
    for i, (pid, name, cat) in enumerate(right_phases):
        cx = x_right_start - i * x_indent
        cy = y_top + i * y_step
        center = _phase_box(cx, cy, pid, name, cat)
        right_centers.append(center)

    # ── V 字の接続線（左腕: 上→下） ──
    for i in range(len(left_centers) - 1):
        ax, ay = left_centers[i]
        bx, by = left_centers[i + 1]
        draw.line([(ax, ay + box_h // 2), (bx, by - box_h // 2)],
                  fill=CHARCOAL, width=2)

    # ── V 字の接続線（右腕: 下→上） ──
    for i in range(len(right_centers) - 1):
        ax, ay = right_centers[i]
        bx, by = right_centers[i + 1]
        draw.line([(ax, ay + box_h // 2), (bx, by - box_h // 2)],
                  fill=CHARCOAL, width=2)

    # ── 底部接続（PH-08 → PH-09） ──
    ph08 = left_centers[7]
    ph09 = right_centers[6]
    _draw_arrow(draw, (ph08[0] + box_w // 2, ph08[1]),
                (ph09[0] - box_w // 2, ph09[1]), CHARCOAL, width=2)

    # ── 水平点線（左右対応フェーズ） ──
    for i in range(7):
        lx, ly = left_centers[i]
        rx, ry = right_centers[i]
        _draw_dashed_line(draw, (lx + box_w // 2 + 10, ly),
                          (rx - box_w // 2 - 10, ry),
                          GRAY, width=1, dash_len=8, gap_len=6)

    # ── 凡例 ──
    ly = H - 80
    lx = 200
    legend_items = [
        ('MAN (管理)', CHARCOAL, LIGHT_CHAR),
        ('SYS (システム)', SAGE, LIGHT_SAGE),
        ('SWE (ソフトウェア)', TERRACOTTA, LIGHT_TERRA),
        ('SUP (サポート)', OLIVE, LIGHT_OLIVE),
    ]
    for label, outline, fill in legend_items:
        _rounded_rect(draw, (lx, ly, lx + 28, ly + 22), radius=4,
                      fill=fill, outline=outline, width=2)
        _text_left(draw, (lx + 36, ly - 1), label, legend_font, CHARCOAL)
        lx += 360

    # ── 点線凡例 ──
    dlx = lx + 40
    _draw_dashed_line(draw, (dlx, ly + 11), (dlx + 40, ly + 11), GRAY, width=1)
    _text_left(draw, (dlx + 48, ly - 1), '検証対応関係', legend_font, GRAY)

    # 図番号
    _text_left(draw, (100, H - 40), '図 5-1: V モデル 15 フェーズ構成（v1.0）',
               small_font, GRAY)

    return _to_bytesio(img)


# ============================================================
# 図5: ツールパス解決フローチャート (§8.4)
# ============================================================
def draw_tool_resolution() -> BytesIO:
    """ツールパス解決のフローチャート（3段階判定）"""
    W, H = 2000, 1400
    img = Image.new('RGB', (W, H), BG)
    draw = ImageDraw.Draw(img)

    title_font = _font(28, bold=True)
    box_font = _font(20, bold=True)
    detail_font = _font(17)
    yes_no_font = _font(18, bold=True)

    _text_center(draw, (W // 2, 30), 'ツールパス解決フロー', title_font, CHARCOAL)

    cx = W // 2

    # ── Start ──
    sy = 100
    _rounded_rect(draw, (cx - 120, sy - 25, cx + 120, sy + 25),
                  radius=25, fill=CHARCOAL, outline=CHARCOAL)
    _text_center(draw, (cx, sy), 'ツール実行要求', box_font, WHITE)

    # ── 判定1: cmd.exe PATH ──
    d1y = 250
    d1w, d1h = 320, 120
    _draw_diamond(draw, (cx, d1y), d1w, d1h, fill=LIGHT_MUST, outline=MUSTARD, width=2)
    _text_center(draw, (cx, d1y - 10), 'cmd.exe PATH', box_font, CHARCOAL)
    _text_center(draw, (cx, d1y + 15), 'に存在？', detail_font, CHARCOAL)

    _draw_arrow(draw, (cx, sy + 25), (cx, d1y - d1h // 2), CHARCOAL, width=2)

    # Yes → 直接実行
    yes1_x = cx + 380
    yes1_y = d1y
    _rounded_rect(draw, (yes1_x - 130, yes1_y - 30, yes1_x + 130, yes1_y + 30),
                  radius=8, fill=LIGHT_SAGE, outline=SAGE, width=2)
    _text_center(draw, (yes1_x, yes1_y), '直接実行', box_font, SAGE)

    _draw_arrow(draw, (cx + d1w // 2, d1y), (yes1_x - 130, yes1_y), SAGE, width=2)
    _text_left(draw, (cx + d1w // 2 + 5, d1y - 25), 'Yes', yes_no_font, SAGE)

    # No ↓
    _text_left(draw, (cx + 8, d1y + d1h // 2 + 2), 'No', yes_no_font, TERRACOTTA)

    # ── 判定2: KNOWN_TOOL_PATHS ──
    d2y = 480
    _draw_diamond(draw, (cx, d2y), d1w, d1h, fill=LIGHT_MUST, outline=MUSTARD, width=2)
    _text_center(draw, (cx, d2y - 10), 'KNOWN_TOOL_PATHS', box_font, CHARCOAL)
    _text_center(draw, (cx, d2y + 15), 'に登録？', detail_font, CHARCOAL)

    _draw_arrow(draw, (cx, d1y + d1h // 2), (cx, d2y - d1h // 2), CHARCOAL, width=2)

    # Yes → フルパス実行
    yes2_x = cx + 380
    yes2_y = d2y
    _rounded_rect(draw, (yes2_x - 130, yes2_y - 30, yes2_x + 130, yes2_y + 30),
                  radius=8, fill=LIGHT_SAGE, outline=SAGE, width=2)
    _text_center(draw, (yes2_x, yes2_y), 'フルパス実行', box_font, SAGE)

    _draw_arrow(draw, (cx + d1w // 2, d2y), (yes2_x - 130, yes2_y), SAGE, width=2)
    _text_left(draw, (cx + d1w // 2 + 5, d2y - 25), 'Yes', yes_no_font, SAGE)

    _text_left(draw, (cx + 8, d2y + d1h // 2 + 2), 'No', yes_no_font, TERRACOTTA)

    # ── 判定3: Git Bash ──
    d3y = 710
    _draw_diamond(draw, (cx, d3y), d1w, d1h, fill=LIGHT_MUST, outline=MUSTARD, width=2)
    _text_center(draw, (cx, d3y - 10), 'Git Bash', box_font, CHARCOAL)
    _text_center(draw, (cx, d3y + 15), 'で発見？', detail_font, CHARCOAL)

    _draw_arrow(draw, (cx, d2y + d1h // 2), (cx, d3y - d1h // 2), CHARCOAL, width=2)

    # Yes → Git Bash 経由実行
    yes3_x = cx + 380
    yes3_y = d3y
    _rounded_rect(draw, (yes3_x - 130, yes3_y - 30, yes3_x + 130, yes3_y + 30),
                  radius=8, fill=LIGHT_SAGE, outline=SAGE, width=2)
    _text_center(draw, (yes3_x, yes3_y), 'Git Bash 経由', box_font, SAGE)

    _draw_arrow(draw, (cx + d1w // 2, d3y), (yes3_x - 130, yes3_y), SAGE, width=2)
    _text_left(draw, (cx + d1w // 2 + 5, d3y - 25), 'Yes', yes_no_font, SAGE)

    _text_left(draw, (cx + 8, d3y + d1h // 2 + 2), 'No', yes_no_font, TERRACOTTA)

    # ── 失敗 ──
    fail_y = 910
    _rounded_rect(draw, (cx - 150, fail_y - 30, cx + 150, fail_y + 30),
                  radius=8, fill=LIGHT_TERRA, outline=TERRACOTTA, width=2)
    _text_center(draw, (cx, fail_y), 'ツール未検出 (missing)', box_font, TERRACOTTA)

    _draw_arrow(draw, (cx, d3y + d1h // 2), (cx, fail_y - 30), TERRACOTTA, width=2)

    # ── 右側の縦矢印を結合して「実行完了」 ──
    end_y = 1050
    end_x = yes1_x
    # 3つの成功ボックスから下に線を引く
    for yy in [yes1_y, yes2_y, yes3_y]:
        draw.line([(end_x, yy + 30), (end_x, end_y - 40)], fill=SAGE, width=2)

    _rounded_rect(draw, (end_x - 120, end_y - 30, end_x + 120, end_y + 30),
                  radius=25, fill=SAGE, outline=SAGE)
    _text_center(draw, (end_x, end_y), '実行完了', box_font, WHITE)

    # 注釈ボックス
    note_y = 1150
    note_x = W // 2
    _rounded_rect(draw, (note_x - 400, note_y, note_x + 400, note_y + 110),
                  radius=8, fill=WHITE, outline=GRAY, width=1)
    _text_left(draw, (note_x - 380, note_y + 10),
               '解決順序: cmd.exe PATH → KNOWN_TOOL_PATHS → Git Bash',
               box_font, CHARCOAL)
    _text_left(draw, (note_x - 380, note_y + 42),
               'KNOWN_TOOL_PATHS: environmentService.ts で管理',
               detail_font, CHARCOAL)
    _text_left(draw, (note_x - 380, note_y + 68),
               '新規ツール追加時: error_prevention.md §E フロー参照',
               detail_font, CHARCOAL)

    # 図番号
    _text_left(draw, (80, H - 40), '図 8-1: ツールパス解決フロー（v1.0）',
               detail_font, GRAY)

    return _to_bytesio(img)


# ============================================================
# テスト用エントリポイント
# ============================================================
if __name__ == '__main__':
    import os

    out_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'docs', 'diagrams')
    os.makedirs(out_dir, exist_ok=True)

    funcs = [
        ('01_repo_structure.png', draw_repo_structure),
        ('02_distribution_model.png', draw_distribution_model),
        ('03_memory_architecture.png', draw_memory_architecture),
        ('04_vmodel.png', draw_vmodel),
        ('05_tool_resolution.png', draw_tool_resolution),
    ]

    for fname, func in funcs:
        buf = func()
        path = os.path.join(out_dir, fname)
        with open(path, 'wb') as f:
            f.write(buf.read())
        print(f'  Generated: {path}')

    print(f'\nAll {len(funcs)} diagrams generated in {out_dir}')
