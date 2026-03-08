---
name: health-check
description: "Performs comprehensive project health assessment across phases and quality metrics."
argument-hint: "[scope: quick|full|action (default: full)]"
---

# /health-check — プロジェクト健全性チェック

TORASAN プロジェクト全体のバランス・整合性・ギャップを横断的に分析し、次アクションを提案する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "full"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | Git リポジトリ内で実行 | `git status` | リポジトリ外では実行不可 |

## チェック項目

### 1. フェーズバランス
project.json の phases を読み、V-model の左右バランスを確認:

```
V-model バランスチェック:

[左側: 設計]          [右側: 検証]
PH-01 計画     ←────→ PH-15 アセスメント
PH-02 アイテム  ←────→ PH-14 安全検証
PH-03 HARA     ←────→ PH-13 システムテスト
PH-04 FSC      ←────→ PH-12 HW統合テスト
PH-05 TSC      ←────→ PH-11 SWテスト
PH-06 システム設計 ←→ PH-12 HW統合テスト
PH-07 HW設計   ←────→ PH-12 HW統合テスト
PH-08 SW要求   ←────→ PH-11 SWテスト
PH-09 SWアーキ  ←───→ PH-11 SWテスト
PH-10 SW実装   ←────→ PH-11 SWテスト

バランス評価:
  左側完了: {left_done}/{left_total}
  右側完了: {right_done}/{right_total}
  判定: {バランス良好 / 左偏重 / 右偏重}
```

### 2. トレーサビリティ健全性
要件チェーンの完全性を検証:

```
SG → FSR → TSR → SR → TC チェーン分析:
  SG: {count}件
  FSR: {count}件 (SG→FSRリンク率: {pct}%)
  TSR: {count}件 (FSR→TSRリンク率: {pct}%)
  SR: {count}件 (TSR→SRリンク率: {pct}%)
  TC: {count}件 (SR→TCリンク率: {pct}%)

  孤児要件: {count}件
  逆トレース欠損: {count}件
  全チェーン完備率: {pct}%
```

### 3. SPICE プロセス成熟度
spice_assessment を分析:

```
SPICE 成熟度マップ:
                    PA1.1  PA2.1  PA2.2  Level
  SYS系 (5 proc):  {avg}  {avg}  {avg}  {avg_lv}
  SWE系 (6 proc):  {avg}  {avg}  {avg}  {avg_lv}
  MAN系 (1 proc):  {val}  {val}  {val}  {lv}
  SUP系 (4 proc):  {avg}  {avg}  {avg}  {avg_lv}

  目標 Level 2 達成率: {achieved}/{total} ({pct}%)
  ボトルネック: {最低レベルのプロセス}
```

### 4. 成果物完備性
各フェーズの OUTPUT ファイルが存在するか確認:

```
成果物チェック:
  ✓ docs/01_safety_plan.md — 存在
  ✗ docs/02_item_definition.md — 未作成
  — docs/03_hara.md — 該当フェーズ未開始

  存在率: {exist}/{expected} ({pct}%)
```

### 5. 数値整合性
/validate の簡易版を実行:
- project.json product_spec の主要数値が成果物に正しく反映されているか
- 矛盾箇所があれば報告

### 6. TBD / Problem / Risk
```
未解決事項:
  TBD: {tbd_count}件
  Problem: {problem_count}件
  未完了ゲート: {gate_fail_count}件

CERTIFY 移行阻害要因:
  - TBD-02: AI自動実行API設計 [影響: 全体]
  - TBD-03: Gitリポジトリ移行ツール [影響: SUP.8]
```

### 7. スキル活用状況
利用可能な26スキルと現在のフェーズ状況から、次に使うべきスキルを提案:

```
推奨スキル活用:
  現在のフェーズ: PH-{XX} {name}
  推奨スキル:
    1. /execute-phase {next_phase} — 次フェーズ実行
    2. /assess-spice — 現在のSPICEレベル確認
    3. /{specific_skill} — {理由}
```

## 出力フォーマット

### quick モード
```
=== TORASAN Health Check (Quick) ===
フェーズ: {done}/{total} 完了
トレース: {pct}% 完備
SPICE: {achieved}/{total} Level 2
TBD: {count}件
判定: {健全 / 注意 / 要対応}
次アクション: /execute-phase PH-{XX}
```

### full モード
上記7項目の全詳細を表示

### action モード
```
=== 推奨アクション（優先順） ===
1. [即時] {action} — {理由}
   → /{skill_name} {args}
2. [今週] {action} — {理由}
   → /{skill_name} {args}
3. [今月] {action} — {理由}
   → /{skill_name} {args}
```

## フェーズ別推奨スキルマップ

health-check の結果に応じて、次フェーズに最適な専門スキルを提案する:

| 次フェーズ | 推奨スキル | 用途 |
|-----------|----------|------|
| PH-01 計画 | /execute-phase 01 | 汎用実行で十分 |
| PH-02 アイテム定義 | /execute-phase 02 | 汎用実行で十分 |
| PH-03 HARA | /fmea, /safety-concept | 故障分析・安全コンセプト |
| PH-04 FSC | /safety-concept fsc | FSR専門導出 |
| PH-05 TSC | /safety-concept tsc | TSR専門導出 |
| PH-06 システム設計 | /system-design | ブロック図・IF・ASIL分解 |
| PH-07 HW設計 | /hw-review | 回路・部品・EMC・安全回路 |
| PH-08 SW要求 | /srs-generate | SRS専門生成 |
| PH-09 SWアーキ | /sw-design | 層構造・状態遷移 |
| PH-10 SW実装 | /driver-gen, /mcu-config, /motor-control, /safety-diag | 各種コード生成 |
| PH-11 SWテスト | /test-design, /test-coverage, /static-analysis | テスト設計・カバレッジ・静的解析 |
| PH-12 HW統合 | /hw-review integration | HW統合テスト |
| PH-13 システムテスト | /systest-design | システムレベルテスト |
| PH-14 安全検証 | /safety-verify | Safety Case・検証レポート |
| PH-15 アセスメント | /assess-spice, /config-audit | SPICE評価・構成監査 |

横断的に使えるスキル:
- /trace — トレーサビリティ検証（PH-04以降いつでも）
- /validate — 数値・型・ゲート検証（成果物生成後）
- /problem-resolve — 問題発生時の追跡・分析
- /commit-change — コミット + changeLog（毎フェーズ完了時）
- /backup — 重要マイルストーンでのバックアップ

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| データソース読み取り失敗 | Read / Glob エラー | 対象不在を報告。該当項目を「N/A」として表示 |
| 分析対象データ不足 | 必要キーの不在 | 不足項目を明示し、データ投入を促す |

## 注意事項
- project.json のみで完結する項目は高速に処理
- ファイル存在チェックは Glob で効率的に実行
- 詳細な内容チェックは各専門スキルに委譲（ここでは概要のみ）
- モード EXPLORE では TBD を許容、CERTIFY では全て ERROR
- 結果は process_records/ には記録しない（情報提供のみ）

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/automotive_spice.md` — SPICE 成熟度モデル、プロセスアセスメント基準

## 関連スキル
- `/dashboard` — プロジェクトダッシュボード表示
- `/assess-spice` — SPICE プロセス評価
- `/config-audit` — 構成管理監査
- `/validate` — 数値・型・ゲート検証
