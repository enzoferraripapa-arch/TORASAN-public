---
name: trace
description: "Generates and manages traceability matrices linking requirements and test cases."
argument-hint: "[mode: matrix|check|gaps]"
---

# /trace — トレーサビリティマトリクス生成・検証

要件IDの階層トレース（SG→FSR→TSR→SR→TC）を検証し、マトリクスを生成する。

引数: $ARGUMENTS（オプション。"check" = 検証のみ | "matrix" = マトリクス生成 | "report" = 詳細レポート → 省略時は "check"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |

## 要件ID体系（PROCESS.md §7）

| レベル | ID形式 | 生成フェーズ | 親リンク先 |
|-------|--------|-----------|----------|
| SG-XXX | 安全目標 | PH-03 | — |
| FSR-XXX | 機能安全要求 | PH-04 | SG |
| TSR-XXX | 技術安全要求 | PH-05 | FSR |
| SR-XXX | SW安全要求 | PH-08 | TSR |
| DR-XXX | 診断要求 | PH-08 | TSR |
| SA-XXX | SWアーキテクチャ | PH-09 | SR |
| UT-XXX | SWユニット | PH-10 | SA |
| TC-XXX | テストケース | PH-11 | SR/DR |

## 手順

### check モード
1. docs/ 配下の全成果物ファイルを走査
2. 正規表現で要件ID（SG-\d{3}, FSR-\d{3} 等）を抽出
3. 以下を検証:
   - **重複チェック**: 同一IDが複数定義されていないか
   - **親リンクチェック**: 各要件が親要件を参照しているか
   - **孤児チェック**: どこからも参照されない要件がないか
   - **カバレッジ**: 全SGがTCまでトレースされているか
4. project.json の traceability カウントを更新
5. 結果を報告:

```
=== トレーサビリティ検証結果 ===
SG: {count}件 | FSR: {count}件 | TSR: {count}件
SR: {count}件 | DR: {count}件 | TC: {count}件

[リンク状況]
リンク済: {linkedCount}件 / 未リンク: {unlinkedCount}件
カバレッジ: {coverage}%

[問題]
⚠ FSR-003: 親SG未指定
⚠ SR-012: TCによるカバレッジなし
```

### matrix モード
1. check を実行した上で
2. 以下のマトリクスを生成:

```
| SG | → FSR | → TSR | → SR | → TC | Status |
|----|-------|-------|------|------|--------|
| SG-001 | FSR-001,002 | TSR-001 | SR-001,002 | TC-001〜003 | 完全 |
| SG-002 | FSR-003 | — | — | — | 不完全 |
```

### report モード
1. matrix を実行した上で
2. 各要件の詳細（タイトル、ASIL、トレースチェーン）を一覧
3. ギャップ分析と推奨アクションを提示

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| データソース読み取り失敗 | Read / Glob エラー | 対象不在を報告。該当項目を「N/A」として表示 |
| 分析対象データ不足 | 必要キーの不在 | 不足項目を明示し、データ投入を促す |

## 注意事項
- 成果物が未生成のフェーズはスキップ（エラーにしない）
- ID の連番飛びは警告のみ（エラーにしない）
- project.json の traceability は必ず最新値に更新

## 関連スキル
- /execute-phase — フェーズ実行で要件を生成
- /srs-generate — SW要求仕様の専門生成
- /test-design — テストケース設計
- /safety-verify — 安全検証レポート

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/iso26262_iec60730.md` — ISO 26262 トレーサビリティ要求、ASIL分解ルール
