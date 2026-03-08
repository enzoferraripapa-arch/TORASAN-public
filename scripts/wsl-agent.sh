#!/usr/bin/env bash
# =============================================================
# wsl-agent.sh — WSL 側 Claude Code にタスクを委任するブリッジ
#
# Usage:
#   bash scripts/wsl-agent.sh "タスクの説明" [options]
#
# Options:
#   --timeout SECONDS    タイムアウト秒数 (default: 300)
#   --cwd PATH           WSL 内の作業ディレクトリ (default: /mnt/c/.../TORASAN)
#   --model MODEL        使用モデル (default: sonnet)
#   --max-turns N        最大ターン数 (default: 10)
#
# Returns: JSON on stdout
#   {"ok": true, "result": "...", "cost_usd": 0.02, "duration_ms": 15000, ...}
# =============================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
WSL_EXE="/c/Windows/System32/wsl.exe"

# デフォルト値
TASK=""
TIMEOUT=300
CWD="/mnt/c/Users/${USER:-user}/Documents/TORASAN"
MODEL="sonnet"
MAX_TURNS=10

# 引数パース
while [[ $# -gt 0 ]]; do
    case "$1" in
        --timeout)    TIMEOUT="$2"; shift 2 ;;
        --cwd)        CWD="$2"; shift 2 ;;
        --model)      MODEL="$2"; shift 2 ;;
        --max-turns)  MAX_TURNS="$2"; shift 2 ;;
        -*)           echo "{\"ok\":false,\"error\":\"Unknown option: $1\"}" >&2; exit 1 ;;
        *)            TASK="$1"; shift ;;
    esac
done

if [ -z "$TASK" ]; then
    echo '{"ok":false,"error":"No task provided. Usage: wsl-agent.sh \"task description\" [options]"}'
    exit 1
fi

# WSL が利用可能か確認
if ! $WSL_EXE --list --running &>/dev/null && ! $WSL_EXE -e true &>/dev/null; then
    echo '{"ok":false,"error":"WSL is not available"}'
    exit 1
fi

# Node.js runner に委任
node "$SCRIPT_DIR/wsl-agent-runner.js" \
    --task "$TASK" \
    --timeout "$TIMEOUT" \
    --cwd "$CWD" \
    --model "$MODEL" \
    --max-turns "$MAX_TURNS"
