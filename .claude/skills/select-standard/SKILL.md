---
name: select-standard
description: "Derives applicable safety standards and classification from product category."
argument-hint: "[mode: derive|check|matrix (default: check)]"
disable-model-invocation: true
---

# /select-standard — 製品別法令・規格選定

製品カテゴリから適用法令・規格・安全分類方法を体系的に導出し、誤適用を防止する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "check"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを必ず参照:
- `.claude/knowledge/product_standard_mapping.md` — 製品カテゴリ→規格→分類マッピングDB
- `.claude/knowledge/iso26262_iec60730.md` — デュアルコンプライアンス対応表

## モード別手順

---

### matrix モード — 全カテゴリのマッピング表表示

1. `product_standard_mapping.md` §1 の全マッピング表を表示
2. `product_standard_mapping.md` §3 の地域別法令表を表示
3. 現在の project.json 設定をハイライト表示

出力:
```
=== 製品カテゴリ→規格マッピング ===

| カテゴリ | 製品安全規格 | 安全分類方法 | 分類単位 |
|---------|-------------|------------|---------|
| 自動車 | ISO 26262 | ASIL判定(S×E×C) | ASIL QM〜D |
| 家電 | IEC 60730 + IEC 60335 | Class分類(機能種別) | Class A/B/C | ← 現在
| 産業機器 | IEC 61508 | SIL判定(リスクグラフ) | SIL 1〜4 |
| 医療機器 | IEC 62304 + IEC 60601 | SWクラス分類 | Class A/B/C |
| 鉄道 | EN 50128/50129 | SIL判定 | SIL 0〜4 |

[地域別法令] (product_standard_mapping.md §3 参照)
```

---

### check モード — 現在設定の妥当性検証

project.json の standard 設定が製品カテゴリに適合しているか検証する。

#### Step 1: 現在の設定を読み取り
project.json から以下を取得:
- `standard.product_override`
- `standard.product_override_class`
- `standard.asil`
- `category`（product_spec から推定）

#### Step 2: 製品カテゴリとの整合性チェック
`product_standard_mapping.md` §6 の設定パターンと照合:

| チェック項目 | 判定条件 | 結果 |
|------------|---------|------|
| 製品規格一致 | product_override と product_override_class の組み合わせが §6 に存在 | PASS/FAIL |
| 分類方法一致 | product_override_class が規格の分類単位と一致 | PASS/FAIL |
| ASIL適用妥当性 | product_override=null の場合のみ ASIL 判定が適用可 | PASS/FAIL |
| プロセス基盤整合 | standard.base が設定されている | PASS/FAIL |

#### Step 3: 成果物中の用語チェック
docs/ ディレクトリ内の成果物をスキャン:
- `grep -r "ASIL" docs/` — product_override 設定時は不整合の可能性
- 製品規格の分類用語（Class B, SIL 等）が正しく使われているか確認
- HARA/FSC の分類列が製品規格と一致しているか確認

#### Step 4: アンチパターン照合
`product_standard_mapping.md` §4 の誤適用アンチパターンと照合し、該当があれば警告。

#### Step 5: 結果報告

```
=== 規格設定検証結果 ===
製品カテゴリ: {category}
製品安全規格: {product_override} {product_override_class}
プロセス基盤: {standard.base}

[検証結果]
  製品規格一致: {PASS/FAIL} — {詳細}
  分類方法一致: {PASS/FAIL} — {詳細}
  ASIL適用妥当性: {PASS/FAIL} — {詳細}
  プロセス基盤整合: {PASS/FAIL} — {詳細}

[成果物スキャン]
  不整合用語: {件数}件
  {不整合の詳細リスト}

[アンチパターン検出]
  該当: {件数}件
  {該当パターンのリスト}

総合判定: {適合 / 要修正}
```

---

### derive モード — 対話的規格導出フロー

製品情報から適用規格を段階的に導出する。

#### Step 1: 製品カテゴリ確認
ユーザーに確認（または project.json から推定）:
```
製品カテゴリを選択してください:
  1. 自動車（ECU, センサ, アクチュエータ）
  2. 家電（洗濯機, エアコン, 冷蔵庫）
  3. 産業機器（PLC, ロボット, 計装）
  4. 医療機器（モニタ, ポンプ）
  5. 鉄道（信号, ATC/ATP）
```

#### Step 2: 対象市場確認
```
対象市場を選択してください（複数可）:
  1. 日本
  2. EU
  3. 北米
  4. 中国
  5. グローバル
```

#### Step 3: 安全機能の種類確認
（カテゴリに応じた質問）
- 家電の場合: 「制御機能は安全に関係しますか？（A=非安全 / B=安全防止 / C=特殊危険防止）」
- 産業の場合: リスクグラフの各パラメータを確認
- 車載の場合: S, E, C の各パラメータを確認

#### Step 4: 規格・分類方法の決定
`product_standard_mapping.md` を参照して適用規格と分類結果を決定。

#### Step 5: 結果報告と project.json 更新提案

```
=== 規格選定結果 ===
製品: {product_category} — {product_name}
対象市場: {regions}

[製品安全規格]
  主規格: {primary_standard} ({version})
  副規格: {secondary_standards}

[安全分類]
  分類方法: {classification_method}
  分類結果: {class/ASIL/SIL}
  分類根拠: {rationale}

[適用法令]
  {region}: {regulations}

[プロセス手法]
  基盤: {process_base}
  流用項目: V-model, SPICE, FMEA, FTTI
  非適用項目: {items_not_applicable}

[project.json 更新提案]
  standard.base: "ISO_26262"
  standard.product_override: "{value}"
  standard.product_override_class: "{value}"

更新を適用しますか？ (y/N)
```

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| データソース読み取り失敗 | Read / Glob エラー | 対象不在を報告。該当項目を「N/A」として表示 |
| 分析対象データ不足 | 必要キーの不在 | 不足項目を明示し、データ投入を促す |

## 注意事項

- PH-01（安全計画）実行前に本スキルでの検証を推奨
- 規格選定は製品の安全機能に基づく — 製品仕様が未確定の場合は暫定選定とする
- `/switch-standard` は規格の「切替」、本スキルは製品からの「選定・検証」という役割分担
- 選定結果を変更する場合は `/switch-standard` で実施

## 関連スキル

- `/switch-standard` — 規格パッケージの切替実行
- `/execute-phase` — フェーズ実行（PH-01 前に本スキルで検証推奨）
- `/dashboard` — プロジェクトダッシュボード表示
- `/validate` — 数値・型・ゲート検証
- `/health-check` — プロジェクト健全性チェック
