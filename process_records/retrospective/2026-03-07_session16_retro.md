# セッション16 反省会議事録

日時: 2026-03-07
ブランチ: master
セッションスコープ: S15持越し6件全完了 + ISS-002完全修正 + ナレッジ拡充 + Documents整理

---

## 1. セッション成果サマリ

| # | コミット | 内容 |
|---|---------|------|
| 1 | ec966bc | S15持越し6件全完了 — SPICE評価・gateLog・S13retro・ISS-002/004・server型チェック |
| 2 | a055ed6 | ISS-002 完全修正 — cppcheck/flawfinder フルパス解決 |
| 3 | db2768a | PPTX高度図形・UML作図ナレッジ追加 |

### 詳細

- **SPICE 評価更新**: SYS.3/SWE.1-6 の PA 評価をエビデンスに基づき更新（SWE.1/2 → Level 1）
- **gateLog 6件追加**: PH-05〜PH-10 のゲート判定を遡及記録
- **S13 反省会再構成**: git log から Session 13 の議事録を復元
- **ISS-002 修正**: flawfinder/cppcheck の PATH 問題を KNOWN_TOOL_PATHS + findToolPath() で解決
- **ISS-004 修正**: PROCESS.md に G-09 セキュリティレビューゲート + §3.2.1 フェーズ別チェックリスト追加
- **server/ 型チェック**: tsconfig.json 修正、@types/ws 追加、fastify-websocket/chokidar 型修正
- **ナレッジ拡充**: pptx_advanced_shapes.md（377行）+ uml_diagramming.md（364行）作成
- **Documents 整理**: torasan-test-project 削除、project_registry.json 更新、OneDrive 残骸メモリ削除

変更規模: 11ファイル, +994行, -41行

## 2. 発生した問題

| # | 問題 | 影響 |
|---|------|------|
| 1 | ISS-002 初回修正が不完全 | flawfinder は `.exe` 拡張子が必要、cppcheck も PATH 外だった。1回目のコミット後に MISSING が残存 |
| 2 | torasan-test-project 空フォルダ削除不可 | Windows NTFS ロックにより空ディレクトリが残存（再起動で解消） |

## 3. 根本原因分析

**問題1: ISS-002 初回修正不完全**
- 原因: Windows で venv 内ツールは `.exe` 拡張子付きで配置されるが、`existsSync('flawfinder')` では `.exe` なしで検索していた。また cppcheck が `C:\Program Files\Cppcheck\` にあり、cmd.exe の PATH に含まれないことを見落とした
- 根本: プレビュー検証を実施したことで発見・修正できた（検証プロセスの有効性を確認）

**問題2: Windows ディレクトリロック**
- 原因: Windows のファイルシステムジャーナルまたはインデックスサービスがディレクトリをロック
- 影響: 軽微（空ディレクトリのみ残存、再起動で解消）

## 4. 良かった点

- S15 持越し 6 件を一括で効率的に処理できた
- プレビュー検証で ISS-002 の不完全修正を即座に発見・追加修正できた（検証プロセスが機能）
- ナレッジファイルの構造が体系的（python-pptx の 182 AutoShape、UML 5 図種のマッピング）
- Documents 整理で不要リポ・残骸メモリを削除し、構成を簡素化できた

## 5. 改善すべき点

| # | 問題 | 対策 | 状態 |
|---|------|------|------|
| 1 | Windows 固有のパス問題（.exe 拡張子）への対応が後手 | KNOWN_TOOL_PATHS パターンを他のツール（clang-tidy 等）にも展開検討。次回ツール追加時に適用 | 未 |
| 2 | コンテキスト消費が大きく compaction が発生 | 大量タスクの一括処理時は中間 compact を検討 | 未 |

## 6. 教訓

- **Windows venv ツールは `.exe` 拡張子必須**: `existsSync()` でチェックする際は `.exe` 付きと無し両方を試すべき
- **プレビュー検証は省略しない**: コード修正後の動作確認で初回修正の漏れを発見できた
- **ナレッジファイルは共有 + PJ固有の両方に配置**: `~/.claude/knowledge/` と `.claude/knowledge/` の二重配置で全プロジェクトから参照可能

## 7. 前回反省会フォローアップ

| # | S15 の未完了項目 | 今回の対応 | 状態 |
|---|----------------|-----------|------|
| 1 | SPICE WP の依存関係を単一変更で完結させられなかった → error_prevention.md にWP修正時チェック項目を追加 | 今セッションでは WP 修正なし。error_prevention.md への追記は未実施 | 未（次回持越し） |
