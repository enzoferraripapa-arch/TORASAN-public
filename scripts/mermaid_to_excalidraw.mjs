/**
 * Mermaid → Excalidraw 一括変換スクリプト (DOM-free)
 *
 * Usage:
 *   node scripts/mermaid_to_excalidraw.mjs [--font <name>]
 *
 * Fonts:
 *   virgil    手書き風（Excalidraw デフォルト）  fontFamily=1
 *   helvetica ゴシック体（ビジネス向き）         fontFamily=2
 *   cascadia  等幅（コード・技術図向き）         fontFamily=3
 *
 * 例: node scripts/mermaid_to_excalidraw.mjs --font helvetica
 */
import { readFileSync, writeFileSync, mkdirSync } from "fs";
import { join, basename } from "path";
import { randomBytes } from "crypto";
import LZString from "lz-string";

// --- Font selection from CLI ---
const FONT_MAP = { virgil: 1, helvetica: 2, cascadia: 3 };
const fontArg = process.argv.indexOf("--font");
const fontName = fontArg !== -1 ? (process.argv[fontArg + 1] || "").toLowerCase() : "virgil";
const FONT_FAMILY = FONT_MAP[fontName] || 1;
if (fontArg !== -1) {
  console.log(`Font: ${fontName} (fontFamily=${FONT_FAMILY})`);
}

const DIAGRAMS_DIR = join(import.meta.dirname, "..", "docs", "diagrams");
const OUTPUT_DIR = join(DIAGRAMS_DIR, "excalidraw");

const FILES = [
  "01_repo_structure.md",
  "02_distribution_model.md",
  "03_memory_architecture.md",
  "04_vmodel.md",
  "05_tool_resolution.md",
];

// --- Color palette (TORASAN theme) ---
const COLORS = {
  root:      { bg: "#2E3048", stroke: "#2E3048", text: "#ffffff" },
  skill:     { bg: "#C8E0D2", stroke: "#4A8C6F", text: "#2E3048" },
  knowledge: { bg: "#F0AA8F", stroke: "#E07A5F", text: "#2E3048" },
  app:       { bg: "#F9E4C0", stroke: "#F2CC8F", text: "#2E3048" },
  tool:      { bg: "#A8BFA0", stroke: "#3D5C36", text: "#2E3048" },
  config:    { bg: "#8A8DA0", stroke: "#2E3048", text: "#2E3048" },
  default:   { bg: "#e8e8e8", stroke: "#999999", text: "#1e1e1e" },

  // Diagram-specific
  man:       { bg: "#8A8DA0", stroke: "#2E3048", text: "#2E3048" },
  sys:       { bg: "#C8E0D2", stroke: "#4A8C6F", text: "#2E3048" },
  swe:       { bg: "#F0AA8F", stroke: "#E07A5F", text: "#2E3048" },
  sup:       { bg: "#A8BFA0", stroke: "#3D5C36", text: "#2E3048" },
  start:     { bg: "#2E3048", stroke: "#2E3048", text: "#ffffff" },
  decision:  { bg: "#F9E4C0", stroke: "#F2CC8F", text: "#2E3048" },
  success:   { bg: "#C8E0D2", stroke: "#4A8C6F", text: "#2E3048" },
  fail:      { bg: "#F0AA8F", stroke: "#E07A5F", text: "#2E3048" },
  done:      { bg: "#4A8C6F", stroke: "#4A8C6F", text: "#ffffff" },

  // Diagram 2
  tier1:     { bg: "#C8E0D2", stroke: "#4A8C6F", text: "#2E3048" },
  tier2:     { bg: "#F0AA8F", stroke: "#E07A5F", text: "#2E3048" },
  sessionBox:{ bg: "#F0AA8F", stroke: "#E07A5F", text: "#2E3048" },
  projectBox:{ bg: "#C8E0D2", stroke: "#4A8C6F", text: "#2E3048" },
  globalBox: { bg: "#F9E4C0", stroke: "#F2CC8F", text: "#2E3048" },
  invisible: { bg: "transparent", stroke: "transparent", text: "#2E3048" },
};

function uid() {
  return randomBytes(8).toString("hex");
}

function makeRect(id, x, y, w, h, label, color) {
  const c = COLORS[color] || COLORS.default;
  return [
    {
      id,
      type: "rectangle",
      x, y,
      width: w,
      height: h,
      strokeColor: c.stroke,
      backgroundColor: c.bg,
      fillStyle: "solid",
      strokeWidth: 2,
      roundness: { type: 3, value: 8 },
      boundElements: [],
    },
    {
      id: uid(),
      type: "text",
      x: x + 10,
      y: y + h / 2 - 10,
      width: w - 20,
      height: 20,
      text: label.replace(/\\n/g, "\n"),
      fontSize: 14,
      fontFamily: FONT_FAMILY,
      textAlign: "center",
      verticalAlign: "middle",
      strokeColor: c.text,
      backgroundColor: "transparent",
      fillStyle: "solid",
      containerId: id,
    },
  ];
}

function makeDiamond(id, x, y, w, h, label, color) {
  const c = COLORS[color] || COLORS.default;
  return [
    {
      id,
      type: "diamond",
      x, y,
      width: w,
      height: h,
      strokeColor: c.stroke,
      backgroundColor: c.bg,
      fillStyle: "solid",
      strokeWidth: 2,
      boundElements: [],
    },
    {
      id: uid(),
      type: "text",
      x: x + w * 0.15,
      y: y + h / 2 - 10,
      width: w * 0.7,
      height: 20,
      text: label.replace(/\\n/g, "\n"),
      fontSize: 12,
      fontFamily: FONT_FAMILY,
      textAlign: "center",
      verticalAlign: "middle",
      strokeColor: c.text,
      backgroundColor: "transparent",
      fillStyle: "solid",
      containerId: id,
    },
  ];
}

function makeArrow(fromId, toId, startX, startY, endX, endY, dotted = false) {
  return {
    id: uid(),
    type: "arrow",
    x: startX,
    y: startY,
    width: endX - startX,
    height: endY - startY,
    points: [[0, 0], [endX - startX, endY - startY]],
    strokeColor: "#1e1e1e",
    backgroundColor: "transparent",
    strokeWidth: 2,
    strokeStyle: dotted ? "dotted" : "solid",
    startBinding: { elementId: fromId, focus: 0, gap: 4 },
    endBinding: { elementId: toId, focus: 0, gap: 4 },
    endArrowhead: dotted ? null : "arrow",
    startArrowhead: null,
  };
}

function buildScene(elements) {
  return {
    type: "excalidraw",
    version: 2,
    source: "torasan-mermaid-converter",
    elements,
    appState: {
      gridSize: null,
      viewBackgroundColor: "#ffffff",
    },
    files: {},
  };
}

function toExcalidrawMd(scene, title, sourceFile) {
  const json = JSON.stringify(scene);
  const compressed = LZString.compressToBase64(json);
  return `---

excalidraw-plugin: parsed
tags: [excalidraw]

---
==⚠  Switch to EXCALIDRAW VIEW in the MORE OPTIONS menu of this document. ⚠== You can decompress Drawing data with the command palette: 'Decompress current Excalidraw file'. For more info check in plugin settings under 'Saving'


# ${title}
## Drawing
\`\`\`compressed-json
${compressed}
\`\`\`
%%
`;
}

// ============================================================
// Diagram-specific layouts (hand-crafted for best appearance)
// ============================================================

function layout_01_repo() {
  // Tree: ROOT → 8 children
  const els = [];
  const rootId = uid();
  els.push(...makeRect(rootId, 300, 20, 200, 50, "TORASAN/", "root"));

  const children = [
    { label: ".claude/skills/\n41 skills", color: "skill" },
    { label: ".claude/knowledge/\n16 files", color: "knowledge" },
    { label: "app/\nReact + Fastify", color: "app" },
    { label: "scripts/\nツール・生成", color: "tool" },
    { label: "PROCESS.md\nV-model 定義", color: "config" },
    { label: "project.json\nSoT", color: "config" },
    { label: "packages/\n規格テンプレート", color: "knowledge" },
    { label: "docs/\n成果物文書", color: "skill" },
  ];

  children.forEach((c, i) => {
    const cid = uid();
    const x = i * 120;
    const y = 130;
    els.push(...makeRect(cid, x, y, 110, 60, c.label, c.color));
    els.push(makeArrow(rootId, cid, 400, 70, x + 55, 130));
  });

  return els;
}

function layout_02_distribution() {
  const els = [];

  // Tier 1 header
  const t1h = uid();
  els.push(...makeRect(t1h, 50, 20, 300, 40, "Tier 1: install.sh → ~/.claude/", "tier1"));

  const tier1 = [
    { label: "汎用スキル\n15本", color: "skill" },
    { label: "汎用ナレッジ\n7本", color: "knowledge" },
    { label: "設定\nsettings.json", color: "config" },
  ];
  tier1.forEach((c, i) => {
    const cid = uid();
    els.push(...makeRect(cid, 50 + i * 110, 80, 100, 55, c.label, c.color));
    els.push(makeArrow(t1h, cid, 200, 60, 100 + i * 110, 80));
  });

  // Tier 2 header
  const t2h = uid();
  els.push(...makeRect(t2h, 450, 20, 300, 40, "Tier 2: /repo-manage sync", "tier2"));

  const tier2 = [
    { label: "ドメインスキル\n26本", color: "swe" },
    { label: "ドメインナレッジ\n9本", color: "knowledge" },
  ];
  tier2.forEach((c, i) => {
    const cid = uid();
    els.push(...makeRect(cid, 460 + i * 130, 80, 120, 55, c.label, c.color));
    els.push(makeArrow(t2h, cid, 600, 60, 520 + i * 130, 80));
  });

  return els;
}

function layout_03_memory() {
  const els = [];
  const zones = [
    {
      title: "Session スコープ（揮発性）",
      color: "sessionBox",
      items: [
        { label: "Auto Memory\nMEMORY.md\n自動ロード 200行上限", color: "sessionBox" },
        { label: "Session State\nsession_state.md\n/session end で上書き", color: "sessionBox" },
      ],
    },
    {
      title: "Project スコープ（Git 永続）",
      color: "projectBox",
      items: [
        { label: "Project Knowledge\n.claude/knowledge/*.md\nドメイン知識", color: "projectBox" },
        { label: "Process Records\nprocess_records/*.md\nSPICE エビデンス", color: "projectBox" },
        { label: "project.json\nSoT\n構成・進捗・変更ログ", color: "projectBox" },
      ],
    },
    {
      title: "Global スコープ（永続）",
      color: "globalBox",
      items: [
        { label: "Project Registry\nproject_registry.json\nPJ 一覧", color: "globalBox" },
        { label: "Skill Manifest\n.shared-skills-manifest.json\n配布履歴", color: "globalBox" },
      ],
    },
  ];

  let yOffset = 20;
  zones.forEach((z) => {
    // Zone title
    const zid = uid();
    els.push(...makeRect(zid, 50, yOffset, 600, 30, z.title, z.color));
    yOffset += 40;

    z.items.forEach((item, i) => {
      const iid = uid();
      els.push(...makeRect(iid, 60 + i * 200, yOffset, 180, 70, item.label, item.color));
    });
    yOffset += 100;
  });

  return els;
}

function layout_04_vmodel() {
  const els = [];

  // --- Automotive SPICE V-model ---
  // Top: SYS.1 (Requirements Elicitation) spanning both sides
  // Left (descending): SYS.2 → SYS.3 → SWE.1 → SWE.2 → SWE.3
  // Right (ascending):  SWE.4 → SWE.5 → SWE.6 → SYS.4 → SYS.5
  // Horizontal dotted arrows: verification traceability between pairs

  const bw = 200, bh = 55, rowH = 85;
  const indent = 45; // V-shape indent per row
  const leftX0 = 30, rightX0 = 680;

  // SYS.1 at top center
  const sys1id = uid();
  els.push(...makeRect(sys1id, 310, 15, 240, 50, "SYS.1\nRequirements Elicitation", "man"));

  // V-model pairs (left descending, right ascending)
  const pairs = [
    { left: "SYS.2\nSystem Requirements\nAnalysis",      right: "SYS.5\nSystem Qualification\nTest",           lc: "sys", rc: "sys" },
    { left: "SYS.3\nSystem Architecture\nDesign",         right: "SYS.4\nSystem Integration\n& Integration Test", lc: "sys", rc: "sys" },
    { left: "SWE.1\nSW Requirements\nAnalysis",           right: "SWE.6\nSW Qualification\nTest",               lc: "swe", rc: "swe" },
    { left: "SWE.2\nSW Architectural\nDesign",            right: "SWE.5\nSW Integration\n& Integration Test",   lc: "swe", rc: "swe" },
    { left: "SWE.3\nSW Detailed Design\n& Unit Construction", right: "SWE.4\nSW Unit\nVerification",           lc: "swe", rc: "swe" },
  ];

  const topY = 90;
  const ids = { left: [], right: [] };

  pairs.forEach((p, i) => {
    const lid = uid(), rid = uid();
    const y = topY + i * rowH;
    const lx = leftX0 + i * indent;
    const rx = rightX0 - i * indent;
    els.push(...makeRect(lid, lx, y, bw, bh, p.left, p.lc));
    els.push(...makeRect(rid, rx, y, bw, bh, p.right, p.rc));
    // Dotted verification/traceability link
    els.push(makeArrow(lid, rid, lx + bw, y + bh / 2, rx, y + bh / 2, true));
    ids.left.push(lid);
    ids.right.push(rid);
  });

  // SYS.1 → SYS.2 (left start)
  els.push(makeArrow(sys1id, ids.left[0],
    310, 15 + 50,
    leftX0 + bw / 2, topY));

  // SYS.5 → SYS.1 return (right end)
  els.push(makeArrow(ids.right[0], sys1id,
    rightX0 + bw / 2, topY,
    310 + 240, 15 + 50));

  // Vertical flow arrows — left side (down)
  for (let i = 0; i < ids.left.length - 1; i++) {
    const y = topY + i * rowH;
    els.push(makeArrow(ids.left[i], ids.left[i + 1],
      leftX0 + i * indent + bw / 2, y + bh,
      leftX0 + (i + 1) * indent + bw / 2, y + rowH));
  }

  // Vertical flow arrows — right side (up)
  for (let i = ids.right.length - 1; i > 0; i--) {
    const y = topY + i * rowH;
    els.push(makeArrow(ids.right[i], ids.right[i - 1],
      rightX0 - i * indent + bw / 2, y,
      rightX0 - (i - 1) * indent + bw / 2, topY + (i - 1) * rowH + bh));
  }

  // Bottom curve: SWE.3 → SWE.4
  const lastRow = pairs.length - 1;
  const bottomY = topY + lastRow * rowH;
  els.push(makeArrow(ids.left[lastRow], ids.right[lastRow],
    leftX0 + lastRow * indent + bw, bottomY + bh / 2 + 15,
    rightX0 - lastRow * indent, bottomY + bh / 2 + 15));

  // Legend: MAN/SUP processes (small boxes, bottom-right)
  const legX = 720, legY = topY + 4 * rowH + 80;
  const legW = 150, legH = 30, legGap = 8;
  const legend = [
    { label: "MAN.3 Project Mgmt", color: "man" },
    { label: "SUP.1 Quality Assur.", color: "sup" },
    { label: "SUP.8 Config Mgmt", color: "sup" },
    { label: "SUP.10 Change Req.", color: "sup" },
  ];
  legend.forEach((l, i) => {
    const lid = uid();
    els.push(...makeRect(lid, legX, legY + i * (legH + legGap), legW, legH, l.label, l.color));
  });

  return els;
}

function layout_05_toolflow() {
  const els = [];
  const cy = 30; // current Y

  // Start
  const startId = uid();
  els.push(...makeRect(startId, 200, 20, 180, 40, "ツール実行要求", "start"));

  // Decision 1
  const d1id = uid();
  els.push(...makeDiamond(d1id, 200, 100, 180, 100, "cmd.exe PATH\nに存在？", "decision"));
  els.push(makeArrow(startId, d1id, 290, 60, 290, 100));

  // R1: Yes
  const r1id = uid();
  els.push(...makeRect(r1id, 450, 120, 140, 40, "直接実行", "success"));
  els.push(makeArrow(d1id, r1id, 380, 150, 450, 140));

  // Decision 2: No
  const d2id = uid();
  els.push(...makeDiamond(d2id, 200, 240, 180, 100, "KNOWN_TOOL_PATHS\nに登録？", "decision"));
  els.push(makeArrow(d1id, d2id, 290, 200, 290, 240));

  // R2: Yes
  const r2id = uid();
  els.push(...makeRect(r2id, 450, 260, 140, 40, "フルパス実行", "success"));
  els.push(makeArrow(d2id, r2id, 380, 290, 450, 280));

  // Decision 3: No
  const d3id = uid();
  els.push(...makeDiamond(d3id, 200, 380, 180, 100, "Git Bash\nで発見？", "decision"));
  els.push(makeArrow(d2id, d3id, 290, 340, 290, 380));

  // R3: Yes
  const r3id = uid();
  els.push(...makeRect(r3id, 450, 400, 140, 40, "Git Bash 経由", "success"));
  els.push(makeArrow(d3id, r3id, 380, 430, 450, 420));

  // FAIL: No
  const failId = uid();
  els.push(...makeRect(failId, 220, 520, 140, 40, "ツール未検出\nmissing", "fail"));
  els.push(makeArrow(d3id, failId, 290, 480, 290, 520));

  // OK (common endpoint)
  const okId = uid();
  els.push(...makeRect(okId, 650, 260, 120, 40, "実行完了", "done"));
  els.push(makeArrow(r1id, okId, 590, 140, 650, 270));
  els.push(makeArrow(r2id, okId, 590, 280, 650, 280));
  els.push(makeArrow(r3id, okId, 590, 420, 650, 290));

  return els;
}

// ============================================================
// Main
// ============================================================
const LAYOUT_MAP = {
  "01_repo_structure.md": { fn: layout_01_repo, title: "リポジトリ構成図" },
  "02_distribution_model.md": { fn: layout_02_distribution, title: "二層配布モデル" },
  "03_memory_architecture.md": { fn: layout_03_memory, title: "メモリ・状態管理アーキテクチャ" },
  "04_vmodel.md": { fn: layout_04_vmodel, title: "V モデル — 15 フェーズ構成" },
  "05_tool_resolution.md": { fn: layout_05_toolflow, title: "ツールパス解決フロー" },
};

function main() {
  mkdirSync(OUTPUT_DIR, { recursive: true });

  for (const file of FILES) {
    const layout = LAYOUT_MAP[file];
    if (!layout) {
      console.log(`[SKIP] ${file}: no layout defined`);
      continue;
    }

    try {
      const elements = layout.fn();
      const scene = buildScene(elements);
      const outName = basename(file, ".md") + ".excalidraw.md";
      const outPath = join(OUTPUT_DIR, outName);
      const md = toExcalidrawMd(scene, layout.title, file);
      writeFileSync(outPath, md, "utf-8");
      const flatEls = elements.flat();
      console.log(`[OK] ${file} → ${outName} (${flatEls.length} elements)`);
    } catch (err) {
      console.log(`[FAIL] ${file}: ${err.message}`);
      console.log(err.stack);
    }
  }

  console.log("\nDone.");
}

main();
