---
name: claude-master
description: "Patrols official Claude Code docs to discover new features and upgrade skills."
argument-hint: "[mode: scan|diff|apply|domain {skill-name}|report|schedule (default: scan)]"
---
# /claude-master — Claude 技術巡回・スキル最新化エンジン

Claude Code 公式ドキュメント・コミュニティ情報を定期巡回し、最新技術をスキル体系に反映する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "scan"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | Git リポジトリ内で実行 | `git status` | リポジトリ外では実行不可 |

## 目的

Claude Code は急速に進化するプラットフォームである。スキル体系を最新の公式ベストプラクティスに追従させることで、フレームワーク全体の品質と効率を継続的に向上させる。

```
    ┌──────────────┐
    │ scan         │ ← 公式ドキュメント・チェンジログ巡回
    └──────┬───────┘
           │ 新機能・変更の検出
    ┌──────▼───────┐
    │ diff         │ ← 現行スキルとのギャップ分析
    └──────┬───────┘
           │ 改善計画
    ┌──────▼───────┐
    │ apply        │ ← スキル・ナレッジ・CLAUDE.md の更新
    └──────┬───────┘
           │ 更新完了
    ┌──────▼───────┐
    │ report       │ ← 適用履歴の記録
    └──────────────┘

    ┌──────────────┐
    │ domain       │ ← 個別スキルのドメイン技術巡回
    └──────────────┘   （安全規格、ツール、MCU etc.）
```

## 手順

### scan モード — 公式ドキュメント巡回

#### Step 1: 公式ソースの巡回

以下のソースを WebFetch / WebSearch ツールで巡回:

**Tier 1: 公式ドキュメント（必須）**
| ソース | URL | チェック内容 |
|--------|-----|-------------|
| Skills ドキュメント | https://code.claude.com/docs/skills | スキルシステムの変更・新機能 |
| Skills ドキュメント(日本語) | https://code.claude.com/docs/ja/skills | 日本語版の差分確認 |
| Sub-agents | https://code.claude.com/docs/sub-agents | サブエージェント新機能 |
| Hooks | https://code.claude.com/docs/hooks | フック機能の変更 |
| Memory | https://code.claude.com/docs/memory | メモリ管理の変更 |
| Permissions | https://code.claude.com/docs/permissions | 権限モデルの変更 |
| Plugins | https://code.claude.com/docs/plugins | プラグインシステム |
| llms.txt | https://code.claude.com/docs/llms.txt | ドキュメント全体のインデックス |

**Tier 2: リリース・変更情報**
| ソース | 方法 | チェック内容 |
|--------|------|-------------|
| Anthropic ブログ | WebSearch "Anthropic Claude Code release {year}" | 新バージョン・新機能 |
| GitHub Issues | WebSearch "claude-code changelog release notes" | バグ修正・機能追加 |
| Agent Skills 標準 | WebFetch agentskills.io | オープンスタンダードの更新 |

**Tier 3: コミュニティ情報（参考）**
| ソース | 方法 | チェック内容 |
|--------|------|-------------|
| Qiita Claude Code | WebSearch "Qiita Claude Code Skills {year}" | 実践ノウハウ |
| Zenn Claude Code | WebSearch "Zenn Claude Code Skills {year}" | 実践ノウハウ |

#### Step 2: 変更点の抽出

巡回結果から以下を抽出:

```
=== Claude Code 巡回レポート ({date}) ===

[新機能・変更]
| # | カテゴリ | 内容 | 影響度 | 出典 |
|---|---------|------|--------|------|
| 1 | Skills | {新しいフロントマターフィールド追加} | HIGH | 公式docs |
| 2 | SubAgent | {新エージェントタイプ追加} | MEDIUM | 公式docs |
| 3 | Hooks | {新イベントタイプ追加} | LOW | 公式docs |

[廃止・非推奨]
| # | 内容 | 代替 | 影響スキル |
|---|------|------|----------|
| 1 | {旧API廃止} | {新API} | /skill-A, /skill-B |

[ベストプラクティス更新]
| # | 内容 | 現状 | 推奨 |
|---|------|------|------|
| 1 | {description 記法の変更} | {現行の書き方} | {新しい書き方} |
```

#### Step 3: 巡回結果を記録

`~/.claude/knowledge/claude_platform_updates.md` に追記。

### diff モード — 現行スキルとのギャップ分析

#### Step 1: scan 結果の読み込み

`~/.claude/knowledge/claude_platform_updates.md` の最新巡回結果を読む。

#### Step 2: 全スキルとの照合

各スキルの SKILL.md を読み、以下を分析:

```
=== スキル技術ギャップ分析 ===

[フロントマター更新]
| スキル | 現状 | 推奨変更 | 優先度 |
|--------|------|---------|--------|
| /fmea | description のみ | context: fork 追加推奨 | MEDIUM |
| /dashboard | disable なし | user-invocable: false 検討 | LOW |

[新機能活用可能]
| 機能 | 対象スキル | 活用方法 | 期待効果 |
|------|----------|---------|---------|
| !`command` | /env-check | 動的コンテキスト注入で環境情報を自動取得 | 手順簡略化 |
| context: fork | /health-check | サブエージェントで並列分析 | 高速化 |
| references/ | /fmea, /motor-control | 重い参照資料を分離 | トークン節約 |

[ナレッジベース更新]
| ナレッジ | 更新内容 | 影響スキル |
|---------|---------|----------|
| claude_code_ops.md | 新フロントマターフィールド追記 | 全スキル |
```

#### Step 3: 優先度付き改善計画を出力

### apply モード — 改善適用

#### Step 1: diff 結果の確認

ユーザーに改善計画を提示し、実施項目を選択。

#### Step 2: スキルの更新

選択された項目に対して:
1. SKILL.md のフロントマター更新
2. 本文の手順更新（新機能の活用）
3. references/ ディレクトリの作成（必要に応じて）
4. scripts/ の追加（自動化可能な部分）

#### Step 3: ナレッジベースの更新

`~/.claude/knowledge/claude_code_ops.md` 等の関連ナレッジを更新。

#### Step 4: CLAUDE.md の更新（必要に応じて）

プロジェクト全体の運用方法に影響する変更がある場合。

#### Step 5: 記録とコミット

`~/.claude/knowledge/claude_platform_updates.md` に適用履歴を記録し、コミット。

### domain モード — 個別スキルのドメイン技術巡回

特定スキルのドメイン知識を最新化する。

#### Step 1: 対象スキルの特定

```bash
/claude-master domain {skill-name}
```

#### Step 2: ドメイン別の巡回ソース

| スキルカテゴリ | 巡回対象 | 検索キーワード |
|-------------|---------|-------------|
| 安全規格系 (fmea, safety-*) | ISO 26262 改訂、IEC 60730 更新 | "ISO 26262 amendment {year}", "IEC 60730 update" |
| SPICE系 (assess-spice, update-record) | Automotive SPICE 更新 | "Automotive SPICE 4.0", "ASPICE update {year}" |
| コード系 (static-analysis, driver-gen) | MISRA C 改訂、ツール更新 | "MISRA C:2012 amendment", "cppcheck release" |
| MCU系 (mcu-config, motor-control) | RL78 更新、BSW標準 | "Renesas RL78 update", "AUTOSAR BSW" |
| テスト系 (test-design, test-coverage) | テスト方法論更新 | "MC/DC coverage tools", "unit test embedded" |
| Claude系 (skill-manage, skill-evolve) | Claude Code 更新 | (scan モードと同一) |

#### Step 3: 発見事項の記録

該当するナレッジベースファイルを更新。

#### Step 4: スキルへの反映

発見した新技術・変更をスキルの手順に反映。

### report モード — 巡回履歴・適用状況

```
=== 技術巡回レポート ===

[巡回履歴]
| 日付 | モード | 発見件数 | 適用件数 | 未適用 |
|------|--------|---------|---------|--------|
| {date} | scan | {n} | {n} | {n} |
| {date} | domain /fmea | {n} | {n} | {n} |

[適用済み改善]
| 日付 | 対象 | 変更内容 | コミット |
|------|------|---------|---------|
| {date} | 全スキル | フロントマター移行 | {hash} |
| {date} | /fmea | references/ 分離 | {hash} |

[未適用（保留）]
| # | 内容 | 理由 | 再評価予定 |
|---|------|------|----------|
| 1 | {機能X} | {安定性未確認} | 次回巡回時 |

[ナレッジ鮮度]
| ナレッジ | 最終更新 | 経過日数 | 判定 |
|---------|---------|---------|------|
| claude_code_ops.md | {date} | {n}日 | {FRESH/STALE} |
| iso26262_iec60730.md | {date} | {n}日 | {FRESH/STALE} |

鮮度基準:
  FRESH: 30日以内に更新
  AGING: 30-90日
  STALE: 90日以上（巡回推奨）
```

### schedule モード — 推奨巡回スケジュール

```
=== 推奨巡回スケジュール ===

[定期巡回]
| 頻度 | 対象 | モード | 推奨タイミング |
|------|------|--------|-------------|
| セッション開始時 | Claude 公式 | scan (軽量) | 前回巡回から7日以上経過時 |
| 週次 | 全ドメイン | scan + diff | 週初めのセッション |
| 月次 | 重点スキル | domain | 月初めのセッション |
| 四半期 | 全スキル | scan + diff + apply | 四半期レビュー |

[次回推奨]
  Claude 公式巡回: {前回日付 + 7日} ({残り日数}日後)
  ドメイン巡回: {対象スキル} ({理由})
  全体レビュー: {日付}

[自動リマインド]
  セッション開始時に前回巡回日を確認し、
  7日以上経過していれば scan を提案する。
```

## 巡回の原則

1. **公式情報優先**: 公式ドキュメント → リリースノート → コミュニティの順で信頼性を判断
2. **破壊的変更は慎重に**: 既存スキルを壊す変更は diff で影響分析後、ユーザー確認必須
3. **段階的適用**: 全スキル一括更新ではなく、影響度 HIGH → MEDIUM → LOW の順で適用
4. **ナレッジとスキルは連動**: ナレッジ更新時は参照スキルの手順も確認・更新
5. **履歴を残す**: 全ての巡回・適用を `claude_platform_updates.md` に記録

## 関連スキル
- /skill-evolve — スキル成熟 PDCA（本スキルは技術面、skill-evolve は運用面）
- /skill-manage — スキル一覧・品質監査
- /env-check — 開発環境のツール検証（ドメイン巡回結果を反映）
- /health-check — プロジェクト健全性（技術鮮度セクション）

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| 入力データ不足 | 必要ファイル/キーの不在 | 不足項目を報告し、前提スキルの実行を促す |
| 処理中断 | 予期しないエラー | エラー内容を報告。変更途中のファイルがあれば `git checkout` で復元 |

## 注意事項
- WebFetch / WebSearch はレート制限に注意。1回の巡回で過度なリクエストを避ける
- 公式ドキュメントの著作権を尊重し、内容をそのまま複製しない（要約・差分のみ記録）
- コミュニティ情報は参考レベル。公式と矛盾する場合は公式を優先
- scan 結果が「変更なし」でも巡回日時は記録する（鮮度管理のため）
- ユーザーが「最新化して」「アップデートして」と言った場合も本スキルの対象

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `~/.claude/knowledge/claude_platform_updates.md` — 巡回履歴、適用済み改善、未適用保留、ナレッジ鮮度記録
- `~/.claude/knowledge/claude_code_ops.md` — 現行の Claude Code 運用知識（更新対象）
- `~/.claude/knowledge/skill_lifecycle.md` — スキルライフサイクル（技術更新による昇格基準）

※ `~/.claude/knowledge/` = 共有ナレッジ（全PJ共通）、`.claude/knowledge/` = PJ固有ナレッジ
