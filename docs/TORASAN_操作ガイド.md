# TORASAN 操作ガイド

TORASAN フレームワークの操作・コマンド・スキルの日本語リファレンスです。

---

## 目次

1. [基本操作](#1-基本操作)
2. [コマンド一覧（クイックリファレンス）](#2-コマンド一覧)
3. [セッション管理](#3-セッション管理)
4. [汎用スキル（15本）](#4-汎用スキル)
5. [ドメインスキル — 安全規格・要件系（9本）](#5-ドメインスキル--安全規格要件系)
6. [ドメインスキル — 設計・実装系（8本）](#6-ドメインスキル--設計実装系)
7. [ドメインスキル — テスト・検証系（3本）](#7-ドメインスキル--テスト検証系)
8. [ドメインスキル — その他（6本）](#8-ドメインスキル--その他)
9. [GUI（Web アプリ）](#9-gui)
10. [スキル配布・インストール](#10-スキル配布)
11. [定期運用タスク](#11-定期運用タスク)

---

## 1. 基本操作

### スキルの呼び出し方

Claude Code のチャット欄で `/スキル名 サブコマンド` と入力します。

```
/session start          ← セッション開始
/health-check full      ← フルヘルスチェック
/fmea show              ← FMEA テーブル表示
```

### 日本語での指示

自然言語で指示することもできます。CLAUDE.md のコマンドマッピングにより自動的に適切なスキルが実行されます。

| 日本語で言えること | 実行されるスキル |
|---|---|
| 「おつです」 | `/session end` |
| 「スキル一覧見せて」 | Web GUI に誘導 (localhost:3000/skills) |
| 「スキル改善して」 | `/skill-evolve cycle` |
| 「最新化して」 | `/claude-master scan` |
| 「マニュアル更新して」 | `python scripts/generate_manuals.py` |
| 「プロジェクト一覧」 | `/repo-manage list` |

---

## 2. コマンド一覧

### よく使うコマンド

| コマンド | 説明 |
|---|---|
| `/session start` | セッション開始（引継ぎ情報読込み） |
| `/session end` | セッション終了（反省会 + 引継ぎ + バックアップ） |
| `/session status` | 現在のセッション状態確認 |
| `/health-check quick` | 簡易ヘルスチェック |
| `/health-check full` | 全項目ヘルスチェック |
| `/execute-phase PH-XX` | V-model フェーズ実行（01〜15） |
| `/validate all` | 全項目の統合検証 |
| `/skill-manage list` | スキル一覧表示 |
| `/skill-manage audit` | スキル品質監査 |
| `/skill-evolve cycle` | スキル改善 PDCA サイクル |
| `/claude-master scan` | 公式ドキュメント巡回・最新化 |
| `/repo-manage list` | 登録プロジェクト一覧 |
| `/repo-manage sync` | ドメインスキル同期 |
| `/trace check` | トレーサビリティ検証 |

### Git 関連

| コマンド | 説明 |
|---|---|
| `/commit-change feat` | 新機能コミット + changeLog 記録 |
| `/commit-change fix` | バグ修正コミット |
| `/commit-change docs` | ドキュメント変更コミット |
| `/backup v1.0` | バージョンタグ作成 |

### マニュアル生成

| コマンド | 説明 |
|---|---|
| `python scripts/generate_manuals.py` | 全文書を再生成（バージョン据え置き） |
| `python scripts/generate_manuals.py -m "変更内容"` | 再生成 + バージョンアップ + 改版履歴記録 |
| `python scripts/generate_manuals.py -m "変更内容" --bump major` | メジャーバージョンアップ（v1.x → v2.0） |
| `python scripts/generate_manuals.py --spec` | 仕様書のみ再生成 |
| `python scripts/generate_manuals.py --ops` | 操作手順書のみ再生成 |
| `python scripts/generate_manuals.py --diff-only` | 差分レポートのみ（再生成なし） |

**バージョン管理ルール:**
- `-m` なしで実行: 再生成のみ（バージョン番号は変わらない。体裁修正・リビルド用）
- `-m "内容"` 付きで実行: マイナーバージョンアップ（v1.1 → v1.2）+ 改版履歴に記録
- `--bump major` 付き: メジャーバージョンアップ（v1.x → v2.0。大規模構成変更時）
- 改版履歴・バージョンはマニフェスト（`.manuals_manifest.json`）で一元管理される

---

## 3. セッション管理

セッション管理はフレームワークの中核です。全作業はセッション単位で管理されます。

### `/session start`

セッション開始時に実行します。以下を行います:
- メモリ（MEMORY.md）から前回の引継ぎ情報を読込み
- 残作業・TBD の確認
- 7日以上巡回していなければ `/claude-master scan` を提案

### `/session end`

セッション終了時に実行します（「おつです」でも可）。以下を行います:
- 反省会（振り返り議事録の作成）
- 引継ぎ情報の MEMORY.md への保存
- バックアップ（Git タグ）
- master ブランチへのマージ

### `/session status`

現在のセッション情報（開始時刻、実行済みスキル等）を表示します。

---

## 4. 汎用スキル

全プロジェクト共通で使えるスキルです。`install.sh` で `~/.claude/skills/` に配布できます。

### execute-phase — V-model フェーズ実行

V-model の各フェーズ（PH-01〜PH-15）を順番に実行します。各フェーズにはゲート条件があり、前提条件を満たさないと次に進めません。

| サブコマンド | 説明 |
|---|---|
| `PH-01` | 安全計画 |
| `PH-02` | アイテム定義 |
| `PH-03` | HARA（ハザード分析・リスク評価） |
| `PH-04` | 機能安全コンセプト |
| `PH-05` | 技術安全コンセプト |
| `PH-06` | システム設計 |
| `PH-07` | HW 設計 |
| `PH-08` | SW 安全要求 |
| `PH-09` | SW アーキテクチャ |
| `PH-10` | SW ユニット設計 |
| `PH-11` | SW テスト仕様（ユニット/統合/適格性） |
| `PH-12` | HW 統合テスト |
| `PH-13` | システムテスト |
| `PH-14` | 安全検証 |
| `PH-15` | 安全アセスメント |

```
/execute-phase PH-06    ← システム設計フェーズを実行
```

### dashboard — ダッシュボード

プロジェクトの進捗・成果物・SPICE レベルを表示します。
Web GUI (localhost:3000) に委譲されるため、CLI では「localhost:3000 で確認できます」と案内します。

### health-check — ヘルスチェック

| サブコマンド | 説明 |
|---|---|
| `quick` | 主要指標のみ簡易チェック |
| `full` | 全項目（フェーズバランス・トレーサビリティ・SPICE 成熟度）を横断分析 |
| `action` | 問題箇所に対するアクション提案 |

### validate — 統合検証

| サブコマンド | 説明 |
|---|---|
| `numbers` | 数値の整合性検証 |
| `types` | 型ルール検証 |
| `gates` | ゲート条件検証 |
| `all` | 全項目一括検証 |

### trace — トレーサビリティ

| サブコマンド | 説明 |
|---|---|
| `check` | トレーサビリティリンクの整合性チェック |
| `matrix` | トレーサビリティマトリクス生成 |
| `report` | トレーサビリティレポート出力 |

### commit-change — コミット

Git コミットと project.json の changeLog 記録を同時に行います。

```
/commit-change feat     ← type: feat でコミット
/commit-change fix      ← type: fix でコミット
```

コミットメッセージのフォーマット:
```
<type>: <日本語の要約>

<詳細>

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
```

### backup — バックアップ

Git タグでバージョンバックアップを作成します。

```
/backup v2.1            ← v2.1 タグを作成
```

### skill-manage — スキル管理

| サブコマンド | 説明 |
|---|---|
| `list` | 全スキル一覧表示 |
| `coverage` | スキルカバレッジ分析（不足領域の特定） |
| `create` | 新規スキルのスキャフォールド生成 |
| `audit` | スキル品質監査（5セッション毎に推奨） |

### skill-evolve — スキル改善エンジン

PDCA サイクルでスキルを改善します。

| サブコマンド | 説明 |
|---|---|
| `feedback` | スキル実行結果のフィードバック記録 |
| `analyze` | ギャップ分析（改善点の特定） |
| `improve` | 改善の実施 |
| `maturity` | 成熟度レベル表示 |
| `cycle` | feedback → analyze → improve の一括実行 |
| `tech-refresh` | 技術リフレッシュ（依存技術の更新確認） |

### claude-master — 技術巡回

Claude Code の公式ドキュメントを巡回し、スキル・ナレッジを最新に保ちます。

| サブコマンド | 説明 |
|---|---|
| `scan` | 公式ドキュメント巡回（差分検出） |
| `diff` | 前回巡回からの差分表示 |
| `apply` | 検出された更新をスキルに反映 |
| `domain` | ドメイン固有の更新チェック |
| `report` | 巡回レポート出力 |
| `schedule` | 巡回スケジュール表示 |

### repo-manage — リポジトリ管理

| サブコマンド | 説明 |
|---|---|
| `list` | 登録済みプロジェクト一覧 |
| `register` | 新規プロジェクト登録 |
| `sync` | ドメインスキルの同期 |
| `move` | プロジェクトディレクトリ移動 |
| `remove` | プロジェクト登録解除 |
| `status` | プロジェクト状態確認 |
| `discover` | 未登録プロジェクトの自動検出 |

### その他の汎用スキル

| スキル | 説明 |
|---|---|
| `env-check` | 開発環境検証（tools/path/config/fix/all） |
| `platform-info` | OS・WSL・アーキテクチャ情報の検出 |
| `assess-spice` | Automotive SPICE 能力レベル評価 |
| `config-audit` | 構成管理監査（SUP.8 対応） |
| `update-record` | SPICE プロセス記録の更新 |
| `manage-tbd` | TBD 項目の管理（list/resolve/add） |
| `problem-resolve` | 問題解決管理（list/add/analyze/resolve/report） |
| `worktree-cleanup` | ワークツリー・ブランチの整理（scan/clean/branches） |
| `ingest` | 既存資産の取り込み |
| `generate-docs` | 成果物ドキュメント生成 |

---

## 5. ドメインスキル — 安全規格・要件系

ISO 26262 / IEC 60730 / IEC 61508 などの安全規格に対応するスキル群です。

### select-standard — 規格選定

製品カテゴリから適用すべき安全規格を導出します。

| サブコマンド | 説明 |
|---|---|
| `derive` | 製品情報から適用規格を導出 |
| `check` | 現在の規格設定を確認 |
| `matrix` | 規格対応マトリクス表示 |

### switch-standard — 規格切替

プロジェクトの適用規格を切り替えます。

```
/switch-standard iso26262    ← ISO 26262 モードに切替
/switch-standard iec60730    ← IEC 60730 モードに切替
/switch-standard iec61508    ← IEC 61508 モードに切替
```

### fmea — FMEA（故障モード影響分析）

| サブコマンド | 説明 |
|---|---|
| `show` | FMEA テーブル表示 |
| `add` | 故障モードの追加 |
| `dc` | 診断カバレッジ（DC）算出 |
| `map` | 故障モード → 安全機構のマッピング |

### safety-concept — 安全コンセプト

| サブコマンド | 説明 |
|---|---|
| `fsc` | 機能安全コンセプト（FSC）生成 |
| `tsc` | 技術安全コンセプト（TSC）生成 |
| `mechanism` | 安全機構の設計 |
| `redundancy` | 冗長性分析 |
| `ftti` | FTTI（フォルトトレラント時間間隔）計算 |
| `all` | 全項目一括実行 |

### safety-diag — 自己診断コード生成

IEC 60730 Annex H に準拠する自己診断コードを生成します。

| サブコマンド | 説明 |
|---|---|
| `all` | 全診断テスト生成 |
| `ram` | RAM テスト |
| `rom` | ROM テスト（CRC） |
| `cpu` | CPU レジスタテスト |
| `clock` | クロック監視テスト |
| `voltage` | 電圧監視テスト |
| `wdt` | ウォッチドッグタイマテスト |

### safety-verify — 安全検証

| サブコマンド | 説明 |
|---|---|
| `verify` | 安全要求の検証実行 |
| `case` | Safety Case 構築 |
| `report` | 検証レポート出力 |
| `gap` | ギャップ分析 |
| `all` | 全項目一括実行 |

### srs-generate — SW 安全要求仕様書

| サブコマンド | 説明 |
|---|---|
| `derive` | TSR から SW 安全要求（SR/DR）を導出 |
| `diagnostic` | 診断要求の生成 |
| `document` | 仕様書ドキュメント出力 |
| `review` | レビューチェックリスト生成 |
| `all` | 全項目一括実行 |

### system-design — システム設計

| サブコマンド | 説明 |
|---|---|
| `block` | ブロック図生成 |
| `interface` | インターフェース定義 |
| `allocation` | 機能割当て |
| `asil` | ASIL 分解 |
| `review` | 設計レビュー |
| `all` | 全項目一括実行 |

### hw-review — HW 設計レビュー

| サブコマンド | 説明 |
|---|---|
| `circuit` | 回路設計レビュー |
| `component` | 部品選定レビュー |
| `emc` | EMC 設計レビュー |
| `safety` | 安全設計レビュー |
| `integration` | 統合レビュー |
| `all` | 全項目一括実行 |

---

## 6. ドメインスキル — 設計・実装系

### sw-design — SW 設計

| サブコマンド | 説明 |
|---|---|
| `architecture` | 層構造設計（APP/BSW/HAL） |
| `state-machine` | 状態遷移設計 |
| `module` | モジュール分割設計 |
| `pattern` | 設計パターン適用 |
| `detailed` | 詳細設計 |
| `review` | 設計レビュー |
| `all` | 全項目一括実行 |

### driver-gen — ドライバ生成

MISRA C:2012 準拠の HAL/BSW ドライバコードを生成します。

```
/driver-gen wdt          ← ウォッチドッグドライバ生成
/driver-gen adc          ← ADC ドライバ生成
/driver-gen all          ← 全ドライバ一括生成
```

対応ペリフェラル: `wdt`, `adc`, `timer`, `uart`, `spi`, `dem`

### mcu-config — MCU 設定コード生成

MCU（RL78/G14 等）のペリフェラル初期化コードを生成します。

対応ペリフェラル: `adc`, `wdt`, `timer`, `gpio`, `uart`, `clock`, `all`

### motor-control — モータ制御

BLDC モータ制御コードの生成・分析・テストを行います。

| サブコマンド | 説明 |
|---|---|
| `generate` | 制御コード生成 |
| `analyze` | 制御ロジック分析 |
| `test` | テストコード生成 |

### memory-map — メモリマップ

| サブコマンド | 説明 |
|---|---|
| `show` | メモリマップ表示 |
| `analyze` | リソース使用量分析 |
| `update` | メモリマップ更新 |

### static-analysis — 静的解析

| サブコマンド | 説明 |
|---|---|
| `all` | 全ツール一括実行 |
| `cppcheck` | cppcheck 実行 |
| `clang-tidy` | clang-tidy 実行 |
| `flawfinder` | flawfinder 実行 |
| `misra` | MISRA C:2012 チェック |

---

## 7. ドメインスキル — テスト・検証系

### test-design — テスト設計

| サブコマンド | 説明 |
|---|---|
| `strategy` | テスト戦略策定 |
| `cases` | テストケース生成 |
| `boundary` | 境界値テスト設計 |
| `equivalence` | 同値分割テスト設計 |
| `negative` | 異常系テスト設計 |
| `all` | 全項目一括実行 |

### test-coverage — テストカバレッジ

| サブコマンド | 説明 |
|---|---|
| `requirements` | 要件カバレッジ分析 |
| `code` | コードカバレッジ分析 |
| `gap` | カバレッジギャップ分析 |
| `all` | 全項目一括実行 |

### systest-design — システムテスト設計

| サブコマンド | 説明 |
|---|---|
| `spec` | テスト仕様書生成 |
| `scenario` | テストシナリオ設計 |
| `acceptance` | 受入条件設計 |
| `all` | 全項目一括実行 |

---

## 8. ドメインスキル — その他

| スキル | 説明 | サブコマンド |
|---|---|---|
| `select-standard` | 製品規格選定 | derive/check/matrix |
| `switch-standard` | 規格パッケージ切替 | iso26262/iec60730/iec61508 |
| `ingest` | 既存資産取り込み | — |
| `generate-docs` | 成果物ドキュメント生成 | PH-XX/trace-matrix/all |

---

## 9. GUI

Web アプリ (localhost:3000) で以下の情報を閲覧できます。読み取り専用の表示は GUI に委譲されています。

| ページ | URL | 内容 |
|---|---|---|
| ダッシュボード | `/` | プロジェクト全体の進捗 |
| スキル一覧 | `/skills` | 全スキルの一覧・状態 |
| 変更履歴 | `/changelog` | project.json の変更ログ |
| セッション | `/sessions` | セッション履歴 |
| トラッカー | `/tracker` | TBD・Issues・Ideas の追跡 |
| 環境 | `/environment` | 開発環境情報 |

### GUI の起動

```bash
cd app && npm run dev
```

---

## 10. スキル配布

### install.sh — 汎用スキルの配布

TORASAN の汎用スキル・ナレッジをユーザーレベル (`~/.claude/`) にコピーし、全プロジェクトで使えるようにします。

```bash
./install.sh              # 配布実行
./install.sh --dry-run    # ドライラン（変更なし）
```

**配布対象スキル（15本）:**
session, dashboard, health-check, skill-manage, skill-evolve, repo-manage, backup, commit-change, config-audit, worktree-cleanup, update-record, env-check, platform-info, claude-master

**配布対象ナレッジ（8本）:**
claude_code_ops.md, claude_platform_updates.md, skill_lifecycle.md, skill_feedback_log.md, git_worktree_branch_management.md, cross_platform_dev.md, memory_paths.md, error_prevention.md

---

## 11. 定期運用タスク

| 頻度 | タスク | コマンド |
|---|---|---|
| 毎セッション開始時 | セッション開始 | `/session start` |
| 毎セッション終了時 | セッション終了 | `/session end` |
| 7日超巡回なし時 | 技術巡回 | `/claude-master scan` |
| スキル実行後 | フィードバック記録 | `/skill-evolve feedback` |
| 5セッション毎 | スキル監査 | `/skill-manage audit` |
| 構成変更時 | マニュアル更新 | `python scripts/generate_manuals.py` |

---

## 付録: ディレクトリ構成

```
TORASAN/
├── .claude/
│   ├── skills/          ← 全スキル（41本）
│   │   ├── session/
│   │   ├── execute-phase/
│   │   ├── fmea/
│   │   └── ...
│   └── knowledge/       ← ナレッジファイル
├── app/                 ← 管理 GUI（React + Fastify）
├── docs/                ← 成果物ドキュメント
│   ├── manuals/         ← 生成マニュアル（DOCX）
│   └── diagrams/        ← アーキテクチャ図
├── projects/            ← 個別PJスナップショット
├── scripts/             ← ユーティリティスクリプト
├── install.sh           ← 汎用スキル配布スクリプト
├── CLAUDE.md            ← プロジェクト設定
└── PROCESS.md           ← ISO 26262 + SPICE プロセス定義
```

---

*TORASAN 操作ガイド v1.0 / 作成日: 2026-03-07*
