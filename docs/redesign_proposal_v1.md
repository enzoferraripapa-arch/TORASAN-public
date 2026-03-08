# TORASAN 環境再設計案 v1

## 現状の問題点

### P1: スキル一覧のシステムプロンプト肥大化（最大ボトルネック）
- 41本のスキル説明が **毎ターン** system-reminder としてロードされる
- 推定 ~3,000トークン/ターンの固定消費
- ユーザーが使わないドメインスキル（motor-control, mcu-config等）も常時ロード

### P2: スキル本文の巨大化
- session.md: 465行、repo-manage: 406行、claude-master: 274行
- 1回の呼出で数千トークン消費
- session は毎セッション開始/終了で必ず呼ばれる

### P3: ナレッジの重複
- .claude/knowledge/ と ~/.claude/knowledge/ で7ファイルが完全重複
- cross_platform_dev.md (716行) と git_worktree_branch_management.md (447行) が特に大きい

### P4: レガシーメモリの残骸
- ~/.claude/projects/ に旧ユーザー名(enzo)のディレクトリが残存
- 10ディレクトリ中、有効なのは3つ程度

### P5: WSLエージェントの活用不足
- 構築済みだが、実運用で使われていない
- Opus で実行中のタスクの一部を Sonnet/Haiku に委任可能

---

## 再設計案

### R1: スキルティア制の導入

スキルを3階層に分類し、system-reminder の肥大化を抑制:

| ティア | ロード方式 | スキル例 | 基準 |
|-------|----------|---------|------|
| T1: Core (5本) | 常時ロード（system-reminder） | session, repo-manage, skill-manage, dashboard, claude-master | 毎セッション使用 |
| T2: Domain (15本) | プロジェクト category に応じてロード | execute-phase, driver-gen, fmea, safety-* | 機能安全PJでのみ必要 |
| T3: Utility (21本) | ユーザーが明示的に呼んだ時のみ | motor-control, mcu-config, memory-map, hw-review | 低頻度 |

**実装方法**: `.claude/skills/` のディレクトリ構成を変更
```
.claude/skills/
  core/          ← T1: 常時ロード
  domain/        ← T2: CLAUDE.md の条件で選択ロード
  utility/       ← T3: オンデマンド
```

**期待効果**: system-reminder を 41本 → 5本に削減（~85%削減）

### R2: スキル本文のスリム化

巨大スキルを分割・圧縮:

| スキル | 現行 | 目標 | 方法 |
|-------|------|------|------|
| session | 465行 | 150行 | start/end を分離、テンプレートを外部化 |
| repo-manage | 406行 | 150行 | サブコマンド別ファイルに分離 |
| skill-evolve | 273行 | 120行 | PDCA テンプレートを knowledge に移動 |
| claude-master | 274行 | 100行 | 巡回ロジックを WSL agent に委任 |

**期待効果**: 主要スキルの呼出時トークンを ~50%削減

### R3: ナレッジ統合・階層化

```
変更前:
  .claude/knowledge/    (16ファイル, 2948行)
  ~/.claude/knowledge/  (7ファイル, 1827行)  ← 7ファイル重複

変更後:
  ~/.claude/knowledge/           (汎用: 5ファイル, ~800行)
    claude_code_ops.md           (圧縮: 182→100行)
    skill_lifecycle.md           (圧縮: 301→150行)
    platform_and_git.md          (統合: cross_platform + git_worktree → 300行)
    claude_platform_updates.md   (据置: 102行)
    memory_paths.md              (据置: 45行)

  .claude/knowledge/             (ドメイン: 9ファイル, ~900行)
    iso26262_iec60730.md         (据置)
    automotive_spice.md          (据置)
    misra_c_2012.md              (据置)
    bldc_safety.md               (据置)
    safety_diagnostics.md        (据置)
    safety_case_gsn.md           (据置)
    fmea_guide.md                (据置)
    srs_template.md              (据置)
    product_standard_mapping.md  (据置)
```

**削除対象**: skill_feedback_log.md → project.json の tracker に統合

**期待効果**: 重複排除で ~1,800行削減、大ファイル圧縮で ~600行追加削減

### R4: WSLエージェント活用戦略

| タスク種別 | 実行先 | モデル | 理由 |
|-----------|-------|-------|------|
| セッション情報収集 | WSL Agent | Haiku | 定型収集、低コスト |
| 技術巡回スキャン | WSL Agent | Sonnet | Web検索+分析、メインコンテキスト汚染防止 |
| 静的解析実行 | WSL Agent | Haiku | cppcheck/clang-tidy の出力解析 |
| コード生成・設計判断 | メイン (Opus) | Opus | 高品質な判断が必要 |
| テスト実行・結果解析 | WSL Agent | Sonnet | 出力が大きい |
| Git操作・コミット | メイン (Opus) | Opus | ユーザー確認が必要 |

**期待効果**: Opus トークン消費を ~30%削減（Sonnet/Haiku への委任分）

### R5: CLAUDE.md の最適化

```
変更前: 128行（TORASAN）+ 52行（グローバル）= 180行

変更後: 80行（TORASAN）+ 40行（グローバル）= 120行
```

最適化方法:
- 指示別動作テーブルを圧縮（アプリ委譲系は1行にまとめる）
- マニュアル管理フローの詳細を knowledge に移動
- 定期技術巡回テーブルを2行に圧縮

### R6: メモリ・レガシー整理

1. 旧ユーザー名(enzo)のメモリディレクトリを削除
2. 重複プロジェクトメモリを統合
3. MEMORY.md を 77→50行に圧縮（冗長な環境情報を削除）

---

## 全体の期待効果

| 項目 | 現行 | 改善後 | 削減率 |
|------|------|--------|-------|
| system-reminder スキル数 | 41本 | 5本 (T1) | -88% |
| 毎ターン固定トークン | ~5,000 | ~2,000 | -60% |
| session スキル呼出 | ~3,000 tok | ~1,000 tok | -67% |
| ナレッジ総行数 | 4,775行 | 1,700行 | -64% |
| CLAUDE.md + MEMORY.md | 257行 | 170行 | -34% |
| Opus 使用率 | 100% | ~70% | -30% cost |

---

## 実装優先順位

1. **R1 (スキルティア制)** ← 最大効果、構造変更のみ
2. **R4 (WSL活用戦略)** ← コスト削減効果大
3. **R2 (スキルスリム化)** ← R1 と同時に実施可能
4. **R3 (ナレッジ統合)** ← 重複排除は即効性あり
5. **R5 (CLAUDE.md最適化)** ← 小改善だが毎ターン効く
6. **R6 (メモリ整理)** ← 一度やれば終わり

---

## 未検討事項（レビューで議論したい）

- Q1: スキルティア制の実装は Claude Code のスキルローディング仕様上可能か？
- Q2: session スキルの start/end 分離は運用上問題ないか？
- Q3: WSL Agent への委任で応答レイテンシが許容範囲内か？
- Q4: ナレッジ統合で必要な情報が失われるリスクは？
- Q5: 管理GUIアプリの規模（474 backend files）は適切か？
