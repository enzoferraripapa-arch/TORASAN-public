---
name: motor-control
description: "Generates BLDC motor control code with safety mechanisms."
argument-hint: "[mode: generate|analyze|test (default: analyze)]"
disable-model-invocation: true
---

# /motor-control — BLDC モータ制御コード生成

洗濯機用 BLDC モータ制御モジュールのコード生成・解析を行う。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "analyze"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | product_spec の必要フィールドが定義済み | project.json 内を確認 | 必要フィールドを設定してから再実行 |
| 3 | 前段フェーズが完了済み | phases[] の status 確認 | 該当フェーズを先に `/execute-phase` で実行 |
| 4 | 出力先ディレクトリが存在する | `ls` で確認 | 自動作成（mkdir -p） |

## モータ仕様（project.json product_spec.motor）

| パラメータ | 値 | マクロ名 |
|-----------|-----|---------|
| タイプ | BLDC（ブラシレスDC） | — |
| 定格出力 | 1,500W | MOTOR_POWER_W |
| 定格回転数 | 1,200 rpm | MOTOR_RATED_RPM |
| 過速度閾値 | 1,500 rpm | MOTOR_OVERSPEED_RPM |
| 定格電流 | 6A (6,000mA) | MOTOR_RATED_MA |
| 最大電流 | 8A (8,000mA) | MOTOR_MAX_MA |

## モジュール構成

### 1. motor_ctrl — モータ制御コア
```
motor_ctrl.h / motor_ctrl.c
├── MotorCtrl_Init()           — 初期化
├── MotorCtrl_Start()          — 起動シーケンス
├── MotorCtrl_Stop()           — 通常停止
├── MotorCtrl_EmergencyStop()  — 緊急停止（<1ms応答）
├── MotorCtrl_SetSpeed(rpm)    — 速度指令
├── MotorCtrl_GetRpm()         — 現在速度取得（IIRフィルタ付）
├── MotorCtrl_GetDuty()        — 現在Duty取得
├── MotorCtrl_CheckOverspeed() — 過速度判定
└── MotorCtrl_MainTask()       — 周期タスク（10ms）
```

### 2. current_mon — 電流監視
```
current_mon.h / current_mon.c
├── CurrentMon_Init()          — 初期化
├── CurrentMon_GetCurrent_mA() — 電流値取得（ADC→mA変換）
├── CurrentMon_CheckOvercurrent() — 過電流判定（>8A）
└── CurrentMon_MainTask()      — 周期タスク（10ms = 100Hz）
```

ADC→電流変換:
```c
/* shunt抵抗値とアンプゲインに依存（要HW仕様確認） */
/* current_mA = (adc_value * ADC_VREF_MV) / (ADC_MAX_VALUE * SHUNT_R_MOHM * AMP_GAIN) */
```

### 3. lid_mon — 蓋/ドア監視
```
lid_mon.h / lid_mon.c
├── LidMon_Init()              — 初期化
├── LidMon_IsOpen()            — 蓋開放判定（デバウンス付）
├── LidMon_IsLocked()          — ロック状態取得
└── LidMon_MainTask()          — 周期タスク（10ms）
```
- SG-003: 蓋開放検知 → 200ms 以内にモータ停止

### 4. 安全状態遷移
```
[NORMAL] → 過速度/過電流/蓋開放/電圧異常/WDTタイムアウト → [SAFE STATE]

SAFE STATE:
  1. PWM出力 = 0（全相Lo）
  2. モータブレーキ
  3. DEM イベント記録
  4. 再起動ブロック（手動リセット要求）
```

## 生成手順

### generate モード
1. project.json の product_spec.motor からパラメータ取得
2. 上記モジュール構成に従いコード生成
3. §5 型ルール準拠（固定幅整数、float==禁止、Uサフィックス）
4. 安全目標(SG)への紐付けをコメントで明示
5. 出力先: `src/app/` ディレクトリ

### analyze モード
1. src/archive_v1/ の既存コードを読み込み
2. 以下を分析:
   - 安全要求(SR)のカバレッジ
   - 型ルール違反の有無
   - product_spec との数値整合性
   - 安全状態遷移の完全性
3. レポート出力

### test モード
1. 各関数に対してテストケースを生成:
   - 正常系: 定格条件での動作
   - 境界値: 閾値付近（1,499rpm / 1,500rpm / 1,501rpm）
   - 異常系: 過速度、過電流、センサ故障
2. TC-XXX 形式で ID 採番
3. トレーサビリティ: TC → SR → TSR → FSR → SG

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json 読み取り失敗 | Read エラー | パスと構文を確認。`git checkout -- project.json` で復元 |
| product_spec フィールド欠落 | キー不在 / null | 必要フィールドを報告し設定を促す。TBD マーカー付きで続行可 |
| 既存ファイル上書き競合 | Read で既存ファイル検出 | `cp {file} {file}.bak` → diff 表示 → ユーザー承認後に上書き |
| TBD 未解消（安全クリティカル） | TBD チェック | **生成を中止**。未解消 TBD を報告 |
| 過速度閾値 ≦ 定格回転数 | `overspeed_rpm ≦ rated_rpm` | 矛盾する値を表示し**生成を中止**。product_spec.motor の閾値設定を修正後に再実行 |
| analyze 時 archive_v1/ 不在 | `ls src/archive_v1/` で確認 | 解析対象が存在しない旨を報告。generate モードへの切替を提案 |
| EmergencyStop 関数が生成コードに未定義 | Grep で `MotorCtrl_EmergencyStop` の関数定義を検索 | **検証失敗**。EmergencyStop は SG-001〜003 必須。生成ロジックを確認し再生成 |
| SHUNT_R_MOHM / AMP_GAIN 未定義 | product_spec に shunt_r_mohm / amp_gain が不在 | **generate を中止**。電流変換が不正確だと SG-002（過電流 ASIL B）が無効化。HW 仕様確定後に設定 |
| test モード TC-ID 重複 | 生成 TC-ID の重複チェック | 重複 ID を列挙し再採番を実行。既存 docs/ 内 TC との照合も実施 |

## 注意事項
- モータ制御は安全最優先 — EmergencyStop は最短パスで実行
- ADC→物理量変換はオーバーフローに注意（uint16_t 範囲内で計算）
- IIR フィルタの係数は固定小数点で実装（float 最小化）
- 割り込み内でのモータ停止処理は再入可能にする

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/bldc_safety.md` — デュアルパスアーキテクチャ、STO 安全トルクオフ、過電流/過速度保護設計
- `.claude/knowledge/misra_c_2012.md` — MISRA C:2012 準拠コーディングルール

## 関連スキル
- `/driver-gen` — BSW/HAL ドライバコード生成
- `/mcu-config` — MCU ペリフェラル初期化コード生成
- `/safety-diag` — 安全診断機能実装
- `/fmea` — FMEA 故障分析
