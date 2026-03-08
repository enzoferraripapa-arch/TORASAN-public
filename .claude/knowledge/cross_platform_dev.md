# Cross-Platform Development Environment Guide
## Windows + WSL + Linux Workflows

v1.0 / 2026-03-01

---

## 1. Windows Development Environment

### 1.1 Git Bash (MSYS2 / MINGW64)

| 項目 | 内容 |
|------|------|
| 実体 | MSYS2 ベースの POSIX エミュレーション層 |
| パス変換 | `/foo` が自動で `C:/Program Files/Git/foo` に変換される |
| 速度 | POSIX エミュレーション層のため native Windows より遅い |
| パッケージ | Git 関連ツールのみ同梱。rsync, zsh 等は別途導入が必要 |
| winpty | 非 MSYS2 プログラム (Python, Node, PHP) は `winpty` 経由で起動が必要 |

**主な制限事項:**

```
# パス自動変換の問題
$ echo /foo          # → C:/Program Files/Git/foo に変換される
$ MSYS_NO_PATHCONV=1 command  # 一時的にパス変換を無効化

# ファイル名制限
# Windows のファイル名規則に従う（引用符・特殊文字不可）

# インタラクティブ端末の問題
$ winpty python       # MinTTY との互換性のため winpty が必要
$ winpty node
```

### 1.2 PATH 管理

| 優先度 | PATH 種別 | 説明 |
|--------|----------|------|
| 1 | System PATH | 先に検索される（結合時に先頭に配置） |
| 2 | User PATH | System PATH の後に追加される |

**重要:** System PATH のエントリが先に検索されるため、同名コマンドが両方にある場合は System が勝つ。

```
# 確認方法（PowerShell）
$env:PATH -split ';'

# PowerToys Environment Variables で GUI 管理可能
# Profile > User > System の評価順序
```

**よくある問題と対策:**

| 問題 | 原因 | 対策 |
|------|------|------|
| 古い Python が起動する | System PATH に古いバージョン | System PATH の順序を修正 |
| `where python` で複数ヒット | 複数バージョンが PATH に存在 | 不要なエントリを削除 |
| Git Bash で PATH が異なる | MSYS2 が独自の PATH を構築 | `~/.bash_profile` で追記 |

### 1.3 winget によるツール導入

```powershell
# 基本操作
winget search <package>           # パッケージ検索
winget install <package>          # インストール
winget upgrade --all              # 全パッケージ更新
winget list                       # インストール済み一覧

# 開発ツール一括セットアップ例
winget install Git.Git
winget install Python.Python.3.12
winget install OpenJS.NodeJS.LTS
winget install LLVM.LLVM
winget install Kitware.CMake
winget install Microsoft.VisualStudioCode
```

> Windows 11 にはプリインストール。Windows 10 は Microsoft Store から App Installer を更新。リポジトリは 9,000+ パッケージ（2025年5月時点）。

### 1.4 Windows 共通の注意点

#### 改行コード (CRLF / LF)

| ファイル種別 | 必要な改行 | 理由 |
|-------------|-----------|------|
| `.sh`, `Dockerfile`, `.yml` | LF | シェル/Linux ツールが CRLF で壊れる |
| `.bat`, `.cmd`, `.ps1` | CRLF | Windows バッチが LF で動かない場合がある |
| `.sln`, `.csproj`, `.vcxproj` | CRLF | Visual Studio が LF で警告を出す |
| `.c`, `.h`, `.py`, `.js` | LF (推奨) | 現代の IDE は両対応 |

**推奨設定 (.gitattributes):**

```gitattributes
# デフォルト: LF で正規化
* text=auto eol=lf

# Windows 専用ファイル: CRLF 強制
*.bat    text eol=crlf
*.cmd    text eol=crlf
*.ps1    text eol=crlf
*.sln    text eol=crlf
*.csproj text eol=crlf
*.vcxproj text eol=crlf

# バイナリ: 変換しない
*.png  binary
*.jpg  binary
*.exe  binary
*.elf  binary
*.hex  binary
*.mot  binary
```

**git 設定:**

```bash
# 2025年の推奨: autocrlf=false + .gitattributes で管理
git config --global core.autocrlf false

# .gitattributes がリポジトリルールを優先するため
# 個人設定の autocrlf は不要
```

#### 大文字・小文字の区別

| 環境 | 大文字小文字 | 注意 |
|------|------------|------|
| Windows (NTFS) | 区別しない | `File.c` と `file.c` は同一ファイル |
| WSL (ext4) | 区別する | `File.c` と `file.c` は別ファイル |
| Git | 設定依存 | `core.ignorecase=true` (Windows デフォルト) |

```bash
# Git で大文字小文字の変更を検出させる
git config core.ignorecase false
# 注意: Windows ファイルシステム上では問題を引き起こす可能性あり
```

#### パス長制限 (MAX_PATH = 260 文字)

```powershell
# レジストリで長いパスを有効化
reg add "HKLM\SYSTEM\CurrentControlSet\Control\FileSystem" /v LongPathsEnabled /t REG_DWORD /d 1 /f

# Git でも有効化
git config --global core.longpaths true
```

> **制限:** Windows Explorer は 260 文字超を未サポート（Windows 11 でも同様）。`node_modules` の深いネストで頻発。

---

## 2. WSL (Windows Subsystem for Linux)

### 2.1 アーキテクチャ

| 項目 | WSL 1 | WSL 2 |
|------|-------|-------|
| カーネル | 変換層 | 実 Linux カーネル (VM) |
| システムコール互換性 | 部分的 | 完全 |
| Linux ファイルシステム性能 | 遅い | 高速 (ネイティブ近い) |
| Windows ファイルアクセス (/mnt/c) | 高速 | **遅い** (9P プロトコル) |
| メモリ | 共有 | 動的割当 |
| Docker | 非対応 | 対応 |
| USB デバイス | 対応 | usbipd-win 必要 |

### 2.2 ファイルシステム性能

**最重要ルール:** ファイル I/O の多い操作は、そのファイルが存在するファイルシステム上で実行する。

| 操作 | ファイル配置 | 性能 |
|------|------------|------|
| WSL で `npm install` | `/home/user/project/` | **高速** |
| WSL で `npm install` | `/mnt/c/Users/.../project/` | **極めて遅い** |
| Windows IDE でファイル編集 | `C:\Users\...\project\` | 高速 |
| Windows IDE で WSL ファイル編集 | `\\wsl$\Ubuntu\home\...` | やや遅い |

**推奨ワークフロー:**

```
開発対象          ファイル配置              ビルド環境
──────────────   ──────────────────────   ──────────
Web アプリ       WSL 内 (/home/user/)     WSL
組込みファーム    Windows (C:\dev\)        Windows (CC-RL等)
共有プロジェクト  Windows → VS Code Remote WSL でビルド
```

### 2.3 パス変換 (wslpath)

```bash
# WSL 内で使用
wslpath "C:\Users\{username}\Documents"    # → /mnt/c/Users/{username}/Documents
wslpath -w /home/user/project           # → \\wsl$\Ubuntu\home\user\project
wslpath -m /home/user/project           # → //wsl$/Ubuntu/home/user/project
wslpath -a relative/path                # 絶対パスで出力

# スクリプト内での活用例
WIN_PATH=$(wslpath -w "$PWD")
echo "Windows path: $WIN_PATH"
```

### 2.4 Windows / WSL 間のコマンド実行

```bash
# WSL から Windows コマンドを実行
cmd.exe /c "dir C:\\"
explorer.exe .                          # 現在のディレクトリを Explorer で開く
notepad.exe file.txt                    # Windows アプリで開く
powershell.exe -Command "Get-Process"

# Windows (PowerShell) から WSL コマンドを実行
wsl ls -la
wsl -- gcc -o main main.c
wsl -d Ubuntu -- bash -c "cd /home/user && make"

# 注意: .exe 拡張子が必要（WSL から Windows コマンド実行時）
```

### 2.5 ネットワーク

#### NAT モード (デフォルト)

```bash
# WSL の IP アドレスを確認
hostname -I                            # WSL 側
cat /etc/resolv.conf | grep nameserver # Windows 側 IP

# Windows → WSL: localhost でアクセス可能（最近のバージョン）
# 外部ネットワーク → WSL: ポートフォワーディングが必要
```

```powershell
# ポートフォワーディング設定 (PowerShell 管理者)
$wslIP = wsl hostname -I
netsh interface portproxy add v4tov4 `
  listenport=8080 listenaddress=0.0.0.0 `
  connectport=8080 connectaddress=$wslIP

# 注意: WSL の IP は再起動で変わる → スクリプト化推奨
```

#### ミラーモード (Windows 11 22H2+, 推奨)

```ini
# %USERPROFILE%\.wslconfig
[wsl2]
networkingMode=mirrored
```

> ミラーモードでは Windows と WSL が同一ネットワークインターフェースを共有。localhost で相互アクセス可能、IP 変更問題なし。

### 2.6 .wslconfig 推奨設定

```ini
# %USERPROFILE%\.wslconfig
[wsl2]
memory=8GB                    # メモリ上限
processors=4                  # CPU コア数
swap=4GB                      # スワップサイズ
networkingMode=mirrored       # ネットワークモード
autoProxy=true                # Windows プロキシ設定を継承
```

---

## 3. クロスプラットフォームツール管理

### 3.1 ツール利用可能性マトリクス

| ツール | Windows Native | Git Bash | WSL | 備考 |
|--------|:-------------:|:--------:|:---:|------|
| git | OK | OK | OK | 各環境で別インスタンス |
| gcc/g++ | MinGW/MSYS2 | MSYS2同梱 | OK | WSL は `apt install gcc` |
| clang/clang-tidy | OK (LLVM) | -- | OK | Windows は VS or LLVM インストーラ |
| cppcheck | OK (インストーラ) | -- | OK | `apt install cppcheck` |
| flawfinder | pip install | pip install | OK | Python 依存 |
| make | MSYS2/MinGW | 同梱(mingw32-make) | OK | Windows は `mingw32-make` |
| cmake | OK (winget) | -- | OK | 両方にインストール推奨 |
| python | OK (winget) | winpty必要 | OK | 環境を分けること |
| node.js | OK (winget/nvm-windows) | winpty必要 | OK (nvm) | WSL内に配置推奨 |
| arm-none-eabi-gcc | OK (ARM公式) | -- | OK (apt) | クロスコンパイラ |
| CC-RL (Renesas) | OK (専用) | -- | -- | **Windows のみ** |
| J-Link | OK (SEGGER) | -- | usbipd | USB パススルー必要 |

### 3.2 静的解析ツール

```bash
# === Windows (PowerShell) ===
winget install -e --id cppcheck.cppcheck
# clang-tidy: Visual Studio 2019+ に同梱、または LLVM 別途インストール

# === WSL / Linux ===
sudo apt update
sudo apt install cppcheck clang-tidy flawfinder

# === 実行例 ===
cppcheck --enable=all --std=c99 --suppress=missingInclude src/
clang-tidy src/*.c -- -std=c99 -I./include
flawfinder src/
```

> **注意:** VS の clang-tidy は CMake + WSL 構成では未対応（Windows ターゲットのみ）。WSL 上で直接実行を推奨。

### 3.3 Python 環境

| 項目 | Windows Python | WSL Python |
|------|---------------|------------|
| インストール | `winget install Python.Python.3.12` | `sudo apt install python3` |
| パッケージ管理 | pip / uv | pip / uv |
| 仮想環境 | `py -m venv .venv` | `python3 -m venv .venv` |
| 制約 | POSIX 依存パッケージが失敗する場合あり | Windows API 非対応 |
| 推奨用途 | Windows ツール連携, GUI アプリ | サーバー, CI/CD, Linux 依存パッケージ |

```bash
# 環境を混在させないこと
# Windows の Python を WSL から呼ばない（逆も同様）
# 各環境に独立した venv を作成する
```

### 3.4 Node.js 環境

| 項目 | Windows Node | WSL Node |
|------|-------------|----------|
| バージョン管理 | nvm-windows | nvm (POSIX) |
| npm install 速度 | 普通 | **高速** (WSL ファイルシステム上) |
| ネイティブモジュール | MSVC ビルドツール必要 | `build-essential` で対応 |
| 推奨用途 | Electron, Windows ネイティブ連携 | Web 開発全般 |

```bash
# WSL 内でのセットアップ (推奨)
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.1/install.sh | bash
nvm install --lts
nvm use --lts
```

### 3.5 シェルスクリプト移植性

| 機能 | Git Bash | WSL | ネイティブ Linux |
|------|:--------:|:---:|:---------------:|
| bash 4+ 機能 | 一部制限 | OK | OK |
| `sed -i` | GNU sed 同梱 | OK | OK |
| `find -exec` | OK | OK | OK |
| `realpath` | OK | OK | OK |
| `mktemp` | OK | OK | OK |
| symlink | 制限あり | OK (/mnt/c上は制限) | OK |
| `chmod +x` | 無視される | OK (ext4上) | OK |
| `/dev/null` | OK (変換) | OK | OK |
| プロセス置換 `<()` | OK | OK | OK |
| シグナル (trap) | 制限あり | OK | OK |

**移植性のためのガイドライン:**

```bash
#!/usr/bin/env bash
# ↑ /bin/bash ではなく env 経由で起動

# OS 判定
case "$(uname -s)" in
  MINGW*|MSYS*) OS="windows-gitbash" ;;
  Linux)
    if grep -qi microsoft /proc/version 2>/dev/null; then
      OS="wsl"
    else
      OS="linux"
    fi
    ;;
esac

# パス区切り文字
if [[ "$OS" == "windows-gitbash" ]]; then
  SEP="\\"
else
  SEP="/"
fi
```

---

## 4. 組込み開発クロスプラットフォーム

### 4.1 クロスコンパイラ

| コンパイラ | Windows | WSL / Linux | 用途 |
|-----------|---------|-------------|------|
| arm-none-eabi-gcc | ARM 公式 DL | `sudo apt install gcc-arm-none-eabi` | ARM Cortex-M |
| CC-RL (Renesas) | e2 studio 同梱 | **非対応** | RL78 |
| CC-RX (Renesas) | e2 studio 同梱 | **非対応** | RX |
| CC-RH (Renesas) | e2 studio 同梱 | **非対応** | RH850 |
| SDCC | OK | OK | 8051, Z80 等 |

```bash
# WSL での ARM ツールチェーンインストール
sudo apt update
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi

# 確認
arm-none-eabi-gcc --version
```

### 4.2 Renesas ツールチェーン (Windows 専用)

```
インストール先: C:\Renesas\CS+\CC\CC-RL\Vx.xx.xx\
IDE: e2 studio または CS+
ライセンス: MyRenesas アカウントで取得
MISRA-C チェッカー: CC-RL に標準搭載

# PATH 追加 (PowerShell)
$env:PATH += ";C:\Renesas\CS+\CC\CC-RL\Vx.xx.xx\bin"

# コマンドラインビルド
ccrl -cpu=S3 -o output.abs src/main.c
```

### 4.3 デバッグツールと USB パススルー

| デバッガ | Windows | WSL (usbipd) | 備考 |
|---------|---------|:------------:|------|
| J-Link (SEGGER) | OK | 可能だが不安定 | 再接続時に切断される場合あり |
| E2 エミュレータ (Renesas) | OK | 非推奨 | e2 studio 連携が Windows 前提 |
| ST-Link | OK | 可能 | OpenOCD 経由 |
| DAPLink | OK | 可能 | pyOCD 経由 |

```powershell
# usbipd-win のインストール
winget install usbipd

# USB デバイス一覧
usbipd list

# WSL に USB デバイスを接続
usbipd bind --busid <BUSID>
usbipd attach --wsl --busid <BUSID>

# WSL 側で確認
lsusb
```

> **推奨:** Renesas デバッグ (E2/E2 Lite) は Windows ネイティブで実施。ARM Cortex-M 系 (J-Link, ST-Link) は WSL 経由も可能だが、安定性を重視するなら Windows 推奨。

### 4.4 Make / CMake クロスプラットフォームビルド

**CMake ツールチェーンファイル例 (ARM Cortex-M):**

```cmake
# toolchain-arm-none-eabi.cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS_INIT   "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_CXX_FLAGS_INIT "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")
```

**CMakePresets.json (クロスプラットフォーム対応):**

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "arm-debug",
      "displayName": "ARM Cortex-M Debug",
      "generator": "Ninja",
      "toolchainFile": "${sourceDir}/cmake/toolchain-arm-none-eabi.cmake",
      "binaryDir": "${sourceDir}/build/arm-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "host-debug",
      "displayName": "Host Debug (Unit Test)",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/host-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    }
  ]
}
```

```bash
# ビルド実行
cmake --preset arm-debug
cmake --build --preset arm-debug

# ホストテスト
cmake --preset host-debug
cmake --build --preset host-debug
ctest --preset host-debug
```

---

## 5. トラブルシューティング

### 5.1 "command not found" 解決パターン

| 環境 | 診断コマンド | 対策 |
|------|------------|------|
| Git Bash | `which <cmd>` / `type <cmd>` | PATH 確認、MSYS2 パッケージ確認 |
| WSL | `which <cmd>` / `command -v <cmd>` | `apt install` で導入 |
| PowerShell | `Get-Command <cmd>` / `where.exe <cmd>` | winget / PATH 追加 |

```bash
# よくある原因と対策
# 1. PATH が通っていない
echo $PATH | tr ':' '\n' | grep -i "<expected_dir>"

# 2. Windows コマンドを WSL から実行しようとしている
which gcc        # WSL の gcc
/mnt/c/..../gcc  # Windows の gcc (非推奨)

# 3. WSL で Windows 用バイナリを実行
file /mnt/c/path/to/binary.exe  # PE32 executable と表示される
# → .exe 拡張子をつけて実行: /mnt/c/path/to/binary.exe

# 4. Git Bash でパスが変換されてしまう
MSYS_NO_PATHCONV=1 docker run -v /c/project:/app ...
```

### 5.2 権限 (Permission) の問題

| 環境 | chmod の動作 | 対策 |
|------|-------------|------|
| Git Bash | NTFS 上では無視される | `git update-index --chmod=+x` で Git に記録 |
| WSL (ext4) | 完全動作 | 通常通り `chmod +x` |
| WSL (/mnt/c) | メタデータ有効時のみ動作 | `/etc/wsl.conf` で設定 |

```ini
# /etc/wsl.conf (WSL 内)
[automount]
enabled = true
options = "metadata,umask=22,fmask=11"
# metadata: Linux パーミッションメタデータを有効化
```

```bash
# Git でファイルの実行権限を設定 (Windows)
git update-index --chmod=+x scripts/build.sh
git commit -m "fix: スクリプトに実行権限を付与"
```

### 5.3 改行コードの問題

```bash
# 症状: シェルスクリプトが "bad interpreter" エラー
$ ./script.sh
-bash: ./script.sh: /bin/bash^M: bad interpreter

# 診断
file script.sh       # "with CRLF line terminators" と表示
cat -A script.sh     # 行末に ^M が見える

# 修正
dos2unix script.sh   # dos2unix がない場合: sed -i 's/\r$//' script.sh

# 根本対策: .gitattributes の設定 (セクション 1.4 参照)
```

### 5.4 エンコーディングの問題

| エンコーディング | 用途 | 問題点 |
|----------------|------|--------|
| UTF-8 (BOM なし) | **推奨** | 最も互換性が高い |
| UTF-8 (BOM あり) | Windows 旧アプリ | BOM がコンパイラエラーの原因になる |
| Shift-JIS (CP932) | 日本語レガシー | Git diff が文字化け、他 OS で表示不可 |
| EUC-JP | Unix レガシー | 現代では非推奨 |

**Git でのエンコーディング管理:**

```gitattributes
# Shift-JIS ファイルを Git 内部では UTF-8 で保持し、
# チェックアウト時に Shift-JIS に戻す
*.c working-tree-encoding=Shift_JIS
*.h working-tree-encoding=Shift_JIS
```

```bash
# Git のログ表示エンコーディング
git config --global i18n.logOutputEncoding utf-8
git config --global i18n.commitEncoding utf-8

# Git Bash のロケール確認
locale  # LANG=C.UTF-8 であることを確認

# Windows Terminal は UTF-8 推奨
# Settings → Profiles → Defaults → Starting directory 確認
```

**レガシーコード移行の推奨手順:**

```bash
# 1. 現在のエンコーディングを確認
file --mime-encoding src/*.c

# 2. Shift-JIS → UTF-8 (BOM なし) に変換
nkf -w --overwrite src/*.c
# または
iconv -f SHIFT_JIS -t UTF-8 src/old.c > src/new.c

# 3. .gitattributes を更新してリポジトリ全体を正規化
# 4. エディタ設定 (.editorconfig) で統一
```

**.editorconfig 推奨設定:**

```ini
# .editorconfig
root = true

[*]
charset = utf-8
end_of_line = lf
insert_final_newline = true
trim_trailing_whitespace = true
indent_style = space
indent_size = 4

[*.{bat,cmd,ps1}]
end_of_line = crlf

[Makefile]
indent_style = tab
```

### 5.5 クイックリファレンス: 環境判定スクリプト

```bash
#!/usr/bin/env bash
# detect_env.sh - 実行環境の診断

echo "=== 実行環境診断 ==="
echo "OS: $(uname -s) / $(uname -r)"
echo "Shell: $SHELL ($BASH_VERSION)"
echo "User: $(whoami)"
echo "PWD: $PWD"

# 環境判定
case "$(uname -s)" in
  MINGW*|MSYS*)
    echo "環境: Git Bash (MSYS2/MINGW64)"
    echo "Git: $(git --version)"
    echo "注意: chmod は NTFS 上で無視されます"
    ;;
  Linux)
    if grep -qi microsoft /proc/version 2>/dev/null; then
      echo "環境: WSL 2"
      echo "Distro: $(lsb_release -ds 2>/dev/null || cat /etc/os-release | head -1)"
      echo "Windows ドライブ: $(ls /mnt/ 2>/dev/null)"
    else
      echo "環境: ネイティブ Linux"
      echo "Distro: $(lsb_release -ds 2>/dev/null)"
    fi
    ;;
esac

echo ""
echo "=== ツール確認 ==="
for cmd in git gcc g++ arm-none-eabi-gcc make cmake python3 node cppcheck clang-tidy; do
  if command -v "$cmd" &>/dev/null; then
    echo "  [OK] $cmd: $(command -v "$cmd")"
  else
    echo "  [--] $cmd: not found"
  fi
done

echo ""
echo "=== エンコーディング ==="
echo "LANG: $LANG"
echo "LC_ALL: ${LC_ALL:-unset}"
locale 2>/dev/null | head -3
```

---

## 6. 推奨ワークフロー一覧

### 6.1 用途別の推奨環境

| 用途 | 推奨環境 | ファイル配置 | 理由 |
|------|---------|------------|------|
| Renesas RL78 開発 | Windows (e2 studio) | Windows (C:\dev\) | CC-RL が Windows 専用 |
| ARM Cortex-M 開発 | WSL or Windows | どちらでも可 | GCC が両対応 |
| 静的解析 (cppcheck) | WSL 推奨 | ソースと同じ側 | Linux 版の方が安定 |
| ユニットテスト | WSL 推奨 | WSL 内 | GCC + CUnit/Unity が容易 |
| CI/CD パイプライン | WSL / Docker | WSL 内 | Linux コンテナと同一環境 |
| ドキュメント作成 | どちらでも | Windows 推奨 | Office/PDF ツール連携 |
| Git 操作 | どちらでも | ソースと同じ側 | 環境跨ぎ操作は遅い |

### 6.2 プロジェクト初期設定チェックリスト

```
[ ] .gitattributes を配置（改行コード・バイナリ定義）
[ ] .editorconfig を配置（エンコーディング・インデント統一）
[ ] core.autocrlf=false を設定
[ ] core.longpaths=true を設定（Windows）
[ ] LongPathsEnabled=1 を設定（Windows レジストリ）
[ ] WSL .wslconfig でメモリ・ネットワーク設定
[ ] WSL /etc/wsl.conf で automount metadata 有効化
[ ] CMakePresets.json でクロスプラットフォームビルド定義
[ ] CI 環境と同一のツールチェーンバージョンを使用
```

---

*CROSS_PLATFORM_DEV.md v1.0 / 作成日: 2026-03-01*
*Sources: Microsoft Learn, GitHub, ARM Developer, Renesas, SEGGER*
