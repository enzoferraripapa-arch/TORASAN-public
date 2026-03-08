"""
TORASAN マニュアル生成・差分管理スクリプト

使い方:
  python scripts/generate_manuals.py                           # 全文書を再生成 + 差分レポート
  python scripts/generate_manuals.py --diff-only                # 差分レポートのみ（生成しない）
  python scripts/generate_manuals.py --spec                     # 仕様書のみ再生成
  python scripts/generate_manuals.py --ops                      # 操作手順書のみ再生成
  python scripts/generate_manuals.py -m "変更内容の要約"         # 改版履歴に記録する変更サマリ
  python scripts/generate_manuals.py --bump major               # メジャーバージョンアップ
"""

import os
import sys
import json
import hashlib
import shutil
import difflib
from datetime import datetime, date
from pathlib import Path

# --- パス定義 ---
REPO_ROOT = Path(__file__).resolve().parent.parent
MANUALS_DIR = REPO_ROOT / 'docs' / 'manuals'
MANIFEST_PATH = MANUALS_DIR / '.manuals_manifest.json'
DIFF_DIR = MANUALS_DIR / 'diff_reports'

# 成果物コピー先（D: ドライブ）
DELIVERABLES_DIR = Path('D:/成果物/TORASAN/マニュアル')

SPEC_DOCX = MANUALS_DIR / 'TORASAN_フレームワーク仕様書.docx'
OPS_DOCX = MANUALS_DIR / 'TORASAN_操作手順書.docx'


# =============================================================
# テキスト抽出（差分比較用）
# =============================================================

def extract_text_from_docx(docx_path):
    """docx からプレーンテキストを抽出（段落 + テーブル）"""
    from docx import Document
    if not docx_path.exists():
        return ''
    doc = Document(str(docx_path))
    lines = []
    for element in doc.element.body:
        tag = element.tag.split('}')[-1]
        if tag == 'p':
            text = element.text or ''
            if text.strip():
                lines.append(text.strip())
        elif tag == 'tbl':
            for row in element.findall('.//' + element.tag.split('}')[0] + '}tr'):
                cells = []
                for cell in row.findall('.//' + element.tag.split('}')[0] + '}tc'):
                    cell_text = cell.text or ''
                    # 全テキストノードを結合
                    all_text = []
                    for p in cell.iter():
                        if p.text:
                            all_text.append(p.text)
                        if p.tail:
                            all_text.append(p.tail)
                    cells.append(''.join(all_text).strip())
                if any(cells):
                    lines.append(' | '.join(cells))
    return '\n'.join(lines)


def compute_hash(file_path):
    """ファイルの SHA-256 ハッシュを計算"""
    if not file_path.exists():
        return None
    h = hashlib.sha256()
    with open(file_path, 'rb') as f:
        for chunk in iter(lambda: f.read(8192), b''):
            h.update(chunk)
    return h.hexdigest()


# =============================================================
# マニフェスト管理
# =============================================================

def load_manifest():
    """マニフェストファイルを読み込み"""
    if MANIFEST_PATH.exists():
        with open(MANIFEST_PATH, 'r', encoding='utf-8') as f:
            return json.load(f)
    return {'version': 0, 'documents': {}, 'history': []}


def save_manifest(manifest):
    """マニフェストファイルを保存"""
    MANIFEST_PATH.parent.mkdir(parents=True, exist_ok=True)
    with open(MANIFEST_PATH, 'w', encoding='utf-8') as f:
        json.dump(manifest, f, ensure_ascii=False, indent=2)


# =============================================================
# 差分レポート生成
# =============================================================

def generate_diff_report(old_text, new_text, doc_name):
    """2つのテキスト間の差分レポートを生成"""
    old_lines = old_text.splitlines()
    new_lines = new_text.splitlines()

    diff = list(difflib.unified_diff(
        old_lines, new_lines,
        fromfile=f'{doc_name} (前回)',
        tofile=f'{doc_name} (今回)',
        lineterm=''
    ))

    # 統計
    added = sum(1 for line in diff if line.startswith('+') and not line.startswith('+++'))
    removed = sum(1 for line in diff if line.startswith('-') and not line.startswith('---'))
    changed_sections = sum(1 for line in diff if line.startswith('@@'))

    return {
        'diff_lines': diff,
        'added': added,
        'removed': removed,
        'changed_sections': changed_sections,
        'has_changes': len(diff) > 0,
    }


def save_diff_report(diff_result, doc_name, version):
    """差分レポートをファイルに保存"""
    DIFF_DIR.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    filename = f'{timestamp}_v{version}_{doc_name}.diff.txt'
    filepath = DIFF_DIR / filename

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(f'# TORASAN マニュアル差分レポート\n')
        f.write(f'# 文書: {doc_name}\n')
        f.write(f'# バージョン: v{version}\n')
        f.write(f'# 生成日時: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}\n')
        f.write(f'# 追加: +{diff_result["added"]} 行 / 削除: -{diff_result["removed"]} 行\n')
        f.write(f'# 変更セクション: {diff_result["changed_sections"]} 箇所\n')
        f.write('#' + '=' * 60 + '\n\n')
        for line in diff_result['diff_lines']:
            f.write(line + '\n')

    return filepath


# =============================================================
# 文書生成（既存スクリプトから呼び出し）
# =============================================================

def get_revision_history(manifest, doc_key):
    """マニフェストの履歴から改版履歴テーブル用データを生成"""
    rows = []
    if 'revision_history' not in manifest:
        manifest['revision_history'] = {}
    doc_revisions = manifest.get('revision_history', {}).get(doc_key, [])
    for rev in doc_revisions:
        rows.append([
            rev['version_label'],
            rev['date'],
            rev['summary'],
            rev.get('approved_by', 'TORA'),
        ])
    return rows


def get_doc_version_label(manifest, doc_key):
    """文書の現在のバージョンラベル（例: v1.2）を取得"""
    revisions = manifest.get('revision_history', {}).get(doc_key, [])
    if revisions:
        return revisions[-1]['version_label']
    return 'v0.1'


def bump_version(current_label, bump_type='minor'):
    """バージョンラベルをインクリメント"""
    if not current_label or current_label == 'v0.1':
        return 'v1.0' if bump_type == 'major' else 'v0.1'
    parts = current_label.lstrip('v').split('.')
    major = int(parts[0]) if len(parts) > 0 else 1
    minor = int(parts[1]) if len(parts) > 1 else 0
    if bump_type == 'major':
        return f'v{major + 1}.0'
    else:
        return f'v{major}.{minor + 1}'


def add_revision_entry(manifest, doc_key, version_label, summary):
    """マニフェストに改版履歴エントリを追加"""
    if 'revision_history' not in manifest:
        manifest['revision_history'] = {}
    if doc_key not in manifest['revision_history']:
        manifest['revision_history'][doc_key] = []
    manifest['revision_history'][doc_key].append({
        'version_label': version_label,
        'date': datetime.now().strftime('%Y-%m-%d'),
        'summary': summary,
        'approved_by': 'TORA',
    })


def generate_spec(manifest):
    """仕様書を生成"""
    sys.path.insert(0, str(Path(__file__).parent))
    from generate_spec_doc import create_spec_doc
    MANUALS_DIR.mkdir(parents=True, exist_ok=True)
    revision_history = get_revision_history(manifest, 'spec')
    version_label = get_doc_version_label(manifest, 'spec')
    create_spec_doc(str(SPEC_DOCX), version_label=version_label,
                    revision_history=revision_history)


def generate_ops(manifest):
    """操作手順書を生成"""
    sys.path.insert(0, str(Path(__file__).parent))
    from generate_ops_manual import create_ops_manual
    MANUALS_DIR.mkdir(parents=True, exist_ok=True)
    revision_history = get_revision_history(manifest, 'ops')
    version_label = get_doc_version_label(manifest, 'ops')
    create_ops_manual(str(OPS_DOCX), version_label=version_label,
                      revision_history=revision_history)


# =============================================================
# 成果物コピー（D: ドライブ）
# =============================================================

def copy_to_deliverables(targets, doc_map):
    """生成済み docx を成果物フォルダにコピー"""
    copied = []
    try:
        DELIVERABLES_DIR.mkdir(parents=True, exist_ok=True)
        for key in targets:
            doc_name, docx_path, _ = doc_map[key]
            if docx_path.exists():
                dest = DELIVERABLES_DIR / docx_path.name
                shutil.copy2(str(docx_path), str(dest))
                copied.append(docx_path.name)
    except OSError as e:
        print(f'  WARNING: 成果物コピー失敗（{e}）— D: ドライブが利用できない可能性があります')
    return copied


# =============================================================
# メイン処理
# =============================================================

def run(targets=None, diff_only=False, message=None, bump_type='minor'):
    """
    メイン実行

    targets: ['spec', 'ops'] or None (= both)
    diff_only: True の場合、生成せず差分計算のみ
    message: 改版履歴に記録する変更サマリ
    bump_type: 'major' or 'minor'
    """
    if targets is None:
        targets = ['spec', 'ops']

    manifest = load_manifest()

    # 改版履歴の初期化（既存マニフェストに revision_history がない場合）
    if 'revision_history' not in manifest:
        manifest['revision_history'] = {}
        # 既存の v1〜v10 を初版として登録（マイグレーション）
        for key in ['spec', 'ops']:
            prev_ver = manifest.get('documents', {}).get(key, {}).get('version', 0)
            if prev_ver > 0:
                manifest['revision_history'][key] = [{
                    'version_label': 'v1.0',
                    'date': '2026-03-02',
                    'summary': '初版作成',
                    'approved_by': 'TORA',
                }]

    results = {}

    doc_map = {
        'spec': ('フレームワーク仕様書', SPEC_DOCX, generate_spec),
        'ops': ('操作手順書', OPS_DOCX, generate_ops),
    }

    for key in targets:
        doc_name, docx_path, gen_func = doc_map[key]

        # --- Step 1: 前回テキスト抽出 ---
        old_text = extract_text_from_docx(docx_path)
        old_hash = compute_hash(docx_path)

        if diff_only:
            print(f'[{doc_name}] 差分チェックのみ（生成スキップ）')
            prev = manifest.get('documents', {}).get(key, {})
            ver_label = get_doc_version_label(manifest, key)
            if prev:
                print(f'  文書バージョン: {ver_label}')
                print(f'  生成回数: {prev.get("version", "?")} ({prev.get("generated_at", "?")})')
                print(f'  ハッシュ: {prev.get("hash", "?")[:16]}...')
            else:
                print(f'  前回記録なし')
            results[key] = {'has_changes': False, 'skipped': True}
            continue

        # --- Step 2: バージョンラベル決定 ---
        current_label = get_doc_version_label(manifest, key)
        if message:
            # 変更サマリが指定された場合のみバージョンアップ
            new_label = bump_version(current_label, bump_type)
            add_revision_entry(manifest, key, new_label, message)
        else:
            new_label = current_label

        # --- Step 3: 再生成 ---
        print(f'[{doc_name}] 生成中... ({new_label})')
        gen_func(manifest)
        print(f'[{doc_name}] 生成完了: {docx_path.name}')

        # --- Step 4: 新テキスト抽出 + ハッシュ ---
        new_text = extract_text_from_docx(docx_path)
        new_hash = compute_hash(docx_path)

        # --- Step 5: 差分計算 ---
        if old_text:
            diff_result = generate_diff_report(old_text, new_text, doc_name)
        else:
            diff_result = {
                'diff_lines': [],
                'added': len(new_text.splitlines()),
                'removed': 0,
                'changed_sections': 0,
                'has_changes': True,
            }

        # --- Step 6: マニフェスト更新 ---
        prev_gen_count = manifest.get('documents', {}).get(key, {}).get('version', 0)
        new_gen_count = prev_gen_count + 1

        if 'documents' not in manifest:
            manifest['documents'] = {}
        manifest['documents'][key] = {
            'version': new_gen_count,
            'version_label': new_label,
            'hash': new_hash,
            'generated_at': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            'file': docx_path.name,
        }

        # --- Step 7: 差分レポート保存 ---
        diff_path = None
        if diff_result['has_changes'] and old_text:
            diff_path = save_diff_report(diff_result, key, new_gen_count)

        # --- Step 8: 履歴記録 ---
        history_entry = {
            'version': new_gen_count,
            'version_label': new_label,
            'document': key,
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            'change_summary': message or '自動再生成',
            'changes': {
                'added': diff_result['added'],
                'removed': diff_result['removed'],
                'sections': diff_result['changed_sections'],
            },
            'prev_hash': old_hash,
            'new_hash': new_hash,
        }
        if diff_path:
            history_entry['diff_report'] = str(diff_path.relative_to(REPO_ROOT))
        if 'history' not in manifest:
            manifest['history'] = []
        manifest['history'].append(history_entry)

        results[key] = diff_result
        results[key]['version'] = new_gen_count
        results[key]['version_label'] = new_label
        results[key]['diff_path'] = diff_path

    # マニフェスト保存
    manifest['version'] = max(
        d.get('version', 0) for d in manifest.get('documents', {}).values()
    ) if manifest.get('documents') else 0
    manifest['last_updated'] = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    save_manifest(manifest)

    # --- 成果物コピー（D: ドライブ） ---
    copied_files = []
    if not diff_only:
        copied_files = copy_to_deliverables(targets, doc_map)

    # --- サマリ出力 ---
    print()
    print('=' * 60)
    print('  TORASAN マニュアル生成 差分レポート')
    print('=' * 60)
    for key in targets:
        doc_name = doc_map[key][0]
        r = results[key]
        if r.get('skipped'):
            print(f'\n  [{doc_name}] スキップ')
            continue
        ver_label = r.get('version_label', '?')
        gen_count = r.get('version', '?')
        print(f'\n  [{doc_name}] {ver_label} (生成#{gen_count})')
        if message:
            print(f'    変更内容: {message}')
        if not r.get('has_changes'):
            print(f'    差分なし')
        else:
            print(f'    追加: +{r["added"]} 行')
            print(f'    削除: -{r["removed"]} 行')
            print(f'    変更セクション: {r["changed_sections"]} 箇所')
            if r.get('diff_path'):
                print(f'    差分レポート: {r["diff_path"].relative_to(REPO_ROOT)}')
    if copied_files:
        print(f'\n  成果物コピー先: {DELIVERABLES_DIR}')
        for f in copied_files:
            print(f'    → {f}')
    print()
    print(f'  マニフェスト: {MANIFEST_PATH.relative_to(REPO_ROOT)}')
    print('=' * 60)


if __name__ == '__main__':
    args = sys.argv[1:]

    diff_only = '--diff-only' in args
    targets = None
    if '--spec' in args:
        targets = ['spec']
    elif '--ops' in args:
        targets = ['ops']

    # --message / -m オプション
    message = None
    for i, arg in enumerate(args):
        if arg in ('-m', '--message') and i + 1 < len(args):
            message = args[i + 1]
            break

    # --bump オプション
    bump_type = 'minor'
    if '--bump' in args:
        idx = args.index('--bump')
        if idx + 1 < len(args) and args[idx + 1] == 'major':
            bump_type = 'major'

    run(targets=targets, diff_only=diff_only, message=message, bump_type=bump_type)
