#!/usr/bin/env bash
# =============================================================
# wsl-setup.sh — WSL エージェント環境セットアップ
#
# 実行方法（Windows Git Bash から）:
#   /c/Windows/System32/wsl.exe -- bash -l /mnt/c/Users/<USERNAME>/Documents/TORASAN/scripts/wsl-setup.sh
#
# 実行内容:
#   1. nvm + Node.js 22 インストール
#   2. Claude Code ネイティブインストール
#   3. 動作確認
#
# 認証は別途手動で:
#   /c/Windows/System32/wsl.exe -- bash -lc "claude auth"
# =============================================================

set -euo pipefail

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

info()  { echo -e "${GREEN}[OK]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; }

echo "========================================="
echo " WSL Agent Environment Setup"
echo "========================================="
echo ""

# ---------------------------------------------------------
# Step 1: Node.js (via nvm)
# ---------------------------------------------------------
echo "--- Step 1/3: Node.js ---"

export NVM_DIR="$HOME/.nvm"

if [ -s "$NVM_DIR/nvm.sh" ]; then
    source "$NVM_DIR/nvm.sh"
fi

if command -v node &>/dev/null && [[ "$(which node)" != /mnt/* ]]; then
    info "Node.js already installed: $(node --version)"
else
    warn "Node.js not found natively. Installing via nvm..."

    if [ ! -s "$NVM_DIR/nvm.sh" ]; then
        echo "  Installing nvm..."
        curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash
        export NVM_DIR="$HOME/.nvm"
        source "$NVM_DIR/nvm.sh"
    fi

    echo "  Installing Node.js 22..."
    nvm install 22
    nvm use 22
    nvm alias default 22

    info "Node.js installed: $(node --version)"
fi

echo "  npm: $(npm --version)"

# ---------------------------------------------------------
# Step 2: Claude Code
# ---------------------------------------------------------
echo ""
echo "--- Step 2/3: Claude Code ---"

# PATH にClaude追加（既存の場合）
if [ -d "$HOME/.claude/local/bin" ]; then
    export PATH="$HOME/.claude/local/bin:$PATH"
fi

if command -v claude &>/dev/null; then
    info "Claude Code already installed: $(claude --version 2>/dev/null || echo 'version unknown')"
else
    warn "Claude Code not found. Installing..."
    curl -fsSL https://claude.ai/install.sh | bash

    # PATH を更新
    if [ -d "$HOME/.claude/local/bin" ]; then
        export PATH="$HOME/.claude/local/bin:$PATH"
    fi

    # .bashrc に PATH 追加（まだなければ）
    if ! grep -q 'claude/local/bin' "$HOME/.bashrc" 2>/dev/null; then
        echo '' >> "$HOME/.bashrc"
        echo '# Claude Code' >> "$HOME/.bashrc"
        echo 'export PATH="$HOME/.claude/local/bin:$PATH"' >> "$HOME/.bashrc"
    fi

    if command -v claude &>/dev/null; then
        info "Claude Code installed: $(claude --version 2>/dev/null || echo 'installed')"
    else
        error "Claude Code installation may have failed. Check manually."
        exit 1
    fi
fi

# ---------------------------------------------------------
# Step 3: 動作確認
# ---------------------------------------------------------
echo ""
echo "--- Step 3/3: Verification ---"

echo "  OS:      $(lsb_release -ds 2>/dev/null || cat /etc/os-release | grep PRETTY_NAME | cut -d= -f2)"
echo "  Node:    $(node --version)"
echo "  npm:     $(npm --version)"
echo "  Claude:  $(claude --version 2>/dev/null || echo 'check manually')"
echo "  Home:    $HOME"

echo ""
echo "========================================="
info "Setup complete!"
echo ""
echo "Next step — authenticate Claude Code:"
echo "  /c/Windows/System32/wsl.exe -- bash -lc \"claude auth\""
echo ""
echo "Then test:"
echo "  /c/Windows/System32/wsl.exe -- bash -lc 'claude -p \"hello\" --output-format json'"
echo "========================================="
