---
name: env-check
description: "Verifies development environment tools and configurations."
argument-hint: "[mode: tools|path|config|fix|all (default: all)]"
disable-model-invocation: true
---
# /env-check — 開発環境検証

開発に必要なツール・設定の存在・バージョン・PATH を一括検証し、不足を報告する。

引数: $ARGUMENTS（オプション。上記 argument-hint 参照。省略時は "all"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | なし（本スキル自体が環境前提を検証する） | — | — |

## 検証対象ツール

### 必須ツール（CRITICAL — なければ開発不可）

| ツール | 用途 | 検証コマンド |
|--------|------|------------|
| git | バージョン管理 | `git --version` |
| bash | シェル実行 | `bash --version` |
| node | app/ ビルド・ツール実行 | `node --version` |
| npm | パッケージ管理 | `npm --version` |

### 推奨ツール（WARNING — なければ一部機能制限）

| ツール | 用途 | 検証コマンド |
|--------|------|------------|
| gcc / cc1 | C コンパイル・カバレッジ | `gcc --version` |
| cppcheck | MISRA C 静的解析 | `cppcheck --version` |
| clang-tidy | 追加静的解析 | `clang-tidy --version` |
| flawfinder | セキュリティ解析 | `flawfinder --version` |
| gcov | コードカバレッジ | `gcov --version` |
| python3 | スクリプト・ツール | `python3 --version` or `python --version` |

### オプションツール（INFO — あると便利）

| ツール | 用途 | 検証コマンド |
|--------|------|------------|
| docker | コンテナビルド環境 | `docker --version` |
| wsl | Windows Subsystem for Linux | `wsl --version` |
| make | ビルド自動化 | `make --version` |

## 手順

### tools モード — ツール存在確認

1. 各ツールの検証コマンドを実行
2. 存在・バージョン・パスを記録
3. 結果をテーブル表示:

```
=== 開発ツール検証 ===
| ツール | 状態 | バージョン | パス |
|--------|------|----------|------|
| git | ✓ OK | 2.44.0 | /usr/bin/git |
| cppcheck | ✗ MISSING | — | — |
| gcc | ✓ OK | 13.2.0 | /usr/bin/gcc |

[サマリ]
必須: {ok}/{total} OK
推奨: {ok}/{total} OK
不足ツール: {list}
```

### path モード — PATH 検証

Windows 環境特有の問題を検出:

1. `$PATH` の内容を表示（重複・不要エントリ検出）
2. ツールの実体パスを `which` / `where` で確認
3. Windows ネイティブ vs WSL vs Git Bash の区別
4. よくある問題パターンの検出:
   - cppcheck が `C:\Program Files\Cppcheck` にあるが PATH 未登録
   - Python が複数バージョン存在
   - WSL 内ツールと Windows ツールの競合

```
=== PATH 検証 ===
シェル: {bash/zsh/cmd}
PATH エントリ数: {n}
重複エントリ: {n}件
Windows/Unix パス混在: {あり/なし}

[ツールパス解決]
| ツール | 解決先 | 環境 |
|--------|-------|------|
| git | /c/Program Files/Git/bin/git | Git for Windows |
| gcc | /usr/bin/gcc | WSL |
| cppcheck | (未解決) | — |
```

### config モード — プロジェクト設定検証

1. `.gitignore` の完全性チェック
2. `.claude/settings.local.json` のパーミッション確認
3. `app/node_modules/` の存在（`npm install` 必要性）
4. CLAUDE.md と PROCESS.md のバージョン整合
5. project.json の構造検証

```
=== プロジェクト設定検証 ===
| 項目 | 状態 | 詳細 |
|------|------|------|
| .gitignore | ✓ OK | lock/docx/claude 除外済 |
| node_modules | ✗ MISSING | npm install 必要 |
| settings | ✓ OK | パーミッション設定済 |
| CLAUDE.md | ✓ 存在 | PROCESS.md と整合（バージョンはファイル末尾を参照） |
```

### fix モード — 自動修正提案

不足ツールに対するインストール提案を環境別に表示:

```
=== 修正提案 ===

[Windows (native)]
  cppcheck: winget install cppcheck
  clang-tidy: winget install LLVM
  flawfinder: pip install flawfinder

[WSL (Ubuntu)]
  cppcheck: sudo apt install cppcheck
  clang-tidy: sudo apt install clang-tidy
  flawfinder: pip3 install flawfinder

[npm (プロジェクトローカル)]
  app依存: cd app && npm install

提案を実行しますか？ (y/N)
```

## 出力

検証結果は表示のみ（ファイルには記録しない）。
fix モードの実行はユーザー確認後に実施。

## 関連スキル
- /platform-info — 環境の詳細情報（OS、アーキテクチャ、WSL検出）
- /static-analysis — cppcheck/clang-tidy の実行前に本スキルで存在確認推奨
- /health-check — プロジェクト健全性チェック（本スキルは環境側をカバー）

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| ツール検出コマンド自体の失敗 | `which` / `command -v` エラー | 該当ツールを「不明」として報告し、次のツールへ続行 |
| バージョン文字列パース失敗 | 想定外の出力形式 | 生出力を表示し手動確認を促す |
| config モードで project.json 不在 | Read エラー | プロジェクト設定検証をスキップし、ツール検証のみ実行 |

## 注意事項
- Windows 環境では `which` の代わりに `command -v` を使用
- WSL ツールと Windows ネイティブツールが混在する場合は両方報告
- インストールコマンドの実行はユーザー確認必須（自動実行しない）
- PATH の変更は永続化方法（.bashrc, .profile, 環境変数）もあわせて提案

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `~/.claude/knowledge/cross_platform_dev.md` — ツール利用可能性マトリクス、winget コマンド、WSL ツール導入手順
- `~/.claude/knowledge/claude_code_ops.md` — settings.local.json 構造、パーミッション設定

※ `~/.claude/knowledge/` = 共有ナレッジ（全PJ共通）、`.claude/knowledge/` = PJ固有ナレッジ
