#!/usr/bin/env bash
# session_end_save.sh — SessionEnd hook
# /session end を経由せずにセッション終了した場合の安全ネット
# session_state.md が直近 2 分以内に更新済みならスキップ（二重書き込み防止）
# 保存先: ~/.claude/projects/{slug}/memory/session_end_breadcrumb.md

set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
SLUG="TORASAN"  # Customize for your project path
MEMORY_DIR="$HOME/.claude/projects/$SLUG/memory"
STATE_FILE="$MEMORY_DIR/session_state.md"
BREADCRUMB="$MEMORY_DIR/session_end_breadcrumb.md"

mkdir -p "$MEMORY_DIR"

# session_state.md が直近 2 分以内に更新されていたらスキップ
if [ -f "$STATE_FILE" ]; then
    STATE_MTIME=$(stat -c %Y "$STATE_FILE" 2>/dev/null || stat -f %m "$STATE_FILE" 2>/dev/null || echo 0)
    NOW=$(date +%s)
    DIFF=$((NOW - STATE_MTIME))
    if [ "$DIFF" -lt 120 ]; then
        echo "SessionEnd: session_state.md is fresh (${DIFF}s ago), skipping breadcrumb" >&2
        exit 0
    fi
fi

# Python コマンド検出（python → python3 フォールバック）
PY="python"
command -v python &>/dev/null || PY="python3"

# stdin JSON を読み取り
INPUT=$(cat 2>/dev/null || echo "{}")

REASON=$(echo "$INPUT" | $PY -c "
import sys, json
try:
    data = json.load(sys.stdin)
    print(data.get('reason', 'unknown'))
except:
    print('unknown')
" 2>/dev/null || echo "unknown")

# Git 情報を収集
BRANCH=$(cd "$PROJECT_DIR" && git branch --show-current 2>/dev/null || echo "unknown")
LAST_COMMIT=$(cd "$PROJECT_DIR" && git log --oneline -1 2>/dev/null || echo "unknown")
UNCOMMITTED=$(cd "$PROJECT_DIR" && git status --porcelain 2>/dev/null | wc -l | tr -d ' ')
TIMESTAMP=$(date -u +%Y-%m-%dT%H:%M:%SZ)

cat > "$BREADCRUMB" << EOF
# Session End Breadcrumb — $TIMESTAMP
reason: $REASON
branch: $BRANCH
last_commit: $LAST_COMMIT
uncommitted: $UNCOMMITTED files
EOF

echo "SessionEnd: breadcrumb saved to $BREADCRUMB" >&2
