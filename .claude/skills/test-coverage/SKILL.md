---
name: test-coverage
description: "Analyzes test coverage metrics including MC/DC, statement, and branch coverage."
argument-hint: "[module or 'all']"
disable-model-invocation: true
---

# /test-coverage — テストカバレッジ分析

テストの要件カバレッジ・コードカバレッジを分析し、ギャップを特定する。

引数: $ARGUMENTS（オプション。"requirements" = 要件カバレッジ | "code" = コードカバレッジ | "gap" = ギャップ分析 | "all" → 省略時は "all"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |

## カバレッジ目標（IEC 60730 Class B / ISO 26262 ASIL B）

| カバレッジ種別 | ASIL B 要求 | 目標 |
|-------------|-----------|------|
| 要件カバレッジ | 100% 必須 | 100% |
| 文カバレッジ (Statement) | 推奨 | 100% |
| 分岐カバレッジ (Branch) | 必須 | 100% |
| MC/DC | 推奨（安全関連） | 安全関連関数のみ |

## 手順

### requirements モード — 要件カバレッジ分析

#### Step 1: 要件の収集
docs/ 配下から全要件IDを抽出:
- SG-XXX（安全目標）
- FSR-XXX（機能安全要求）
- TSR-XXX（技術安全要求）
- SR-XXX（SW安全要求）
- DR-XXX（診断要求）

#### Step 2: TC の収集
テストケース(TC-XXX)を全て抽出し、各TCがカバーする要件IDを特定

#### Step 3: カバレッジマトリクス生成

```
=== 要件カバレッジマトリクス ===

[SG → FSR カバレッジ]
| SG-ID | FSR数 | TC数 | カバー率 |
|-------|-------|------|---------|
| SG-001 | 3 | 9 | 100% |
| SG-002 | 2 | 6 | 100% |
| SG-003 | 2 | 4 | 100% |
| SG-005 | 5 | 12 | 80% ⚠ |

[SR カバレッジ]
| SR-ID | SR名称 | TC数 | 正常 | 境界 | 異常 | 判定 |
|-------|-------|------|------|------|------|------|
| SR-001 | 速度監視 | 5 | 1 | 3 | 1 | OK |
| SR-002 | 過速度停止 | 3 | 1 | 1 | 1 | OK |
| SR-015 | RAM診断 | 0 | 0 | 0 | 0 | NG ⚠ |

[サマリ]
要件総数: {total}
TC総数: {tc_total}
カバー済: {covered} ({pct}%)
未カバー: {uncovered} ({pct}%) ⚠
```

#### Step 4: 未カバー要件の報告
```
[未カバー要件一覧]
⚠ SR-015: RAM March C 診断 — TCなし
⚠ DR-003: クロックモニタ診断 — TCなし
推奨アクション: /test-design cases SR-015 で TC を生成
```

### code モード — コードカバレッジ分析

#### Step 1: ソースコード走査
src/ 配下の全 .c ファイルを対象

#### Step 2: テスト存在確認
各関数に対応するテストがあるか確認:
- テストファイル命名規則: `test_{module_name}.c`
- テスト関数命名規則: `test_{function_name}_{scenario}`

#### Step 3: カバレッジ推定
（実測データがない場合は構造ベースで推定）

```
=== コードカバレッジ推定 ===

| モジュール | 関数数 | テスト済 | 文カバレッジ推定 | 分岐カバレッジ推定 |
|-----------|-------|---------|-------------|-------------|
| motor_ctrl | 8 | 6 | 75% | 60% |
| current_mon | 4 | 2 | 50% | 40% |
| safety_diag | 6 | 0 | 0% ⚠ | 0% ⚠ |

安全関連関数の MC/DC カバレッジ:
| 関数 | 判定条件数 | MC/DC対象 | 状況 |
|------|----------|---------|------|
| MotorCtrl_CheckOverspeed | 1 | Yes | 未テスト ⚠ |
| CurrentMon_CheckOvercurrent | 1 | Yes | 未テスト ⚠ |
```

#### Step 4: gcov/lcov 実行（環境がある場合）
```bash
# ホストPC上でのユニットテスト実行とカバレッジ収集
gcc --coverage -o test_runner tests/*.c src/*.c
./test_runner
gcov src/*.c
lcov --capture --directory . --output-file coverage.info
```

### gap モード — ギャップ分析

要件カバレッジとコードカバレッジの両面からギャップを特定:

```
=== カバレッジギャップ分析 ===

[Critical Gaps — 即時対応必要]
1. SR-015 (RAM診断): TC未作成、コード未テスト
   → 安全関連、ASIL B 必須カバレッジ
   → 推奨: /test-design cases SR-015

2. safety_diag モジュール: テストゼロ
   → 全6関数が安全関連
   → 推奨: /test-design negative で異常系テスト生成

[High Gaps — 早期対応推奨]
3. 分岐カバレッジ 60% < 目標100%
   → motor_ctrl の条件分岐にテスト不足
   → 推奨: /test-design boundary で境界値テスト追加

[Summary]
要件カバレッジ: {pct}% (目標: 100%)
文カバレッジ推定: {pct}% (目標: 100%)
分岐カバレッジ推定: {pct}% (目標: 100%)
ギャップ数: Critical {n} / High {n} / Low {n}
ASIL B 達成判定: {YES/NO}
```

## 出力
カバレッジ分析結果を以下に記録:
- docs/11_test_spec.md（カバレッジセクション追記）
- project.json traceability カウント更新
- process_records/SWE.4, SWE.5, SWE.6 更新

## 関連スキル
- /test-design — ギャップに対する TC 生成（境界値・異常系）
- /static-analysis — コードカバレッジと併せて静的解析も実施推奨
- /systest-design — システムレベル（SG/FSR）のカバレッジは本スキル + /systest-design で検証
- /safety-verify — カバレッジ結果を安全検証レポートへ反映
- /trace — 要件カバレッジの基盤となるトレーサビリティ検証

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| データソース読み取り失敗 | Read / Glob エラー | 対象不在を報告。該当項目を「N/A」として表示 |
| 分析対象データ不足 | 必要キーの不在 | 不足項目を明示し、データ投入を促す |

## 注意事項
- カバレッジ実測値がない場合は「推定」と明記
- 安全関連関数は MC/DC を重点チェック
- ASIL B では分岐カバレッジ 100% が必須 — 下回る場合は ERROR
- ギャップは severity 順に並べ、対応アクションを具体的に提示
- /test-design スキルと連携して TC 生成を推奨

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `.claude/knowledge/iso26262_iec60730.md` — ASIL B カバレッジ目標値、MC/DC 要求基準
