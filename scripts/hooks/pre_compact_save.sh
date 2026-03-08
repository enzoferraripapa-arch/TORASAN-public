#!/usr/bin/env bash
# pre_compact_save.sh — PreCompact hook
# コンテキスト圧縮前にセッション状態をスナップショット保存する
# stdin: Claude Code が渡す JSON（trigger, custom_instructions 等）
# 保存先: ~/.claude/projects/{slug}/memory/compact_breadcrumb.md

set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
SLUG="TORASAN"  # Customize for your project path
MEMORY_DIR="$HOME/.claude/projects/$SLUG/memory"
BREADCRUMB="$MEMORY_DIR/compact_breadcrumb.md"

mkdir -p "$MEMORY_DIR"

# Python コマンド検出（python → python3 フォールバック）
PY="python"
command -v python &>/dev/null || PY="python3"

# stdin JSON を読み取り（空の場合もある）
INPUT=$(cat 2>/dev/null || echo "{}")

# Python で JSON パース（jq 未インストールのため）
TRIGGER=$(echo "$INPUT" | $PY -c "
import sys, json
try:
    data = json.load(sys.stdin)
    print(data.get('trigger', 'unknown'))
except:
    print('unknown')
" 2>/dev/null || echo "unknown")

CUSTOM=$(echo "$INPUT" | $PY -c "
import sys, json
try:
    data = json.load(sys.stdin)
    print(data.get('custom_instructions', ''))
except:
    print('')
" 2>/dev/null || echo "")

# Git 情報を収集
BRANCH=$(cd "$PROJECT_DIR" && git branch --show-current 2>/dev/null || echo "unknown")
LAST_COMMIT=$(cd "$PROJECT_DIR" && git log --oneline -1 2>/dev/null || echo "unknown")
UNCOMMITTED=$(cd "$PROJECT_DIR" && git status --porcelain 2>/dev/null | wc -l | tr -d ' ')
TIMESTAMP=$(date -u +%Y-%m-%dT%H:%M:%SZ)

cat > "$BREADCRUMB" << EOF
# Compact Breadcrumb — $TIMESTAMP
branch: $BRANCH
last_commit: $LAST_COMMIT
uncommitted: $UNCOMMITTED files
trigger: $TRIGGER
focus: $CUSTOM
EOF

echo "PreCompact: breadcrumb saved to $BREADCRUMB" >&2
