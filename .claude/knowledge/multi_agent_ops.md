# マルチエージェント運用ナレッジ

## 1. 方式比較表

| 項目 | Agent Teams | WSL エージェント | subagent (Agent ツール) |
|------|-----------|----------------|----------------------|
| 通信 | 双方向（メールボックス） | 不可 | 結果返却のみ |
| コンテキスト | 各 teammate 独立 | 独立プロセス | 呼出元に要約返却 |
| コスト | 高（teammate 数に線形） | 中（WSL overhead） | 低（haiku 指定可） |
| Git 競合 | リスクあり | 低（read-only 運用） | なし |
| CLAUDE.md | 自動読込 | 手動指定 | 自動読込 |
| セッション復元 | 不可（experimental 制約） | N/A | N/A |
| Windows 対応 | in-process のみ（tmux 不可） | WSL 経由 | 完全対応 |

## 2. 使い分けガイドライン

| ユースケース | 推奨方式 | 理由 |
|-------------|---------|------|
| 軽量レビュー（2観点） | subagent × 2 (haiku) | コスト最小、session Phase D で実績あり（S14〜S26 継続） |
| 重量レビュー（3+観点） | subagent × 3 (haiku) | Agent Teams 未対応のため subagent 拡張で代替 |
| 読み取り専用調査 | subagent (Explore) | メインコンテキスト保護に最適 |
| 長時間バッチ処理 | WSL エージェント | バックグラウンド実行、タイムアウト制御 |
| 協調設計・議論 | Agent Teams（CLI 手動） | セッション内自動化は不可。ユーザーが CLI で直接操作 |

## 3. Agent Teams 設定（TORASAN）

```json
// .claude/settings.json
{
  "env": {
    "CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS": "1"
  }
}
```

- モード: in-process（Windows 環境のため）
- 制限: 1チーム/セッション、ネスト不可、セッション再開で復元不可
- 新 hook: TeammateIdle, TaskCompleted（Phase 3 で導入予定）

## 4. Hook アーキテクチャ

```
SessionStart → session_start_context.sh → breadcrumb 注入
     ↓
  セッション実行中
     ↓
PreCompact → pre_compact_save.sh → compact_breadcrumb.md 保存
     ↓
  コンテキスト圧縮
     ↓
SessionEnd → session_end_save.sh → session_end_breadcrumb.md 保存
```

breadcrumb は次セッションの SessionStart で消費（.consumed にリネーム）。

## 5. 試行ログ

### Trial 1: Hook スクリプトレビュー（S26, 2026-03-08）

| 日付 | 方式 | タスク | teammate数 | コスト | 時間 | 品質(1-5) | 備考 |
|------|------|--------|-----------|-------|------|----------|------|
| 2026-03-08 | Agent Teams | Hook レビュー | 2 | — | — | — | **失敗**: `spawnTeam` ツール未提供 |
| 2026-03-08 | subagent×2 (haiku) | Hook レビュー | 2 | ~96K tokens | ~2.5min | 4/5 | Reviewer-A: 15件, Reviewer-B: 3件, クロス検証1件 |

**Agent Teams 結果**:
- `team_name="hook-review"` → エラー: `Team "hook-review" does not exist. Call spawnTeam first to create the team.`
- Agent ツールの `team_name` パラメータは認識される（未知パラメータエラーではない）
- しかし `spawnTeam` ツールが現環境で利用不可 → チーム作成不能
- 結論: Agent Teams は**セッション内からプログラム的に利用できない**（ユーザーが CLI で直接チーム生成するフローのみ対応の可能性）

**Baseline 結果**:
- Reviewer-A（ロバストネス）: Critical 3件, Major 6件, Minor 6件 — 網羅的
- Reviewer-B（コレクトネス）: P0 1件（stat 互換性、実環境検証で false positive 判定）, OK 2件
- クロス検証: stat コマンド懸念が両レビュアーで一致（ただし実環境では問題なし）
- false positive 率: Reviewer-B の P0 は Git Bash で stat -c %Y が正常動作するため過剰

### Go/No-Go 判定

**判定: No-Go（機構未対応）**

Agent Teams の `spawnTeam` ツールが現環境で利用不可。
trial 2, 3 は前提条件（trial 1 成功）未達のためスキップ。

| 基準 | 閾値 | 結果 | 判定 |
|------|------|------|------|
| 品質 | ≧ baseline | 測定不能 | — |
| コスト | ≦ 2倍 | 測定不能 | — |
| 安定性 | 3回無エラー | 1回目で機構エラー | NG |

**推奨アクション**:
1. Agent Teams は**ユーザー主導の CLI 操作**として利用する（セッション内自動化は不可）
2. 現行の subagent×2 (haiku) を Phase D レビューの標準として継続
3. Agent Teams の experimental 解除・ツール拡張を `/claude-master scan` で監視
4. 再評価時期: 2026-04（1ヶ月後）または Claude Code メジャーアップデート時

### Go/No-Go 判定基準（参考）
Agent Teams → Phase 3 に進む条件:
- 品質: WSL/subagent 方式と同等以上
- コスト: 2倍以内
- 安定性: 3回の試行で致命的エラーなし

## 6. WSL SPICE 改善デーモン（S26 構築）

### 概要
4つの WSL エージェントが並行して process_records を改善し、
5つ目のエージェントがプロセス横断整合性チェックを実施する反復改善システム。

### アーキテクチャ
```
scripts/wsl-spice-improver.py  （Python オーケストレーター）
  ├── WorktreeManager        Git worktree 作成・マージ・削除
  ├── AgentRunner            wsl-agent.sh 経由で WSL エージェント起動
  └── ProgressTracker        進捗 JSON 管理

1イテレーション: Assess → Improve(4並行) → Merge → CrossCheck → Verify
```

### エージェント分担
| Agent | 担当 |
|-------|------|
| sys-improver | SYS.3, SYS.4, SYS.5 |
| swe-improver | SWE.1〜SWE.6 |
| man-improver | MAN.3 |
| sup-improver | SUP.1, SUP.8, SUP.9, SUP.10 |
| cross-checker | 全プロセス横断整合性（要件トレーサビリティ、V字対応等6観点） |

### 品質保証
- 各イテレーション前に `git tag spice-iter-{n}-start` でスナップショット
- マージ品質ゲート: process_records/ 以外の変更を自動ブロック
- PA 値劣化検出時は自動ロールバック
- 失敗エージェントは部分スキップ（成功分のみマージ）
- atexit で worktree 残存防止

### 実行方法
```bash
python scripts/wsl-spice-improver.py --dry-run              # 構成確認
python scripts/wsl-spice-improver.py --max-iterations 3      # 本番実行
python scripts/wsl-spice-improver.py --agents sup-improver   # 単一テスト
```

### 見積もり
- 1イテレーション: ~15分（4並行 + cross-checker）
- 3イテレーション: ~45分
- コスト: ~$3-7 USD（sonnet × 5エージェント × 3回）
