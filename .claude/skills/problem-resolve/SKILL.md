---
name: problem-resolve
description: "Manages problem reports and resolution tracking per SPICE SUP.9."
argument-hint: "[action: report|resolve|list|analyze]"
disable-model-invocation: true
---


# /problem-resolve — 問題解決管理

SUP.9（問題解決管理）プロセスを支援する問題追跡・分析・解決スキル。

引数: $ARGUMENTS（オプション。"list" = 問題一覧 | "add {概要}" = 問題登録 | "analyze PRB-XXX" = 根本原因分析 | "resolve PRB-XXX" = 解決記録 | "report" = 問題解決レポート → 省略時は "list"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在し有効な JSON である | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | 更新対象セクションが存在する | キー存在確認 | セクションを初期化して続行（ユーザー確認後） |

## 問題管理構造

project.json の problemLog に記録:
```json
{
  "id": "PRB-{連番}",
  "date_reported": "YYYY-MM-DD",
  "reporter": "",
  "severity": "Critical / High / Medium / Low",
  "status": "Open / Analyzing / Resolved / Closed / Deferred",
  "title": "問題の概要",
  "description": "詳細説明",
  "affected_phase": "PH-XX",
  "affected_reqs": ["SR-XXX"],
  "root_cause": "",
  "resolution": "",
  "date_resolved": "",
  "verification": "",
  "related_changes": ["CHG-XXX"]
}
```

## 手順

### list モード — 問題一覧
```
=== 問題管理一覧 ===
| PRB-ID | 重大度 | 状態 | タイトル | フェーズ | 経過日数 |
|--------|-------|------|---------|---------|---------|
| PRB-001 | High | Open | {title} | PH-10 | 3日 |

[サマリ]
  Open: {n}件 (Critical: {n}, High: {n}, Medium: {n}, Low: {n})
  Analyzing: {n}件
  Resolved: {n}件
  Closed: {n}件
  Deferred: {n}件
```

### add モード — 問題登録
1. 新しい PRB-ID を採番（既存最大+1）
2. ユーザーから以下を収集:
   - タイトル（引数から取得 or 確認）
   - 重大度（Critical/High/Medium/Low）
   - 影響フェーズ
   - 影響要件（あれば）
3. project.json の problemLog に追記
4. process_records/SUP.9_problem_resolution.md を更新

### analyze モード — 根本原因分析
指定された PRB-XXX について:

1. **5 Why 分析**:
   ```
   PRB-XXX: {title}
   Why 1: なぜ発生したか → {原因1}
   Why 2: なぜ{原因1}か → {原因2}
   Why 3: なぜ{原因2}か → {原因3}
   Why 4: なぜ{原因3}か → {原因4}
   Why 5: なぜ{原因4}か → {根本原因}
   ```

2. **影響範囲分析**:
   - 影響する要件（SG/FSR/TSR/SR）
   - 影響するフェーズ
   - 影響する成果物
   - 回帰テストの必要性

3. **対策案の提示**:
   - 暫定対策（immediate）
   - 恒久対策（permanent）
   - 再発防止策（preventive）

4. status を "Analyzing" に更新

### resolve モード — 解決記録
1. 実施した対策を記録:
   - root_cause: 根本原因
   - resolution: 実施した対策
   - date_resolved: 解決日
   - verification: 検証方法と結果
   - related_changes: 関連する CHG-ID
2. status を "Resolved" に更新
3. 影響フェーズのゲート再検証が必要か確認
4. changeLog に関連エントリを追加（/commit-change 連携）

### report モード — 問題解決レポート
```
=== 問題解決レポート ===
期間: {from} 〜 {to}

[統計]
  新規登録: {n}件
  解決済: {n}件
  未解決: {n}件
  平均解決時間: {days}日

[重大度別]
  Critical: {open}/{total} 未解決
  High: {open}/{total} 未解決

[根本原因分類]
  要件不備: {n}件
  設計ミス: {n}件
  実装バグ: {n}件
  テスト漏れ: {n}件
  プロセス不備: {n}件

[再発防止策の有効性]
  {分析結果}

[SPICE SUP.9 PA評価への影響]
  問題解決プロセスの成熟度評価を更新
```

## 重大度判定基準

| 重大度 | 基準 |
|-------|------|
| Critical | 安全目標(SG)に影響 / 出荷停止レベル |
| High | 安全要求(SR/DR)に影響 / ASIL適合に影響 |
| Medium | 機能要求に影響 / 性能劣化 |
| Low | 軽微な不具合 / コスメティック |

## 関連スキル
- /commit-change — 問題修正のコミットと changeLog 連携
- /health-check — 未解決問題の全体俯瞰
- /trace — 問題が影響する要件チェーンの確認
- /update-record — SUP.9 プロセス記録の更新
- /test-design — 回帰テストケースの追加

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json パース失敗 | JSON decode エラー | 構文エラー箇所を報告。`git checkout -- project.json` で復元 |
| 書き込み失敗 | Write エラー | 変更内容を表示し手動適用を提案 |
| 更新内容の整合性エラー | バリデーション | 更新前の値を表示し `git checkout -- project.json` でロールバック |

## 注意事項
- Critical/High 問題は即時対応（Deferred 不可）
- 解決済み(Resolved)はレビュー後に Closed に遷移
- 問題 ID は連番維持（欠番不可）
- CERTIFY モードでは全 Critical/High が Closed 必須

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/automotive_spice.md` — SUP.9 GP証拠表、問題解決 BP 実施基準
