---
name: switch-standard
description: "Switches project safety standard configuration between ISO 26262, IEC 60730, and IEC 61508 with impact analysis. Use when users mention 'switch standard', 'change standard', 'change ASIL', 'product type change', or 'package switch'. Handles: standard switch, ASIL, IEC 60730, ISO 26262, IEC 61508, package configuration."
disable-model-invocation: true
argument-hint: "[target: iso26262|iec60730|iec61508]"
---

# /switch-standard — 安全規格パッケージ切替

プロジェクトの適用規格を切り替え、影響するフェーズを再確認する。

引数: $ARGUMENTS（切替先。"iso26262" | "iec60730" | "iec61508" → 省略時は現在の設定を表示）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在し有効な JSON である | `Read project.json` | `/session start` でプロジェクトを初期化 |
| 2 | 更新対象セクションが存在する | キー存在確認 | セクションを初期化して続行（ユーザー確認後） |

## パッケージ対応表

| 製品例 | asil | product_override | product_override_class |
|-------|------|-----------------|----------------------|
| 車載ECU | D | null | — |
| 家電制御（洗濯機等） | B | IEC60730 | ClassB |
| 産業機器 | C | IEC61508 | SIL2 |

## 手順

### 引数なし: 現在設定の表示
```
=== 現在の規格設定 ===
ベース規格: {standard.base} ({standard.base_version})
ASIL: {standard.asil}
製品オーバーライド: {standard.product_override}
製品分類: {standard.product_override_class}
MISRA: {standard.misra}
```

### 引数あり: 規格切替

#### Step 1: 変更内容の確認
1. 現在の設定と変更後の設定を比較表示
2. ユーザーに確認を求める

#### Step 2: project.json 更新
1. `standard` セクションを更新:
   - base, asil, product_override, product_override_class
2. 旧 `packages` セクションが残存していれば `_packages_removed` に置換

#### Step 3: 影響分析
1. 全15フェーズについて以下を確認:
   - ASIL レベル変更による要求事項の変化
   - 安全メカニズム（WDT, CRC, RAM test等）の要否変化
   - テストカバレッジ基準の変化
2. completed フェーズは再検証が必要かマーク

#### Step 4: changeLog 記録
```json
{
  "type": "前提変更",
  "item": "standard",
  "from": "{旧規格}",
  "to": "{新規格}",
  "reason": "製品ターゲット変更",
  "affectedPhases": ["再検証が必要なフェーズ"]
}
```

#### Step 5: 報告
```
=== 規格切替完了 ===
{旧規格} → {新規格}
影響フェーズ: {count}件
再検証必要: {フェーズ一覧}
```

#### Step 6: 切替後検証（推奨）
規格切替後、`/select-standard check` を実行して以下を確認:
- 新しい規格設定が製品カテゴリと整合しているか
- 成果物中の安全分類用語（ASIL/Class/SIL）が新規格と一致しているか
- 誤適用アンチパターンに該当しないか

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| project.json パース失敗 | JSON decode エラー | 構文エラー箇所を報告。`git checkout -- project.json` で復元 |
| 書き込み失敗 | Write エラー | 変更内容を表示し手動適用を提案 |
| 更新内容の整合性エラー | バリデーション | 更新前の値を表示し `git checkout -- project.json` でロールバック |

## 注意事項
- CERTIFY モードでは規格切替に伴い全ゲート再検証が必要
- 既存成果物の ASIL 表記を更新する必要がある場合は警告
- product_spec（MCU/モータ等）は規格切替では変更しない
- 切替後は必ず `/select-standard check` で整合性を検証すること

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/iso26262_iec60730.md` — デュアルコンプライアンス対応表、ISO 26262 と IEC 60730 の規格比較

## 関連スキル
- `/dashboard` — プロジェクトダッシュボード表示
- `/execute-phase` — フェーズ実行
- `/validate` — 数値・型・ゲート検証
- `/health-check` — プロジェクト健全性チェック
