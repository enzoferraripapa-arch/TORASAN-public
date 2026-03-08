"""
TORASAN 仕様書共有定数モジュール

複数の文書生成スクリプト間でスキル数・ナレッジ数等の数値を一元管理する。
変更時はここを更新すれば generate_spec_doc.py / generate_architecture_doc.py に自動反映される。
"""

# ── スキル数 ──
SKILL_COUNT_UNIVERSAL = 14    # 汎用（Tier 1 — グローバル配布）
SKILL_COUNT_DOMAIN = 26       # ドメイン（Tier 2 — レジストリ配布）
SKILL_COUNT_TOTAL = SKILL_COUNT_UNIVERSAL + SKILL_COUNT_DOMAIN  # 40

# ── ナレッジ数 ──
KNOWLEDGE_COUNT_UNIVERSAL = 8   # グローバル（~/.claude/knowledge/）
KNOWLEDGE_COUNT_DOMAIN = 13     # ドメイン（.claude/knowledge/）
KNOWLEDGE_COUNT_TOTAL = KNOWLEDGE_COUNT_UNIVERSAL + KNOWLEDGE_COUNT_DOMAIN  # 21

# ── フェーズ数 ──
PHASE_COUNT = 15

# ── 図表モード ──
# 'drawingml' = 編集可能な Word ネイティブ図形 / 'png' = Pillow 300DPI PNG
DIAGRAM_MODE = 'drawingml'

# ── 文書メタ情報 ──
# 初版作成日（表紙用 — 再生成で上書きされない固定値）
SPEC_INITIAL_DATE = '2026-03-02'
OPS_INITIAL_DATE = '2026-03-02'

# フレームワーク・プロセスバージョン
FRAMEWORK_VERSION = 'v5.0'
PROCESS_VERSION = 'v3.2'
