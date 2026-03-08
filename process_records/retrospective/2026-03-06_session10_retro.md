# セッション反省会議事録

日時: 2026-03-06
ブランチ: master
セッションスコープ: WSL エージェント連携システム構築（Phase 0〜2 + 認証）

---

## 1. セッション成果サマリ

| # | 作業内容 | コミット |
|---|---------|---------|
| 1 | WSL セットアップスクリプト作成 (wsl-setup.sh) | 6802b62 |
| 2 | Phase 1 スクリプトブリッジ (wsl-agent.sh + wsl-agent-runner.js) | 6802b62 |
| 3 | Phase 2 MCP Server 4ツール実装 (delegate_task, check_task_status, get_task_result, list_tasks) | 6802b62 |
| 4 | WSL Claude Code 認証完了 (setup-token + OAuth トークン) | 6802b62 |
| 5 | Phase 1 統合テスト成功 (Ubuntu 24.04, Claude 2.1.70) | - |
| 6 | 一時認証スクリプト7本作成→全削除（最終的に不要） | - |

## 2. 発生した問題

1. **WSL Claude Code の OAuth 認証が自動化困難** — `claude auth login` は TUI（@clack/prompts）で raw モードの対話的 UI を使用。tmux send-keys、pexpect、直接 PTY 書込み、ローカル HTTP コールバックいずれも失敗
2. **ローカル HTTP コールバック 400 エラー** — redirect_uri が `platform.claude.com` 固定のため、`127.0.0.1:PORT/callback` に直接送ると token exchange で redirect_uri 不一致
3. **コンテキスト消費が大量** — 認証トラブルシューティングで複数のアプローチを試行し、コンテキスト継続が必要に

## 3. 根本原因分析

- `claude auth login` は GUI ブラウザ + TUI プロンプトの対話を前提とした設計。WSL（ブラウザなし）+ 自動化という組み合わせは想定外
- `claude setup-token` コマンドの存在を発見するまでに時間がかかった。`--help` の出力を早期に確認すべきだった
- 試行錯誤の各アプローチ（pexpect、tmux、HTTP callback）を深追いしすぎた

## 4. 良かった点

- `claude setup-token` の発見と活用 — 1年有効の長期 OAuth トークンで認証問題を完全解決
- Phase 1 + Phase 2 の設計・実装自体はスムーズに完了
- ユーザーの「WSL内にブラウザ入れれば？」というヒントが `setup-token` 発見のきっかけに
- 一時スクリプト7本を全削除してリポをクリーンに保った

## 5. 改善すべき点

| # | 問題 | 対策 | 状態 |
|---|------|------|------|
| 1 | 認証方法の調査が不十分で試行錯誤に時間を費やした | CLIの全コマンド（`--help`）を最初に網羅的に確認する | 未 |
| 2 | 1つのアプローチに深入りしすぎた（pexpect, tmux等） | 3回失敗したら別アプローチに切り替えるルールを設ける | 未 |
| 3 | コンテキスト継続が必要になるほど長時間化 | 大きなタスクは事前に分割し、認証は別セッションで扱う | 未 |

## 6. 教訓

1. **`claude setup-token` は WSL/Docker/CI 環境での認証の正解** — `auth login` ではなく `setup-token` で長期トークンを発行し、`CLAUDE_CODE_OAUTH_TOKEN` 環境変数で注入する
2. **CLIツールの全コマンド一覧を最初に確認する** — `claude --help` の末尾にある `setup-token` コマンドを見逃していた
3. **OAuth PKCE フローの redirect_uri は変更不可** — ローカルコールバックサーバーへの直接送信は redirect_uri 不一致で必ず失敗する
4. **`claude auth login` の TUI は raw モード** — `-echo -icanon -isig` で tmux send-keys は届くが TUI が読み取らない。stdin 入力ではなくローカル HTTP サーバー or 専用プロンプト（setup-token）経由でのみコード受付

## 7. 前回反省会フォローアップ

前回（セッション9）の改善項目を確認:
- 前回は反省会議事録のみで改善項目なし → フォローアップ不要
