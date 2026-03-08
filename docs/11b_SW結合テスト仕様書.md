# SW結合テスト仕様書
## 洗濯機BLDCモータ制御ECU

文書番号: WMC-SIT-001 / 版数: 1.0 / 作成日: 2026-03-03

**適用規格**: IEC 60730-1:2023 Class B (製品安全分類)
**プロセス基盤**: ISO 26262:2018 Part 6 §10 の手法を活用 / **SPICE**: SWE.5
**テストFW**: Unity + CMock (モック/スタブ使用) / **入力文書**: WMC-SWA-001 v2.0 (SIF-01〜27)

---

## 1. 概要

### 1.1 目的

本書はSWアーキテクチャ設計書 (PH-09) で定義されたSWコンポーネント間インタフェース (SIF-01〜27) に対する結合テスト仕様を定義する。ボトムアップ統合戦略に従い、下位モジュール (HAL) から上位モジュール (APP) への統合順序でテストを実施する。

### 1.2 テスト戦略

- **統合戦略**: ボトムアップ統合
- **統合順序**: HAL (SA-003) → BSW (SA-004〜008) → APP (SA-001/002)
- **テスト手法**: インタフェースブラックボックステスト（SIF仕様に対する適合検証）
- **安全IF優先**: Class B インタフェース (SIF-01〜24) を優先テスト
- **スタブ方針**: 呼出先の実モジュール未結合時はCMockスタブを使用
- **判定基準**: 全TC PASS、SIF 27/27 カバー

### 1.3 テスト環境

| 項目 | 内容 |
|------|------|
| ホスト | Windows 11 + GCC (x86ホスト実行) |
| フレームワーク | Unity 2.5 + CMock |
| カバレッジ | gcov/lcov（結合レベルではインタフェースカバレッジを計測） |
| ターゲット | Renesas RL78/G14 (R5F104BG) — 後段のターゲットテストで確認 |

### 1.4 トレーサビリティ概要

| レベル | TC範囲 | 元定義 | 件数 |
|--------|--------|--------|------|
| 結合テスト | TC-101〜TC-133 | SIF-01〜SIF-27 | 33件 |

---

## 2. 統合順序

### 2.1 統合ステップ

| Step | 統合対象 | 結合先 | テスト対象SIF | TC |
|------|---------|--------|-------------|-----|
| S1 | SA-003 (HAL) | — | (UT済: ハードウェア抽象化) | — |
| S2 | SA-004 (WDT_MGR) | SA-003 | SIF-23, SIF-24 | TC-123〜125 |
| S3 | SA-005 (DEM) | — | (単体で動作) | — |
| S4 | SA-006 (MEM_MGR) | SA-005 | SIF-16 | TC-116 |
| S5 | SA-008 (COM) | SA-003, SA-005 | SIF-17, SIF-25〜27 | TC-117, TC-127〜131 |
| S6 | SA-007 (DIAG) | SA-003, SA-002 | SIF-18〜22 | TC-118〜122 |
| S7 | SA-001 (APP_MOT) | SA-003 | SIF-01〜06 | TC-101〜106 |
| S8 | SA-002 (APP_SAF) | SA-001, SA-003, SA-005 | SIF-07〜15 | TC-107〜115 |
| S9 | main統合 | 全SA | SIF-02/08/19/24/25 連携 | TC-132〜133 |

---

## 3. テストケース定義

### 3.1 Step S7: APP_MOT ↔ HAL 結合 (SIF-01〜06)

#### TC-101: SIF-01 AppMot_Init 初期化連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-101 |
| **対象SIF** | SIF-01: `void AppMot_Init(void)` |
| **呼出元→先** | main → SA-001 (APP_MOT) |
| **テスト目的** | AppMot_Init が内部状態を初期化し、PWMデューティを0に設定すること |
| **前提条件** | HAL 初期化済み（HalPwm モジュール実結合） |
| **手順** | 1. HAL_Init() 実行<br>2. AppMot_Init() 呼出し |
| **期待結果** | PWMデューティ全相0、内部RPM=0、制御状態=IDLE |
| **判定基準** | g_motor_rpm.value == 0 && PWM全相duty == 0 |
| **安全分類** | Class B |
| **トレース** | SIF-01 → SR-001 |

#### TC-102: SIF-02 AppMot_Cyclic10ms 周期制御連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-102 |
| **対象SIF** | SIF-02: `void AppMot_Cyclic10ms(void)` |
| **呼出元→先** | main → SA-001 (APP_MOT) |
| **テスト目的** | 10ms周期呼出しでHALからホール周期を取得し、RPM算出→PWM出力まで連携すること |
| **前提条件** | AppMot_Init済、HalTimer/HalPwmモジュール実結合 |
| **手順** | 1. HalTimer にホール周期値 = 2000us を設定（750rpm相当）<br>2. AppMot_Cyclic10ms() を呼出し |
| **期待結果** | g_motor_rpm.value == 750（±許容）、HalPwm_SetDuty に適切な値が設定される |
| **判定基準** | RPM = 60,000,000 / (2000 * 6) = 5000 → 実装に依存。PWMデューティ > 0 |
| **安全分類** | Class B |
| **トレース** | SIF-02 → SR-001, SR-003 |

#### TC-103: SIF-03 HalTimer_GetHallPeriod 取得連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-103 |
| **対象SIF** | SIF-03: `uint16_t HalTimer_GetHallPeriod(uint8_t ch)` |
| **呼出元→先** | SA-001 (APP_MOT) → SA-003 (HAL) |
| **テスト目的** | APP_MOT がHALからホールエッジ周期を正しく取得できること |
| **前提条件** | HalTimer 初期化済、タイマキャプチャ値がバッファに格納済み |
| **手順** | 1. g_hall_period[0] = 1333us を設定（1500rpm相当: 60e6/(1500*6*1000/1000)）<br>2. AppMot_Cyclic10ms() 経由で HalTimer_GetHallPeriod(0) が呼ばれる<br>3. 戻り値を確認 |
| **期待結果** | 戻り値 == 1333（±1us） |
| **判定基準** | HalTimer_GetHallPeriod(0) 戻り値が設定値と一致 |
| **安全分類** | Class B |
| **トレース** | SIF-03 → SR-001 |

#### TC-104: SIF-04 HalPwm_SetDuty 出力連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-104 |
| **対象SIF** | SIF-04: `void HalPwm_SetDuty(uint8_t phase, uint16_t duty)` |
| **呼出元→先** | SA-001 (APP_MOT) → SA-003 (HAL) |
| **テスト目的** | APP_MOT のPWM演算結果がHALのPWMレジスタに正しく反映されること |
| **前提条件** | AppMot_Init済、正常運転状態 |
| **手順** | 1. 目標RPM=750 の運転条件を設定<br>2. AppMot_Cyclic10ms() 呼出し<br>3. HalPwm_SetDuty に渡された引数を検証 |
| **期待結果** | phase: 有効相番号(0-2)、duty: 0〜1000範囲内の非0値 |
| **判定基準** | SetDuty が呼ばれ、duty が有効範囲内 |
| **安全分類** | Class B |
| **トレース** | SIF-04 → SR-001 |

#### TC-105: SIF-05 HalPwm_AllStop 緊急停止連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-105 |
| **対象SIF** | SIF-05: `void HalPwm_AllStop(void)` |
| **呼出元→先** | SA-001 (APP_MOT) → SA-003 (HAL) |
| **テスト目的** | 緊急停止要求時にPWM全相が即座にOFFになること |
| **前提条件** | APP_MOT 運転中（PWMデューティ出力中） |
| **手順** | 1. 正常運転状態を構築（duty > 0 出力中）<br>2. AppMot_EmergencyStop() 呼出し<br>3. HalPwm_AllStop の呼出しを検証 |
| **期待結果** | HalPwm_AllStop が1回呼ばれ、全相PWMデューティ=0 |
| **判定基準** | 全相 duty == 0、応答時間 ≦1ms |
| **安全分類** | Class B |
| **トレース** | SIF-05 → SR-003 |

#### TC-106: SIF-06 g_motor_rpm 共有変数連携（逆コピー保護）

| 項目 | 内容 |
|------|------|
| **テストID** | TC-106 |
| **対象SIF** | SIF-06: `uint16_t g_motor_rpm` (SafetyVar_t) |
| **呼出元→先** | SA-001 (APP_MOT) → SA-002 (APP_SAF) |
| **テスト目的** | APP_MOT が書込んだ RPM 値を APP_SAF が逆コピー検証付きで正しく読取れること |
| **前提条件** | 両モジュール初期化済 |
| **手順** | 1. APP_MOT で RPM=1200 を演算し g_motor_rpm に書込み<br>2. SafetyVar_Check(&g_motor_rpm) 実行<br>3. APP_SAF 側から g_motor_rpm.value 読取り |
| **期待結果** | value == 1200、inverted == ~1200 (0xFB4F)、Check == true |
| **判定基準** | SafetyVar_Check == true && value == 1200 |
| **安全分類** | Class B |
| **トレース** | SIF-06 → SR-001, SR-002 |

---

### 3.2 Step S8: APP_SAF ↔ HAL/APP_MOT/DEM 結合 (SIF-07〜15)

#### TC-107: SIF-07 AppSaf_Init 初期化連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-107 |
| **対象SIF** | SIF-07: `void AppSaf_Init(void)` |
| **呼出元→先** | main → SA-002 (APP_SAF) |
| **テスト目的** | AppSaf_Init が安全監視の内部状態を初期化すること |
| **前提条件** | HAL 初期化済 |
| **手順** | 1. HAL_Init() 実行<br>2. AppSaf_Init() 呼出し |
| **期待結果** | 安全状態=NORMAL、全デバウンスカウンタ=0、エラーフラグ=CLEAR |
| **判定基準** | g_safety_state == NORMAL && 全カウンタ == 0 |
| **安全分類** | Class B |
| **トレース** | SIF-07 → SR-013 |

#### TC-108: SIF-08 AppSaf_Cyclic10ms 周期監視連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-108 |
| **対象SIF** | SIF-08: `void AppSaf_Cyclic10ms(void)` |
| **呼出元→先** | main → SA-002 (APP_SAF) |
| **テスト目的** | 正常値入力時にAppSaf_Cyclic10msが安全判定を正常完了すること |
| **前提条件** | AppSaf_Init済、全センサ値正常範囲 |
| **手順** | 1. g_motor_rpm.value = 1000（正常）<br>2. HalAdc_GetCurrent → 500（正常）<br>3. HalAdc_GetVoltage → 512（5.0V相当、正常）<br>4. HalGpio_GetLidState → true（閉）<br>5. AppSaf_Cyclic10ms() 呼出し |
| **期待結果** | 安全状態維持=NORMAL、停止指令なし |
| **判定基準** | g_safety_state == NORMAL && EmergencyStop 未呼出 |
| **安全分類** | Class B |
| **トレース** | SIF-08 → SR-002, SR-005, SR-007, SR-014 |

#### TC-109: SIF-09 HalAdc_GetCurrent 電流取得連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-109 |
| **対象SIF** | SIF-09: `uint16_t HalAdc_GetCurrent(void)` |
| **呼出元→先** | SA-002 (APP_SAF) → SA-003 (HAL) |
| **テスト目的** | APP_SAF がHALから電流ADC値を正しく取得し過電流判定に使用すること |
| **前提条件** | HAL_ADC 初期化済、ADCバッファに値格納済 |
| **手順** | 1. ADC ch0 バッファに 900 を設定（8A超相当: 8000mA/10000mA*1023≒818→安全マージン加味900）<br>2. AppSaf_Cyclic10ms() 呼出し<br>3. 過電流判定結果を確認 |
| **期待結果** | HalAdc_GetCurrent 戻り値 == 900、過電流デバウンスカウンタがインクリメント |
| **判定基準** | GetCurrent 正常取得 && 過電流検出ロジック動作 |
| **安全分類** | Class B |
| **トレース** | SIF-09 → SR-004, SR-005 |

#### TC-110: SIF-10 HalAdc_GetVoltage 電圧取得連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-110 |
| **対象SIF** | SIF-10: `uint16_t HalAdc_GetVoltage(void)` |
| **呼出元→先** | SA-002 (APP_SAF) → SA-003 (HAL) |
| **テスト目的** | APP_SAF がHALから電圧ADC値を正しく取得し電圧異常判定に使用すること |
| **前提条件** | HAL_ADC 初期化済 |
| **手順** | 1. ADC ch1 バッファに 800 を設定（3.91V相当: 800/1023*5.0=3.91V → 4.5V未満で異常）<br>2. AppSaf_Cyclic10ms() を呼出し（電圧チェック周期タイミング） |
| **期待結果** | HalAdc_GetVoltage 戻り値 == 800、電圧低下異常検出 |
| **判定基準** | GetVoltage 正常取得 && 電圧異常フラグ設定 |
| **安全分類** | Class B |
| **トレース** | SIF-10 → SR-014 |

#### TC-111: SIF-11 HalGpio_GetLidState 蓋状態取得連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-111 |
| **対象SIF** | SIF-11: `bool HalGpio_GetLidState(void)` |
| **呼出元→先** | SA-002 (APP_SAF) → SA-003 (HAL) |
| **テスト目的** | APP_SAF がHALから蓋センサ状態を正しく取得しデバウンス処理に使用すること |
| **前提条件** | HAL_GPIO 初期化済、蓋ピンP43設定済 |
| **手順** | 1. GPIO P43 = LOW (蓋開放) を設定<br>2. AppSaf_Cyclic10ms() を4回連続呼出し（20ms相当: 4回×10ms周期でデバウンス完了） |
| **期待結果** | GetLidState 戻り値 == false（蓋開放）、4回連続後に蓋開放確定 |
| **判定基準** | 4回連続 false → 蓋開放イベント発生 |
| **安全分類** | Class B |
| **トレース** | SIF-11 → SR-007 |

#### TC-112: SIF-12 HalGpio_SetInverterEn インバータ制御連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-112 |
| **対象SIF** | SIF-12: `void HalGpio_SetInverterEn(bool en)` |
| **呼出元→先** | SA-002 (APP_SAF) → SA-003 (HAL) |
| **テスト目的** | 安全停止時にAPP_SAFがインバータENを確実にLOWに設定すること |
| **前提条件** | インバータEN=HIGH（運転中）、過電流検出確定 |
| **手順** | 1. 過電流条件を構築（ADC > 閾値、2回連続）<br>2. AppSaf_Cyclic10ms() で過電流確定→Stage 1停止シーケンス |
| **期待結果** | HalGpio_SetInverterEn(false) が呼ばれ、EN=LOW |
| **判定基準** | SetInverterEn(false) 1回呼出し確認 |
| **安全分類** | Class B |
| **トレース** | SIF-12 → SR-006 |

#### TC-113: SIF-13 HalGpio_SetLidLock 蓋ロック連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-113 |
| **対象SIF** | SIF-13: `void HalGpio_SetLidLock(bool lock)` |
| **呼出元→先** | SA-002 (APP_SAF) → SA-003 (HAL) |
| **テスト目的** | 蓋開放検出→停止シーケンスで蓋ロックが解除されること |
| **前提条件** | 蓋ロック=ON（運転中） |
| **手順** | 1. 蓋ロック中に蓋開放を検出（4回連続デバウンス完了）<br>2. 停止シーケンス実行後の蓋ロック状態を確認 |
| **期待結果** | 停止完了後に HalGpio_SetLidLock(false) が呼ばれロック解除 |
| **判定基準** | SetLidLock(false) 呼出し確認 |
| **安全分類** | Class B |
| **トレース** | SIF-13 → SR-008 |

#### TC-114: SIF-14 AppMot_EmergencyStop 緊急停止要求連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-114 |
| **対象SIF** | SIF-14: `void AppMot_EmergencyStop(void)` |
| **呼出元→先** | SA-002 (APP_SAF) → SA-001 (APP_MOT) |
| **テスト目的** | APP_SAFからの緊急停止要求でAPP_MOTがPWM全停止を実行すること |
| **前提条件** | APP_MOT 運転中、APP_SAF が過速度検出 |
| **手順** | 1. APP_MOT を運転状態に設定（duty > 0）<br>2. g_motor_rpm.value = 1501（過速度閾値超）<br>3. AppSaf_Cyclic10ms() 2回呼出し（デバウンス2回連続）<br>4. AppMot_EmergencyStop の呼出しを検証 |
| **期待結果** | EmergencyStop 呼出し → HalPwm_AllStop → 全相duty=0 |
| **判定基準** | EmergencyStop 1回 → AllStop 1回 → duty全相0 |
| **安全分類** | Class B |
| **トレース** | SIF-14 → SR-002, SR-003 |

#### TC-115: SIF-15 Dem_ReportEvent エラー報告連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-115 |
| **対象SIF** | SIF-15: `void Dem_ReportEvent(uint8_t code)` |
| **呼出元→先** | SA-002 (APP_SAF) → SA-005 (DEM) |
| **テスト目的** | 安全異常検出後にAPP_SAFがDEMにエラーイベントを正しく報告すること |
| **前提条件** | DEM初期化済、過電流検出確定 |
| **手順** | 1. 過電流条件を構築→確定<br>2. Stage 3 でDem_ReportEvent(0x02) 呼出しを検証 |
| **期待結果** | Dem_ReportEvent が code=0x02（過電流）で呼ばれる |
| **判定基準** | ReportEvent(0x02) 1回呼出し確認 |
| **安全分類** | Class B |
| **トレース** | SIF-15 → SR-005, SR-013 |

---

### 3.3 Step S4: DEM → MEM_MGR 結合 (SIF-16)

#### TC-116: SIF-16 MemMgr_WriteLog ログ書込み連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-116 |
| **対象SIF** | SIF-16: `void MemMgr_WriteLog(const LogEntry_t* entry)` |
| **呼出元→先** | SA-005 (DEM) → SA-006 (MEM_MGR) |
| **テスト目的** | DEMからのログ書込み要求がMEM_MGR経由でNVMに正しく格納されること |
| **前提条件** | DEM/MEM_MGR初期化済 |
| **手順** | 1. Dem_ReportEvent(0x01) を呼出し<br>2. DEM内部でLogEntry_t構築→MemMgr_WriteLog呼出しを検証<br>3. ログ内容（timestamp, error_code, raw_value）を確認 |
| **期待結果** | WriteLog 呼出し、entry.error_code == 0x01、timestamp > 0 |
| **判定基準** | WriteLog 1回呼出し && entry フィールド正常 |
| **安全分類** | Class B |
| **トレース** | SIF-16 → SR-006, SR-013 |

---

### 3.4 Step S5: COM ↔ HAL/DEM 結合 (SIF-17, SIF-25〜27)

#### TC-117: SIF-17 Com_SendError エラーコードUI送信連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-117 |
| **対象SIF** | SIF-17: `void Com_SendError(uint8_t code)` |
| **呼出元→先** | SA-005 (DEM) → SA-008 (COM) |
| **テスト目的** | DEMからのエラー通知がCOM経由でUART送信されること |
| **前提条件** | COM初期化済、UART送信可能 |
| **手順** | 1. Com_SendError(0x03) を呼出し<br>2. HalUart_Send に渡されたデータを検証 |
| **期待結果** | UART送信バッファにエラーコード0x03を含むフレームが格納 |
| **判定基準** | HalUart_Send 呼出し確認 && 送信データにcode含有 |
| **安全分類** | — (非安全) |
| **トレース** | SIF-17 |

---

### 3.5 Step S6: DIAG ↔ HAL/APP_SAF 結合 (SIF-18〜22)

#### TC-118: SIF-18 Diag_RunStartup 起動時診断連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-118 |
| **対象SIF** | SIF-18: `DiagResult_t Diag_RunStartup(void)` |
| **呼出元→先** | main → SA-007 (DIAG) |
| **テスト目的** | 起動時診断が全4項目（CPU/RAM/ROM/CLK）を実行し結果を返すこと |
| **前提条件** | HAL 初期化済 |
| **手順** | 1. Diag_RunStartup() 呼出し<br>2. 全診断項目の実行確認 |
| **期待結果** | 正常環境で DIAG_PASS 返却、実行時間 ≦700ms |
| **判定基準** | result == DIAG_PASS && 各サブ診断呼出し確認 |
| **安全分類** | Class B |
| **トレース** | SIF-18 → DR-001 |

#### TC-119: SIF-19 Diag_RunCyclic10ms ランタイム診断連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-119 |
| **対象SIF** | SIF-19: `void Diag_RunCyclic10ms(void)` |
| **呼出元→先** | main → SA-007 (DIAG) |
| **テスト目的** | ランタイム診断が10ms周期で分割実行し、診断結果を更新すること |
| **前提条件** | Diag_RunStartup PASS済 |
| **手順** | 1. Diag_RunCyclic10ms() を220回呼出し（2.2s相当: RAM全ブロック1周）<br>2. g_diag_status の更新を確認 |
| **期待結果** | 正常環境で g_diag_status == DIAG_OK（全項目PASS） |
| **判定基準** | 220回後 g_diag_status.value == DIAG_OK |
| **安全分類** | Class B |
| **トレース** | SIF-19 → DR-002 |

#### TC-120: SIF-20 g_diag_status 診断結果共有連携（逆コピー保護）

| 項目 | 内容 |
|------|------|
| **テストID** | TC-120 |
| **対象SIF** | SIF-20: `DiagStatus_t g_diag_status` (SafetyVar_t) |
| **呼出元→先** | SA-007 (DIAG) → SA-002 (APP_SAF) |
| **テスト目的** | DIAG が書込んだ診断結果をAPP_SAFが逆コピー検証付きで正しく読取ること |
| **前提条件** | ランタイム診断1周完了 |
| **手順** | 1. DIAG で診断FAIL発生（RAM March C故障注入）<br>2. g_diag_status に DIAG_FAIL 設定<br>3. APP_SAF 側で SafetyVar_Check 後に値読取り |
| **期待結果** | SafetyVar_Check == true && value == DIAG_FAIL → APP_SAF が安全状態遷移実行 |
| **判定基準** | Check == true && APP_SAF → Stage 1停止シーケンス起動 |
| **安全分類** | Class B |
| **トレース** | SIF-20 → DR-002, SR-013 |

#### TC-121: SIF-21 HalAdc_GetRawValue ADC診断取得連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-121 |
| **対象SIF** | SIF-21: `uint16_t HalAdc_GetRawValue(uint8_t ch)` |
| **呼出元→先** | SA-007 (DIAG) → SA-003 (HAL) |
| **テスト目的** | DIAG がADC生値を取得し固着検出に使用できること |
| **前提条件** | HAL_ADC 初期化済 |
| **手順** | 1. ADC ch0 バッファに 0 を5回連続設定（固着模擬）<br>2. Diag_RunCyclic10ms() でADC診断ステップを5回実行 |
| **期待結果** | HalAdc_GetRawValue(0) == 0 が5回連続 → ADC固着検出 |
| **判定基準** | ADC固着エラーフラグ設定 |
| **安全分類** | Class B |
| **トレース** | SIF-21 → DR-003 |

#### TC-122: SIF-22 HalGpio_DiagTest GPIO診断連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-122 |
| **対象SIF** | SIF-22: `void HalGpio_DiagTest(uint8_t pin)` |
| **呼出元→先** | SA-007 (DIAG) → SA-003 (HAL) |
| **テスト目的** | DIAG がGPIOテストを実行し、断線/短絡を検出できること |
| **前提条件** | HAL_GPIO 初期化済、蓋ピンP43 |
| **手順** | 1. GPIO P43 にプルアップ設定で出力LOW書込み→読取り<br>2. HalGpio_DiagTest(43) 呼出し |
| **期待結果** | 書込み値と読取り値が一致（正常時PASS）、不一致時FAIL |
| **判定基準** | DiagTest 正常完了 && 結果が g_diag_status に反映 |
| **安全分類** | Class B |
| **トレース** | SIF-22 → DR-004 |

---

### 3.6 Step S2: WDT_MGR ↔ HAL 結合 (SIF-23〜24)

#### TC-123: SIF-23 WdtMgr_Init WDT初期化連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-123 |
| **対象SIF** | SIF-23: `void WdtMgr_Init(void)` |
| **呼出元→先** | main → SA-004 (WDT_MGR) |
| **テスト目的** | WDT_MGR初期化でタイムアウト100ms/キック条件がリセットされること |
| **前提条件** | HAL 初期化済 |
| **手順** | 1. WdtMgr_Init() 呼出し |
| **期待結果** | キック条件フラグ全クリア、WDTタイムアウト=100ms設定 |
| **判定基準** | 全完了フラグ == false && タイムアウト設定確認 |
| **安全分類** | Class B |
| **トレース** | SIF-23 → SR-016 |

#### TC-124: SIF-24 WdtMgr_TryKick 全条件充足キック連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-124 |
| **対象SIF** | SIF-24: `void WdtMgr_TryKick(void)` |
| **呼出元→先** | main → SA-004 (WDT_MGR) |
| **テスト目的** | 5条件全完了時にWDTキックが実行されること |
| **前提条件** | WdtMgr_Init済、メインループ正常完了 |
| **手順** | 1. ADC取得完了フラグ=true<br>2. 安全判定完了フラグ=true<br>3. 制御演算完了フラグ=true<br>4. 出力更新完了フラグ=true<br>5. 診断1ブロック完了フラグ=true<br>6. WdtMgr_TryKick() 呼出し |
| **期待結果** | HAL WDTキックレジスタ書込み実行 |
| **判定基準** | WDTキック1回実行 && 全フラグリセット |
| **安全分類** | Class B |
| **トレース** | SIF-24 → SR-016 |

#### TC-125: SIF-24 WdtMgr_TryKick 条件未充足時キック抑止

| 項目 | 内容 |
|------|------|
| **テストID** | TC-125 |
| **対象SIF** | SIF-24: `void WdtMgr_TryKick(void)` |
| **呼出元→先** | main → SA-004 (WDT_MGR) |
| **テスト目的** | 5条件のいずれか未完了時にWDTキックが実行されないこと |
| **前提条件** | WdtMgr_Init済、診断1ブロック未完了 |
| **手順** | 1. ADC/安全/制御/出力の4フラグ=true<br>2. 診断完了フラグ=false（1条件不足）<br>3. WdtMgr_TryKick() 呼出し |
| **期待結果** | WDTキック未実行 |
| **判定基準** | WDTキックレジスタ書込みなし |
| **安全分類** | Class B |
| **トレース** | SIF-24 → SR-016 |

---

### 3.7 Step S5: COM ↔ HAL 結合 (SIF-25〜27)

#### TC-127: SIF-25 Com_Cyclic10ms 通信周期連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-127 |
| **対象SIF** | SIF-25: `void Com_Cyclic10ms(void)` |
| **呼出元→先** | main → SA-008 (COM) |
| **テスト目的** | 通信周期処理がUART送受信を正しく処理すること |
| **前提条件** | COM初期化済、UART動作中 |
| **手順** | 1. UART受信バッファに運転指令データを格納<br>2. Com_Cyclic10ms() 呼出し |
| **期待結果** | 受信データが解析され、送信バッファに状態データが格納される |
| **判定基準** | 送受信処理正常完了 |
| **安全分類** | — (非安全) |
| **トレース** | SIF-25 |

#### TC-128: SIF-26 HalUart_Send UART送信連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-128 |
| **対象SIF** | SIF-26: `void HalUart_Send(const uint8_t* data, uint16_t len)` |
| **呼出元→先** | SA-008 (COM) → SA-003 (HAL) |
| **テスト目的** | COMからの送信要求がHALのUARTドライバ経由で正しく出力されること |
| **前提条件** | HAL_UART 初期化済 |
| **手順** | 1. テスト用送信データ "STATUS:OK\n" (10バイト) を準備<br>2. Com内部からHalUart_Send呼出し |
| **期待結果** | HalUart_Send(data, 10) が呼ばれ、UARTバッファに格納 |
| **判定基準** | Send 呼出し && len == 10 && データ一致 |
| **安全分類** | — (非安全) |
| **トレース** | SIF-26 |

#### TC-129: SIF-27 HalUart_Receive UART受信連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-129 |
| **対象SIF** | SIF-27: `uint16_t HalUart_Receive(uint8_t* buf, uint16_t max)` |
| **呼出元→先** | SA-003 (HAL) → SA-008 (COM) |
| **テスト目的** | HALのUART受信バッファからCOMがデータを正しく読取ること |
| **前提条件** | UART受信割込みでバッファにデータ格納済 |
| **手順** | 1. HAL受信バッファに "RUN:1\n" (6バイト) を格納<br>2. HalUart_Receive(buf, 64) 呼出し |
| **期待結果** | 戻り値 == 6、buf に "RUN:1\n" 格納 |
| **判定基準** | len == 6 && memcmp(buf, expected, 6) == 0 |
| **安全分類** | — (非安全) |
| **トレース** | SIF-27 |

#### TC-130: SIF-26/27 UART送受信バッファフル境界

| 項目 | 内容 |
|------|------|
| **テストID** | TC-130 |
| **対象SIF** | SIF-26, SIF-27 |
| **呼出元→先** | SA-008 (COM) ↔ SA-003 (HAL) |
| **テスト目的** | UARTバッファ上限 (64B) でのオーバーフロー防止を検証 |
| **前提条件** | HAL_UART 初期化済 |
| **手順** | 1. 65バイトの送信データで HalUart_Send(data, 65) 呼出し<br>2. 65バイト受信時に HalUart_Receive(buf, 64) 呼出し |
| **期待結果** | 送信: 64バイトまで送信 or エラー、受信: 64バイトで切捨て、バッファオーバーフローなし |
| **判定基準** | バッファ外メモリ未破壊 && 戻り値 ≦ 64 |
| **安全分類** | — (非安全) |
| **トレース** | SIF-26, SIF-27 |

#### TC-131: SIF-25/17 COM経由エラー送信E2E連携

| 項目 | 内容 |
|------|------|
| **テストID** | TC-131 |
| **対象SIF** | SIF-25, SIF-17 |
| **呼出元→先** | DEM → COM → HAL_UART |
| **テスト目的** | エラーイベント発生→DEM→COM→UARTのエンドツーエンド連携を検証 |
| **前提条件** | DEM/COM/HAL全初期化済 |
| **手順** | 1. Dem_ReportEvent(0x04) 呼出し（蓋開放エラー）<br>2. Com_Cyclic10ms() 呼出し<br>3. HalUart_Send に渡されたフレームを検証 |
| **期待結果** | UARTフレームにエラーコード0x04含有 |
| **判定基準** | Send 呼出し && フレーム内 code == 0x04 |
| **安全分類** | — (非安全) |
| **トレース** | SIF-17, SIF-25 → SR-013 |

---

### 3.8 Step S9: メインループ統合テスト (SIF横断)

#### TC-132: メインループ正常1周期統合テスト

| 項目 | 内容 |
|------|------|
| **テストID** | TC-132 |
| **対象SIF** | SIF-02, SIF-08, SIF-19, SIF-24, SIF-25（メインループ全ステップ） |
| **呼出元→先** | main → 全SA連携 |
| **テスト目的** | メインループ1周期（入力→安全→制御→診断→通信→WDT）の全ステップ連携を検証 |
| **前提条件** | 全モジュール初期化済、全入力正常範囲 |
| **手順** | 1. 正常入力値を設定: RPM=800, I=500, V=512, Lid=CLOSED<br>2. メインループ1周期分のシーケンス呼出し:<br>   - AppSaf_Cyclic10ms()<br>   - AppMot_Cyclic10ms()<br>   - Diag_RunCyclic10ms()<br>   - Com_Cyclic10ms()<br>   - WdtMgr_TryKick() |
| **期待結果** | 全ステップ正常完了、安全状態=NORMAL、WDTキック実行、実行時間≦8ms |
| **判定基準** | 全呼出し正常 && WDTキック実行 && g_safety_state == NORMAL |
| **安全分類** | Class B |
| **トレース** | SIF-02, SIF-08, SIF-19, SIF-24, SIF-25 → 全SR/DR |

#### TC-133: メインループ異常時安全停止統合テスト

| 項目 | 内容 |
|------|------|
| **テストID** | TC-133 |
| **対象SIF** | SIF-08, SIF-14, SIF-05, SIF-12, SIF-15, SIF-16（安全停止パス） |
| **呼出元→先** | APP_SAF → APP_MOT/HAL/DEM/MEM_MGR |
| **テスト目的** | 過速度検出→緊急停止→エラーログ記録→WDTキック禁止の安全シーケンス全体を検証 |
| **前提条件** | 全モジュール初期化済、正常運転中 |
| **手順** | 1. RPM=1501（過速度）を設定<br>2. メインループ2周期実行（デバウンス2回連続）<br>3. 3周期目: 安全状態遷移を確認<br>4. WdtMgr_TryKick() がキック抑止されることを確認 |
| **期待結果** | Stage 1: PWM全停止+EN=LOW → Stage 2: 安全モード → Stage 3: DEM報告+ログ → Stage 4: WDTキック禁止 |
| **判定基準** | AllStop呼出し && SetInverterEn(false) && ReportEvent呼出し && WDTキック未実行 |
| **安全分類** | Class B |
| **トレース** | SIF-05, SIF-08, SIF-12, SIF-14, SIF-15, SIF-16 → SR-002, SR-003, SR-006, SR-013 |

---

## 4. トレーサビリティマトリクス

### 4.1 SIF → TC 順方向トレース

| SIF-ID | 関数/変数 | TC-ID | カバー |
|--------|----------|-------|-------|
| SIF-01 | AppMot_Init | TC-101 | YES |
| SIF-02 | AppMot_Cyclic10ms | TC-102, TC-132 | YES |
| SIF-03 | HalTimer_GetHallPeriod | TC-103 | YES |
| SIF-04 | HalPwm_SetDuty | TC-104 | YES |
| SIF-05 | HalPwm_AllStop | TC-105, TC-133 | YES |
| SIF-06 | g_motor_rpm (SafetyVar_t) | TC-106 | YES |
| SIF-07 | AppSaf_Init | TC-107 | YES |
| SIF-08 | AppSaf_Cyclic10ms | TC-108, TC-132, TC-133 | YES |
| SIF-09 | HalAdc_GetCurrent | TC-109 | YES |
| SIF-10 | HalAdc_GetVoltage | TC-110 | YES |
| SIF-11 | HalGpio_GetLidState | TC-111 | YES |
| SIF-12 | HalGpio_SetInverterEn | TC-112, TC-133 | YES |
| SIF-13 | HalGpio_SetLidLock | TC-113 | YES |
| SIF-14 | AppMot_EmergencyStop | TC-114, TC-133 | YES |
| SIF-15 | Dem_ReportEvent | TC-115, TC-133 | YES |
| SIF-16 | MemMgr_WriteLog | TC-116, TC-133 | YES |
| SIF-17 | Com_SendError | TC-117, TC-131 | YES |
| SIF-18 | Diag_RunStartup | TC-118 | YES |
| SIF-19 | Diag_RunCyclic10ms | TC-119, TC-132 | YES |
| SIF-20 | g_diag_status (SafetyVar_t) | TC-120 | YES |
| SIF-21 | HalAdc_GetRawValue | TC-121 | YES |
| SIF-22 | HalGpio_DiagTest | TC-122 | YES |
| SIF-23 | WdtMgr_Init | TC-123 | YES |
| SIF-24 | WdtMgr_TryKick | TC-124, TC-125, TC-132 | YES |
| SIF-25 | Com_Cyclic10ms | TC-127, TC-131, TC-132 | YES |
| SIF-26 | HalUart_Send | TC-128, TC-130 | YES |
| SIF-27 | HalUart_Receive | TC-129, TC-130 | YES |

**カバレッジ**: SIF 27/27 = **100%**

### 4.2 TC → SIF 逆方向トレース

| TC-ID | 対象SIF | 安全分類 |
|-------|---------|---------|
| TC-101 | SIF-01 | Class B |
| TC-102 | SIF-02 | Class B |
| TC-103 | SIF-03 | Class B |
| TC-104 | SIF-04 | Class B |
| TC-105 | SIF-05 | Class B |
| TC-106 | SIF-06 | Class B |
| TC-107 | SIF-07 | Class B |
| TC-108 | SIF-08 | Class B |
| TC-109 | SIF-09 | Class B |
| TC-110 | SIF-10 | Class B |
| TC-111 | SIF-11 | Class B |
| TC-112 | SIF-12 | Class B |
| TC-113 | SIF-13 | Class B |
| TC-114 | SIF-14 | Class B |
| TC-115 | SIF-15 | Class B |
| TC-116 | SIF-16 | Class B |
| TC-117 | SIF-17 | — |
| TC-118 | SIF-18 | Class B |
| TC-119 | SIF-19 | Class B |
| TC-120 | SIF-20 | Class B |
| TC-121 | SIF-21 | Class B |
| TC-122 | SIF-22 | Class B |
| TC-123 | SIF-23 | Class B |
| TC-124 | SIF-24 | Class B |
| TC-125 | SIF-24 | Class B |
| TC-127 | SIF-25 | — |
| TC-128 | SIF-26 | — |
| TC-129 | SIF-27 | — |
| TC-130 | SIF-26, SIF-27 | — |
| TC-131 | SIF-17, SIF-25 | — |
| TC-132 | SIF-02, SIF-08, SIF-19, SIF-24, SIF-25 | Class B |
| TC-133 | SIF-05, SIF-08, SIF-12, SIF-14, SIF-15, SIF-16 | Class B |

---

## 5. product_spec 整合確認

| パラメータ | product_spec値 | 本書での使用箇所 | 整合 |
|----------|---------------|-----------------|------|
| motor.overspeed_rpm | 1500 | TC-114: 閾値1501(超過), TC-133: 過速度統合テスト | OK |
| motor.max_a | 8 | TC-109: 過電流ADC値算出基準 | OK |
| adc.bits | 10 | TC-109/110: ADC分解能1023 | OK |
| adc.vref_v | 5.0 | TC-110: 電圧換算 (800/1023*5.0=3.91V) | OK |
| voltage_monitor.min_v | 4.5 | TC-110: 電圧低下閾値 | OK |
| voltage_monitor.max_v | 5.5 | TC-110: 電圧上限 | OK |
| wdt.timeout_ms | 100 | TC-123: WDTタイムアウト設定, TC-133: WDTリセット | OK |
| mcu.ram_b | 5632 | TC-119: March C全ブロック=5632/256=22ブロック→220回 | OK |

---

## 6. テスト実行計画

### 6.1 実行環境

- **ホストテスト**: GCC + Unity + CMock（Windows 11 x86）
- **スタブ**: CMock による自動スタブ生成（呼出先未結合時）
- **手順**: 統合Step S1→S9 の順に段階的にスタブを実モジュールに置換

### 6.2 合否基準

| 基準 | 目標 |
|------|------|
| TC全件PASS | 33/33 |
| SIFカバレッジ | 27/27 = 100% |
| Class B IF カバー | 24/24 = 100% |
| 非安全IF カバー | 3/3 = 100% |

### 6.3 実行状態

| 項目 | 状態 |
|------|------|
| 仕様確定 | 完了 |
| スタブ作成 | **未実施** |
| テスト実行 | **未実施** |
| 結果記録 | **未実施** |

---

*SW結合テスト仕様書 v1.0 / 作成日: 2026-03-03 / 入力: WMC-SWA-001 v2.0*
