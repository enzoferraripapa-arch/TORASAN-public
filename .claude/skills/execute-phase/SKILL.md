---
name: execute-phase
description: "Executes V-model development phases (PH-01 to PH-15) with gate verification."
argument-hint: "[PH-XX or phase number]"
---

# /execute-phase — フェーズ実行

指定されたフェーズ（PH-01〜PH-15）を PROCESS.md §3 の手順に従い実行する。

引数: $ARGUMENTS（例: "PH-01", "01", "1" → いずれも PH-01 として解釈）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在し有効な JSON である | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | 更新対象セクションが存在する | キー存在確認 | セクションを初期化して続行（ユーザー確認後） |

## 実行手順

### Step 1: 前提確認
1. `project.json` を読み、現在のフェーズ状態を確認
2. 指定フェーズの前段フェーズが完了しているか確認（PH-01 は前提なし）
3. モードが EXPLORE の場合は順序制約を緩和

### Step 2: INPUT 確認
1. `PROCESS.md` §3 の該当フェーズセクションを読む（該当部分のみ）
2. INPUT に列挙されたファイル・成果物の存在を確認
3. 不足がある場合はユーザーに報告し、続行するか確認

### Step 3: PROCEDURE 実行
1. PROCESS.md に記載された手順を 1 ステップずつ実行
2. 成果物（OUTPUT に記載のファイル）を生成または更新
3. 要件IDは §7 の命名規則に従う（SG-XXX, FSR-XXX 等）
4. 数値は `project.json` の `product_spec` から参照（§6 準拠）
5. コード生成時は §5 型ルールを遵守

### Step 4: GATE 検証
1. PROCESS.md §3.2 共通ゲート（G-01〜G-07）を検証:
   - G-01: INPUT 完備
   - G-02: OUTPUT 生成済
   - G-03: 要件ID重複なし
   - G-04: 親要件への双方向トレース完備
   - G-05: 数値が project.json と整合
   - G-06: project.json phases 更新済
   - G-07: process_records 更新済
2. 該当フェーズ固有のゲート条件も検証
3. 全ゲート PASS の場合のみ次へ進む
4. FAIL がある場合はユーザーに報告

### Step 5: 記録更新
1. `project.json` の該当フェーズを更新:
   - status: "completed"
   - docStatus: 生成した文書のステータス
   - gateStatus: "PASS" または個別結果
2. `traceability` カウントを更新
3. `process_records/` の該当プロセス記録を更新（BP実施記録、WP一覧）
4. `spice_assessment` の該当プロセス PA 値を更新

### Step 6: Git コミット
1. 変更ファイルをステージング
2. コミットメッセージ形式:
```
feat: PH-{XX} {フェーズ名} 完了 — {主要成果物の要約}

{生成した成果物一覧}
ゲート: 全{N}項目 PASS

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
```

## エラー時の動作
- GATE FAIL → 具体的な不足項目を提示し修正案を提案
- INPUT 不足 → 必要なフェーズの実行を提案
- ファイル生成失敗 → 原因を報告しリトライ提案

## フェーズ別の専門スキル連携

各フェーズで以下の専門スキルを併用すると品質が向上する:

| フェーズ | 推奨スキル |
|---------|----------|
| PH-03 HARA | /fmea（故障モード分析）、/safety-concept（安全コンセプト設計） |
| PH-04 FSC | /safety-concept fsc（FSR導出） |
| PH-05 TSC | /safety-concept tsc（TSR導出）、/safety-concept mechanism（安全メカニズム割当） |
| PH-06 システム設計 | /system-design（ブロック図・IF定義・ASIL分解） |
| PH-07 HW設計 | /hw-review（設計レビューチェックリスト） |
| PH-08 SW要求 | /srs-generate（SRS専門生成・レビュー） |
| PH-09 SWアーキ | /sw-design（層構造・状態遷移・モジュール分割） |
| PH-10 SW実装 | /driver-gen, /mcu-config, /motor-control, /safety-diag, /memory-map |
| PH-11 SWテスト | /test-design, /test-coverage, /static-analysis |
| PH-12 HW統合 | /hw-review integration（統合テスト項目） |
| PH-13 システムテスト | /systest-design（システムレベルテスト設計） |
| PH-14 安全検証 | /safety-verify（安全論証・検証レポート） |
| PH-15 アセスメント | /assess-spice, /config-audit |

## 注意事項
- PROCESS.md は該当フェーズのセクションのみ読むこと（全文読み込み禁止）
- 数値のハードコーディング禁止（必ず project.json 参照）
- モード EXPLORE では TBD を許容、CERTIFY では TBD=0 必須

## 関連スキル
- /dashboard — プロジェクト進捗の確認
- /commit-change — フェーズ完了時のGitコミット
- /update-record — プロセス記録の更新
- /trace — トレーサビリティの検証

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/iso26262_iec60730.md` — ISO 26262 要求事項、ASIL判定基準
- `.claude/knowledge/automotive_spice.md` — PA レーティングスケール、BP サマリ
