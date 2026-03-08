---
name: session
description: "Manages session lifecycle: start, end (retrospective, handoff, backup, master merge), and status."
argument-hint: "[mode: start|end|status]"
---
# /session --- セッションライフサイクル管理

セッションの開始（新規/既存プロジェクト）・終了（反省会・引継ぎ・バックアップ・masterマージ）・状態確認を統合管理する。

引数: $ARGUMENTS（必須。"start" = 開始フロー | "end" = 終了フロー | "status" = 現状サマリ → 省略時は "start"）

短縮形:
- `--continue` = start と同一フローだが Phase C を簡略表示（session_state.md 不在時は通常表示にフォールバック）
- `おつです` / `otsudesu` = end と同義

---

## start モード --- セッション開始

3 フェーズ構成: **A: 自動検出 → B: サブエージェント情報収集 → C: コンパクト表示**

### Phase A: コンテキスト自動検出

ユーザーへの質問を最小化し、環境から自動判定する。

#### A-1: パス分析

1. `$PWD` にワークツリーパス `.claude/worktrees/{name}` が含まれるか確認
   - **YES** → `context_type = "worktree"`、メインリポパスを算出（`.claude/worktrees/{name}` より上位）
   - **NO** → `$PWD` を `project_path` とする

#### A-2: プロジェクト判定

`project_path`（ワークツリーの場合はメインリポパス）で判定:

```
project.json が存在？
  YES → projectName を読み取り → context_type = "project"
  NO  → 追加ワーキングディレクトリを探索
        1件以上見つかった → context_type = "multi_candidate"
        0件              → context_type = "unknown"
```

#### A-3: ユーザー確認（`multi_candidate` / `unknown` の場合のみ）

- **`multi_candidate`**:
  ```
  セッションを開始します。プロジェクトを選んでください:
  (a) {projectName1} ({category1})
  (b) {projectName2} ({category2})
  ...
  (x) 新規プロジェクトを始める
  ```
  選択後に Phase B へ進む。

- **`unknown`**:
  ```
  セッションを開始します。
  プロジェクトが見つかりませんでした。
  (a) 新規プロジェクトを始める
  (b) 別のディレクトリを指定する
  ```
  **新規プロジェクト選択時** → 新規プロジェクトフロー（後述）へ分岐。

- **`project`**: ユーザーへの質問なし。自動で Phase B へ。

---

### Phase B: サブエージェント情報収集（メインコンテキスト保護）

**単一のサブエージェント（Explore タイプ）** に以下を一括委任し、コンパクトなテキストサマリで返却させる。メインコンテキストには生データ（session_state.md 全文、git log 出力、knowledge ファイル内容）を読み込まない。

サブエージェントへの委任内容:

```
{project_path} のセッション情報を収集し、以下の項目をコンパクトに報告せよ。

【パス正規化手順 — 必ずこの順序で実行】
0. まず `ls ~/.claude/projects/ | grep {プロジェクト名}` で実際のスラグを確認する
   → 推測で組み立てず、実在するディレクトリ名を使う（最重要）
1. 実在確認できない場合のみ手動算出:
   (a) ドライブレター C: → C-
   (b) バックスラッシュ \ → ハイフン -
   (c) アンダースコア _ はそのまま保持
   例: C:\Users\<USERNAME>\Documents\TORASAN → C--Users-<USERNAME>-Documents-TORASAN

1. session_state（引継ぎ情報）:
   - 検索パス（Step 0 で確認した実在スラグを使用）:
     (a) ~/.claude/projects/{wt-slug}/memory/session_state.md
     (b) ~/.claude/projects/{main-slug}/memory/session_state.md
   - 抽出項目: last_work, arrival_point, next_action, unresolved_retro_items[]
   - 見つからない場合: "なし" と報告

2. git_status:
   - git branch --show-current
   - git status --porcelain のファイル数
   - git rev-list --count master..HEAD（master からの差分）
   - git fetch origin --quiet && git rev-list --count @{upstream}..HEAD 2>/dev/null || git rev-list --count origin/master..HEAD（未 push コミット数）
   - git branch -r | grep 'origin/claude/' | wc -l（残存リモートブランチ数）
   - 抽出項目: current_branch, uncommitted_count, commits_ahead, unpushed_count, stale_remote_branches

3. patrol_check（7日巡回判定）:
   - ~/.claude/knowledge/claude_platform_updates.md の巡回履歴から最終日を特定
   - 抽出項目: last_patrol_date, days_elapsed, patrol_recommended（7日超で true）

4. project_summary（project.json から抽出）:
   - 共通: projectName, category, mode
   - category が機能安全系の場合:
     standard.base, standard.product_override, standard.product_override_class,
     phases（completed数/total数, 最初の未完了フェーズ名）,
     tbd_items の件数, spice_assessment.target_level
   - category が一般の場合:
     phases（completed数/total数）
   - TORASAN フレームワーク管理リポの場合（CLAUDE.md に "スキル・フレームワーク管理リポ" と記載）:
     .claude/skills/ 配下のスキル数, .claude/worktrees/ 配下のWT数

5. cross_pj_check（TORASAN フレームワークリポ以外の場合のみ）:
   - ~/.claude/knowledge/env_state.md を読む（不在時は skip）
   - ~/.claude/.shared-skills-manifest.json の shared_skills_hash を取得
   - ~/.claude/project_registry.json から $PWD に対応する framework_commit を取得
   - 抽出: env_state_updated_at, skills_stale(bool), framework_stale(bool),
     last_changes[] (直近3件のタイトル)
```

※ `{wt-slug}` / `{main-slug}` のパス正規化アルゴリズム（**厳密に以下の順序で適用**）:

```
入力: Windows 絶対パス（例: C:\Users\<USERNAME>\Documents\TORASAN）
Step 1: ドライブレター C: → C-        → C-\Users\<USERNAME>\Documents\TORASAN
Step 2: バックスラッシュ \ → ハイフン - → C--Users-<USERNAME>-Documents-TORASAN
※ アンダースコア _ はそのまま保持（置換しない）
※ スペース・特殊文字もそのまま保持
```

**検証方法**: `ls ~/.claude/projects/ | grep TORASAN` で実際のスラグを確認してから使用すること。
推測で組み立てず、必ずファイルシステムを確認する。

### Phase C: コンパクト表示 + 確認

サブエージェントの結果を `context_type` と `category` に応じたフォーマットで表示する。

#### フレームワークリポ（TORASAN）の場合:

```
=== TORASAN フレームワーク管理 ===
ブランチ: {branch} (master +{n}) | 未コミット: {uncommitted}件
スキル: {skill_count}本 | WT: {worktree_count}個
{repo_hygiene_line}
{patrol_line}

前回: {last_work} → {arrival_point}
次回推奨: {next_action}

何をしますか？
```

#### 機能安全プロジェクトの場合（standard.base が設定されている）:

```
=== {projectName} セッション再開 ===
{standard.base} {product_override_class} | {mode}
ブランチ: {branch} (master +{n}) | TBD: {tbd_count}件
進捗: {completed}/{total} | 次: {current_phase}
{repo_hygiene_line}
{patrol_line}

前回: {last_work}
次回推奨: {next_action}

何をしますか？
```

#### 一般プロジェクトの場合:

```
=== {projectName} セッション再開 ===
{category}
ブランチ: {branch} (master +{n})
進捗: {completed}/{total}
{repo_hygiene_line}
{patrol_line}

前回: {last_work}
次回推奨: {next_action}

何をしますか？
```

**条件行ルール**:
- `{tracker_line}`: `open_issues + open_ideas > 0` の場合のみ表示 → `課題: {open_issues}件 | アイデア: {open_ideas}件`
- `{patrol_line}`: `patrol_recommended == true` の場合のみ表示 → `INFO: 技術巡回 {days_elapsed}日経過 → /claude-master scan 推奨`
- `{cross_pj_line}`: TORASAN 以外のPJで `skills_stale` or `framework_stale` が true の場合のみ表示 → `INFO: フレームワーク更新あり → /repo-manage sync 推奨` または `INFO: 汎用スキル更新あり → install.sh 実行推奨`
- `{repo_hygiene_line}`: 以下のいずれかに該当する場合に表示:
  - `unpushed_count > 0` → `WARN: 未pushコミット {unpushed_count}件 → セッション開始前に push 推奨`
  - `stale_remote_branches > 0` → `WARN: 残存リモートブランチ {stale_remote_branches}本 → 整理推奨`
- session_state.md が不在の場合: 「前回」「次回推奨」行を省略
- `unresolved_retro_items` がある場合: 次回推奨に「改善項目の対応」を含める

### Phase D: マルチエージェントレビューゲート（全モード共通）

セッション中の**重要な判断・成果物**に対し、複数エージェントによるクロスレビューを実施する。
これは「凡ミス防止」のための構造的ガードレールである。

#### D-1: トリガー判定（実行するかの決定）

```
Phase D トリガー判定:
  (1) start 完了後:
      → Phase B の結果に矛盾がある場合のみ FULL レビュー
      → 矛盾なし（パス解決成功 + git status 整合）は SKIP
  (2) 成果物作成後:
      → 変更ファイル数 >= 3 → FULL レビュー
      → 変更ファイル数 < 3 → SKIP
  (3) end Step 2 後（反省会議事録）:
      → 常時 FULL レビュー（省略不可）
  (4) フェーズゲート（execute-phase 完了時）:
      → 安全関連フェーズ（PH-04〜PH-12）→ FULL レビュー
      → その他 → SKIP
```

#### D-2: レビューモード

| モード | エージェント数 | 用途 |
|--------|-------------|------|
| FULL | 2（並行） | 3ファイル以上変更、反省会、安全関連フェーズ |
| SKIP | 0 | 上記条件に該当しない場合 |

#### D-3: エージェント構成と観点

**2エージェント並行**（Agent ツール × 2、haiku モデルでコスト節約）:

| エージェント | 観点 | チェック項目 |
|------------|------|------------|
| Reviewer-A（正確性） | 事実・データの正しさ | パス存在確認、数値の整合、参照先の実在、git 状態の矛盾 |
| Reviewer-B（網羅性） | 漏れ・抜けの検出 | 未対応項目、変更漏れファイル、テンプレート項目の欠落 |

#### D-4: エージェントプロンプトテンプレート

**start 後（Phase B 結果レビュー）:**

```
Reviewer-A プロンプト:
「Phase B が報告した以下の情報の正確性を検証せよ。
 - session_state.md のパス: {path} → 実在するか ls で確認
 - git ブランチ: {branch} → git branch --show-current と一致するか
 - 巡回日: {date} → claude_platform_updates.md の記載と一致するか
 結果: OK / NG（NG の場合は正しい値を報告）」

Reviewer-B プロンプト:
「Phase B が報告した情報に漏れがないか確認せよ。
 - session_state.md の unresolved_retro_items は全件報告されているか
 - git status に未コミットファイルの見落としがないか
 - project.json のフェーズ進捗と報告値が一致するか
 結果: OK / 漏れあり（漏れの内容を報告）」
```

**end 後（反省会議事録レビュー）:**

```
Reviewer-A プロンプト:
「反省会議事録 {file_path} を読み、以下を検証せよ。
 - セッション成果サマリが git log と一致するか
 - 発生した問題が全て記載されているか（git diff で変更→revert があれば問題扱い）
 - 改善策が具体的で実行可能か（「気をつける」等の曖昧表現は NG）
 結果: OK / 修正提案（具体的な追記・修正内容）」

Reviewer-B プロンプト:
「反省会議事録 {file_path} を読み、以下を検証せよ。
 - 前回反省会フォローアップが記載されているか（session_state.md の unresolved 参照）
 - 改善すべき点の「状態」列が正しいか（済/未の判定根拠）
 - 教訓が error_prevention.md の既存パターンと重複していないか
 結果: OK / 漏れあり（具体的な追記内容）」
```

**成果物レビュー（3ファイル以上変更時）:**

```
Reviewer-A プロンプト:
「以下の変更ファイルの整合性を検証せよ。
 変更ファイル一覧: {file_list}
 - ファイル間の相互参照が正しいか（壊れたリンクがないか）
 - 命名規則・フォーマットの統一性
 - 規格関連の場合: product_override に適合した用語が使われているか
 結果: OK / 不整合あり（具体的な箇所と修正提案）」

Reviewer-B プロンプト:
「以下の変更に対する漏れを検出せよ。
 変更ファイル一覧: {file_list}
 - 変更に伴い更新が必要だが未更新のファイルはないか（Grep で関連参照を検索）
 - テスト・ドキュメントの追従は必要か
 - changeLog / TBD の更新は必要か
 結果: OK / 漏れあり（対応すべきファイルと内容）」
```

#### D-5: 結果処理フロー

```
レビュー結果の処理:
  両方 OK → 次のステップへ進む（レビュー結果は表示しない）
  片方 NG → メインエージェントが NG 内容を確認し修正を実施
  両方 NG → メインエージェントが両方の指摘を統合し修正を実施
  矛盾（A=OK, B=NG on same item）→ メインエージェントが独自に検証して判断

修正後の再レビュー:
  → 不要（修正内容が明確な事実誤認・漏れの場合）
  → 必要（修正が構造的変更を含む場合 → 再度 D-3 から実行）
```

#### D-6: コンテキスト管理

- レビューエージェントは **haiku モデル**を使用（コスト・速度の最適化）
- レビュー結果は Phase C 表示に含めない（コンテキスト節約）
- 問題検出時のみユーザーに報告（`REVIEW: {指摘内容} → 修正済み`）
- レビューで消費するターン数の目安: 各エージェント 3-5 ターン以内

#### `--continue` 時の Phase C:

Phase A + B は通常と同一。Phase C のみ簡略テンプレート:

```
=== 再開: {next_action} ===
ブランチ: {branch} (master +{n}) | 未コミット: {uncommitted}件
→ {next_action}
```

session_state.md が不在の場合: 通常フロー（上記のフル表示）にフォールバック。

---

### 新規プロジェクトフロー

Phase A で `unknown` → ユーザーが「新規プロジェクトを始める」を選択した場合に実行。

#### Step 0N-1: ヒアリング

ユーザーに以下を対話形式で確認:

```
=== 新規プロジェクトセットアップ ===
1. プロジェクト名: ___
2. カテゴリ:
   (a) 組み込み開発（機能安全）  — ISO 26262 / IEC 61508 / IEC 60730
   (b) デスクトップアプリ         — Windows / macOS / Linux
   (c) モバイルアプリ             — iOS / Android / Flutter
   (d) Web開発                    — フロントエンド / バックエンド / フルスタック
   (e) 特許・知財整理             — 特許出願 / 知財管理
   (f) 研究・調査                 — 論文 / 技術調査 / PoC
   (g) その他                     — カスタム構成
3. 概要（1-2文）: ___
```

#### Step 0N-2: カテゴリ別テンプレート構成決定

カテゴリに応じた project.json のフェーズ構成・ディレクトリ構造を決定:

| カテゴリ | フェーズ構成 | 規格 | ディレクトリ |
|---------|------------|------|------------|
| 組み込み（機能安全） | PH-01〜PH-15（全フェーズ） | ISO 26262 | PROCESS.md 準拠 |
| デスクトップ | 要件→設計→実装→テスト→リリース | なし | src/, tests/, docs/ |
| モバイル | 要件→UI設計→実装→テスト→デプロイ | なし | lib/, test/, assets/ |
| Web | 要件→設計→実装→テスト→デプロイ | なし | src/, public/, api/ |
| 特許・知財 | 調査→出願書類→図面→レビュー | なし | patents/, figures/ |
| 研究・調査 | 課題設定→調査→分析→報告 | なし | research/, data/ |
| その他 | ユーザーと対話で決定 | なし | カスタム |

- 機能安全カテゴリの場合: `/select-standard` を呼び出して規格選択
- それ以外: 簡易 project.json を生成

#### Step 0N-3: プロジェクト初期化

1. project.json を生成（カテゴリ別テンプレート適用）
2. ディレクトリ構造を作成
3. Git 初期コミット: `feat: {projectName} プロジェクト初期化`

#### Step 0N-4: 表示へ合流

→ Phase C（コンパクト表示）へ進む

---

## end モード --- セッション終了

ユーザーが「おつです」「otsudesu」と言った場合もこのフローを実行する。

### Step 0: コンテキスト状態チェック

1. 現在のコンテキスト使用率を推定
2. 60% 超過と推定される場合:
   ```
   INFO: コンテキスト使用率が高めです。
   → 終了フロー前に `/compact セッション終了処理に集中` を推奨
   ```
3. ユーザーが compact を実施した場合、またはスキップを選択した場合、次へ進む

### Step 1: 規格整合性チェック（最終確認）

**条件**: 既存プロジェクト（project.json あり）かつ `product_override != null` の場合のみ

1. セッション中に作成・修正した成果物を特定: `git diff --name-only`
2. 成果物中にASILを製品分類として誤用していないか確認
3. 問題があれば修正してから次のステップへ

### Step 1.5: スキルフィードバック確認

1. セッション中に実行されたスキルを列挙
2. `~/.claude/knowledge/skill_feedback_log.md` と突合し、未記録のスキルがあれば:
   ```
   INFO: 以下のスキルのフィードバックが未記録です:
   - /xxx (実行済み・未フィードバック)
   → /skill-evolve feedback で記録しますか？
   ```
3. ユーザーがフィードバックを記録する場合は `/skill-evolve feedback` を呼び出す
4. スキップする場合は次のステップへ

### Step 1.6: 環境スナップショット更新チェック（TORASAN フレームワークリポの場合のみ）

1. `git diff --name-only HEAD~1 HEAD` で以下のいずれかが含まれるか確認:
   - `.mcp.json` / `install.sh` / `settings.json` / `.claude/skills/` 配下
2. 含まれる場合:
   ```
   INFO: 環境変更が検出されました。env_state.md を更新しますか？（推奨）
   変更: {検出されたファイル一覧}
   ```
3. 承認された場合: `~/.claude/knowledge/env_state.md` の `_updated_at`, `torasan_commit`, 変更ログを更新
4. スキップ可能（週次 schedule タスクが自動更新する）

### Step 2: 反省会（レトロスペクティブ）

セッション中の作業を振り返り、以下の構成で議事録を作成:

```markdown
# セッション反省会議事録

日時: {date}
ブランチ: {branch}
セッションスコープ: {scope}

---

## 1. セッション成果サマリ
（実施した作業・コミット一覧をテーブルで）

## 2. 発生した問題
（セッション中に検出した不具合・誤り・手戻り。なければ「なし」）

## 3. 根本原因分析
（問題があった場合のみ）

## 4. 良かった点
（うまくいったこと、改善が機能したこと）

## 4.5 改善効果の検証
（前回の改善策が今セッションで効果を発揮したか検証。前回 §5 の「済」項目と対照）
| # | 前回の改善策 | 今回の結果 | 効果判定 |
|---|------------|----------|---------|
有効な改善は MEMORY.md に恒久的な教訓として記録。無効な改善は §5 に新対策を起票。

## 5. 改善すべき点
| # | 問題 | 対策 | 分類 | 状態 |
（分類: process=プロセス改善→unresolved_retro_items で追跡 / skill=スキル改善→/skill-evolve に委譲）

## 6. 教訓
（今後に活かすべき知見）

## 7. 前回反省会フォローアップ
データソース（優先順）:
1. session_state.md の unresolved_retro_items（Phase B で抽出済み）
2. process_records/retrospective/ 配下の直近議事録（ls -t で特定）
（前回の「未」項目が今回対応されたか確認。未対応の場合は理由を記載し §5 に再起票）
```

保存先: `process_records/retrospective/{date}_session_retro.md`

### Step 2.5: マルチエージェントレビュー（Phase D 適用）

Phase D §D-1 のトリガー判定 (3) により、反省会議事録は**常時 FULL レビュー**を実施。
§D-4 の「end 後（反省会議事録レビュー）」テンプレートに従い、Reviewer-A（正確性）と Reviewer-B（網羅性）を並行起動。
結果処理は §D-5 に従い、NG の場合は Step 2 に戻り議事録を修正。

### Step 3: セッション引継ぎ

1. 現在の作業状態を収集:
   - `git status` / `git branch -v` / `git log --oneline -5`
   - `git log master..HEAD --oneline` で master との差分コミット数
   - `project.json` のフェーズ進捗・TBD数
2. 以下の**両方のメモリパス**に `session_state.md` を記録:
   - WT用: `~/.claude/projects/{project-path-slug}--claude-worktrees-{name}/memory/`
   - メインリポ用: `~/.claude/projects/{project-path-slug}/memory/`

   ※ `{project-path-slug}` の算出方法は Phase B 参照

記録テンプレート:

```markdown
# セッション状態 --- {date}

## 直前の作業
- 作業内容: {scope}
- 到達点: {到達点}
- 次のアクション: {次のアクション}

## Git 状態
- ブランチ: {branch}
- 最新コミット: {hash} {message}
- 未コミット変更: {あり/なし}
- master との差分: {n} commits ahead
- master マージ: {済/未}

## プロジェクト状態
- フェーズ進捗: {completed_phases} / {total_phases}
- TBD: {tbd_count}件
- 未解決問題: {issues}

## 重要なコンテキスト
（反省会で得た教訓・注意事項）

## 未完了タスク
（反省会の「未」改善項目を含む）

## 未解決改善項目（unresolved_retro_items）
（反省会 §5 の「未」項目をここに転記。次セッション start 時に Phase C でフォローアップ表示される）
- [ ] {改善項目1}
- [ ] {改善項目2}
```

3. 永続的な教訓は両方の `MEMORY.md` にも追記
4. 反省会 §5 で「未」の改善項目は `unresolved_retro_items` セクションに必ず転記:
   - **前セッションから引き継いだ未解決項目のうち今回も未対応のもの** + **今回新規の「未」項目** を統合
   - つまり: `new_unresolved = (前回引継ぎで今回未対応) ∪ (今回 §5 の「未」)`
   - 転記前に既存の session_state.md を読み取り、前回の unresolved_retro_items を取得すること

### Step 3.5: ワークツリー整理確認

1. `.claude/worktrees/` 配下のワークツリー一覧を確認
2. 不要なワークツリーがあれば `/worktree-cleanup` を提案:
   ```
   INFO: 以下のワークツリーが残存しています:
   - .claude/worktrees/{name1} (ブランチ: claude/{name1})
   → /worktree-cleanup で整理しますか？
   ```

### Step 4: バックアップ + master マージ

以下の順序で実行:

1. **未コミット確認**: `git status` で未コミット変更があればコミット実施
2. **タグ作成**: `git tag v{N+1}-session-end`
3. **master マージ**:
   - `git checkout master`
   - `git merge {branch} --no-ff -m "merge: {branch} セッション成果マージ"`
   - マージ後タグ: `git tag v{N+2}-merged`
   - 元のブランチに戻る: `git checkout {branch}`
   - マージ衝突時はユーザーに報告し手動解決を依頼
4. **リモート同期**（master マージ成功後に実行）:
   - `git push origin master --tags` で master とタグをリモートに push
   - マージ済みリモートブランチを削除:
     ```
     git branch -r --merged master | grep 'origin/claude/' | sed 's|origin/||' | xargs -I{} git push origin --delete "{}"
     ```
   - `git fetch --prune` でリモート追跡ブランチを整理
   - push 失敗時は警告を表示して続行（ネットワーク問題の可能性）
5. **完了報告**:

```
=== セッション終了 ===
反省会議事録: process_records/retrospective/{file}
引継ぎ: session_state.md 更新済み（WT + main 両方）
バックアップ: {tag_name} ({hash})
master マージ: {済/衝突あり（手動解決要）}
リモート同期: {済/失敗（要手動push）}
ブランチ整理: {N}本削除 / {残存数}本残存
お疲れ様でした！
```

---

## status モード --- 現状サマリ

session-handoff の summary 機能を統合。現在のセッション状態を非破壊的に表示する。

### 収集・表示項目

```
=== セッション状態 ===
ワークツリー: {path} (ブランチ: {branch})
master との差分: {n} commits ahead
セッション中のコミット: {count}
変更ファイル（未コミット）: {count}

[フェーズ進捗]
{completed}/{total} フェーズ完了
直近の作業フェーズ: PH-{XX} ({status})

[技術巡回状態]
最終巡回: {date}（{N}日前）
巡回推奨: {はい/いいえ}

[引継ぎ情報]
session_state.md: {存在する/存在しない}
最終更新: {date}
```

**ベストプラクティス**: status は状態を読むだけで変更を加えない。調査的なタスクはサブエージェントに委任して、メインコンテキストを保護する。

---

## 関連スキル

| スキル | 連携タイミング |
|--------|--------------|
| /dashboard | 詳細ダッシュボードが必要な場合に独立利用（start では自動呼出しない） |
| /health-check | start 時の環境確認補完（必要時に手動実行） |
| /skill-evolve | end Step 1.5 でフィードバック記録 |
| /claude-master | start Phase C で巡回提案（patrol_recommended 時） |
| /env-check | 環境問題発生時に手動実行 |
| /worktree-cleanup | end Step 3.5 でWT整理 |
| /backup | end Step 4 でタグ作成（内部処理として統合） |
| /select-standard | start 新規PJ Step 0N-2 で規格選択 |
| /commit-change | end Step 4 で未コミット変更のコミット |

---

## 注意事項

### メモリ管理方針（session-handoff から継承）

- `session_state.md` は毎回**上書き**（最新1回分のみ保持）
- 永続的な知見は `MEMORY.md` に記録（session_state とは別管理）
- セッション固有の一時情報のみ `session_state.md` に記録
- ナレッジベースとの使い分け:
  - `.claude/knowledge/` = ドメイン知識（規格・技術リファレンス、Git管理）
  - `~/.claude/projects/*/memory/` = 運用知識（プロジェクト固有の経験・決定、ローカル）
  - `session_state.md` = 揮発性状態（次セッション開始時に消費）

### `--continue` 時の挙動

- Phase A（自動検出）+ Phase B（サブエージェント情報収集）は通常フローと同一
- Phase C のみ簡略テンプレートで表示（ブランチ + 次アクションの 2 行）
- session_state.md が不在の場合: 通常フロー（フル表示）にフォールバック
- 「つづきを」「resume」「continue」発話時にも同等の処理

### 公式ベストプラクティス適用

| プラクティス | 適用箇所 | 効果 |
|------------|---------|------|
| サブエージェント委任 | start Phase B / status モード | session_state.md・git出力・knowledge がメインコンテキストに入らない |
| `/compact` 手動実行 | end Step 0 | 終了フロー用のコンテキスト確保 |
| Plan Mode 推奨 | 3ファイル以上変更時 | ~40% トークン節約（Phase C の推奨に含める） |
| Git = 最終永続化 | end Step 4 | コミット・タグが全セッションを生き残る |

---

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:

| ファイル | 参照内容 | 参照方法 |
|---------|---------|---------|
| `~/.claude/knowledge/claude_code_ops.md` | コンテキスト管理、メモリシステム、セッション引継ぎパターン | サブエージェント経由 |
| `~/.claude/knowledge/memory_paths.md` | メモリパス構成（WT用/メインリポ用の両パス解決） | サブエージェント経由 |
| `.claude/knowledge/product_standard_mapping.md` | 規格整合性チェック、アンチパターン（S4） | end Step 1 で直接参照 |
| `~/.claude/knowledge/claude_platform_updates.md` | 技術巡回履歴、最終巡回日（7日判定用） | サブエージェント経由 |
| `~/.claude/knowledge/skill_lifecycle.md` | スキル品質基準（本スキル自身の構造品質確認） | 必要時に直接参照 |
| `~/.claude/knowledge/error_prevention.md` | 凡ミス防止チェックリスト、マルチエージェントレビュー必須条件 | Phase D / Phase B で参照 |

※ `~/.claude/knowledge/` = 共有ナレッジ（全PJ共通）、`.claude/knowledge/` = PJ固有ナレッジ
