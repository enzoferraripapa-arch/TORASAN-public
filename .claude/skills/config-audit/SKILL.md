---
name: config-audit
description: "Performs configuration management audit per SPICE SUP.8."
argument-hint: "[mode: checklist|baseline|report|all (default: all)]"
disable-model-invocation: true
---


# /config-audit — 構成管理監査

SUP.8（構成管理）プロセスの正式な監査チェックリスト実行と監査レポート生成。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "all"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | Git リポジトリ内で実行 | `git status` | リポジトリ外では実行不可 |

## 構成管理の対象

| カテゴリ | 対象 | 管理方法 |
|---------|------|---------|
| ソースコード | src/ | Git |
| 成果物文書 | docs/ | Git |
| プロセス記録 | process_records/ | Git |
| プロジェクト設定 | project.json, CLAUDE.md, PROCESS.md | Git |
| ビルド環境 | scripts/, app/ | Git |
| スキル定義 | .claude/commands/ | Git |
| 生成文書 | docx/ | 再生成可能（Git外） |

## 手順

### checklist モード — 監査チェックリスト

#### 1. バージョン管理
- [ ] 全成果物が Git で管理されているか
- [ ] .gitignore が適切に設定されているか
- [ ] 機密情報（.env, credentials）が除外されているか
- [ ] コミットメッセージが規約に従っているか（type: 日本語要約）
- [ ] Co-Authored-By が適切に付与されているか

#### 2. ベースライン管理
- [ ] 各フェーズ完了時に Git タグが付与されているか
- [ ] タグの命名規則が統一されているか（v{N}-backup）
- [ ] タグから成果物を復元可能か

#### 3. 変更管理
- [ ] 全変更が changeLog に記録されているか
- [ ] CHG-ID が連番で採番されているか
- [ ] 変更の影響フェーズが特定されているか
- [ ] 変更後にゲート再検証が実施されているか

#### 4. 成果物整合性
- [ ] project.json の processVersion と PROCESS.md のバージョンが一致するか
- [ ] project.json の phases 状態と実際の成果物存在が一致するか
- [ ] トレーサビリティカウント（traceability）が実際の要件数と一致するか
- [ ] spice_assessment が process_records の内容と整合するか

#### 5. ファイル構成
- [ ] PROCESS.md §10 のファイル構成と実際のディレクトリが一致するか
- [ ] 不要なファイル（一時ファイル、バックアップ）が残っていないか
- [ ] process_records/ に全16プロセスのファイルが存在するか

#### 6. 数値整合性
- [ ] /validate numbers の結果が全て PASS か
- [ ] product_spec の値がソースコードのマクロと一致するか

### baseline モード — ベースライン検証
1. `git tag` で全タグを一覧
2. 各タグの内容を検証:
   - タグ時点の成果物が完備しているか
   - タグメッセージに状態情報が含まれているか
3. 最新タグからの差分を確認:
   - 変更ファイル数
   - 未コミットの変更有無

```
=== ベースライン検証 ===
| タグ | 日時 | コミット | フェーズ状態 | 判定 |
|------|------|---------|-----------|------|
| v1-backup | {date} | {hash} | PH-01〜03完了 | ✓ |
| v2-backup | {date} | {hash} | PH-01〜08完了 | ✓ |

現在の状態:
  最新タグ: {tag}
  タグからの差分: {n}コミット, {n}ファイル変更
  未コミット変更: {あり/なし}
```

### report モード — 監査レポート生成

```
=== 構成管理監査レポート ===
監査日: {date}
監査対象: TORASAN プロジェクト

[チェックリスト結果]
  ✓ 合格: {n}/{total} 項目
  ✗ 不合格: {n}/{total} 項目
  — 該当なし: {n}/{total} 項目

[不合格項目]
  1. {項目}: {不合格理由} → {推奨対策}

[ベースライン状態]
  タグ数: {n}
  最新ベースライン: {tag} ({date})
  ベースライン健全性: {PASS/FAIL}

[変更管理状態]
  changeLog エントリ: {n}件
  最終変更: CHG-{XXX} ({date})
  未記録の変更: {あり/なし}

[総合判定]
  構成管理プロセス成熟度: {N/P/L/F}
  SUP.8 Level達成見込み: Level {n}

[改善推奨]
  1. {具体的な改善提案}
```

## SPICE SUP.8 との対応

| BP | 内容 | チェック項目 |
|----|------|-----------|
| BP1 | 構成管理戦略の策定 | Git運用ルール、.gitignore |
| BP2 | 構成項目の特定 | 管理対象一覧、ファイル構成 |
| BP3 | ベースラインの確立 | Git タグ、バージョン管理 |
| BP4 | 変更管理 | changeLog、影響分析 |
| BP5 | 構成状態の報告 | 監査レポート |
| BP6 | 構成監査の実施 | 本チェックリスト |

## 関連スキル
- /backup — ベースライン（タグ）の作成
- /validate — 数値整合性の詳細検証
- /commit-change — 変更管理のコミット側
- /update-record — SUP.8 プロセス記録の更新
- /health-check — プロジェクト全体の健全性確認

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| データソース読み取り失敗 | Read / Glob エラー | 対象不在を報告。該当項目を「N/A」として表示 |
| 分析対象データ不足 | 必要キーの不在 | 不足項目を明示し、データ投入を促す |

## 注意事項
- 監査結果は process_records/SUP.8_config_management.md に記録
- CERTIFY モードでは全チェック項目 PASS 必須
- 不合格項目は /problem-resolve で問題登録を推奨
- 監査は定期的に実施（フェーズ完了時、リリース前）

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/automotive_spice.md` — SUP.8 GP証拠表、構成管理 BP 実施基準
