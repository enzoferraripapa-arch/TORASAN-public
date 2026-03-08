#!/usr/bin/env python3
"""
add_prereq_errorhandling.py — 全スキルに Prerequisites / Error Handling セクションを一括挿入

Usage:
    python scripts/add_prereq_errorhandling.py --dry-run   # 変更内容を表示のみ
    python scripts/add_prereq_errorhandling.py              # 実際に書き込み
"""

import re
import sys
from pathlib import Path

SKILLS_DIR = Path(".claude/skills")

# ── カテゴリマッピング ──────────────────────────────────
# A: 安全クリティカル生成系, B: project.json更新系, C: 外部ツール依存系
# D: 情報収集/表示系, E: メタ/設計/複合系
CATEGORY_MAP = {
    # (A) 安全クリティカル生成系
    "driver-gen": "A", "safety-diag": "A", "mcu-config": "A",
    "motor-control": "A", "safety-concept": "A", "srs-generate": "A",
    "memory-map": "A",
    # (B) project.json 更新系
    "execute-phase": "B", "switch-standard": "B", "assess-spice": "B",
    "update-record": "B", "commit-change": "B", "problem-resolve": "B",
    "manage-tbd": "B", "ingest": "B",
    # (C) 外部ツール依存系
    "static-analysis": "C", "generate-docs": "C", "md2pptx": "C",
    "env-check": "C", "worktree-cleanup": "C", "backup": "C",
    # (D) 情報収集/表示系
    "health-check": "D", "dashboard": "D", "validate": "D",
    "trace": "D", "test-coverage": "D", "config-audit": "D",
    "platform-info": "D", "select-standard": "D",
    # (E) メタ/設計/複合系
    "session": "E", "skill-manage": "E", "skill-evolve": "E",
    "claude-master": "E", "repo-manage": "E", "fmea": "E",
    "sw-design": "E", "system-design": "E", "hw-review": "E",
    "test-design": "E", "systest-design": "E",
}

# install.sh で配布される汎用スキル
SHARED_SKILLS = {
    "session", "dashboard", "health-check", "skill-manage", "skill-evolve",
    "repo-manage", "backup", "commit-change", "config-audit",
    "worktree-cleanup", "update-record", "env-check", "platform-info",
    "claude-master",
}

# 既に Error Handling 相当のセクションを持つスキル（統合対象）
EXISTING_ERROR_SECTIONS = {
    "execute-phase": "## エラー時の動作",
    "validate": "## 問題検出時の動作",
}

# session は本文内に十分な前提条件・エラー処理があるためスキップ
SKIP_SKILLS = {"session"}


# ── テンプレート定義 ──────────────────────────────────

def get_prereq_template(skill_name: str, category: str) -> str:
    """カテゴリに応じた Prerequisites テンプレートを返す"""
    is_shared = skill_name in SHARED_SKILLS

    # 共通ヘッダ
    lines = ["## Prerequisites（前提条件）", "",
             "本スキル実行前に以下を確認:", ""]

    if category == "A":
        lines.extend([
            "| # | 条件 | 確認方法 | 未充足時 |",
            "|---|------|---------|---------|",
            "| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |",
            "| 2 | product_spec の必要フィールドが定義済み | project.json 内を確認 | 必要フィールドを設定してから再実行 |",
            "| 3 | 前段フェーズが完了済み | phases[] の status 確認 | 該当フェーズを先に `/execute-phase` で実行 |",
            "| 4 | 出力先ディレクトリが存在する | `ls` で確認 | 自動作成（mkdir -p） |",
        ])
    elif category == "B":
        lines.extend([
            "| # | 条件 | 確認方法 | 未充足時 |",
            "|---|------|---------|---------|",
            "| 1 | project.json が存在し有効な JSON である | `Read project.json` | `/session start` でプロジェクトを初期化 |",
            "| 2 | 更新対象セクションが存在する | キー存在確認 | セクションを初期化して続行（ユーザー確認後） |",
        ])
    elif category == "C":
        lines.extend([
            "| # | 条件 | 確認方法 | 未充足時 |",
            "|---|------|---------|---------|",
            "| 1 | 必要な外部ツールがインストール済み | `which {tool}` / `{tool} --version` | エラーメッセージでインストール手順を提示 |",
            "| 2 | 対象ファイルが存在する | Glob/ls で確認 | 対象不在を報告して終了 |",
        ])
    elif category == "D":
        lines.extend([
            "| # | 条件 | 確認方法 | 未充足時 |",
            "|---|------|---------|---------|",
        ])
        if not is_shared:
            lines.append(
                "| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |"
            )
        else:
            lines.append(
                "| 1 | Git リポジトリ内で実行 | `git status` | リポジトリ外では実行不可 |"
            )
    else:  # E
        lines.extend([
            "| # | 条件 | 確認方法 | 未充足時 |",
            "|---|------|---------|---------|",
        ])
        if not is_shared:
            lines.append(
                "| 1 | project.json が存在する | `Read project.json` | `/session start` でプロジェクトを初期化 |"
            )
        else:
            lines.append(
                "| 1 | Git リポジトリ内で実行 | `git status` | リポジトリ外では実行不可 |"
            )

    lines.append("")
    return "\n".join(lines)


def get_error_template(skill_name: str, category: str) -> str:
    """カテゴリに応じた Error Handling テンプレートを返す"""
    lines = ["## Error Handling（エラー時の対応）", ""]

    if category == "A":
        lines.extend([
            "| 障害 | 検出方法 | 復旧手順 |",
            "|------|---------|---------|",
            "| project.json 読み取り失敗 | Read エラー | パスと構文を確認。`git checkout -- project.json` で復元 |",
            "| product_spec フィールド欠落 | キー不在 / null | 必要フィールドを報告し設定を促す。TBD マーカー付きで続行可 |",
            "| 既存ファイル上書き競合 | Read で既存ファイル検出 | `cp {file} {file}.bak` → diff 表示 → ユーザー承認後に上書き |",
            "| TBD 未解消（安全クリティカル） | TBD チェック | **生成を中止**。未解消 TBD を報告 |",
        ])
    elif category == "B":
        lines.extend([
            "| 障害 | 検出方法 | 復旧手順 |",
            "|------|---------|---------|",
            "| project.json パース失敗 | JSON decode エラー | 構文エラー箇所を報告。`git checkout -- project.json` で復元 |",
            "| 書き込み失敗 | Write エラー | 変更内容を表示し手動適用を提案 |",
            "| 更新内容の整合性エラー | バリデーション | 更新前の値を表示し `git checkout -- project.json` でロールバック |",
        ])
    elif category == "C":
        lines.extend([
            "| 障害 | 検出方法 | 復旧手順 |",
            "|------|---------|---------|",
            "| ツール未インストール | `which` 失敗 | インストール手順を提示。該当チェックをスキップ |",
            "| ツール実行エラー | 非0終了コード | stderr を報告。対象を分割して再実行を提案 |",
            "| 出力パース失敗 | フォーマット不一致 | 生出力を表示し手動確認を促す |",
        ])
    elif category == "D":
        lines.extend([
            "| 障害 | 検出方法 | 復旧手順 |",
            "|------|---------|---------|",
            "| データソース読み取り失敗 | Read / Glob エラー | 対象不在を報告。該当項目を「N/A」として表示 |",
            "| 分析対象データ不足 | 必要キーの不在 | 不足項目を明示し、データ投入を促す |",
        ])
    else:  # E
        lines.extend([
            "| 障害 | 検出方法 | 復旧手順 |",
            "|------|---------|---------|",
            "| 入力データ不足 | 必要ファイル/キーの不在 | 不足項目を報告し、前提スキルの実行を促す |",
            "| 処理中断 | 予期しないエラー | エラー内容を報告。変更途中のファイルがあれば `git checkout` で復元 |",
        ])

    lines.append("")
    return "\n".join(lines)


# ── パーサー + 挿入ロジック ──────────────────────────────

def find_first_h2(text: str) -> int | None:
    """frontmatter の後にある最初の ## の行位置を返す"""
    lines = text.split("\n")
    in_frontmatter = False
    frontmatter_ended = False

    for i, line in enumerate(lines):
        if line.strip() == "---":
            if not in_frontmatter:
                in_frontmatter = True
                continue
            else:
                frontmatter_ended = True
                continue

        if frontmatter_ended and line.startswith("## "):
            return i

    return None


def find_section_line(text: str, section_name: str) -> int | None:
    """指定セクション名の行位置を返す"""
    lines = text.split("\n")
    for i, line in enumerate(lines):
        if line.strip() == section_name or line.strip().startswith(section_name):
            return i
    return None


def find_insert_positions(text: str, skill_name: str) -> tuple[int | None, int | None]:
    """Prerequisites と Error Handling の挿入位置を返す"""
    # Prerequisites: 最初の H2 の直前
    prereq_pos = find_first_h2(text)

    # Error Handling: ## 注意事項 の直前
    error_pos = find_section_line(text, "## 注意事項")

    # 注意事項がなければ ## 関連スキル の直前
    if error_pos is None:
        error_pos = find_section_line(text, "## 関連スキル")

    # それもなければ ## ナレッジベース参照 の直前
    if error_pos is None:
        error_pos = find_section_line(text, "## ナレッジベース参照")
        if error_pos is None:
            error_pos = find_section_line(text, "## ナレッジ参照")

    return prereq_pos, error_pos


def has_section(text: str, section_name: str) -> bool:
    """指定セクションが既に存在するか"""
    return bool(re.search(rf"^{re.escape(section_name)}", text, re.MULTILINE))


def insert_sections(text: str, skill_name: str, category: str) -> str:
    """Prerequisites と Error Handling を挿入"""
    lines = text.split("\n")

    prereq_pos, error_pos = find_insert_positions(text, skill_name)

    # 既存セクションチェック
    has_prereq = has_section(text, "## Prerequisites")
    has_error = (
        has_section(text, "## Error Handling")
        or skill_name in EXISTING_ERROR_SECTIONS
    )

    insertions = []  # (position, content) — 後ろから挿入するためにソート

    if not has_error and error_pos is not None:
        error_template = get_error_template(skill_name, category)
        insertions.append((error_pos, error_template))

    if not has_prereq and prereq_pos is not None:
        prereq_template = get_prereq_template(skill_name, category)
        insertions.append((prereq_pos, prereq_template))

    # 後ろから挿入（位置がずれないように）
    insertions.sort(key=lambda x: x[0], reverse=True)

    for pos, content in insertions:
        lines.insert(pos, content)

    return "\n".join(lines)


# ── メイン処理 ──────────────────────────────────

def main():
    dry_run = "--dry-run" in sys.argv

    if not SKILLS_DIR.exists():
        print(f"ERROR: {SKILLS_DIR} が見つかりません")
        sys.exit(1)

    results = {"modified": [], "skipped": [], "error": []}

    for skill_dir in sorted(SKILLS_DIR.iterdir()):
        skill_md = skill_dir / "SKILL.md"
        if not skill_md.exists():
            continue

        name = skill_dir.name

        if name in SKIP_SKILLS:
            results["skipped"].append(f"{name} (スキップ対象)")
            continue

        category = CATEGORY_MAP.get(name, "E")
        text = skill_md.read_text(encoding="utf-8")

        # 既に両方あればスキップ
        has_prereq = has_section(text, "## Prerequisites")
        has_error = (
            has_section(text, "## Error Handling")
            or name in EXISTING_ERROR_SECTIONS
        )

        if has_prereq and has_error:
            results["skipped"].append(f"{name} (既存セクションあり)")
            continue

        new_text = insert_sections(text, name, category)

        if new_text == text:
            results["skipped"].append(f"{name} (変更なし)")
            continue

        if dry_run:
            # 変更行数を計算
            old_lines = len(text.split("\n"))
            new_lines = len(new_text.split("\n"))
            added = new_lines - old_lines
            sections_added = []
            if not has_prereq:
                sections_added.append("Prerequisites")
            if not has_error:
                sections_added.append("Error Handling")
            results["modified"].append(
                f"{name} [{category}] +{added}行 ({', '.join(sections_added)})"
            )
        else:
            try:
                skill_md.write_text(new_text, encoding="utf-8")
                sections_added = []
                if not has_prereq:
                    sections_added.append("Prerequisites")
                if not has_error:
                    sections_added.append("Error Handling")
                results["modified"].append(
                    f"{name} [{category}] ({', '.join(sections_added)})"
                )
            except Exception as e:
                results["error"].append(f"{name}: {e}")

    # レポート出力
    mode = "DRY-RUN" if dry_run else "EXECUTED"
    print(f"\n=== Prerequisites / Error Handling 一括挿入 [{mode}] ===\n")

    print(f"[変更] {len(results['modified'])}件:")
    for item in results["modified"]:
        print(f"  + {item}")

    print(f"\n[スキップ] {len(results['skipped'])}件:")
    for item in results["skipped"]:
        print(f"  - {item}")

    if results["error"]:
        print(f"\n[エラー] {len(results['error'])}件:")
        for item in results["error"]:
            print(f"  ! {item}")

    print(f"\n合計: {len(results['modified'])} 変更 / "
          f"{len(results['skipped'])} スキップ / "
          f"{len(results['error'])} エラー")


if __name__ == "__main__":
    main()
