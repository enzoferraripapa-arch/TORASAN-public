---
type: session
id: S19
date: 2026-03-07
branch: master
scope: "S18 持越し課題全件実行 — MCP/Templater/00-Context/S1-S18移行/DrawingML改善"
achievements:
  - "MCP プラグイン2種導入（smithery-ai/mcp-obsidian + obsidian-claude-code-mcp v1.1.8）"
  - "Templater v2.18.1 導入 + 3テンプレート（issue/idea/session）+ フォルダ自動適用"
  - "00-Context エージェントメモリノート作成（運用知識・教訓・ゴールデンルール）"
  - "S1〜S18 全18セッションの Obsidian ノート化完了（YAML frontmatter 付き）"
  - "DrawingML _Y_SCALE=2.0 除去 → 図ごと明示サイズ指定 + ボックス拡大 + V モデル1行統合"
  - "MOC 更新（agent-memory リンク追加）"
  - "Obsidian 再起動 + プラグイン有効化完了"
issues_found:
  - "obsidian-claude-code-mcp プラグインディレクトリ名が manifest ID と不一致（obsidian-claude-code-mcp → claude-code-mcp にリネーム要）"
  - "bash 環境で npx/node が PATH に不在（/c/Program Files/nodejs を手動追加で対応）"
issues_resolved:
  - "プラグインディレクトリ名修正済み"
improvement_items:
  - desc: "DrawingML 出力の実際の Word 表示確認（目視レビュー）"
    status: open
  - desc: ".gitattributes CRLF 正規化対応"
    status: open
carryover_from: S18
tags:
  - session
  - obsidian
  - drawingml
  - mcp
---

# セッション19 反省会議事録

日時: 2026-03-07
ブランチ: master

## 1. セッション成果サマリ

| # | 作業 | 状態 |
|---|------|------|
| 1 | MCP プラグイン2種導入（.mcp.json + Obsidian プラグイン） | 完了 |
| 2 | Templater v2.18.1 導入 + テンプレート3種作成 | 完了 |
| 3 | 00-Context エージェントメモリノート作成 | 完了 |
| 4 | S1〜S18 全18セッション Obsidian ノート移行 | 完了 |
| 5 | DrawingML ビジュアル品質改善（_Y_SCALE 除去 + 明示サイズ） | 完了 |
| 6 | Obsidian 再起動 + プラグイン有効化確認 | 完了 |

## 2. 発生した問題

1. **プラグインディレクトリ名不一致**: `obsidian-claude-code-mcp` で作成したが、manifest.json の ID は `claude-code-mcp`。Obsidian はディレクトリ名 = プラグイン ID で認識するためリネームが必要だった。
2. **Node.js PATH 問題**: bash 環境で `npx` が PATH に無い。`/c/Program Files/nodejs` を手動追加で対応。

## 3. 根本原因分析

- ディレクトリ名: GitHub リポ名とプラグイン ID が異なるパターン（npm パッケージ名 vs Obsidian 内部 ID）
- PATH: Windows bash 環境で Program Files 下のツールが自動 PATH に入らない既知問題

## 4. 良かった点

1. **並列エージェント活用**: S1-S16 移行を3エージェントで並列処理し、大量ファイル変換を効率化
2. **持越し課題の完全消化**: S18 の持越し4件 + DrawingML 長期課題を全て処理
3. **段階的検証**: DrawingML 改善で変更→テスト→サイズ検証のサイクルを回した
4. **自律的操作**: Obsidian 再起動・プラグイン設定をユーザー手動操作なしで完了

## 5. 改善すべき点

| # | 問題 | 対策 | 状態 |
|---|------|------|------|
| 1 | プラグインディレクトリ名を推測で作成した | manifest.json の ID を先に確認してからディレクトリ作成 | 済（今回修正済み） |
| 2 | DrawingML の目視確認を省略 | Word で実際に開いて表示確認するステップを追加 | 未 |
| 3 | .gitattributes 未対応（S18 から継続） | 次回セッションで対応 | 未 |

## 6. 教訓

1. Obsidian プラグインのディレクトリ名は manifest.json の `id` フィールドと一致させる必要がある（GitHub リポ名とは異なることがある）
2. 大量ファイル変換は並列エージェントで分割すると効率的（S1-S6 / S7-S12 / S13-S16 の3分割）
3. DrawingML の `_Y_SCALE` のような一律スケールは問題の根本解決にならない — 図ごとの明示サイズが正解
4. Windows bash 環境の PATH 問題は繰り返し発生するため、スクリプト内で明示的に PATH 追加するのが安全

## 7. 前回反省会フォローアップ

| S18 改善項目 | 状態 |
|------------|------|
| MCP プラグイン導入 | 完了（2種導入） |
| Templater プラグイン導入 | 完了 |
| 00-Context エージェントメモリノート作成 | 完了 |
| S1〜S16 Obsidian ノート化 | 完了（S1〜S18 全件） |
| Obsidian 改行コード CRLF 問題 | 未（.gitattributes 対応待ち） |
