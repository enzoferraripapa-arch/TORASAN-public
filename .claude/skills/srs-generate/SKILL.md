---
name: srs-generate
description: "Generates Software Requirements Specification from safety concepts."
argument-hint: "[mode: derive|diagnostic|document|review|all (default: all)]"
---

# /srs-generate — SW安全要求仕様書(SRS)生成

PH-08（SW安全要求）/ SWE.1（SW要件抽出）を支援する SRS 専門スキル。
TSR から SR/DR を導出し、SW安全要求仕様書を生成する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "all"）

> **数値参照ルール**: 本スキル内の閾値・タイミング・メモリ容量は project.json product_spec からの**例示値**です。
> 実行時は必ず `project.json` を Read して最新値を使用してください。DR テーブル等の数値をハードコード出力しないこと。

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | product_spec の必要フィールドが定義済み | project.json 内を確認 | 必要フィールドを設定してから再実行 |
| 3 | 前段フェーズが完了済み | phases[] の status 確認 | 該当フェーズを先に `/execute-phase` で実行 |
| 4 | 出力先ディレクトリが存在する | `ls` で確認 | 自動作成（mkdir -p） |

## 入力（PH-05 TSC の成果物）
- docs/05_tsc.md（技術安全コンセプト — TSR 一覧）
- project.json product_spec（数値パラメータ）
- docs/FMEA.md（故障モード→診断要求の根拠）

## 要件ID体系（PROCESS.md §7）
- **SR-XXX**: SW安全要求（機能・性能・タイミング要求）
- **DR-XXX**: 診断要求（自己診断・故障検出要求）

## 手順

### derive モード — TSR→SR 展開
各 TSR について SW で実現すべき要求を導出:

1. TSR を読み込み、HW/SW 分担を確認
2. SW 担当部分を SR として定義:

```
SR-{XXX}: {SW安全要求名}
  親: TSR-{XXX}
  ASIL: {継承}
  種別: 機能 / 性能 / タイミング / インターフェース
  要求内容: {what the SW shall do}
  入力: {入力データ・信号}
  出力: {出力データ・アクション}
  タイミング: {周期/応答時間/デッドライン}
  安全状態遷移: {異常時の動作}
  検証方法: テスト / レビュー / 解析
  検証基準: {合否判定の定量基準}
```

3. 展開ルール:
   - 1つの TSR → 1〜3個の SR（検出・判定・反応で分割）
   - product_spec の数値を具体値として埋め込み
   - タイミング要求は FTTI（/safety-concept）から逆算

例:
```
TSR-001: モータ速度監視（1500rpm閾値）
  ↓ 展開
SR-001: ホールセンサ信号から回転速度を10ms周期で算出する
SR-002: 算出速度が1500rpmを超えた場合、1ms以内にEmergencyStopを起動する
SR-003: 速度算出にはIIRフィルタ(fc≈1Hz)を適用し、ノイズ耐性を確保する
```

### diagnostic モード — DR（診断要求）生成
FMEA と IEC 60730 Annex H から診断要求を導出:

| DR-ID | 診断対象 | 検出方式 | 周期 | DC目標 | FMEA |
|-------|---------|---------|------|-------|------|
| DR-001 | CPU レジスタ | パターンテスト | 起動+100ms | 95% | FMEA-005 |
| DR-002 | RAM | March C | 起動+100ms(分割) | 95% | FMEA-006 |
| DR-003 | ROM | CRC32 | 起動+100ms(分割) | 99% | FMEA-007 |
| DR-004 | クロック | 独立比較 | 100ms | 90% | FMEA-008 |
| DR-005 | 電圧 | ADC窓比較 | 100ms(10Hz) | 60% | FMEA-010 |
| DR-006 | WDT | キック監視 | 50ms | 90% | FMEA-005 |

各 DR のテンプレート:
```
DR-{XXX}: {診断要求名}
  親: TSR-{XXX}
  FMEA参照: FMEA-{XXX}
  検出対象: {故障モード}
  検出方式: {アルゴリズム/手法}
  パラメータ: {project.json からの値}
  実行タイミング: 起動時 / 周期({ms}) / イベント駆動
  DC目標: {percent}%
  異常時動作: SafetyMgr_TransitionSafe({fault_id})
```

### document モード — SRS 文書生成
docs/08_srs.md を以下の構成で生成:

```markdown
# SW安全要求仕様書 (SRS)

## 1. 概要
  - 目的、適用範囲、参照文書

## 2. SW安全要求 (SR)
  ### SR-001: {名称}
  - 親要件: TSR-XXX
  - ASIL: B
  - 要求内容: ...
  - 検証方法: ...

## 3. 診断要求 (DR)
  ### DR-001: {名称}
  - FMEA参照: FMEA-XXX
  - 検出方式: ...
  - DC目標: ...

## 4. 非機能要求
  - メモリ制約: Flash {product_spec.mcu.flash_kb}KB, RAM {product_spec.mcu.ram_kb}KB
  - タイミング制約: メインループ ≦ {product_spec.timing.main_loop_ms}ms
  - MISRA C:2012 準拠（§5 型ルール）
  - 動的メモリ確保禁止

## 5. トレーサビリティマトリクス
  TSR → SR/DR の対応表

## 6. 変更履歴
```

### review モード — 要件レビュー
以下の観点でレビュー:

| チェック項目 | 基準 |
|------------|------|
| 完全性 | 全 TSR が SR/DR に展開されているか |
| 一貫性 | SR 間に矛盾がないか |
| 検証可能性 | 各 SR に定量的な判定基準があるか |
| 追跡可能性 | 全 SR/DR が TSR に紐付いているか |
| 実現可能性 | RL78/G14 のリソースで実現可能か |
| テスタビリティ | 各 SR に対して TC が設計可能か |
| 数値整合 | product_spec の値と矛盾しないか |

## 出力
```
=== SRS 生成結果 ===
SR: {count}件（TSR {tsr_count}件から展開）
DR: {count}件（FMEA {fmea_count}件から展開）
非機能要求: {count}件
トレーサビリティ: TSR→SR リンク率 {pct}%
レビュー: {PASS/FAIL}
出力ファイル: docs/08_srs.md
```

## 関連スキル
- /safety-concept — 上流の FSR/TSR を生成（本スキルの入力）
- /trace — SG→FSR→TSR→SR→TC のトレーサビリティ検証
- /sw-design — SR を受けて SW アーキテクチャを設計（下流）
- /test-design — SR/DR に対するテストケースを生成（検証側）
- /fmea — DR の根拠となる故障モード分析

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json 読み取り失敗 | Read エラー | パスと構文を確認。`git checkout -- project.json` で復元 |
| product_spec フィールド欠落 | キー不在 / null | 必要フィールドを報告し設定を促す。TBD マーカー付きで続行可 |
| 既存ファイル上書き競合 | Read で既存ファイル検出 | `cp {file} {file}.bak` → diff 表示 → ユーザー承認後に上書き |
| TBD 未解消（安全クリティカル） | TBD チェック | **生成を中止**。未解消 TBD を報告 |
| docs/05_tsc.md 不在（derive モード） | Read エラー / ファイル未検出 | **derive を中止**。TSR が取得不可。先に `/safety-concept tsc` で TSC を生成 |
| docs/FMEA.md 不在（diagnostic モード） | Read エラー / ファイル未検出 | **diagnostic を中止**。FMEA 結果が取得不可。先に `/fmea` を実行 |
| SR/DR ID 重複 | 生成後の ID ユニーク性チェック | 重複 ID を列挙し自動再採番。既存 docs/08_srs.md 内 ID との照合も実施 |
| TSR→SR リンク率 < 100% | review モードで未展開 TSR を検出 | 未展開 TSR を一覧表示し**警告**。derive モード再実行で補完を促す |

## 注意事項
- SR/DR の ID は §7 の命名規則に従い連番
- 全数値は project.json product_spec から取得（ハードコーディング禁止）
- 診断要求(DR)は FMEA との整合を必ず確認
- SR は「何を」実現するかを記述（「どう」実現するかは PH-09/10）

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/srs_template.md` — IEEE 29148 文書構造、EARS パターン記法、要件記述の具体例
