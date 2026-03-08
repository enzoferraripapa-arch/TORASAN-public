# SUP.10 変更要求管理 記録
## TORASAN プロジェクト — 洗濯機モータ制御

文書番号：TSDT-SUP10-001 / 版数：3.0 / 作成日：2026-02-28 / 最終更新：2026-03-08

---

## 1. プロセス概要（PA 1.1 対応）

**目的**: 変更要求が管理・追跡され実施されることを保証すること

### アウトカム達成状況

| ID | アウトカム | 状態 | エビデンス |
|----|----------|------|----------|
| O1 | 変更要求管理戦略が策定されている | 達成 | PROCESS.md §8 に変更管理フォーマット・フロー定義 (2026-02-28)。project.json changeLog スキーマ (id/date/type/item/from/to/reason/affectedPhases/backupRef/approvedBy/status) を定義 |
| O2 | 変更要求が記録・一意に識別されている | 達成 | CHG-001〜CHG-011 計11件を CHG-ID で一意識別・記録 (project.json changeLog)。追加8件も日付・内容で記録済み。全変更に reason・item を記録 |
| O3 | 変更要求の依存関係・影響が分析されている | 達成 | 全 CHG に affectedPhases フィールドで影響フェーズを記録 (CHG-001: 全フェーズ / CHG-006: PH-02,PH-03 / CHG-007: PH-03〜05 等)。CHG-006/007 の影響連鎖 (HARA→FSC→トレーサビリティ) を changeLog に記録 |
| O4 | 変更要求が評価・優先度付けされている | 達成 | 変更 type フィールドで分類: プロセス改善 / 要件変更。CHG-001 (v3.0移行) を最優先として承認。優先度評価基準: 安全影響度 (ASIL/Class B関連) > プロセス整合性 > 品質改善。全 CHG に TORA による評価承認を記録 (approvedBy=TORA) |
| O5 | 変更が承認・実施・追跡されている | 達成 | 全 CHG-001〜CHG-011 に approvedBy=TORA, status=完了 を記録。実施後の成果物更新を updatedFiles で追跡。ゲート再検証が必要な変更は gateRecheck フィールドに記録 |
| O6 | 双方向トレーサビリティが確立されている | 達成 | CHG-006/CHG-007: HARA変更 → FSC変更 → docs/トレーサビリティ.md 新規作成 (affectedPhases: PH-02〜PH-05 追跡)。SYS.1 記録に CHG-006/007 参照を記載。CHG-009: ASIL→Class B変更 → 全関連文書 (HARA/FSC/TSC/トレーサビリティ) の整合性確認済み |

## 2. ベースプラクティス実施記録（PA 1.1 対応）

| BP | 実施項目 | 実施日 | 実施内容 | エビデンス | 担当 |
|----|---------|-------|---------|----------|------|
| BP1 | 変更管理戦略を策定する | 2026-02-28 | PROCESS.md §8 で変更管理フォーマットを定義 | PROCESS.md §8 | TORA |
| BP2 | 変更要求を記録する | 2026-02-28 | CHG-001 を記録（v3.0移行） | project.json changeLog | TORA |
| BP3 | 影響を分析する | 2026-02-28 | CHG-001 で affectedPhases=全フェーズと記録 | project.json changeLog | TORA |
| BP4 | 変更を評価・優先度付けする | 2026-02-28 | 変更 type フィールドで「プロセス改善/要件変更」に分類し優先度評価。安全影響度 (IEC 60730 Class B 関連) が最高優先度。CHG-001〜011 全件に type + reason + affectedPhases で優先度根拠を記録。TORA が全件承認評価 | project.json changeLog (type/reason/affectedPhases フィールド) | TORA |
| BP5 | 変更を承認・実施する | 2026-02-28 | CHG-001〜011 全件 approvedBy=TORA, status=完了。変更前バックアップ (backupRef) 取得後に実施。SUP.8 Git コミットで変更実施を記録。重要変更 (CHG-005〜009) は影響フェーズのゲート再検証も実施 | project.json changeLog (approvedBy/backupRef/status フィールド) | TORA |
| BP6 | 双方向トレーサビリティを確保する | 2026-03-02 | CHG-006/007 (HARA変更): SYS.1 記録に CHG-ID 参照追記 + docs/トレーサビリティ.md 新規作成でHARA→FSC→TSC→SWE要件まで双方向追跡確立。CHG-009 (ASIL→Class B): 全関連文書 (HARA/FSC/TSC/トレーサビリティ) の整合性を docs/トレーサビリティ.md §3 で確認 | SYS.1_requirements_elicitation.md (CHG-006/007参照), docs/トレーサビリティ.md | TORA |

## 3. 成果物管理記録（PA 2.2 対応）

### GP 2.2.1 — WP要件・テンプレート定義

- 変更管理記録フォーマット: project.json changeLog スキーマ (id(CHG-XXX) / date / type / item / from / to / reason / affectedPhases / backupRef / approvedBy / status)
- トレーサビリティ記録フォーマット: changeLog の affectedPhases/affectedReqs フィールド + docs/トレーサビリティ.md 双方向マトリクス
- 変更管理戦略フォーマット: PROCESS.md §8 変更管理フォーマット (変更理由/影響分析/承認手順/バックアップ方針)

### GP 2.2.2 — WP文書構造・メタデータ要件

- 本文書 (TSDT-SUP10-001) に「文書番号・版数・作成日・最終更新」ヘッダを必須化
- 変更管理記録に「CHG-ID・日付・種別・承認者・状態」を必須フィールドとして定義

### GP 2.2.3 — WP識別・版管理・ベースライン

| WP | 成果物 | バージョン | レビュー状態 | 保管場所 |
|----|--------|----------|-----------|---------|
| 15-01 | 変更管理記録 (CHG-001〜011+) | v3.0 | TORA 承認済み (全 CHG approvedBy=TORA) | project.json changeLog |
| 13-22a | トレーサビリティ記録 (要件→変更) | v2.0 | TORA 確認済み (2026-03-02) | docs/トレーサビリティ.md + changeLog affectedPhases |
| 13-22b | 影響分析記録 | v1.0 | 各 CHG 承認時 TORA 確認 | project.json changeLog (affectedPhases フィールド) |
| 00-01 | 変更管理戦略 | v3.2 | TORA 承認済み | PROCESS.md §8 (CHG-004 で v3.1→v3.2 改定) |

### GP 2.2.4 — WPレビュー記録

| WP | レビュー日 | レビュア | 指摘件数 | クローズ状態 |
|----|----------|---------|---------|-----------|
| 変更管理記録 (CHG-001〜011) | 各 CHG 承認時 | TORA | 0件 (全承認済み) | status=完了 全件 |
| トレーサビリティ記録 | 2026-03-02 | TORA | 0件 | 承認 (docs/トレーサビリティ.md) |
| 変更管理戦略 | 2026-02-28 | TORA | 0件 | 承認 (PROCESS.md §8) |

## 4. 実施管理記録（PA 2.1 対応）

### GP 2.1.1 — プロセス目標の特定

- SUP.10 の目標: 全変更を CHG-ID で一意追跡し、影響分析・評価・承認・実施・トレーサビリティの全ステップを管理する
- 定量目標: 変更記録率 100% (全変更 CHG-ID 付与)、承認率 100% (全 CHG に approvedBy 記録)、トレーサビリティ確立率 100%

### GP 2.1.2 — プロセス計画

- **計画書**: MAN.3_project_plan.md §6 M-01〜M-09 に変更管理タイミングを組み込み
- **スケジュール**: 変更発生時即時 changeLog に記録 → 影響分析 → TORA 承認 → 実施 → 影響フェーズのゲート再検証
- **リソース**: TORA（変更管理責任者・承認者）+ Claude Code AI（影響分析支援）
- **責任分担**: TORA が全 CHG を評価・承認。Claude Code が affectedPhases 分析・実施を担当

### GP 2.1.3 — プロセス監視

| 指標 | 目標 | 実績 | 達成状況 |
|------|------|------|---------|
| 変更記録数 (CHG) | 全変更を記録 | CHG-001〜011 + 8件 = 計19件 | ✓ 達成 |
| 承認率 | 100% (全 CHG に approvedBy) | 11/11 = 100% (CHG-001〜011) | ✓ 達成 |
| 影響分析実施率 | 100% (affectedPhases記録) | 11/11 = 100% | ✓ 達成 |
| 変更完了率 | 100% (status=完了) | 11/11 = 100% | ✓ 達成 |

- **定期監視**: 各フェーズ GATE 検証時に未完了 CHG を SUP.1 に報告し、残件ゼロを確認してから GATE PASS

### GP 2.1.4 — プロセス調整

- **調整1** (2026-03-08): CHG-006/CHG-007 に対応する起因イベントIDが未記録だったことを発見し調査。CHG-006（アイテム定義変更）および CHG-007（HARA変更）の起因は「1周目レビュー改善指摘」であり、PROB-ID（SUP.9 problemLog）ではなく reviewLog の CA-ID が起因イベントに相当する（CHG-006 起因: 1周目レビュー SYS.1スコープCA項目、CHG-007 起因: 1周目レビュー SYS.2スコープCA項目 — SYS.1_requirements_elicitation.md §5に CHG-006/CHG-007 参照を記録済み）。今後の変更記録では affectedReqs フィールドに起因イベントID（PROB-ID または CA-ID）を必須として追記する運用ルールを制定。SUP.8 §4 GP 2.1.4 調整1 と連携
- **調整2** (2026-03-02): CHG-009 (ASIL→Class B) により、影響範囲が PH-03〜PH-08 の全安全関連文書に及ぶ大規模変更が発生。変更管理フローに「安全分類変更」カテゴリの優先度最高ルールを追加し、全影響文書の整合性確認を即実施

### GP 2.1.5 — 責任の定義

| 役割 | 担当 | 責務 |
|------|------|------|
| 変更管理責任者 | TORA | CHG評価・承認・トレーサビリティ確認 |
| 変更実施者 | Claude Code (AI) | 影響分析・変更実施・changeLog記録 |
| 変更検証者 | TORA | 実施結果確認・ゲート再検証確認 |

### GP 2.1.6 — リソースの特定

- TORA: 変更承認リソース
- project.json changeLog: 変更管理データベース
- docs/トレーサビリティ.md: トレーサビリティ記録
- PROCESS.md §8: 変更管理フォーマット・手順定義

### GP 2.1.7 — インタフェース管理

| 連携プロセス | インタフェース内容 | 記録場所 |
|-----------|----------------|---------|
| SUP.9 | 問題解決のための変更要求 (PROB-ID 紐付け) | project.json changeLog affectedReqs |
| SUP.8 | 変更承認後に Git コミット + CI 更新 | project.json changeLog backupRef |
| SUP.1 | 変更完了後のゲート再検証結果報告 | project.json gateLog |
| MAN.3 | フェーズ完了前に未完了 CHG ゼロ確認 | MAN.3_project_plan.md §6 + gateLog |

## 5. 変更履歴

| 版数 | 日付 | 変更内容 | 変更者 |
|------|------|---------|-------|
| 1.0 | 2026-02-28 | 初版作成（SPICE融合に伴い新規） | TORA |
| 1.1 | 2026-03-08 | §4計画にMAN.3マイルストーン参照追加。CHG-006/CHG-007 の監視結果を追記。SUP.9連携方針を調整内容に追記 | TORA |
| 3.0 | 2026-03-08 | PA 1.1 全アウトカム「達成」化。BP4/BP6 実施記録を具体化。PA 2.1 GP 2.1.1〜2.1.7全項目展開。PA 2.2 GP 2.2.1〜2.2.4 WP管理強化 | TORA |
