#!/usr/bin/env bash
# session_start_context.sh — SessionStart hook
# 前セッションの breadcrumb が存在すれば stdout に出力し、
# Claude のコンテキストに注入する（stdout → additionalContext）
# 消費後はファイルを .consumed にリネームして二重注入を防止

set -euo pipefail

SLUG="TORASAN"  # Customize for your project path
MEMORY_DIR="$HOME/.claude/projects/$SLUG/memory"

COMPACT_BC="$MEMORY_DIR/compact_breadcrumb.md"
SESSION_BC="$MEMORY_DIR/session_end_breadcrumb.md"

FOUND=0

# Session End breadcrumb（より新しい情報を優先）
if [ -f "$SESSION_BC" ]; then
    echo "--- Previous Session End State ---"
    cat "$SESSION_BC"
    echo "---"
    mv "$SESSION_BC" "${SESSION_BC}.consumed" 2>/dev/null || echo "SessionStart: warn: failed to rename session_end_breadcrumb" >&2
    FOUND=1
fi

# Compact breadcrumb（Session End が無い場合のフォールバック）
if [ "$FOUND" -eq 0 ] && [ -f "$COMPACT_BC" ]; then
    echo "--- Previous Compact State ---"
    cat "$COMPACT_BC"
    echo "---"
    mv "$COMPACT_BC" "${COMPACT_BC}.consumed" 2>/dev/null || echo "SessionStart: warn: failed to rename compact_breadcrumb" >&2
    FOUND=1
fi

if [ "$FOUND" -eq 0 ]; then
    echo "SessionStart: no breadcrumb found" >&2
fi
exit 0
