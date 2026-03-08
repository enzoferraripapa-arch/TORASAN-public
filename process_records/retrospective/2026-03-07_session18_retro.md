---
type: session
id: S18
date: 2026-03-07
branch: master
scope: "Obsidian Vault 導入 — 課題・アイデア・セッション管理基盤構築"
achievements:
  - "Obsidian v1.12.4 インストール（Windows版、直接ダウンロード）"
  - "TORASAN を Vault として構成（除外フォルダ設定済み）"
  - "4ゾーン構造作成（00-Context, 10-Issues, 20-Ideas, 30-Sessions）"
  - "ISS-001〜008 を個別ノート化（YAML frontmatter 付き）"
  - "IDEA-001〜004 を個別ノート化（YAML frontmatter 付き）"
  - "Dataview プラグイン導入・ダッシュボード3種作成"
  - "MOC（Map of Content）作成"
  - "Obsidian CLI 有効化・PATH 設定"
  - "Windows 起動時の自動起動ショートカット作成"
issues_found:
  - "winget が利用不可 → 直接ダウンロードで対応"
  - "Obsidian CLI が app.json の enableCli では有効化不可 → UI 操作必要"
  - "CLI PATH 登録で PowerShell エラー → 手動スクリプトで対応"
issues_resolved: []
improvement_items:
  - desc: "MCP プラグイン（obsidian-claude-code-mcp）導入"
    status: open
  - desc: "Templater プラグイン導入"
    status: open
  - desc: "00-Context エージェントメモリノート作成"
    status: open
  - desc: "S1〜S16 の Obsidian ノート化（段階的移行）"
    status: open
carryover_from: S17
tags:
  - session
  - obsidian
  - knowledge-management
---

# セッション18 反省会議事録

日時: 2026-03-07
ブランチ: master

## 1. セッション成果サマリ

| # | 作業 | 状態 |
|---|------|------|
| 1 | Obsidian 導入調査（API・プラグイン・図形機能・OS選定）| 完了 |
| 2 | Obsidian v1.12.4 インストール（Windows版）| 完了 |
| 3 | TORASAN Vault 構成 + 除外フォルダ設定 | 完了 |
| 4 | 4ゾーン構造 + ISS/IDEA 個別ノート展開 | 完了 |
| 5 | Dataview プラグイン + ダッシュボード3種 | 完了 |
| 6 | MOC + S17 サンプルセッションノート | 完了 |
| 7 | CLI 有効化 + PATH 設定 | 完了 |
| 8 | 自動起動ショートカット作成 | 完了 |

## 2. 発生した問題

1. **winget 不在**: `winget` コマンドが利用不可。直接 GitHub からインストーラをダウンロードして対応。
2. **CLI 有効化**: `app.json` の `enableCli: true` ではCLI有効化不可。Obsidian UI の設定から手動トグルが必要だった。
3. **PATH 登録エラー**: Obsidian 内蔵の「CLI を PATH に登録」が PowerShell エラーで失敗。手動スクリプト（`scripts/add_obsidian_path.ps1`）で対応。

## 3. 根本原因分析

- winget: Windows 環境に winget がプリインストールされていなかった（Microsoft Store 版 App Installer 未導入）
- CLI 有効化: Obsidian の CLI 設定はアプリ内部で IPC エンドポイントを起動する処理が必要で、JSON 設定だけでは不十分
- PATH 登録: Obsidian の PowerShell 実行環境と bash 経由の PowerShell で環境変数操作の互換性問題

## 4. 良かった点

1. **マルチエージェント調査が効果的**: Obsidian の API・プラグイン・図形機能・OS選定を並列調査し、短時間で包括的な情報を収集できた
2. **Dataview + frontmatter 設計**: project.json の既存データを活用し、Obsidian 側にシームレスに展開できた
3. **CLI 活用**: インストール後の検証を CLI 経由で自動化でき、ユーザーの「自律的に操作して」という期待に応えられた
4. **段階的導入**: MCP 連携やテンプレートは次回以降に分離し、今回は基盤構築に集中できた

## 5. 改善すべき点

| # | 問題 | 対策 | 状態 |
|---|------|------|------|
| 1 | winget 不在を事前に確認すべきだった | 事前に `which winget` で確認してからインストール方法を選択 | 済（今回対応済み） |
| 2 | ユーザーに手動操作を依頼しすぎた | PowerShell/CLI で可能な操作は自律的に実行する | 済（セッション中に改善） |
| 3 | Obsidian が改行コードを変換する | .gitattributes で制御するか、Obsidian 設定で対応 | 未 |

## 6. 教訓

1. Windows 環境では winget の有無を仮定せず、直接ダウンロードをフォールバックとして常に準備する
2. ユーザーは自律的な操作を強く期待している — 「できないの？」と聞かれる前に自分で実行する
3. Obsidian CLI (v1.12.4) は IPC ベースで Obsidian 本体が起動中でないと使えない
4. YAML frontmatter + Dataview の組み合わせは project.json のデータ管理を効果的に補完できる

## 7. 前回反省会フォローアップ

| S17 改善項目 | 状態 |
|------------|------|
| DrawingML ビジュアル品質改善 | 長期課題として継続 |
