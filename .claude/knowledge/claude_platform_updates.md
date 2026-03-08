# Claude プラットフォーム更新記録

本ドキュメントは /claude-master スキルによる公式ドキュメント巡回結果、
適用済み改善、未適用保留事項、ナレッジ鮮度を記録する。

---

## 1. 巡回履歴

| 日付 | モード | Tier | 発見件数 | 適用件数 | 未適用 | 実施者 |
|------|--------|------|---------|---------|--------|--------|
| 2026-03-02 | 初期調査 | 1-3 | — | — | — | 手動調査 |

### 2026-03-02 — 初期調査

公式スキルシステムを調査し、全37スキルを `.claude/skills/` 形式に移行。

**発見事項:**
- `.claude/skills/{name}/SKILL.md` が公式推奨形式
- YAML フロントマター（name, description, disable-model-invocation, context, agent, allowed-tools, hooks 等）
- description は英語三人称 + "Use when..." パターンが最適
- Progressive Disclosure: SKILL.md + references/ + scripts/
- `context: fork` でサブエージェント実行が可能
- `!`command`` で動的コンテキスト注入が可能
- SKILL.md は 500行以下推奨
- description の文字予算: コンテキストウィンドウの 2%（16,000文字フォールバック）

**適用済み:**
- 全37スキルを .claude/commands/ → .claude/skills/ に移行
- 全スキルに YAML フロントマター追加
- 英語 description + "Use when..." パターン
- 破壊的スキル7種に disable-model-invocation: true

**未適用（次回検討）:**
- Progressive Disclosure (references/ 分離): 大規模スキルで検討
- context: fork: health-check, config-audit 等の重い分析スキルで検討
- !`command` 動的コンテキスト: env-check, platform-info で検討
- allowed-tools 制限: read-only スキルで検討
- hooks: スキルライフサイクルイベントでの自動化検討
- scripts/: validate.sh 等の自動検証で検討

---

## 2. 公式ドキュメント監視ポイント

### Tier 1: 公式ドキュメント

| ソース | URL | 最終確認 | 状態 |
|--------|-----|---------|------|
| Skills | https://code.claude.com/docs/skills | 2026-03-02 | FRESH |
| Sub-agents | https://code.claude.com/docs/sub-agents | 未確認 | STALE |
| Hooks | https://code.claude.com/docs/hooks | 未確認 | STALE |
| Memory | https://code.claude.com/docs/memory | 未確認 | STALE |
| Permissions | https://code.claude.com/docs/permissions | 未確認 | STALE |
| Plugins | https://code.claude.com/docs/plugins | 未確認 | STALE |
| llms.txt | https://code.claude.com/docs/llms.txt | 未確認 | STALE |

### Tier 2: リリース情報

| ソース | 最終確認 | 状態 |
|--------|---------|------|
| Anthropic Blog | 未確認 | STALE |
| Claude Code GitHub | 未確認 | STALE |
| Agent Skills Standard | 未確認 | STALE |

---

## 3. ナレッジ鮮度マップ

| ナレッジ | 作成日 | 最終更新 | 経過日数 | 判定 | 巡回推奨 |
|---------|--------|---------|---------|------|---------|
| claude_code_ops.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |
| claude_platform_updates.md | 2026-03-02 | 2026-03-02 | 0 | FRESH | — |
| skill_lifecycle.md | 2026-03-02 | 2026-03-02 | 0 | FRESH | — |
| iso26262_iec60730.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |
| automotive_spice.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |
| misra_c_2012.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |
| fmea_guide.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |
| safety_case_gsn.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |
| srs_template.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |
| safety_diagnostics.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |
| bldc_safety.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |
| cross_platform_dev.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |
| git_worktree_branch_management.md | 2026-03-01 | 2026-03-01 | 1 | FRESH | — |

鮮度基準:
- FRESH: 30日以内
- AGING: 31-90日
- STALE: 91日以上（巡回推奨）

---

## 4. スキルドメイン巡回記録

| スキルカテゴリ | 最終巡回 | 次回推奨 | 巡回キーワード |
|-------------|---------|---------|-------------|
| Claude 系 | 2026-03-02 | 2026-03-09 | Claude Code Skills, Sub-agents |
| 安全規格系 | 未実施 | 優先 | ISO 26262 amendment, IEC 60730 |
| SPICE 系 | 未実施 | 優先 | Automotive SPICE 4.0, ASPICE |
| コード品質系 | 未実施 | 優先 | MISRA C:2012 amendment, cppcheck |
| MCU/組込み系 | 未実施 | 通常 | Renesas RL78, BLDC control |
| テスト系 | 未実施 | 通常 | MC/DC tools, embedded unit test |
