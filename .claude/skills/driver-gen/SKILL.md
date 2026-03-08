---
name: driver-gen
description: "Generates MISRA C:2012 compliant device driver code with safety annotations."
argument-hint: "[module: wdt|adc|gpio|timer|uart|dem|all (default: ask)]"
disable-model-invocation: true
---

# /driver-gen — BSW/HAL ドライバコード生成

プロジェクトの src/bsw/ にドライバモジュールを MISRA C:2012 準拠で生成する。

引数: $ARGUMENTS（モジュール名。上記 argument-hint 参照。省略時はユーザーに確認）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | product_spec の必要フィールドが定義済み | project.json 内を確認 | 必要フィールドを設定してから再実行 |
| 3 | 前段フェーズが完了済み | phases[] の status 確認 | 該当フェーズを先に `/execute-phase` で実行 |
| 4 | 出力先ディレクトリが存在する | `ls` で確認 | 自動作成（mkdir -p） |

## BSW アーキテクチャ

```
┌─────────────────────────────────────┐
│          Application Layer           │
│  (motor_ctrl, current_mon, lid_mon)  │
├─────────────────────────────────────┤
│          BSW Service Layer           │
│  (safety_mgr, dem, os_scheduler)     │
├─────────────────────────────────────┤
│          HAL (HW Abstraction)        │
│  (hal_adc, hal_wdt, hal_gpio, ...)   │
├─────────────────────────────────────┤
│          RL78/G14 Hardware           │
└─────────────────────────────────────┘
```

## 各モジュール仕様

### hal_wdt — ウォッチドッグタイマ
- `HAL_WDT_Init(void)` — HW WDT 初期化（100ms タイムアウト）
- `HAL_WDT_Kick(void)` — カウンタリフレッシュ
- パラメータ: product_spec.wdt

### hal_adc — A/Dコンバータ
- `HAL_ADC_Init(void)` — ADC 初期化（10bit, 5V基準）
- `HAL_ADC_Read(uint8_t channel)` → `uint16_t` — 単発変換読取
- `HAL_ADC_StartContinuous(uint8_t channel)` — 連続変換開始
- `HAL_ADC_GetResult(void)` → `uint16_t` — 最新変換値取得
- パラメータ: product_spec.adc

### hal_gpio — 汎用入出力
- `HAL_GPIO_Init(void)` — 全ピン初期化
- `HAL_GPIO_Write(uint8_t port, uint8_t pin, uint8_t value)` — 出力設定
- `HAL_GPIO_Read(uint8_t port, uint8_t pin)` → `uint8_t` — 入力読取
- ピンアサイン: モータ出力、ホールセンサ入力、蓋検知、LED

### hal_timer — タイマ
- `HAL_Timer_Init(void)` — 1ms ティックタイマ初期化
- `HAL_Timer_GetTick(void)` → `uint32_t` — 経過 ms 取得
- `HAL_Timer_PWM_SetDuty(uint16_t duty_permil)` — PWM Duty 設定（‰単位）
- パラメータ: product_spec.mcu.clock_mhz から分周比計算

### hal_uart — シリアル通信
- `HAL_UART_Init(uint32_t baudrate)` — UART 初期化
- `HAL_UART_Send(const uint8_t* data, uint16_t len)` — 送信
- `HAL_UART_Receive(uint8_t* buf, uint16_t max_len)` → `uint16_t` — 受信

### dem — 診断イベントマネージャ
- `DEM_Init(void)` — 初期化
- `DEM_ReportEvent(uint16_t event_id, uint8_t status)` — イベント報告
- `DEM_GetEventStatus(uint16_t event_id)` → `uint8_t` — 状態取得
- `DEM_ClearEvent(uint16_t event_id)` — イベントクリア
- イベントID: FMEA-ID と対応

## 生成手順

### Step 1: テンプレート適用
各モジュールに対して:
1. ヘッダファイル（.h）: インクルードガード、関数プロトタイプ、マクロ
2. ソースファイル（.c）: 実装、static 変数、内部関数

### Step 2: パラメータ注入
project.json の product_spec から数値を #define として生成

### Step 3: §5 型ルール適用
- 全整数型: uint8_t, uint16_t, uint32_t, int8_t, int16_t, int32_t
- ブール: uint8_t + STD_TRUE(1U) / STD_FALSE(0U)
- リテラル: U サフィックス必須
- キャスト: 全て明示的
- 禁止: bool, double, malloc, goto, union, recursion

### Step 4: 既存ファイル確認・出力

1. 出力先 `src/bsw/{module}/` に既存ファイルがあるか Read で確認
2. 既存ファイルがある場合:
   - バックアップ作成（`cp {file} {file}.bak`）
   - 既存コードとの diff を生成してユーザーに提示
   - ユーザーの明示承認後に Write で上書き
3. 新規の場合: Write ツールで出力

> 参照: error_prevention.md SS6「成果物作成時チェックリスト」EP-B

#### 出力先ディレクトリ構成
```
src/bsw/
├── hal/
│   ├── hal_adc.h / hal_adc.c
│   ├── hal_wdt.h / hal_wdt.c
│   ├── hal_gpio.h / hal_gpio.c
│   ├── hal_timer.h / hal_timer.c
│   └── hal_uart.h / hal_uart.c
├── dem/
│   ├── dem.h / dem.c
└── common/
    └── std_types.h  (STD_TRUE, STD_FALSE, NULL_PTR 等)
```

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json 読み取り失敗 | Read エラー | パスと構文を確認。`git checkout -- project.json` で復元 |
| product_spec フィールド欠落 | キー不在 / null | 必要フィールドを報告し設定を促す。TBD マーカー付きで続行可 |
| 既存ファイル上書き競合 | Read で既存ファイル検出 | `cp {file} {file}.bak` → diff 表示 → ユーザー承認後に上書き |
| TBD 未解消（安全クリティカル） | TBD チェック | **生成を中止**。未解消 TBD を報告 |
| §5 型ルール違反を含むコード生成 | 生成後セルフチェック（bool禁止, 固定幅整数, Uサフィックス, malloc禁止等） | **生成を保留**。違反箇所を列挙し修正後に `/static-analysis` で再検証 |
| ISR_ プレフィックス衝突 | 既存ハンドラ名との Grep 照合 | 衝突ハンドラ名を報告。ユーザーにリネーム方針を確認後に再生成 |
| archive_v1 との API 互換性破壊 | src/archive_v1/ ヘッダと生成コードの関数シグネチャ diff | 差分を提示しユーザーに承認を求める。互換ラッパー追加 or 移行を選択 |

## 注意事項
- src/archive_v1/ の既存ヘッダと API 互換性を保つ
- 各モジュールは他モジュールへの依存を最小化（HAL は HW のみ依存）
- 割り込みハンドラは ISR_ プレフィックスを付与
- RAM 使用量を意識（5,632B 制約）— static 変数のサイズをコメントで明示

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/misra_c_2012.md` — コーディングルール、型アノテーション規約、逸脱許可の記述方法

## 関連スキル
- `/mcu-config` — MCU ペリフェラル初期化コード生成
- `/motor-control` — BLDC モータ制御コード生成
- `/static-analysis` — 静的解析統合実行
- `/sw-design` — SW アーキテクチャ設計
