---
name: ingest
description: "Imports and categorizes existing assets (documents, code, specifications) into the TORASAN project structure. Use when users say 'import', 'ingest', 'bring in files', 'asset import', or 'migrate existing'. Handles: import, ingest, asset migration, file categorization, legacy import."
disable-model-invocation: true
argument-hint: "[source path or asset type]"
---
# /ingest — 既存資産取り込み

既存ドキュメント・コードを TORASAN フレームワークの V-model フェーズに取り込む。

引数: $ARGUMENTS（オプション。取り込み対象のパスまたはフェーズ指定。例: "docs/legacy/" | "PH-08"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在し有効な JSON である | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | 更新対象セクションが存在する | キー存在確認 | セクションを初期化して続行（ユーザー確認後） |

## 手順（INGEST.md 準拠）

### Step 1: 資産スキャン
1. 対象ディレクトリ（引数指定 or docs/archive_v1/）を走査
2. ファイル一覧を作成:
```
=== 資産スキャン結果 ===
| # | ファイル | 種別 | サイズ | 推定フェーズ |
|---|---------|------|-------|-----------|
| 1 | xxx.md | 要件仕様 | 12KB | PH-08 |
```

### Step 2: 分類・マッピング
各ファイルについて:
1. 内容を読み、以下を判定:
   - **文書種別**: 要件定義 / 設計書 / テスト仕様 / コード / その他
   - **対応フェーズ**: PH-01〜PH-15 のどれに該当するか
   - **要件ID**: 既存の ID 体系があるか
   - **品質**: そのまま使えるか / 変換が必要か / 参考レベルか

2. マッピング結果を表示:
```
[マッピング結果]
PH-02 アイテム定義: file_a.md (変換必要)
PH-08 SW安全要求: file_b.md (そのまま使用可)
PH-10 SWユニット: src/*.c (型ルール検証必要)
未分類: file_c.md
```

### Step 3: 変換・統合
ユーザー確認後、各ファイルを処理:
1. **そのまま使用可**: 所定のディレクトリにコピー、要件ID付与
2. **変換必要**: TORASAN フォーマットに変換
   - 要件IDの採番（§7 準拠）
   - product_spec 数値の整合確認（§6 準拠）
   - 型ルール準拠確認（§5 準拠、Cソースの場合）
3. **参考レベル**: docs/reference/ に配置、関連フェーズへの参照リンク追加

### Step 4: トレーサビリティ接続
1. 取り込んだ要件に ID を採番
2. 親子関係を設定（SG→FSR→TSR→SR→TC）
3. project.json の traceability カウントを更新

### Step 5: 記録更新
1. project.json の phases を更新（取り込みにより状態が変わったフェーズ）
2. 該当する process_records/ を更新
3. changeLog にインジェスト記録を追加

## 出力
```
=== 資産取り込み完了 ===
取り込み: {count}件
  - そのまま使用: {count}件
  - 変換済: {count}件
  - 参考配置: {count}件
  - スキップ: {count}件
新規要件ID: {id_range}
影響フェーズ: {phase_list}
```

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json パース失敗 | JSON decode エラー | 構文エラー箇所を報告。`git checkout -- project.json` で復元 |
| 書き込み失敗 | Write エラー | 変更内容を表示し手動適用を提案 |
| 更新内容の整合性エラー | バリデーション | 更新前の値を表示し `git checkout -- project.json` でロールバック |

## 注意事項
- 既存ファイルを上書きしない（必ず新規作成または追記）
- 変換前の原本は docs/archive_v1/ に保持
- 大量ファイルの場合はバッチ単位でユーザー確認を挟む
