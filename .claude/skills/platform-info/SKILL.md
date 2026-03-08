---
name: platform-info
description: "Detects and reports platform, OS, shell, and WSL status."
argument-hint: "[mode: detect|paths|wsl|all]"
disable-model-invocation: true
---
# /platform-info — プラットフォーム情報

実行環境（OS、シェル、WSL、アーキテクチャ）を検出し、環境依存の注意点を報告する。

引数: $ARGUMENTS（オプション。"detect" = 環境検出 | "paths" = パス変換ヘルパー | "wsl" = WSL詳細 | "all" → 省略時は "detect"）

## Prerequisites（前提条件）

本スキル実行前に以下を確認:

| # | 条件 | 確認方法 | 未充足時 |
|---|------|---------|---------|
| 1 | なし（任意の環境で実行可能） | — | — |

## 手順

### detect モード — 環境検出

以下のコマンドで実行環境を特定:

```bash
# OS 検出
uname -s          # Linux, MINGW64_NT-10.0, Darwin
uname -r          # カーネルバージョン
uname -m          # x86_64, aarch64

# シェル検出
echo $SHELL       # /bin/bash, /bin/zsh
echo $0           # 実行中シェル
$SHELL --version

# Windows 固有
echo $MSYSTEM     # MINGW64, UCRT64 (Git Bash)
cmd.exe /c ver 2>/dev/null    # Windows バージョン

# WSL 検出
cat /proc/version 2>/dev/null | grep -i microsoft
wsl.exe --version 2>/dev/null
```

#### 出力フォーマット

```
=== プラットフォーム情報 ===

[OS]
  プラットフォーム: Windows 11 / macOS / Linux (Ubuntu)
  アーキテクチャ: x86_64
  カーネル: {version}

[シェル環境]
  シェル: bash {version}
  実行コンテキスト: Git Bash / WSL / ネイティブ Linux / Terminal.app
  MSYSTEM: {MINGW64 / なし}

[WSL]
  WSL利用可能: はい/いいえ
  WSLバージョン: {1/2}
  デフォルトディストリビューション: {Ubuntu-24.04}
  WSL内ツール: {利用可能ツールリスト}

[パス形式]
  ワーキングディレクトリ: {pwd}
  Windows形式: C:\Users\...
  Unix形式: /c/Users/...
  WSL形式: /mnt/c/Users/...

[環境依存の注意点]
  - {注意点1}
  - {注意点2}
```

### paths モード — パス変換ヘルパー

Windows / Unix / WSL 間のパス変換ルールを表示:

```
=== パス変換ガイド ===

[変換ルール]
| 形式 | パターン | 例 |
|------|---------|-----|
| Windows | C:\Users\{username}\... | バックスラッシュ区切り |
| Git Bash | /c/Users/{username}/... | /c/ = C:\ |
| WSL | /mnt/c/Users/{username}/... | /mnt/c/ = C:\ |
| Cygwin | /cygdrive/c/Users/... | /cygdrive/c/ = C:\ |

[現在の環境でのパス]
  プロジェクトルート:
    Windows: C:\Users\<USERNAME>\Documents\TORASAN
    Unix: /c/Users/<USERNAME>/Documents/TORASAN
    WSL: /mnt/c/Users/<USERNAME>/Documents/TORASAN

[よくある問題]
  - スペースを含むパスは必ずダブルクォートで囲む
  - Git Bash では `//` で始まるパスは UNC パスと解釈される
  - WSL から Windows ツール呼出: `cmd.exe /c {command}`
  - Windows から WSL ツール呼出: `wsl {command}`
```

### wsl モード — WSL 詳細

WSL が利用可能な場合の詳細情報:

```
=== WSL 環境詳細 ===

[ディストリビューション]
| 名前 | バージョン | 状態 | デフォルト |
|------|----------|------|---------|
| Ubuntu-24.04 | 2 | Running | ✓ |

[WSL内ツール]
| ツール | バージョン | 用途 |
|--------|----------|------|
| gcc | {ver} | C コンパイル |
| cppcheck | {ver} | 静的解析 |
| make | {ver} | ビルド |

[ファイルシステム性能]
  Windows → WSL (/mnt/c/): 低速（クロスFS）
  WSL ネイティブ (/home/): 高速
  推奨: ビルドはWSLネイティブ、ソースはWindows側

[WSL活用提案]
  - cppcheck が Windows で使えない場合: wsl cppcheck {file}
  - gcc が必要な場合: wsl gcc -o {out} {src}
  - 両環境から同じソースにアクセス可能
```

## 環境依存の注意点テーブル

| 環境 | 注意点 |
|------|--------|
| Git Bash (Windows) | `find` は Windows 版が優先される場合あり → `/usr/bin/find` を明示 |
| Git Bash (Windows) | `python` が見つからない場合 `python3` or `py` を試行 |
| WSL | /mnt/c/ 経由のファイルアクセスは I/O が遅い |
| WSL | Windows の .exe は WSL から直接実行可能 |
| macOS | `sed` は BSD 版（GNU と構文が異なる） |
| Linux | ツールが apt/yum/pacman のどれかに依存 |

## 関連スキル
- /env-check — ツールの存在・バージョン検証（本スキルは環境情報、env-check はツール検証）
- /static-analysis — WSL 経由での静的解析ツール実行に本スキルの情報が有用
- /worktree-cleanup — ワークツリーのパスは環境依存

## Error Handling（エラー時の対応）

| 障害 | 検出方法 | 復旧手順 |
|------|---------|---------|
| データソース読み取り失敗 | Read / Glob エラー | 対象不在を報告。該当項目を「N/A」として表示 |
| 分析対象データ不足 | 必要キーの不在 | 不足項目を明示し、データ投入を促す |

## 注意事項
- 本スキルは情報表示のみ（設定変更は行わない）
- WSL 内のツールを使う場合は `wsl` コマンド経由で呼び出す
- パス変換は手動実装せず、`wslpath` コマンドの利用を推奨
- 環境検出結果はセッション内でキャッシュ可能（変化しないため）

## ナレッジベース参照

本スキル実行時に以下のナレッジベースを参照して品質を向上:
- `~/.claude/knowledge/cross_platform_dev.md` — Windows/WSL/Linux 環境詳細、パス変換、ファイルシステム性能、トラブルシューティング

※ `~/.claude/knowledge/` = 共有ナレッジ（全PJ共通）、`.claude/knowledge/` = PJ固有ナレッジ
