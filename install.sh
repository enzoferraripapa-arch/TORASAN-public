#!/usr/bin/env bash
# install.sh — 共有スキル・ナレッジを ~/.claude/ に配布
#
# Usage: ./install.sh [--dry-run]
#
# TORASAN フレームワークの汎用スキル・ナレッジを
# ユーザーレベル (~/.claude/) にコピーして全プロジェクトで使えるようにする。

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TARGET_SKILLS="$HOME/.claude/skills"
TARGET_KNOWLEDGE="$HOME/.claude/knowledge"
MANIFEST="$HOME/.claude/.shared-skills-manifest.json"

# 共有スキル一覧
SHARED_SKILLS=(
  session
  dashboard
  health-check
  skill-manage
  skill-evolve
  repo-manage
  backup
  commit-change
  config-audit
  worktree-cleanup
  update-record
  env-check
  platform-info
  claude-master
)

# 共有ナレッジ一覧
SHARED_KNOWLEDGE=(
  claude_code_ops.md
  claude_platform_updates.md
  skill_lifecycle.md
  skill_feedback_log.md
  git_worktree_branch_management.md
  cross_platform_dev.md
  memory_paths.md
  error_prevention.md
)

DRY_RUN=false
for arg in "$@"; do
  case "$arg" in
    --dry-run) DRY_RUN=true ;;
  esac
done

SOURCE_COMMIT=$(cd "$SCRIPT_DIR" && git rev-parse --short HEAD 2>/dev/null || echo "unknown")

echo "=== TORASAN 共有スキル配布 ==="
echo "ソース: $SCRIPT_DIR ($SOURCE_COMMIT)"
echo ""

# スキル配布
echo "[Skills]"
for skill in "${SHARED_SKILLS[@]}"; do
  src="$SCRIPT_DIR/.claude/skills/$skill"
  dst="$TARGET_SKILLS/$skill"
  if [ ! -d "$src" ]; then
    echo "  [SKIP] $skill (ソースなし)"
    continue
  fi
  if [ "$DRY_RUN" = true ]; then
    echo "  [DRY] $skill"
  else
    mkdir -p "$dst"
    cp -r "$src"/* "$dst/"
    echo "  [OK]  $skill"
  fi
done

echo ""

# ナレッジ配布
echo "[Knowledge]"
for knowledge in "${SHARED_KNOWLEDGE[@]}"; do
  src="$SCRIPT_DIR/.claude/knowledge/$knowledge"
  dst="$TARGET_KNOWLEDGE/$knowledge"
  if [ ! -f "$src" ]; then
    echo "  [SKIP] $knowledge (ソースなし)"
    continue
  fi
  if [ "$DRY_RUN" = true ]; then
    echo "  [DRY] $knowledge"
  else
    mkdir -p "$TARGET_KNOWLEDGE"
    cp "$src" "$dst"
    echo "  [OK]  $knowledge"
  fi
done

# マニフェスト記録
if [ "$DRY_RUN" = false ]; then
  cat > "$MANIFEST" << MANIFEST_EOF
{
  "source": "$SCRIPT_DIR",
  "commit": "$SOURCE_COMMIT",
  "synced_at": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "skills_count": ${#SHARED_SKILLS[@]},
  "knowledge_count": ${#SHARED_KNOWLEDGE[@]}
}
MANIFEST_EOF

  echo ""
  echo "=== 配布完了 ==="
  echo "スキル: ${#SHARED_SKILLS[@]} 本 → $TARGET_SKILLS"
  echo "ナレッジ: ${#SHARED_KNOWLEDGE[@]} 本 → $TARGET_KNOWLEDGE"
  echo "マニフェスト: $MANIFEST"
else
  echo ""
  echo "=== ドライラン完了（変更なし） ==="
fi
