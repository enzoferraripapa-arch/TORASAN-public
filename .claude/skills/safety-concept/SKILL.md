---
name: safety-concept
description: "Creates functional safety concepts mapping safety goals to technical requirements."
argument-hint: "[mode: fsc|tsc|mechanism|redundancy|ftti|all (default: all)]"
---

# /safety-concept — 安全コンセプト設計

PH-04（FSC: 機能安全コンセプト）・PH-05（TSC: 技術安全コンセプト）を支援する安全設計専門スキル。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "all"）

> **数値参照ルール**: 本スキル内の応答時間・タイミング・閾値は project.json product_spec からの**例示値**です。
> 実行時は必ず `project.json` を Read して最新値を使用してください。FSR/TSR の数値をハードコード出力しないこと。

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | product_spec の必要フィールドが定義済み | project.json 内を確認 | 必要フィールドを設定してから再実行 |
| 3 | 前段フェーズが完了済み | phases[] の status 確認 | 該当フェーズを先に `/execute-phase` で実行 |
| 4 | 出力先ディレクトリが存在する | `ls` で確認 | 自動作成（mkdir -p） |

## 安全目標（PH-03 HARA の入力）

> ※ 以下は docs/03_hara.md からの例示。実行時は docs/03_hara.md を Read して最新の SG 一覧を使用すること。

| SG-ID | 安全目標 | ASIL | フォールセーフ状態 |
|-------|---------|------|----------------|
| SG-001 | モータ過速度防止 | B | モータ停止 |
| SG-002 | モータ過電流防止 | B | モータ停止 + 電流遮断 |
| SG-003 | 蓋開放時モータ停止 | B | モータ停止 (≦{timing.lid_response_ms}ms) |
| SG-004 | 漏水時モータ停止 | A | モータ停止 |
| SG-005 | MCU異常時安全状態遷移 | B | モータ停止 + 出力無効化 |
| SG-006 | 電源異常時安全状態遷移 | B | モータ停止 |

## 手順

### fsc モード — 機能安全コンセプト（FSR 生成）
各 SG に対して FSR（機能安全要求）を導出:

1. **安全機能の定義**: SG を実現するために必要な機能
2. **検出機能**: 異常を検出する機能
3. **反応機能**: 異常検出後に安全状態へ遷移する機能
4. **警告機能**: ユーザーへの通知（該当する場合）

生成テンプレート:
```
FSR-{XXX}: {安全機能名}
  親: SG-{XXX}
  ASIL: {継承 or 分解後}
  種別: 検出 / 反応 / 警告
  機能説明: {what}
  トリガ条件: {when}
  応答時間要求: {how fast}
  フォールセーフ動作: {safe action}
```

例:
```
FSR-001: モータ速度監視機能
  親: SG-001
  ASIL: B
  種別: 検出
  機能説明: モータ回転速度を周期的に監視し、1500rpm超過を検出する
  トリガ条件: 運転中常時（10ms周期）
  応答時間要求: 検出から100ms以内に反応機能を起動
  フォールセーフ動作: FSR-002（緊急停止）を起動
```

### tsc モード — 技術安全コンセプト（TSR 生成）
各 FSR を技術要求（TSR）に展開:

1. **HW/SW 割当**: FSR をHW実装/SW実装に分配
2. **診断要求**: 各安全機能の故障検出手段
3. **安全メカニズム**: 具体的な検出・反応メカニズム
4. **DC 目標**: 検出カバレッジ目標値

生成テンプレート:
```
TSR-{XXX}: {技術要求名}
  親: FSR-{XXX}
  ASIL: {継承}
  実装: HW / SW / HW+SW
  技術仕様:
    - 検出方式: {具体的な手段}
    - パラメータ: project.json から取得
    - 周期/応答時間: {timing}
  DC目標: {percent}%
  根拠: IEC 60730 Annex H / ISO 26262 Part 5 表D.x
```

### mechanism モード — 安全メカニズム割当
IEC 60730 Annex H + ISO 26262 の安全メカニズムカタログ:

| 故障カテゴリ | メカニズム | DC典型値 | TSR |
|------------|----------|---------|-----|
| CPU暴走 | WDT (100ms) | 60-90% | TSR-015 |
| CPU演算異常 | レジスタテスト | 90-99% | TSR-008 |
| RAM故障 | March C テスト | 90-99% | TSR-009 |
| ROM故障 | CRC32 検証 | 99% | TSR-010 |
| クロック異常 | 独立クロック比較 | 60-90% | TSR-011 |
| 電圧異常 | 電圧モニタ | 60-90% | TSR-013/014 |
| 外部通信故障 | CRC + タイムアウト | 90-99% | — |
| センサ故障 | 範囲チェック + クロスチェック | 60-90% | — |

各メカニズムを TSR に割当て、DC 合計が ASIL B 要求を満たすか検証

### redundancy モード — 冗長化戦略
ASIL B 要件に基づく冗長化設計:

1. **単一チャネル + 自己診断**（基本戦略）
   - SW 診断による故障検出
   - HW WDT によるバックアップ
   - コスト効率が高い

2. **二重チャネル（必要な場合）**
   - 安全遮断パスの独立性確保
   - MCU 故障時もモータ停止可能なHW遮断

3. **共通原因故障(CCF)対策**
   - HW/SW の設計独立性
   - 多様なテスト手法の適用
   - 環境条件への耐性

### ftti モード — フォールトトレラント時間間隔
各安全目標の FTTI（Fault Tolerant Time Interval）を算出:

| SG | 危険事象 | 暴露時間 | FTTI | 診断周期 | 反応時間 | 余裕 |
|----|---------|---------|------|---------|---------|------|
| SG-001 | 過速度→ドラム破損 | 運転中 | 500ms | 10ms | 1ms | 489ms |
| SG-002 | 過電流→発火 | 運転中 | 200ms | 10ms | 1ms | 189ms |
| SG-003 | 蓋開放→接触 | 運転中 | 200ms | 10ms | 1ms | 189ms |

FTTI > 診断周期 + 反応時間 であることを確認

## 出力
```
=== 安全コンセプト設計結果 ===
FSR: {count}件生成（SG {sg_count}件から展開）
TSR: {count}件生成（FSR {fsr_count}件から展開）
安全メカニズム: {count}件割当
DC 達成状況: 全SG で ASIL B 要求達成 {YES/NO}
FTTI 検証: 全SG で余裕あり {YES/NO}
出力ファイル: docs/04_fsc.md, docs/05_tsc.md
```

## 関連スキル
- /fmea — FMEA 結果を FSR/TSR に反映（安全メカニズム割当の根拠）
- /safety-diag — 安全メカニズムに対する診断コード生成
- /srs-generate — TSR から SW安全要求(SR/DR)を導出（下流）
- /safety-verify — 安全コンセプトの検証（Safety Case 構築）

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json 読み取り失敗 | Read エラー | パスと構文を確認。`git checkout -- project.json` で復元 |
| product_spec フィールド欠落 | キー不在 / null | 必要フィールドを報告し設定を促す。TBD マーカー付きで続行可 |
| 既存ファイル上書き競合 | Read で既存ファイル検出 | `cp {file} {file}.bak` → diff 表示 → ユーザー承認後に上書き |
| TBD 未解消（安全クリティカル） | TBD チェック | **生成を中止**。未解消 TBD を報告 |
| docs/03_hara.md 不在 | Read エラー / ファイル未検出 | **fsc/tsc モードを中止**。SG 一覧が取得不可。先に `/execute-phase PH-03` で HARA を実施 |
| FTTI < 診断周期 + 反応時間 | ftti モードで各 SG の時間計算 | 対象 SG と計算値（FTTI, 診断周期, 反応時間）を表示し**生成を中止**。タイミング設計を見直し |
| DC 合計が ASIL B 要求未達 | mechanism モードで DC 集計 | 不足メカニズムと現在 DC を列挙し**中止**。追加メカニズムの検討を促す |
| 既存 docs/04_fsc.md・05_tsc.md との競合 | Read で既存ファイル検出 | 既存内容との diff を提示。ユーザーに上書き/マージ/中止を選択させる |

## 注意事項
- SG→FSR→TSR の展開はトレーサビリティを厳密に維持
- DC 値は IEC 60730 / ISO 26262 の表から引用（根拠を明記）
- FTTI 計算は最悪値を使用（楽観的な見積もり禁止）
- パラメータは必ず project.json product_spec から取得
- FSR/TSR の ID は §7 の命名規則に従う

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/iso26262_iec60730.md` — 安全メカニズムカタログ Table D.2-D.14、DC カテゴリ定義、ASIL 分解ルール
