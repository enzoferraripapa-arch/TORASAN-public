---
name: memory-map
description: "Designs and validates memory map for embedded systems with safety partitioning."
argument-hint: "[action: show|analyze|update (default: show)]"
disable-model-invocation: true
---

# /memory-map — メモリマップ管理・リソース計算

RL78/G14 の Flash/RAM メモリレイアウトを管理し、リソース残量を計算する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "show"）

> **数値参照ルール**: メモリ容量・テストパラメータは全て project.json `product_spec.mcu` / `product_spec.ram_test` から取得すること。
> 本スキル内の数値（64KB、5632B、256B 等）は例示値。ハードコード値をそのまま出力に使用しないこと。

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | product_spec の必要フィールドが定義済み | project.json 内を確認 | 必要フィールドを設定してから再実行 |
| 3 | 前段フェーズが完了済み | phases[] の status 確認 | 該当フェーズを先に `/execute-phase` で実行 |
| 4 | 出力先ディレクトリが存在する | `ls` で確認 | 自動作成（mkdir -p） |

## メモリ仕様（project.json product_spec.mcu から取得）

| リソース | product_spec パス | 用途 |
|---------|------------------|------|
| Flash | `mcu.flash_kb` KB | プログラム + CRC格納 + ベクタテーブル |
| RAM | `mcu.ram_b` B (`mcu.ram_kb` KB) | スタック + 静的変数 + March C テストバッファ |

## 手順

### show モード
メモリマップを図示:

```
=== Flash メモリマップ (64KB) ===
0x00000 ┌─────────────────────┐
        │ ベクタテーブル       │ ~256B
0x00100 ├─────────────────────┤
        │ プログラムコード     │ {used}KB
        │ (text + rodata)     │
{addr}  ├─────────────────────┤
        │ 定数テーブル        │ {used}KB
        │ (ADC変換テーブル等)  │
{addr}  ├─────────────────────┤
        │ 空き領域            │ {free}KB ({pct}%)
0xFFFF0 ├─────────────────────┤
        │ CRC32 格納領域      │ 4B
0xFFFFF └─────────────────────┘

=== RAM メモリマップ (5,632B) ===
0x00000 ┌─────────────────────┐
        │ スタック領域         │ {size}B
        ├─────────────────────┤
        │ 静的変数 (.bss)     │ {size}B
        ├─────────────────────┤
        │ 初期値付変数 (.data) │ {size}B
        ├─────────────────────┤
        │ March C テストバッファ│ 256B
        ├─────────────────────┤
        │ 空き領域            │ {free}B ({pct}%)
0x015FF └─────────────────────┘
```

### analyze モード
1. src/ 配下のソースファイルをスキャン
2. 可能であれば `size` コマンド（またはコンパイル出力の .map ファイル）から使用量取得
3. 手動計算:
   - 各 .c ファイルの static 変数を集計（RAM使用量推定）
   - 各モジュールのコードサイズを推定（Flash使用量推定）
4. リソース残量を計算:

```
=== メモリ使用量分析 ===
[Flash 64KB]
  プログラム: {used}KB ({pct}%)
  定数:       {used}KB ({pct}%)
  CRC格納:    4B
  空き:       {free}KB ({pct}%)
  ⚠ 警告ライン: 80% = 51.2KB

[RAM 5,632B]
  スタック:   {used}B ({pct}%)
  .bss:       {used}B ({pct}%)
  .data:      {used}B ({pct}%)
  March C buf: 256B (4.5%)
  空き:       {free}B ({pct}%)
  ⚠ 警告ライン: 85% = 4,787B

[March C テスト]
  ブロックサイズ: 256B
  ブロック数:     22 (= 5,632 / 256)
  全周期:         2.2秒

[リスク評価]
  Flash 使用率 {pct}%: {OK/注意/危険}
  RAM 使用率 {pct}%: {OK/注意/危険}
  スタックオーバーフローリスク: {評価}
```

### update モード
1. 分析結果をもとにメモリマップを更新
2. docs/ に memory_map.md を生成または更新
3. project.json の product_spec に使用量情報があれば更新

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json 読み取り失敗 | Read エラー | パスと構文を確認。`git checkout -- project.json` で復元 |
| product_spec フィールド欠落 | キー不在 / null | 必要フィールドを報告し設定を促す。TBD マーカー付きで続行可 |
| 既存ファイル上書き競合 | Read で既存ファイル検出 | `cp {file} {file}.bak` → diff 表示 → ユーザー承認後に上書き |
| TBD 未解消（安全クリティカル） | TBD チェック | **生成を中止**。未解消 TBD を報告 |
| RAM 使用率 > 85%（警告ライン超過） | analyze モードで静的変数 + スタック + バッファ集計 | 使用量内訳を表示し**警告**。update モードは保留。モジュール別削減候補を提示 |
| Flash 使用率 > 80%（警告ライン超過） | analyze モードでコードサイズ集計 | 使用量内訳を表示し**警告**。コード最適化候補（未使用モジュール等）を提示 |
| March C バッファ + 静的変数 > RAM 総量 | `march_c_buf + bss + data + stack > ram_b` | スタックオーバーフローリスクを報告し**中止**。March C ブロックサイズ縮小 or 変数削減を検討 |
| ROM_CRC32_EXPECTED_ADDR が TBD | product_spec / リンカスクリプトで未確定 | **update モードを中止**。CRC 格納アドレス確定後に再実行。show/analyze は実行可 |

## 注意事項
- RAM 5.5KB は厳しい制約 — 動的メモリ確保は禁止（T-07）
- March C テストは 256B ブロック単位で分割実行（RAM全域をテストするため）
- スタックサイズは最大コールチェーン深さから算出（再帰禁止 T-08 で制約可能）
- Flash の末尾アドレスに CRC32 期待値を格納（ROM_CRC32_EXPECTED_ADDR）

## 関連スキル
- `/mcu-config` — MCU ペリフェラル初期化コード生成
- `/driver-gen` — BSW/HAL ドライバコード生成
- `/safety-diag` — 安全診断機能実装
- `/sw-design` — SW アーキテクチャ設計
