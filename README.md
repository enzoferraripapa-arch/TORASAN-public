# TORASAN

**T**oolkit for **O**rganized **R**isk **A**nalysis and **S**afety **A**ssurance **N**avigation

A process framework and knowledge toolkit for AI-assisted functional safety development.
Built on ISO 26262, IEC 61508, IEC 60730, Automotive SPICE, and MISRA-C:2012.

> **Vision**: To preserve and democratize functional safety engineering knowledge
> before it is lost to the global labor shortage crisis.

[日本語版は下にあります / Japanese version below](#japanese)

---

## What is this?

TORASAN is a collection of:

- **41 Skills** (`.claude/skills/`) -- Structured prompts for Claude Code that encode domain expertise in functional safety, FMEA, HARA, SRS generation, test design, SPICE assessment, and more
- **22 Knowledge modules** (`.claude/knowledge/`) -- Reference material covering ISO 26262, IEC 60730, MISRA-C, Automotive SPICE, safety diagnostics, and BLDC motor control
- **Process definition** (`PROCESS.md`) -- A complete V-model process mapped to Automotive SPICE process areas (MAN.3, SYS.1-5, SWE.1-6, SUP.1/8/9/10)
- **Working example** -- A full BLDC motor controller (IEC 60730 Class B) with safety requirements, architecture, source code, unit tests, FMEA, traceability, and SPICE records

This is not a product. This is a process toolkit. You bring the product; TORASAN provides the safety engineering structure.

## Who is this for?

- Embedded safety engineers dealing with ISO 26262 / IEC 61508 / IEC 60730
- Engineers at small/mid-size suppliers where "the safety person" is one person
- Anyone exploring AI-assisted development for safety-critical systems
- Automotive SPICE practitioners who want structured process records

## Quick Start

1. Clone this repo
2. Install [Claude Code](https://docs.anthropic.com/en/docs/claude-code)
3. Copy shared skills to your global config:
   ```bash
   ./install.sh
   ```
4. Open your project, run skills:
   ```
   /session start
   /execute-phase PH-01
   ```

## Repository Structure

```
TORASAN/
├── .claude/skills/       41 skills (14 general + 27 domain)
├── .claude/knowledge/    22 knowledge modules
├── PROCESS.md            V-model process definition (ISO 26262 + SPICE)
├── project.json          Project configuration (Single Source of Truth)
├── docs/                 Safety plan, HARA, FSC, SRS, test specs, ...
├── src/                  BLDC motor controller (C99, MISRA-C compliant)
├── test/                 Unit / Integration / Qualification tests (Unity)
├── process_records/      SPICE process evidence (18 processes)
├── scripts/              Automation (doc gen, SPICE improvement, WSL bridge)
├── templates/            Project templates
├── platforms/            MCU platform definitions
└── install.sh            Deploy shared skills to ~/.claude/
```

## Skills Overview

| Category | Count | Examples |
|----------|-------|---------|
| Safety Analysis | 7 | safety-concept, fmea, safety-diag, safety-verify, srs-generate |
| Development | 6 | sw-design, system-design, driver-gen, mcu-config, motor-control |
| Testing | 4 | test-design, test-coverage, systest-design, static-analysis |
| Process | 6 | execute-phase, assess-spice, trace, validate, update-record |
| Management | 8 | session, health-check, skill-manage, skill-evolve, repo-manage |
| Documentation | 4 | generate-docs, md2pptx, ingest, manage-tbd |
| Platform | 6 | platform-info, env-check, config-audit, worktree-cleanup |

## The Working Example

The included BLDC motor controller demonstrates a complete IEC 60730 Class B development:

- 15-phase V-model execution (PH-01 through PH-15)
- Safety goals, HARA, FMEA, fault tree analysis
- Software safety requirements (SR-001 to SR-018)
- Architecture with safety monitoring, failsafe, diagnostic manager
- Unit + integration + qualification tests
- Traceability matrix
- SPICE Level 2 achieved on 14/16 processes

## AI-Assisted Development

This entire framework was developed with Claude Code (Anthropic).
That is the point. Safety engineering knowledge can be encoded as AI skills
and reused across projects.

Every skill follows a consistent structure:
- Prerequisites and error handling
- Step-by-step execution procedure
- Output format specification
- Cross-references to project.json and process records

## License

GPLv3. See [LICENSE](LICENSE).

Take it. Use it. Improve it. Share it back.

## Disclaimer

See [DISCLAIMER.md](DISCLAIMER.md). TORASAN does not certify products.
All safety decisions are yours.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

Good first issues:
- Add IEC 62304 (medical device) safety skills
- Add EN ISO 13849-1 (machinery) safety patterns
- Translate README to German / Korean / Chinese
- Add STM32 HAL platform definition
- Improve FMEA templates for your domain

## About

Built by a Japanese embedded systems engineer with functional safety experience.
Not a corporation. Not a startup. Just an engineer who thinks this knowledge
should not be locked behind expensive consulting fees.

If you improve it -- share it back.

---

<a id="japanese"></a>

# 日本語 / Japanese

AI を活用した機能安全開発のためのプロセスフレームワーク＆ナレッジツールキット。
ISO 26262、IEC 61508、IEC 60730、Automotive SPICE、MISRA-C:2012 に対応。

> **ビジョン**: 世界的な人材不足で失われつつある機能安全エンジニアリングの知識を、
> 保存し、民主化すること。

---

## これは何？

TORASAN は以下の集合体です:

- **41 スキル** (`.claude/skills/`) -- 機能安全、FMEA、HARA、SRS 生成、テスト設計、SPICE 評価などのドメイン知識を構造化プロンプトとしてエンコードした Claude Code 用スキル
- **22 ナレッジモジュール** (`.claude/knowledge/`) -- ISO 26262、IEC 60730、MISRA-C、Automotive SPICE、安全診断、BLDC モーター制御をカバーするリファレンス資料
- **プロセス定義** (`PROCESS.md`) -- Automotive SPICE プロセス領域 (MAN.3, SYS.1-5, SWE.1-6, SUP.1/8/9/10) にマッピングした完全な V モデルプロセス
- **動作する実例** -- 安全要求、アーキテクチャ、ソースコード、ユニットテスト、FMEA、トレーサビリティ、SPICE 記録を備えた BLDC モーターコントローラ (IEC 60730 Class B)

これは製品ではありません。プロセスツールキットです。製品はあなたが持ち込む。TORASAN は安全エンジニアリングの構造を提供します。

## 誰のため？

- ISO 26262 / IEC 61508 / IEC 60730 に取り組む組み込み安全エンジニア
- 「安全担当」が一人しかいない中小サプライヤーのエンジニア
- 安全重要システムの AI 支援開発を探求している人
- 構造化されたプロセス記録が欲しい Automotive SPICE 実践者

## クイックスタート

1. このリポをクローン
2. [Claude Code](https://docs.anthropic.com/en/docs/claude-code) をインストール
3. 共有スキルをグローバル設定にコピー:
   ```bash
   ./install.sh
   ```
4. プロジェクトを開いてスキルを実行:
   ```
   /session start
   /execute-phase PH-01
   ```

## リポジトリ構成

```
TORASAN/
├── .claude/skills/       41 スキル (汎用 14 + ドメイン 27)
├── .claude/knowledge/    22 ナレッジモジュール
├── PROCESS.md            V モデルプロセス定義 (ISO 26262 + SPICE)
├── project.json          プロジェクト設定 (Single Source of Truth)
├── docs/                 安全計画, HARA, FSC, SRS, テスト仕様, ...
├── src/                  BLDC モーターコントローラ (C99, MISRA-C 準拠)
├── test/                 ユニット / 統合 / 適格性テスト (Unity)
├── process_records/      SPICE プロセスエビデンス (18 プロセス)
├── scripts/              自動化 (ドキュメント生成, SPICE 改善, WSL ブリッジ)
├── templates/            プロジェクトテンプレート
├── platforms/            MCU プラットフォーム定義
└── install.sh            共有スキルを ~/.claude/ にデプロイ
```

## スキル一覧

| カテゴリ | 数 | 例 |
|----------|----|----|
| 安全分析 | 7 | safety-concept, fmea, safety-diag, safety-verify, srs-generate |
| 開発 | 6 | sw-design, system-design, driver-gen, mcu-config, motor-control |
| テスト | 4 | test-design, test-coverage, systest-design, static-analysis |
| プロセス | 6 | execute-phase, assess-spice, trace, validate, update-record |
| 管理 | 8 | session, health-check, skill-manage, skill-evolve, repo-manage |
| ドキュメント | 4 | generate-docs, md2pptx, ingest, manage-tbd |
| プラットフォーム | 6 | platform-info, env-check, config-audit, worktree-cleanup |

## 動作する実例

同梱の BLDC モーターコントローラは、IEC 60730 Class B の完全な開発を実演しています:

- 15 フェーズの V モデル実行 (PH-01 〜 PH-15)
- 安全目標、HARA、FMEA、故障木解析
- ソフトウェア安全要求 (SR-001 〜 SR-018)
- 安全監視・フェイルセーフ・診断マネージャを備えたアーキテクチャ
- ユニット + 統合 + 適格性テスト
- トレーサビリティマトリクス
- 14/16 プロセスで SPICE Level 2 達成

## AI 支援開発

このフレームワーク全体が Claude Code (Anthropic) で開発されました。
それこそがポイントです。安全エンジニアリングの知識は AI スキルとしてエンコードし、
プロジェクト横断で再利用できます。

すべてのスキルは一貫した構造に従います:
- 前提条件とエラーハンドリング
- ステップバイステップの実行手順
- 出力フォーマット仕様
- project.json とプロセス記録への相互参照

## ライセンス

GPLv3。[LICENSE](LICENSE) を参照。

持っていけ。使え。改善しろ。共有しろ。

## 免責事項

[DISCLAIMER.md](DISCLAIMER.md) を参照。TORASAN は製品を認証しません。
すべての安全判断はあなたの責任です。

## コントリビュート

[CONTRIBUTING.md](CONTRIBUTING.md) を参照。

初めての貢献に最適:
- IEC 62304 (医療機器) の安全スキル追加
- EN ISO 13849-1 (機械) の安全パターン追加
- README のドイツ語 / 韓国語 / 中国語翻訳
- STM32 HAL プラットフォーム定義の追加
- あなたの分野の FMEA テンプレート改善

## About

機能安全の経験を持つ日本の組み込みシステムエンジニアが作りました。
企業じゃない。スタートアップでもない。この知識が高額なコンサルティング費用の
裏に閉じ込められるべきではないと考える、ただのエンジニア。

改善したら――共有してくれ。
