---
name: static-analysis
description: "Runs cppcheck, clang-tidy, flawfinder and checks MISRA C:2012 compliance."
argument-hint: "[target: file|directory|all]"
---

# /static-analysis — 静的解析統合実行

cppcheck + clang-tidy + flawfinder を統合実行し、MISRA C:2012 準拠状況を詳細レポートする。

引数: $ARGUMENTS（オプション。"all" | "cppcheck" | "clang-tidy" | "flawfinder" | "misra" | ファイルパス → 省略時は "all"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | 必要な外部ツールがインストール済み | `which {tool}` / `{tool} --version` | エラーメッセージでインストール手順を提示 |
| 2 | 対象ファイルが存在する | Glob/ls で確認 | 対象不在を報告して終了 |

## ツール概要

| ツール | 目的 | 重点チェック |
|-------|------|-----------|
| cppcheck | バグ・未定義動作検出 | MISRA C:2012、メモリリーク、null参照 |
| clang-tidy | コーディングスタイル・モダンC | 型変換、未使用変数、複雑度 |
| flawfinder | セキュリティ脆弱性 | バッファオーバーフロー、危険関数 |
| type_check.sh | §5 TORASAN型ルール | T-01〜T-10 独自ルール |

## 手順

### Step 1: 対象ファイル特定
1. src/ 配下の .c / .h ファイルを列挙
3. 引数でファイル指定がある場合はそのファイルのみ

### Step 2: cppcheck 実行
```bash
cppcheck --enable=all --std=c99 --suppress=missingIncludeSystem \
  --addon=misra.json --template='{file}:{line}:{severity}:{id}:{message}' \
  {target_files}
```
- MISRA C:2012 アドオンが利用可能な場合は有効化
- 利用不可の場合はスキップし手動チェック項目を列挙

### Step 3: clang-tidy 実行
```bash
clang-tidy {target_files} -- -std=c99 \
  -Wconversion -Wsign-conversion -Wall -Wextra -pedantic
```
- 型変換警告を重点チェック（§5 T-10 対応）

### Step 4: flawfinder 実行
```bash
flawfinder --minlevel=2 --columns {target_files}
```
- セキュリティリスクの高い関数呼び出しを検出

### Step 5: §5 型ルールチェック
`scripts/type_check.sh` を実行し、以下の TORASAN 固有ルールを検証:

| ルール | 検出パターン | 重大度 |
|-------|-----------|-------|
| T-01 | bool, true, false, stdbool.h | ERROR |
| T-02 | int, long, short（修飾なし） | ERROR |
| T-03a | float == / float != | ERROR |
| T-03b | double | ERROR |
| T-03c | math.h | WARNING |
| T-04 | bool判定で0/1リテラル | WARNING |
| T-05 | enum { A, B } （値未指定） | ERROR |
| T-06 | union | ERROR |
| T-07 | malloc, calloc, realloc, free | ERROR |
| T-08 | 再帰呼び出し | ERROR |
| T-09 | goto | ERROR |
| T-10 | 暗黙型変換 | WARNING |

### Step 6: 統合レポート

```
=== 静的解析レポート ===
実行日時: {timestamp}
対象ファイル: {count}件

[cppcheck]
  エラー: {count} / 警告: {count} / スタイル: {count}
  MISRA違反: {count}件
  重大な指摘:
    - {file}:{line} {message}

[clang-tidy]
  エラー: {count} / 警告: {count}
  重大な指摘:
    - {file}:{line} {message}

[flawfinder]
  リスクレベル4-5: {count} / レベル2-3: {count}
  重大な指摘:
    - {file}:{line} {message} (risk={level})

[TORASAN 型ルール (§5)]
  PASS: {count} / FAIL: {count}
  違反:
    - T-01: {file}:{line} bool使用
    - T-07: {file}:{line} malloc使用

[総合]
  ERROR: {total_errors}件（修正必須）
  WARNING: {total_warnings}件（修正推奨）
  MISRA C:2012 適合率: {pct}%
```

### Step 7: 自動修正提案
ERROR レベルの指摘について:
- 修正コードの提案
- 「自動修正しますか？」とユーザーに確認

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| ツール未インストール | `which` 失敗 | インストール手順を提示。該当チェックをスキップ |
| ツール実行エラー | 非0終了コード | stderr を報告。対象を分割して再実行を提案 |
| 出力パース失敗 | フォーマット不一致 | 生出力を表示し手動確認を促す |

## 注意事項
- ツールがインストールされていない場合はスキップし、手動チェック項目を提示
- MISRA C:2012 の全ルールカバーは cppcheck アドオン依存
- 偽陽性（false positive）は suppress コメントで管理
- 解析結果は process_records/SUP.1_quality_assurance.md に記録

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/misra_c_2012.md` — mandatory/required ルール一覧、よくある違反パターン、cppcheck 設定例

## 関連スキル
- `/driver-gen` — BSW/HAL ドライバコード生成
- `/sw-design` — SW アーキテクチャ設計
- `/test-design` — テスト設計・テストケース生成
- `/env-check` — 開発環境チェック
