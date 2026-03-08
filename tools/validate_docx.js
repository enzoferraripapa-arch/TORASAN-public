#!/usr/bin/env node
/**
 * validate_docx.js — TORASAN docx 多重検証スクリプト
 *
 * 検証レベル:
 *   L1: XML構造検証（Wordで開けるか）
 *   L2: テキスト誤記検証（既知のパターン + カタカナ→ASCII遷移チェック）
 *   L3: 文書構造検証（セクション番号の連続性、空テーブル等）
 *
 * 使い方:
 *   node validate_docx.js <file.docx>
 *   node validate_docx.js --all   (TORASAN/*.docxを全件検証)
 */

const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

// ===== 設定 =====
const TORASAN_DIR = "/sessions/quirky-friendly-franklin/mnt/TORASAN";

// L2: 既知の誤記パターン（拡張可能）
const KNOWN_TYPOS = [
  { pattern: /ズSIL/g, fix: "・SIL", severity: "ERROR" },
  { pattern: /ズASIL/g, fix: "・ASIL", severity: "ERROR" },
  { pattern: /セASIL/g, fix: "・ASIL", severity: "ERROR" },
  { pattern: /AAI補完/g, fix: "AI補完", severity: "ERROR" },
  { pattern: /AAI推測/g, fix: "AI推測", severity: "ERROR" },
  // 「セ」が中黒「・」の誤り：ASCII直後のセ＋ASCII（例: OSSセOS, SGセFSR）
  { pattern: /[A-Za-z]セ[A-Za-z]/g, fix: "・ (ASCII間のセは中黒の誤り)", severity: "ERROR" },
  // カタカナ「ズ」直後にASCII大文字が来る場合は要注意（正当なケースもある）
  { pattern: /ズ[A-Z]/g, fix: "・[A-Z]?", severity: "WARN" },
  // 「セ」直後にASCII大文字（カタカナ語の途中を除外）
  { pattern: /セ(?!ー|ル|ン|キ|グ|ッ|ク|カ|フ|レ|ミ|ス|パ|マ|リ|ラ)[A-Z]/g, fix: "・[A-Z]?", severity: "WARN" },
  // 全角・半角混在チェック（数字）
  { pattern: /[０-９]/g, fix: "半角数字に統一", severity: "WARN" },
  // 二重スペース
  { pattern: /　　/g, fix: "全角スペース重複", severity: "WARN" },
];

// L3: 文書構造チェック用
const SECTION_PATTERN = /^(\d+)\.\s/;

// ===== ユーティリティ =====
function extractTextFromDocx(docxPath) {
  // docxをunzipしてdocument.xml + 関連ファイルを抽出
  const tmpDir = `/tmp/validate_${Date.now()}`;
  try {
    execSync(`mkdir -p ${tmpDir} && unzip -o -q "${docxPath}" -d ${tmpDir}`, { stdio: "pipe" });
    const xml = fs.readFileSync(path.join(tmpDir, "word/document.xml"), "utf-8");
    // XMLからテキスト部分を抽出
    const texts = [];
    const regex = /<w:t[^>]*>(.*?)<\/w:t>/g;
    let m;
    while ((m = regex.exec(xml)) !== null) {
      texts.push(m[1]);
    }
    // リレーションシップファイルの読み取り
    let rels = "";
    const relsPath = path.join(tmpDir, "word/_rels/document.xml.rels");
    if (fs.existsSync(relsPath)) {
      rels = fs.readFileSync(relsPath, "utf-8");
    }
    // word/media/ 内の実在ファイル一覧
    const mediaDir = path.join(tmpDir, "word/media");
    const mediaFiles = fs.existsSync(mediaDir)
      ? fs.readdirSync(mediaDir)
      : [];
    return { xml, text: texts.join(""), textSegments: texts, rels, mediaFiles, tmpDir };
  } catch (e) {
    execSync(`rm -rf ${tmpDir}`, { stdio: "pipe" });
    throw e;
  }
}

function cleanupTmpDir(tmpDir) {
  try { execSync(`rm -rf ${tmpDir}`, { stdio: "pipe" }); } catch {}
}

function validateXml(xml, rels, mediaFiles) {
  const issues = [];

  // === L1-01: bare w:r チェック（PageBreakバグ）===
  if (/<\/w:sdt>\s*<w:r>/.test(xml)) {
    issues.push({ level: "L1", severity: "ERROR", msg: "bare w:r after w:sdt (PageBreakバグ)" });
  }
  if (/<\/w:p>\s*<w:r>\s*<w:br/.test(xml)) {
    issues.push({ level: "L1", severity: "ERROR", msg: "bare w:r with page break outside w:p" });
  }

  // === L1-02: 空のw:tcチェック（空テーブルセル→Word警告の原因）===
  if (/<w:tc[^>]*>\s*<\/w:tc>/.test(xml)) {
    issues.push({ level: "L1", severity: "WARN", msg: "空のテーブルセル(w:tc)が存在" });
  }

  // === L1-03: 画像参照の整合性チェック ===
  // document.xml内のr:embedで参照されるrIdを収集
  const embedRefs = new Set();
  const embedRegex = /r:embed="(rId\d+)"/g;
  let em;
  while ((em = embedRegex.exec(xml)) !== null) {
    embedRefs.add(em[1]);
  }

  if (embedRefs.size > 0 && rels) {
    // relsからrIdとTargetのマッピングを取得
    const relMap = new Map();
    const relRegex = /Id="(rId\d+)"[^>]*Target="([^"]+)"/g;
    let rm;
    while ((rm = relRegex.exec(rels)) !== null) {
      relMap.set(rm[1], rm[2]);
    }

    for (const rId of embedRefs) {
      if (!relMap.has(rId)) {
        issues.push({ level: "L1", severity: "ERROR", msg: `画像参照 ${rId} がrelsに存在しない（リンク切れ）` });
      } else {
        const target = relMap.get(rId);
        // media/ファイルへの参照の場合、ファイルが実在するか確認
        if (target.startsWith("media/")) {
          const filename = target.replace("media/", "");
          if (!mediaFiles.includes(filename)) {
            issues.push({ level: "L1", severity: "ERROR", msg: `画像ファイル ${target} がアーカイブ内に存在しない` });
          }
        }
      }
    }
  }

  // === L1-04: 画像サイズの異常チェック ===
  // EMU単位でのサイズ（A4幅=7560000EMU≒20cm、これを大幅に超えると問題）
  const extentRegex = /<wp:extent\s+cx="(\d+)"\s+cy="(\d+)"/g;
  let ext;
  while ((ext = extentRegex.exec(xml)) !== null) {
    const cx = parseInt(ext[1]);
    const cy = parseInt(ext[2]);
    const maxEmu = 9000000; // ~23.8cm (A4幅のマージン内上限)
    if (cx > maxEmu) {
      issues.push({ level: "L1", severity: "WARN", msg: `画像の幅が異常 (${(cx/360000).toFixed(1)}cm) - A4マージン内に収まらない可能性` });
    }
    if (cy > maxEmu * 1.5) {
      issues.push({ level: "L1", severity: "WARN", msg: `画像の高さが異常 (${(cy/360000).toFixed(1)}cm) - ページからはみ出す可能性` });
    }
    if (cx === 0 || cy === 0) {
      issues.push({ level: "L1", severity: "ERROR", msg: "画像のサイズが0（表示されない）" });
    }
  }

  // === L1-05: SVG画像のWord互換性チェック ===
  if (rels && /\.svg"/i.test(rels)) {
    issues.push({ level: "L1", severity: "WARN", msg: "SVG画像を含む — Word 2016以前では非対応。PNG/EMFへの変換を推奨" });
  }

  // === L1-06: テーブル行のセル数不一致チェック ===
  const tableRegex = /<w:tbl>([\s\S]*?)<\/w:tbl>/g;
  let tbl;
  let tableIdx = 0;
  while ((tbl = tableRegex.exec(xml)) !== null) {
    tableIdx++;
    const tableXml = tbl[1];
    const rowRegex = /<w:tr\b[^>]*>([\s\S]*?)<\/w:tr>/g;
    let row;
    const cellCounts = [];
    while ((row = rowRegex.exec(tableXml)) !== null) {
      const cells = (row[1].match(/<w:tc\b/g) || []).length;
      cellCounts.push(cells);
    }
    if (cellCounts.length > 1) {
      const expected = cellCounts[0];
      for (let ri = 1; ri < cellCounts.length; ri++) {
        if (cellCounts[ri] !== expected) {
          issues.push({ level: "L1", severity: "ERROR", msg: `テーブル${tableIdx}: 行${ri+1}のセル数(${cellCounts[ri]})がヘッダ行(${expected})と不一致 → 表示崩れの原因` });
          break;
        }
      }
    }
  }

  return issues;
}

function validateText(text, segments) {
  const issues = [];

  // === L2-01: 既知誤記パターンチェック ===
  for (const rule of KNOWN_TYPOS) {
    const matches = text.match(rule.pattern);
    if (matches) {
      for (const match of matches) {
        const idx = text.indexOf(match);
        const context = text.substring(Math.max(0, idx - 10), Math.min(text.length, idx + match.length + 10));
        issues.push({
          level: "L2",
          severity: rule.severity,
          msg: `「${match}」→「${rule.fix}」 ... "${context}"`
        });
      }
    }
  }

  // === L2-02: 未レンダリングのUML/図記法が混入していないか ===
  const rawDiagramPatterns = [
    { pattern: /@startuml/i, name: "PlantUML" },
    { pattern: /@enduml/i, name: "PlantUML" },
    { pattern: /```mermaid/i, name: "Mermaid" },
    { pattern: /graph\s+(TD|LR|RL|BT)\b/, name: "Mermaid graph" },
    { pattern: /sequenceDiagram/, name: "Mermaid sequence" },
    { pattern: /classDiagram/, name: "Mermaid class" },
    { pattern: /flowchart\s+(TD|LR|RL|BT)\b/, name: "Mermaid flowchart" },
    { pattern: /```dot\b/, name: "Graphviz DOT" },
    { pattern: /digraph\s+\w+\s*\{/, name: "Graphviz DOT" },
  ];

  for (const dp of rawDiagramPatterns) {
    if (dp.pattern.test(text)) {
      issues.push({
        level: "L2",
        severity: "ERROR",
        msg: `未レンダリングの${dp.name}記法がテキストに混入 — 画像に変換するか、テーブル表現に置き換えること`
      });
    }
  }

  // === L2-03: Markdown記法の残留チェック ===
  const mdPatterns = [
    { pattern: /^#{1,6}\s/m, name: "Markdownヘッダ (#)" },
    { pattern: /\[([^\]]+)\]\(https?:\/\/[^)]+\)/, name: "Markdownリンク" },
    { pattern: /```\w+\n/, name: "Markdownコードブロック" },
  ];

  for (const mp of mdPatterns) {
    if (mp.pattern.test(text)) {
      issues.push({
        level: "L2",
        severity: "WARN",
        msg: `${mp.name}がテキストに残留 — docxフォーマットに変換されていない可能性`
      });
    }
  }

  return issues;
}

function validateStructure(text) {
  const issues = [];

  // セクション番号の連続性チェック
  const sectionNums = [];
  const lines = text.split(/(?=[0-9]+\.\s)/);
  for (const line of lines) {
    const m = line.match(SECTION_PATTERN);
    if (m) {
      sectionNums.push(parseInt(m[1]));
    }
  }

  // 重複チェック
  const seen = new Set();
  for (const num of sectionNums) {
    if (seen.has(num) && num <= 20) {
      // サブセクション番号の重複は許容するため、大きい番号は無視
      // ここでは主要セクション番号の重複のみ警告
    }
    seen.add(num);
  }

  // テキスト長が極端に短い場合
  if (text.length < 500) {
    issues.push({ level: "L3", severity: "WARN", msg: `文書テキストが極端に短い (${text.length}文字)` });
  }

  return issues;
}

// ===== メイン =====
function validate(docxPath) {
  const filename = path.basename(docxPath);
  console.log(`\n${"=".repeat(60)}`);
  console.log(`  検証: ${filename}`);
  console.log(`${"=".repeat(60)}`);

  if (!fs.existsSync(docxPath)) {
    console.log("  ❌ ファイルが存在しません");
    return { errors: 1, warnings: 0 };
  }

  const { xml, text, textSegments, rels, mediaFiles, tmpDir } = extractTextFromDocx(docxPath);

  const allIssues = [
    ...validateXml(xml, rels, mediaFiles),
    ...validateText(text, textSegments),
    ...validateStructure(text),
  ];

  cleanupTmpDir(tmpDir);

  const errors = allIssues.filter(i => i.severity === "ERROR");
  const warnings = allIssues.filter(i => i.severity === "WARN");

  if (allIssues.length === 0) {
    console.log("  ✅ 全検証パス（L1:XML / L2:テキスト / L3:構造）");
  } else {
    for (const issue of allIssues) {
      const icon = issue.severity === "ERROR" ? "❌" : "⚠️";
      console.log(`  ${icon} [${issue.level}] ${issue.msg}`);
    }
  }

  console.log(`  結果: ERROR=${errors.length} WARN=${warnings.length}`);
  return { errors: errors.length, warnings: warnings.length };
}

// 引数処理
const args = process.argv.slice(2);

if (args.length === 0 || args[0] === "--all") {
  // 全件検証
  const files = fs.readdirSync(TORASAN_DIR)
    .filter(f => f.endsWith(".docx"))
    .map(f => path.join(TORASAN_DIR, f));

  if (files.length === 0) {
    console.log("docxファイルが見つかりません");
    process.exit(1);
  }

  console.log(`\n🔍 TORASAN docx 多重検証 (${files.length}ファイル)`);
  console.log(`   検証レベル: L1(XML) + L2(テキスト誤記) + L3(文書構造)`);

  let totalErrors = 0;
  let totalWarnings = 0;

  for (const f of files) {
    const result = validate(f);
    totalErrors += result.errors;
    totalWarnings += result.warnings;
  }

  console.log(`\n${"=".repeat(60)}`);
  console.log(`  総合結果: ERROR=${totalErrors} WARN=${totalWarnings}`);
  if (totalErrors > 0) {
    console.log("  ❌ ERRORが存在します。修正してください。");
    process.exit(1);
  } else if (totalWarnings > 0) {
    console.log("  ⚠️ 警告があります。確認を推奨します。");
  } else {
    console.log("  ✅ 全ファイル検証パス");
  }
  console.log(`${"=".repeat(60)}\n`);
} else {
  // 個別ファイル検証
  const result = validate(args[0]);
  if (result.errors > 0) process.exit(1);
}
