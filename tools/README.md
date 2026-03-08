# TORASAN ツール一覧

文書生成・検証・トレーサビリティ管理のためのスクリプト群。

## 前提条件

```bash
npm install -g docx exceljs
```

## ファイル一覧

| ファイル | 用途 | 出力先 |
|---------|------|--------|
| `gen_system_spec.js` | TORASAN_システム仕様書.docx 生成 | `projects/{id}/output/` |
| `gen_process_spec.js` | PROCESS_仕様書.docx 生成 | `projects/{id}/output/` |
| `gen_ingest_spec.js` | INGEST_仕様書.docx 生成 | `projects/{id}/output/` |
| `gen_trace_evidence.js` | トレーサビリティエビデンス.docx 生成 | `projects/{id}/output/` |
| `gen_trace_matrix.js` | トレーサビリティマトリクス.xlsx 生成 | `projects/{id}/output/` |
| `validate_docx.js` | docx 多重検証（L1:XML / L2:テキスト / L3:構造）| — |
| `trace_data.json` | トレーサビリティ定義データ（32セクション + 36関係 + 41 MDソース）| — |

## 実行例

```bash
# 全文書再生成（demo_ecuプロジェクト向け）
node tools/gen_system_spec.js
node tools/gen_process_spec.js
node tools/gen_ingest_spec.js
node tools/gen_trace_evidence.js
node tools/gen_trace_matrix.js

# 生成後にプロジェクトフォルダへコピー
cp *.docx *.xlsx projects/demo_ecu/output/

# 全文書検証
node tools/validate_docx.js --all
```

## フォルダ構成（マルチプロジェクト対応）

```
TORASAN/
├── CLAUDE.md            # Claude Code起動設定
├── PROCESS.md           # ISO 26262開発プロセス手順書（共通）
├── INGEST.md            # 既存資産取り込み手順書（共通）
├── tools/               # 共通ツール群（本フォルダ）
├── templates/           # プロジェクトテンプレート
├── projects/            # プロジェクト別フォルダ
│   └── {project_id}/
│       ├── project.json # プロジェクト状態
│       ├── output/      # 生成文書
│       ├── reviews/     # レビュー記録
│       ├── docs/        # プロジェクト固有ドキュメント
│       └── src/         # ソースコード
└── archive/             # 不要ファイル保管
```

## TCL分類

本ツール群はISO 26262 Part 8 cl.11に基づきTCL3に分類。全出力は人間によるレビューが必須。
