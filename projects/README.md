# TORASAN プロジェクト一覧

各プロジェクトは `projects/{project_id}/` に格納されます。

## プロジェクト構成

```
projects/{project_id}/
├── project.json      # プロジェクト状態・進捗・ASIL設定
├── output/           # 生成文書（docx, xlsx）
├── reviews/          # レビュー記録
├── docs/             # プロジェクト固有ドキュメント
└── src/              # ソースコード（該当する場合）
```

## 新規プロジェクト作成手順

1. `templates/project_template.json` をコピー
2. `projects/{新プロジェクトID}/project.json` として配置
3. 必要なサブフォルダを作成
4. CLAUDE.md の指示に従ってプロジェクトを開始

## 現在のプロジェクト

| ID | 名称 | ASIL | 状態 |
|----|------|------|------|
| demo_ecu | Demo ECU 機能安全開発 | B | initial |

> **Note**: bldc_washer は TORASAN 本体に吸収済み（S22）。ルート project.json が Single Source of Truth。
