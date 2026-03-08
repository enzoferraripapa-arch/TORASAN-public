const fs = require("fs");
const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell,
  Header, Footer, AlignmentType, LevelFormat,
  TableOfContents, HeadingLevel, BorderStyle, WidthType, ShadingType,
  PageNumber, PageBreak
} = require("docx");

const border = { style: BorderStyle.SINGLE, size: 1, color: "999999" };
const borders = { top: border, bottom: border, left: border, right: border };
const cellMargins = { top: 60, bottom: 60, left: 100, right: 100 };
const headerShading = { fill: "2E5D34", type: ShadingType.CLEAR };
const altShading = { fill: "F2F8F3", type: ShadingType.CLEAR };
const noShading = { fill: "FFFFFF", type: ShadingType.CLEAR };

const TW = 9026; // A4 content width with 1" margins

function headerCell(text, width) {
  return new TableCell({
    borders, width: { size: width, type: WidthType.DXA }, shading: headerShading, margins: cellMargins,
    verticalAlign: "center",
    children: [new Paragraph({ alignment: AlignmentType.CENTER, children: [new TextRun({ text, bold: true, font: "Arial", size: 20, color: "FFFFFF" })] })]
  });
}
function cell(text, width, opts = {}) {
  return new TableCell({
    borders, width: { size: width, type: WidthType.DXA },
    shading: opts.shaded ? altShading : noShading, margins: cellMargins,
    children: [new Paragraph({ alignment: opts.center ? AlignmentType.CENTER : AlignmentType.LEFT,
      children: [new TextRun({ text, font: "Arial", size: 20, bold: !!opts.bold })] })]
  });
}
function h1(text) { return new Paragraph({ heading: HeadingLevel.HEADING_1, spacing: { before: 360, after: 200 }, children: [new TextRun({ text, font: "Arial", size: 32, bold: true, color: "2E5D34" })] }); }
function h2(text) { return new Paragraph({ heading: HeadingLevel.HEADING_2, spacing: { before: 280, after: 160 }, children: [new TextRun({ text, font: "Arial", size: 26, bold: true, color: "3B8745" })] }); }
function h3(text) { return new Paragraph({ heading: HeadingLevel.HEADING_3, spacing: { before: 200, after: 120 }, children: [new TextRun({ text, font: "Arial", size: 22, bold: true, color: "404040" })] }); }
function p(text, opts = {}) { return new Paragraph({ spacing: { after: 120 }, children: [new TextRun({ text, font: "Arial", size: 20, ...opts })] }); }
function pBold(label, text) { return new Paragraph({ spacing: { after: 120 }, children: [new TextRun({ text: label, font: "Arial", size: 20, bold: true }), new TextRun({ text, font: "Arial", size: 20 })] }); }
function makeTable(headers, rows, colWidths) {
  return new Table({
    width: { size: colWidths.reduce((a, b) => a + b, 0), type: WidthType.DXA }, columnWidths: colWidths,
    rows: [
      new TableRow({ children: headers.map((h, i) => headerCell(h, colWidths[i])) }),
      ...rows.map((row, ri) => new TableRow({
        children: row.map((c, ci) => cell(c, colWidths[ci], { shaded: ri % 2 === 1, center: ci === 0 }))
      }))
    ]
  });
}

const children = [];

// === COVER PAGE ===
children.push(new Paragraph({ spacing: { before: 3000 }, children: [] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 200 },
  children: [new TextRun({ text: "TORASAN", font: "Arial", size: 28, color: "2E5D34", bold: true })] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 100 },
  children: [new TextRun({ text: "\u6A5F\u80FD\u5B89\u5168\u958B\u767A\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8", font: "Arial", size: 24, color: "666666" })] }));
children.push(new Paragraph({ spacing: { before: 400 }, alignment: AlignmentType.CENTER,
  children: [new TextRun({ text: "\u65E2\u5B58\u8CC7\u7523 \u53D6\u308A\u8FBC\u307F\u30FB\u518D\u6574\u7406\u624B\u9806\u66F8", font: "Arial", size: 48, bold: true, color: "2E5D34" })] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 100, after: 100 },
  children: [new TextRun({ text: "INGEST \u30D7\u30ED\u30BB\u30B9\u4ED5\u69D8\u66F8", font: "Arial", size: 28, color: "3B8745" })] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 600 }, border: { top: { style: BorderStyle.SINGLE, size: 6, color: "2E5D34", space: 1 } }, children: [] }));

const infoW = [2800, 6226];
const infoRows = [
  ["\u6587\u66F8\u756A\u53F7", "TSDT-IN-001"],
  ["\u7248\u6570", "1.0"],
  ["\u4F5C\u6210\u65E5", "2026-02-27"],
  ["\u30B9\u30C6\u30FC\u30BF\u30B9", "\u8349\u6848"],
  ["\u4F5C\u6210\u8005", "TORASAN"],
  ["\u627F\u8A8D\u8005", "TBD"],
];
children.push(new Paragraph({ spacing: { before: 400 }, children: [] }));
children.push(new Table({
  width: { size: TW, type: WidthType.DXA }, columnWidths: infoW,
  rows: infoRows.map((r) => new TableRow({
    children: [
      cell(r[0], infoW[0], { bold: true, shaded: true }),
      cell(r[1], infoW[1], { shaded: false }),
    ]
  }))
}));

children.push(new Paragraph({ children: [new PageBreak()] }));

// === TOC ===
children.push(h1("\u76EE\u6B21"));
children.push(new TableOfContents("Table of Contents", { hyperlink: true, headingStyleRange: "1-3" }));
children.push(new Paragraph({ children: [new PageBreak()] }));

// === SECTION 1: OVERVIEW ===
children.push(h1("1. \u6982\u8981 [IN-01]"));
children.push(p("トレースID: IN-01 | ↑ SYS-01, SYS-02, SYS-06, SYS-09", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md 'このファイルについて / 設計思想' (L1-L26)", { italics: true, color: "888888", size: 14 }));
children.push(h2("1.1 \u6587\u66F8\u306E\u76EE\u7684"));
children.push(p("\u672C\u6587\u66F8\u306F\u3001Claude Code\u304C\u65E2\u5B58\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u8CC7\u7523\uFF08\u30BD\u30FC\u30B9\u30B3\u30FC\u30C9\u30FB\u8A2D\u8A08\u30C9\u30AD\u30E5\u30E1\u30F3\u30C8\u30FB\u56DE\u8DEF\u56F3\u30FB\u30C6\u30B9\u30C8\u8A18\u9332\u30FB\u8B70\u4E8B\u9332\u7B49\uFF09\u3092\u8AAD\u307F\u8FBC\u307F\u3001PROCESS.md\u306EV\u30E2\u30C7\u30EB\u30D7\u30ED\u30BB\u30B9\u306B\u518D\u30DE\u30C3\u30D4\u30F3\u30B0\u30FB\u518D\u6587\u66F8\u5316\u3059\u308B\u305F\u3081\u306E\u624B\u9806\u66F8\u3067\u3042\u308B\u3002"));

children.push(h2("1.2 \u8A2D\u8A08\u601D\u60F3"));
children.push(p("\u4E00\u6C17\u306B\u6574\u7406\u305B\u305A\u3001\u30B9\u30AD\u30E3\u30F3\u2192\u5206\u985E\u2192\u30DE\u30C3\u30D4\u30F3\u30B0\u2192\u518D\u6587\u66F8\u5316\u3092\u6BB5\u968E\u7684\u306B\u5B9F\u65BD\u3059\u308B\u3002"));
children.push(p("\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u5358\u4F4D\u3067\u306F\u306A\u304F\u6A5F\u80FD\u30FB\u6280\u8853\u8981\u7D20\u5358\u4F4D\u3067\u6574\u7406\u3057\u3001\u6D41\u7528\u52B9\u7387\u3092\u9AD8\u3081\u308B\u3002"));
children.push(p("\u9AD8\u4FA1\u5024\u8CC7\u7523\u304B\u3089\u512A\u5148\u51E6\u7406\u3057\u3001\u5B8C\u74A7\u306A\u6574\u7406\u3088\u308A\u300C\u4F7F\u3048\u308B\u72B6\u614B\u306B\u3059\u308B\u300D\u3092\u512A\u5148\u3059\u308B\u3002"));
children.push(p("\u5927\u91CF\u8CC7\u7523\u306F\u30D0\u30C3\u30C1\u51E6\u7406\u3067\u5BFE\u5FDC\u3002\u4E2D\u65AD\u30FB\u518D\u958B\u53EF\u80FD\u306A\u8A2D\u8A08\u3068\u3059\u308B\u3002"));

children.push(h2("1.3 \u4F7F\u3044\u65B9"));
children.push(p("1. \u672C\u30D5\u30A1\u30A4\u30EB\u3092 CLAUDE.md \u3068\u540C\u3058\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u30D5\u30A9\u30EB\u30C0\u306B\u914D\u7F6E\u3059\u308B"));
children.push(p("2. \u65E2\u5B58\u8CC7\u7523\u3092 legacy/ \u30D5\u30A9\u30EB\u30C0\u306B\u96C6\u3081\u308B"));
children.push(p("3. Claude \u3092\u8D77\u52D5\u3057\u3066\u300C\u8CC7\u7523\u6574\u7406\u3092\u958B\u59CB\u3057\u3066\u304F\u3060\u3055\u3044\u300D\u3068\u6307\u793A\u3059\u308B"));

// === SECTION 2: FOLDER STRUCTURE ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("2. \u30D5\u30A9\u30EB\u30C0\u69CB\u6210 [IN-02]"));
children.push(p("トレースID: IN-02 | ↑ SYS-03 | → PR-10", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md 'フォルダ構成（INGEST用）' (L29-L49)", { italics: true, color: "888888", size: 14 }));
children.push(makeTable(
  ["\u30D5\u30A9\u30EB\u30C0 / \u30D5\u30A1\u30A4\u30EB", "\u5F79\u5272"],
  [
    ["CLAUDE.md", "\u958B\u767A\u30D7\u30ED\u30BB\u30B9\u624B\u9806\u66F8"],
    ["INGEST.md", "\u672C\u30D5\u30A1\u30A4\u30EB\uFF08\u8CC7\u7523\u53D6\u308A\u8FBC\u307F\u624B\u9806\u66F8\uFF09"],
    ["ingest.json", "\u6574\u7406\u72B6\u614B\u306E\u7BA1\u7406\u30D5\u30A1\u30A4\u30EB\uFF08\u81EA\u52D5\u751F\u6210\uFF09"],
    ["legacy/", "\u65E2\u5B58\u8CC7\u7523\u3092\u3053\u3053\u306B\u96C6\u3081\u308B\uFF08\u8AAD\u307F\u53D6\u308A\u5C02\u7528\uFF09"],
    ["legacy/project_A/", "\u904E\u53BB\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8A\u4E00\u5F0F"],
    ["legacy/project_B/", "\u904E\u53BB\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8B\u4E00\u5F0F"],
    ["legacy/misc/", "\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u4E0D\u660E\u30FB\u65AD\u7247\u7684\u306A\u8CC7\u7523"],
    ["docs/", "PROCESS.md\u306E\u6210\u679C\u7269\uFF08\u518D\u6574\u7406\u5F8C\u306E\u51FA\u529B\u5148\uFF09"],
    ["src/", "PROCESS.md\u306E\u30B3\u30FC\u30C9\uFF08\u518D\u6574\u7406\u5F8C\u306E\u51FA\u529B\u5148\uFF09"],
    ["ingest_work/inventory.md", "\u8CC7\u7523\u76EE\u9332"],
    ["ingest_work/classification.md", "\u5206\u985E\u7D50\u679C"],
    ["ingest_work/mapping.md", "V\u30E2\u30C7\u30EB\u30DE\u30C3\u30D4\u30F3\u30B0\u7D50\u679C"],
    ["ingest_work/gap_analysis.md", "\u30AE\u30E3\u30C3\u30D7\u5206\u6790\u7D50\u679C"],
    ["ingest_work/reuse_log.md", "\u518D\u5229\u7528\u30FB\u518D\u6587\u66F8\u5316\u306E\u8A18\u9332"],
  ],
  [3800, 5226]
));

// === SECTION 3: STARTUP ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("3. \u8D77\u52D5\u6642\u306E\u52D5\u4F5C [IN-03]"));
children.push(p("トレースID: IN-03 | ↑ SYS-04", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md '起動時の動作 (STEP 0-2)' (L52-L93)", { italics: true, color: "888888", size: 14 }));

children.push(h2("3.1 STEP 0\uFF1A\u65E2\u5B58\u6574\u7406\u72B6\u614B\u306E\u78BA\u8A8D"));
children.push(p("ingest.json \u304C\u5B58\u5728\u3059\u308B\u5834\u5408\u306F\u8AAD\u307F\u8FBC\u307F\u3001\u30B9\u30AD\u30E3\u30F3\u6E08\u307F\u30FB\u5206\u985E\u5B8C\u4E86\u30FB\u30DE\u30C3\u30D4\u30F3\u30B0\u5B8C\u4E86\u30FB\u518D\u6587\u66F8\u5316\u5B8C\u4E86\u30FB\u672A\u51E6\u7406\u306E\u4EF6\u6570\u3092\u8868\u793A\u3059\u308B\u3002"));

children.push(h2("3.2 STEP 1\uFF1A\u5BFE\u8C61\u30D5\u30A9\u30EB\u30C0\u306E\u78BA\u8A8D"));
children.push(p("legacy/ \u30D5\u30A9\u30EB\u30C0\u304C\u5B58\u5728\u3059\u308B\u304B\u78BA\u8A8D\u3059\u308B\u3002\u5B58\u5728\u3057\u306A\u3044\u5834\u5408\u306F\u30E6\u30FC\u30B6\u30FC\u306B\u914D\u7F6E\u3092\u6307\u793A\u3059\u308B\u3002"));

children.push(h2("3.3 STEP 2\uFF1A\u6574\u7406\u5BFE\u8C61\u306E\u7D5E\u308A\u8FBC\u307F"));
children.push(p("legacy/ \u914D\u4E0B\u306E\u30D5\u30A1\u30A4\u30EB\u6570\u304C100\u4EF6\u3092\u8D85\u3048\u308B\u5834\u5408\u3001\u4EE5\u4E0B\u306E\u51E6\u7406\u65B9\u6CD5\u3092\u30E6\u30FC\u30B6\u30FC\u306B\u78BA\u8A8D\u3059\u308B\u3002"));
children.push(makeTable(
  ["\u9078\u629E\u80A2", "\u5185\u5BB9"],
  [
    ["A", "\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u5358\u4F4D\u3067\u51E6\u7406\u3059\u308B\uFF081\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u305A\u3064\uFF09"],
    ["B", "\u30D5\u30A1\u30A4\u30EB\u7A2E\u5225\u3067\u51E6\u7406\u3059\u308B\uFF08\u30B3\u30FC\u30C9\u2192\u30C9\u30AD\u30E5\u30E1\u30F3\u30C8\u2192\u305D\u306E\u4ED6\u306E\u9806\uFF09"],
    ["C", "\u65B0\u898F\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u3068\u306E\u95A2\u9023\u6027\u304C\u9AD8\u3044\u30D5\u30A9\u30EB\u30C0\u3092\u512A\u5148\u6307\u5B9A\u3059\u308B"],
    ["D", "\u307E\u305A\u5168\u4F53\u30B9\u30AD\u30E3\u30F3\u3060\u3051\u5B9F\u884C\u3057\u3066\u76EE\u9332\u3092\u4F5C\u308B"],
  ],
  [1200, 7826]
));

// === SECTION 4: STEP 1 SCAN ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("4. STEP 1\uFF1A\u30B9\u30AD\u30E3\u30F3\u30FB\u76EE\u9332\u4F5C\u6210 [IN-04]"));
children.push(p("トレースID: IN-04 | » IN-05", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md 'STEP 1：スキャン・目録作成' (L96-L151)", { italics: true, color: "888888", size: 14 }));

children.push(h2("4.1 \u76EE\u7684"));
children.push(p("legacy/ \u914D\u4E0B\u306E\u3059\u3079\u3066\u306E\u30D5\u30A1\u30A4\u30EB\u3092\u4E00\u89A7\u5316\u3057\u3001\u57FA\u672C\u60C5\u5831\u3092\u8A18\u9332\u3059\u308B\u3002\u30D5\u30A1\u30A4\u30EB\u3092\u958B\u3044\u3066\u5185\u5BB9\u3092\u8AAD\u3080\u524D\u306B\u300C\u4F55\u304C\u3042\u308B\u304B\u300D\u3092\u628A\u63E1\u3059\u308B\u3002"));

children.push(h2("4.2 \u30D5\u30A1\u30A4\u30EB\u7A2E\u5225\u306E\u81EA\u52D5\u5224\u5B9A\u30EB\u30FC\u30EB"));
children.push(makeTable(
  ["\u62E1\u5F35\u5B50", "\u5224\u5B9A\u7A2E\u5225"],
  [
    [".c .h .cpp .s .asm", "\u30BD\u30FC\u30B9\u30B3\u30FC\u30C9"],
    [".docx .doc .xlsx .xls", "Office\u6587\u66F8"],
    [".pdf", "PDF\u6587\u66F8"],
    [".md .txt", "\u30C6\u30AD\u30B9\u30C8\u6587\u66F8"],
    [".sch .brd .kicad_sch .DSN", "\u56DE\u8DEF\u56F3"],
    [".pcb .kicad_pcb", "\u57FA\u677F\u8A2D\u8A08"],
    [".csv", "\u30C7\u30FC\u30BF\u30FB\u30C6\u30B9\u30C8\u7D50\u679C"],
    [".zip .tar .gz", "\u30A2\u30FC\u30AB\u30A4\u30D6\uFF08\u8981\u5C55\u958B\uFF09"],
    [".png .jpg .svg .dxf", "\u56F3\u9762\u30FB\u753B\u50CF"],
    ["\u305D\u306E\u4ED6", "\u4E0D\u660E"],
  ],
  [3200, 5826]
));

children.push(h2("4.3 \u51FA\u529B\uFF1Ainventory.md"));
children.push(p("\u30B9\u30AD\u30E3\u30F3\u7D50\u679C\u306F ingest_work/inventory.md \u306B\u4FDD\u5B58\u3059\u308B\u3002\u5404\u30D5\u30A1\u30A4\u30EB\u306BID\uFF08F-XXX\uFF09\u3092\u4ED8\u4E0E\u3057\u3001\u30D1\u30B9\u30FB\u7A2E\u5225\u30FB\u30B5\u30A4\u30BA\u30FB\u66F4\u65B0\u65E5\u30FB\u5206\u985E\u72B6\u614B\u3092\u8A18\u9332\u3059\u308B\u3002"));

// === SECTION 5: STEP 2 CLASSIFICATION ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("5. STEP 2\uFF1A\u5206\u985E\u30FB\u8A55\u4FA1 [IN-05]"));
children.push(p("トレースID: IN-05 | → PR-03 | » IN-04, IN-06", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md 'STEP 2：分類・評価' (L154-L223)", { italics: true, color: "888888", size: 14 }));

children.push(h2("5.1 \u76EE\u7684"));
children.push(p("\u5404\u30D5\u30A1\u30A4\u30EB\u306E\u5185\u5BB9\u3092\u8AAD\u307F\u8FBC\u307F\u3001\u4EE5\u4E0B\u306E3\u8EF8\u3067\u8A55\u4FA1\u3059\u308B\uFF1A\u5185\u5BB9\u7A2E\u5225\uFF08\u4F55\u304C\u66F8\u304B\u308C\u3066\u3044\u308B\u304B\uFF09\u3001\u6D41\u7528\u4FA1\u5024\uFF08\u65B0\u898F\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u3078\u306E\u6D41\u7528\u53EF\u5426\uFF09\u3001V\u30E2\u30C7\u30EB\u5BFE\u5FDC\uFF08\u3069\u306E\u30D5\u30A7\u30FC\u30BA\u306B\u76F8\u5F53\u3059\u308B\u304B\uFF09\u3002"));

children.push(h2("5.2 \u30C9\u30AD\u30E5\u30E1\u30F3\u30C8\u7CFB\u5185\u5BB9\u7A2E\u5225\u5224\u5B9A\u57FA\u6E96"));
children.push(makeTable(
  ["\u30AD\u30FC\u30EF\u30FC\u30C9\u30FB\u7279\u5FB4", "\u5185\u5BB9\u7A2E\u5225", "V\u30E2\u30C7\u30EB\u30D5\u30A7\u30FC\u30BA\u5019\u88DC"],
  [
    ["HARA\u3001\u30CF\u30B6\u30FC\u30C9\u3001ASIL\u3001SIL\u3001\u30EA\u30B9\u30AF\u8A55\u4FA1", "\u6A5F\u80FD\u5B89\u5168\u5206\u6790", "PH-03"],
    ["\u5B89\u5168\u76EE\u6A19\u3001Safety Goal\u3001FSR\u3001TSR", "\u5B89\u5168\u8981\u6C42", "PH-04/05"],
    ["\u8981\u4EF6\u3001\u8981\u6C42\u3001Requirement\u3001\u4ED5\u69D8", "\u8981\u4EF6\u30FB\u4ED5\u69D8\u66F8", "PH-08"],
    ["\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3\u3001\u69CB\u6210\u3001\u30D6\u30ED\u30C3\u30AF\u56F3", "\u8A2D\u8A08\u6587\u66F8", "PH-09"],
    ["\u30C6\u30B9\u30C8\u30B1\u30FC\u30B9\u3001TC-\u3001\u8A66\u9A13\u9805\u76EE", "\u30C6\u30B9\u30C8\u4ED5\u69D8", "PH-11/13"],
    ["\u30C6\u30B9\u30C8\u7D50\u679C\u3001Pass\u3001Fail\u3001\u8A66\u9A13\u7D50\u679C", "\u30C6\u30B9\u30C8\u8A18\u9332", "PH-11/13\uFF08\u7D50\u679C\uFF09"],
    ["\u8B70\u4E8B\u9332\u3001\u6253\u3061\u5408\u308F\u305B\u3001MTG", "\u8B70\u4E8B\u9332", "\u53C2\u8003\u8CC7\u6599"],
    ["\u56DE\u8DEF\u56F3\u3001schematic\u3001BOM", "HW\u8A2D\u8A08", "PH-07"],
  ],
  [3600, 2213, 3213]
));

children.push(h2("5.3 \u30B3\u30FC\u30C9\u7CFB\u5185\u5BB9\u7A2E\u5225\u5224\u5B9A\u57FA\u6E96"));
children.push(makeTable(
  ["\u7279\u5FB4", "\u5185\u5BB9\u7A2E\u5225", "V\u30E2\u30C7\u30EB\u5019\u88DC"],
  [
    ["main\u95A2\u6570\u30FB\u521D\u671F\u5316\u51E6\u7406", "\u30E1\u30A4\u30F3\u5236\u5FA1", "PH-10"],
    ["\u5272\u308A\u8FBC\u307F\u30CF\u30F3\u30C9\u30E9\uFF08ISR\uFF09", "\u5272\u308A\u8FBC\u307F\u51E6\u7406", "PH-10"],
    ["\u30E2\u30FC\u30BF\u5236\u5FA1\u30FBPWM\u30FB\u30A8\u30F3\u30B3\u30FC\u30C0", "\u30E2\u30FC\u30BF\u5236\u5FA1", "PH-10"],
    ["\u8A3A\u65AD\u30FB\u6545\u969C\u691C\u51FA\u30FBwatchdog", "\u5B89\u5168\u6A5F\u69CB", "PH-10\uFF08\u5B89\u5168\u95A2\u9023\uFF09"],
    ["\u901A\u4FE1\uFF08UART/CAN/SPI/I2C\uFF09", "\u901A\u4FE1\u30C9\u30E9\u30A4\u30D0", "PH-10"],
    ["\u30C6\u30B9\u30C8\u30B3\u30FC\u30C9\u30FBmock\u30FBstub", "\u30C6\u30B9\u30C8\u95A2\u9023\u30B3\u30FC\u30C9", "PH-11"],
  ],
  [3600, 2213, 3213]
));

children.push(h2("5.4 \u6D41\u7528\u4FA1\u5024\u306E\u8A55\u4FA1\u57FA\u6E96"));
children.push(makeTable(
  ["\u30E9\u30F3\u30AF", "\u5B9A\u7FA9", "\u30A2\u30AF\u30B7\u30E7\u30F3"],
  [
    ["\u2B50\u2B50\u2B50 \u305D\u306E\u307E\u307E\u4F7F\u3048\u308B", "\u5185\u5BB9\u304C\u660E\u78BA\u30FB\u5B8C\u7D50\u30FB\u76F4\u63A5\u9069\u7528\u53EF\u80FD", "\u512A\u5148\u7684\u306B\u518D\u6587\u66F8\u5316"],
    ["\u2B50\u2B50 \u4FEE\u6B63\u3057\u3066\u4F7F\u3048\u308B", "\u5185\u5BB9\u306F\u6709\u52B9\u3060\u304C\u5F62\u5F0F\u30FB\u7528\u8A9E\u30FB\u30EC\u30D9\u30EB\u306E\u8ABF\u6574\u304C\u5FC5\u8981", "\u4FEE\u6B63\u3057\u3066\u518D\u6587\u66F8\u5316"],
    ["\u2B50 \u53C2\u8003\u306E\u307F", "\u5185\u5BB9\u306F\u53E4\u3044\u30FB\u65AD\u7247\u7684\u30FB\u6587\u8108\u4F9D\u5B58\u304C\u5F37\u3044", "\u53C2\u8003\u8CC7\u6599\u3068\u3057\u3066\u4FDD\u7BA1"],
    ["\u2717 \u5EC3\u68C4\u5019\u88DC", "\u91CD\u8907\u30FB\u7121\u52B9\u30FB\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u56FA\u6709\u3059\u304E\u308B", "\u30A2\u30FC\u30AB\u30A4\u30D6"],
  ],
  [2600, 3813, 2613]
));

children.push(h2("5.5 \u51FA\u529B\uFF1Aclassification.md"));
children.push(p("\u5206\u985E\u7D50\u679C\u306F ingest_work/classification.md \u306B\u4FDD\u5B58\u3059\u308B\u3002\u5404\u30D5\u30A1\u30A4\u30EBID\u30FB\u5185\u5BB9\u7A2E\u5225\u30FB\u6D41\u7528\u4FA1\u5024\u30FBV\u30E2\u30C7\u30EB\u5019\u88DC\u30FB\u6458\u8981\u3092\u8A18\u9332\u3059\u308B\u3002"));

// === SECTION 6: STEP 3 RECLASSIFICATION ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("6. STEP 3\uFF1A\u6A5F\u80FD\u30FB\u6280\u8853\u8981\u7D20\u3078\u306E\u518D\u5206\u985E [IN-06]"));
children.push(p("トレースID: IN-06 | » IN-05, IN-07", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md 'STEP 3：機能要素への再分類' (L226-L274)", { italics: true, color: "888888", size: 14 }));

children.push(h2("6.1 \u76EE\u7684"));
children.push(p("\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u5358\u4F4D\u306E\u8CC7\u7523\u3092\u6A5F\u80FD\u30FB\u6280\u8853\u8981\u7D20\u5358\u4F4D\u306B\u518D\u7DE8\u6210\u3059\u308B\u3002\u8907\u6570\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u306B\u6563\u5728\u3059\u308B\u540C\u7A2E\u306E\u8CC7\u7523\u3092\u96C6\u7D04\u3057\u3001\u6D41\u7528\u52B9\u7387\u3092\u9AD8\u3081\u308B\u3002"));

children.push(h2("6.2 \u6A5F\u80FD\u8981\u7D20\u30AB\u30C6\u30B4\u30EA\uFF08\u7D44\u307F\u8FBC\u307F\u5411\u3051\u6A19\u6E96\u5206\u985E\uFF09"));
children.push(makeTable(
  ["\u30AB\u30C6\u30B4\u30EAID", "\u6A5F\u80FD\u8981\u7D20", "\u5178\u578B\u7684\u306A\u8CC7\u7523"],
  [
    ["FE-MOT", "\u30E2\u30FC\u30BF\u5236\u5FA1", "BLDC/DC/\u30B9\u30C6\u30C3\u30D1\u5236\u5FA1\u30B3\u30FC\u30C9\u30FB\u5236\u5FA1\u4ED5\u69D8"],
    ["FE-COM", "\u901A\u4FE1", "UART/CAN/SPI/I2C/Ethernet \u30C9\u30E9\u30A4\u30D0"],
    ["FE-DIAG", "\u8A3A\u65AD\u30FB\u5B89\u5168\u6A5F\u69CB", "\u904E\u96FB\u6D41/\u904E\u6E29\u5EA6/watchdog/\u30BB\u30EB\u30D5\u30C6\u30B9\u30C8"],
    ["FE-SENS", "\u30BB\u30F3\u30B5\u51E6\u7406", "\u30A8\u30F3\u30B3\u30FC\u30C0/\u6E29\u5EA6/\u96FB\u6D41/\u96FB\u5727"],
    ["FE-PWR", "\u96FB\u6E90\u7BA1\u7406", "\u96FB\u6E90\u30B7\u30FC\u30B1\u30F3\u30B9\u30FB\u96FB\u5727\u76E3\u8996"],
    ["FE-CTRL", "\u5236\u5FA1\u6F14\u7B97", "PID/\u30D5\u30A3\u30EB\u30BF/\u72B6\u614B\u6A5F\u68B0"],
    ["FE-HMI", "UI/HMI", "\u8868\u793A\u30FB\u64CD\u4F5C\u30FB\u30D1\u30E9\u30E1\u30FC\u30BF\u8A2D\u5B9A"],
    ["FE-SAFE", "\u5B89\u5168\u95A2\u9023\u6587\u66F8", "HARA/SRS/\u5B89\u5168\u8981\u6C42/\u30C6\u30B9\u30C8\u4ED5\u69D8"],
    ["FE-HW", "HW\u8A2D\u8A08\u8CC7\u6599", "\u56DE\u8DEF\u56F3/BOM/\u57FA\u677F\u8A2D\u8A08"],
    ["FE-MISC", "\u305D\u306E\u4ED6", "\u4E0A\u8A18\u306B\u5206\u985E\u3067\u304D\u306A\u3044\u3082\u306E"],
  ],
  [1500, 2213, 5313]
));

children.push(h2("6.3 \u51FA\u529B\uFF1Amapping.md"));
children.push(p("\u518D\u5206\u985E\u7D50\u679C\u306F ingest_work/mapping.md \u306B\u4FDD\u5B58\u3059\u308B\u3002\u6A5F\u80FD\u8981\u7D20\u3054\u3068\u306B\u95A2\u9023\u30D5\u30A1\u30A4\u30EB\u30FB\u6D41\u7528\u4FA1\u5024\u30FB\u6458\u8981\u3092\u96C6\u7D04\u3059\u308B\u3002"));

// === SECTION 7: STEP 4 GAP ANALYSIS ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("7. STEP 4\uFF1A\u30AE\u30E3\u30C3\u30D7\u5206\u6790 [IN-07]"));
children.push(p("トレースID: IN-07 | ↑ SYS-05 | → PR-03 | » IN-06, IN-08", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md 'STEP 4：ギャップ分析' (L277-L326)", { italics: true, color: "888888", size: 14 }));

children.push(h2("7.1 \u76EE\u7684"));
children.push(p("\u518D\u5206\u985E\u3057\u305F\u8CC7\u7523\u3092PROCESS.md\u306EV\u30E2\u30C7\u30EB\u30D5\u30A7\u30FC\u30BA\u3068\u7167\u5408\u3057\u3001\u65B0\u898F\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u3067\u4E0D\u8DB3\u3057\u3066\u3044\u308B\u30D5\u30A7\u30FC\u30BA\u30FB\u8981\u4EF6\u30FB\u6210\u679C\u7269\u3092\u7279\u5B9A\u3059\u308B\u3002"));

children.push(h2("7.2 \u30AB\u30D0\u30EC\u30C3\u30B8\u8A55\u4FA1\u57FA\u6E96"));
children.push(makeTable(
  ["\u8A55\u4FA1", "\u610F\u5473", "\u63A8\u5968\u30A2\u30AF\u30B7\u30E7\u30F3"],
  [
    ["\u25CE \u5145\u5B9F", "\u95A2\u9023\u8CC7\u7523\u304C\u5341\u5206\u306B\u63C3\u3063\u3066\u3044\u308B", "\u6D41\u7528\u53EF\u80FD\uFF08\u30BF\u30B0\u4ED8\u4E0E\u3067\u305D\u306E\u307E\u307E\u4F7F\u7528\uFF09"],
    ["\u25B3 \u90E8\u5206\u7684", "\u4E00\u90E8\u306E\u8CC7\u7523\u306F\u3042\u308B\u304C\u4E0D\u8DB3\u304C\u3042\u308B", "\u65E2\u5B58\u8CC7\u7523\u3092\u5143\u306B\u88DC\u5B8C\u304C\u5FC5\u8981"],
    ["\u26A0\uFE0F \u8981\u66F4\u65B0", "\u53C2\u8003\u306B\u306A\u308B\u304C\u5927\u5E45\u4FEE\u6B63\u304C\u5FC5\u8981", "\u65E2\u5B58\u8CC7\u7523\u3092\u53C2\u8003\u306B\u5927\u5E45\u4FEE\u6B63"],
    ["\u2717 \u306A\u3057", "\u95A2\u9023\u8CC7\u7523\u304C\u5B58\u5728\u3057\u306A\u3044", "\u65B0\u898F\u4F5C\u6210\u304C\u5FC5\u8981"],
  ],
  [2000, 3513, 3513]
));

children.push(h2("7.3 \u51FA\u529B\uFF1Agap_analysis.md"));
children.push(p("\u30AE\u30E3\u30C3\u30D7\u5206\u6790\u7D50\u679C\u306F ingest_work/gap_analysis.md \u306B\u4FDD\u5B58\u3059\u308B\u3002V\u30E2\u30C7\u30EB\u30D5\u30A7\u30FC\u30BA\u5225\u306E\u30AB\u30D0\u30EC\u30C3\u30B8\u3001\u512A\u5148\u5BFE\u5FDC\u30EA\u30B9\u30C8\u3001\u6D41\u7528\u53EF\u80FD\u306A\u9AD8\u4FA1\u5024\u8CC7\u7523\u3092\u8A18\u9332\u3059\u308B\u3002"));

// === SECTION 8: STEP 5 RE-DOCUMENTATION ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("8. STEP 5\uFF1A\u518D\u6587\u66F8\u5316\u30FB\u8CC7\u7523\u5909\u63DB [IN-08]"));
children.push(p("トレースID: IN-08 | ↑ SYS-07 | → PR-03, PR-04 | » IN-07, IN-09", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md 'STEP 5：再文書化・資産変換' (L329-L387)", { italics: true, color: "888888", size: 14 }));

children.push(h2("8.1 \u76EE\u7684"));
children.push(p("\u5206\u985E\u30FB\u8A55\u4FA1\u6E08\u307F\u306E\u8CC7\u7523\u3092PROCESS.md\u306E\u30D5\u30A9\u30FC\u30DE\u30C3\u30C8\u306B\u5909\u63DB\u3057\u3001docs/ \u304A\u3088\u3073 src/ \u306B\u6B63\u5F0F\u306A\u6210\u679C\u7269\u3068\u3057\u3066\u51FA\u529B\u3059\u308B\u3002"));

children.push(h2("8.2 \u30C9\u30AD\u30E5\u30E1\u30F3\u30C8\u7CFB\u306E\u5909\u63DB\u624B\u9806"));
children.push(p("1. \u30D5\u30A1\u30A4\u30EB\u3092\u8AAD\u307F\u8FBC\u307F\u5185\u5BB9\u3092\u89E3\u6790\u3059\u308B"));
children.push(p("2. PROCESS.md\u306E\u5BFE\u5FDC\u30D5\u30A7\u30FC\u30BA\u306E\u30D5\u30A9\u30FC\u30DE\u30C3\u30C8\u306B\u5F93\u3063\u3066\u5909\u63DB\u3059\u308B"));
children.push(p("3. \u4E0D\u8DB3\u3057\u3066\u3044\u308B\u9805\u76EE\uFF08\u8981\u4EF6ID\u30FBSIL\u30EC\u30D9\u30EB\u30FB\u30C8\u30EC\u30FC\u30B9\u60C5\u5831\u7B49\uFF09\u3092\u88DC\u5B8C\u3059\u308B"));
children.push(p("4. \u88DC\u5B8C\u3057\u305F\u90E8\u5206\u306B\u306F\u300C\u3010AI\u88DC\u5B8C\u30FB\u8981\u78BA\u8A8D\u3011\u300D\u30BF\u30B0\u3092\u4ED8\u3051\u308B"));
children.push(p("5. docs/ \u306B\u51FA\u529B\u3059\u308B"));

children.push(h2("8.3 \u30B3\u30FC\u30C9\u7CFB\u306E\u5909\u63DB\u624B\u9806"));
children.push(p("1. \u30D5\u30A1\u30A4\u30EB\u3092\u8AAD\u307F\u8FBC\u307F\u69CB\u9020\u30FB\u6A5F\u80FD\u30FB\u5B89\u5168\u95A2\u9023\u90E8\u5206\u3092\u89E3\u6790\u3059\u308B"));
children.push(p("2. \u5B89\u5168\u95A2\u9023\u95A2\u6570\u306B @req / @asil / @test \u30BF\u30B0\u3092\u8FFD\u52A0\u3059\u308B\uFF08TBD\u53EF\uFF09"));
children.push(p("3. \u30BF\u30B0\u3092\u4ED8\u3051\u3089\u308C\u306A\u304B\u3063\u305F\u7B87\u6240\u306B\u300C/* [\u8981\u4EF6ID\u672A\u4ED8\u4E0E - \u8981\u78BA\u8A8D] */\u300D\u3092\u4ED8\u3051\u308B"));
children.push(p("4. src/ \u306B\u51FA\u529B\u3059\u308B"));
children.push(p("5. \u5BFE\u5FDC\u3059\u308B\u30E6\u30CB\u30C3\u30C8\u8A2D\u8A08\u66F8\u306E\u9805\u76EE\u3092 docs/10_SW\u30E6\u30CB\u30C3\u30C8\u8A2D\u8A08.md \u306B\u8FFD\u8A18\u3059\u308B"));

children.push(h2("8.4 \u3010AI\u88DC\u5B8C\u30FB\u8981\u78BA\u8A8D\u3011\u30BF\u30B0\u306E\u4F7F\u7528\u30EB\u30FC\u30EB"));
children.push(makeTable(
  ["\u9805\u76EE", "\u30EB\u30FC\u30EB"],
  [
    ["\u4ED8\u4E0E\u5BFE\u8C61", "AI\u304C\u63A8\u6E2C\u30FB\u751F\u6210\u3057\u305F\u5185\u5BB9\u306B\u306F\u5FC5\u305A\u4ED8\u4E0E\u3059\u308B"],
    ["\u6280\u8853\u5224\u65AD", "\u30E6\u30FC\u30B6\u30FC\u306B\u78BA\u8A8D\u304C\u5FC5\u8981\u306A\u6280\u8853\u5224\u65AD\u306B\u306F\u5FC5\u305A\u4ED8\u4E0E\u3059\u308B"],
    ["\u5B89\u5168\u8981\u6C42\u30FBASIL\u30FBDC", "\u7279\u306B\u53B3\u683C\u306B\u4ED8\u4E0E\u3059\u308B\uFF08\u5B89\u5168\u306B\u76F4\u7D50\u3059\u308B\u305F\u3081\uFF09"],
    ["\u8A8D\u8A3C\u30E2\u30FC\u30C9\u79FB\u884C\u524D", "\u3059\u3079\u3066\u306E\u3010AI\u88DC\u5B8C\u30FB\u8981\u78BA\u8A8D\u3011\u3092\u89E3\u6D88\u3059\u308B\u3053\u3068"],
  ],
  [3000, 6026]
));

children.push(h2("8.5 \u51FA\u529B\uFF1Areuse_log.md"));
children.push(p("\u518D\u5229\u7528\u30FB\u518D\u6587\u66F8\u5316\u306E\u8A18\u9332\u306F ingest_work/reuse_log.md \u306B\u4FDD\u5B58\u3059\u308B\u3002\u5143\u30D5\u30A1\u30A4\u30EBID\u30FB\u51FA\u529B\u5148\u30FB\u5909\u63DB\u7A2E\u5225\u30FBAI\u88DC\u5B8C\u7B87\u6240\u30FB\u78BA\u8A8D\u8005\u30FB\u78BA\u8A8D\u65E5\u3092\u8A18\u9332\u3059\u308B\u3002"));

// === SECTION 9: STEP 6 SUMMARY ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("9. STEP 6\uFF1A\u6574\u7406\u5B8C\u4E86\u30B5\u30DE\u30EA\u30FC [IN-09]"));
children.push(p("トレースID: IN-09 | » IN-08", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md 'STEP 6：整理完了サマリー' (L390-L429)", { italics: true, color: "888888", size: 14 }));

children.push(h2("9.1 \u76EE\u7684"));
children.push(p("INGEST\u30D7\u30ED\u30BB\u30B9\u306E\u5168\u4F53\u7D50\u679C\u3092\u53EF\u8996\u5316\u3057\u3001\u6B21\u306E\u30A2\u30AF\u30B7\u30E7\u30F3\u3092\u63D0\u793A\u3059\u308B\u3002"));

children.push(h2("9.2 \u30B5\u30DE\u30EA\u30FC\u51FA\u529B\u9805\u76EE"));
children.push(p("\u51E6\u7406\u6E08\u307F\u8CC7\u7523\u6570\uFF08\u30B9\u30AD\u30E3\u30F3\u30FB\u5206\u985E\u5B8C\u4E86\u30FB\u518D\u6587\u66F8\u5316\u5B8C\u4E86\uFF09"));
children.push(p("V\u30E2\u30C7\u30EB\u30AB\u30D0\u30EC\u30C3\u30B8\uFF08\u5404\u30D5\u30A7\u30FC\u30BA\u306E\u8CC7\u7523\u5145\u8DB3\u72B6\u6CC1\uFF09"));
children.push(p("AI\u88DC\u5B8C\u30FB\u8981\u78BA\u8A8D\u30BF\u30B0\u306E\u4EF6\u6570\uFF08\u8A8D\u8A3C\u524D\u306B\u89E3\u6D88\u304C\u5FC5\u8981\uFF09"));
children.push(p("\u9AD8\u4FA1\u5024\u8CC7\u7523\uFF08\u2B50\u2B50\u2B50\uFF09\u306E\u4EF6\u6570\u3068\u518D\u6587\u66F8\u5316\u5B8C\u4E86\u6570"));
children.push(p("\u63A8\u5968\u30CD\u30AF\u30B9\u30C8\u30A2\u30AF\u30B7\u30E7\u30F3"));

// === SECTION 10: SPECIAL CASES ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("10. \u7279\u6B8A\u30B1\u30FC\u30B9\u306E\u53D6\u308A\u6271\u3044 [IN-10]"));

children.push(h2("10.1 \u8AAD\u307F\u53D6\u308A\u4E0D\u53EF\u30D5\u30A1\u30A4\u30EB"));
children.push(p("Claude Code\u304C\u76F4\u63A5\u8AAD\u3081\u306A\u3044\u30D5\u30A1\u30A4\u30EB\uFF08\u7D19\u30FB\u624B\u66F8\u304D\u30FB\u7279\u6B8A\u30D5\u30A9\u30FC\u30DE\u30C3\u30C8\u7B49\uFF09\u306E\u5834\u5408\u3001\u30E6\u30FC\u30B6\u30FC\u306B\u5BFE\u5FDC\u65B9\u6CD5\u3092\u78BA\u8A8D\u3059\u308B\uFF1A\u5185\u5BB9\u3092\u30C6\u30AD\u30B9\u30C8\u3067\u8CBC\u308A\u4ED8\u3051 / \u30B9\u30AD\u30C3\u30D7\u3057\u3066\u5F8C\u3067\u51E6\u7406 / \u5EC3\u68C4\u5019\u88DC\u3068\u3057\u3066\u30DE\u30FC\u30AF\u3002"));

children.push(h2("10.2 \u6A5F\u5BC6\u60C5\u5831\u30FB\u9867\u5BA2\u56FA\u6709\u60C5\u5831\u306E\u691C\u51FA"));
children.push(p("\u9867\u5BA2\u540D\u30FB\u88FD\u54C1\u578B\u756A\u30FB\u6A5F\u5BC6\u30DE\u30FC\u30AD\u30F3\u30B0\u304C\u542B\u307E\u308C\u308B\u30D5\u30A1\u30A4\u30EB\u3092\u691C\u51FA\u3057\u305F\u5834\u5408\u3001\u30E6\u30FC\u30B6\u30FC\u306B\u5BFE\u5FDC\u65B9\u6CD5\u3092\u78BA\u8A8D\u3059\u308B\uFF1A\u6A5F\u5BC6\u60C5\u5831\u3092\u30DE\u30B9\u30AF\u3057\u3066\u51E6\u7406 / \u51E6\u7406\u5BFE\u8C61\u304B\u3089\u9664\u5916 / \u305D\u306E\u307E\u307E\u51E6\u7406\uFF08\u8CAC\u4EFB\u306F\u30E6\u30FC\u30B6\u30FC\uFF09\u3002"));

children.push(h2("10.3 \u91CD\u8907\u30D5\u30A1\u30A4\u30EB\u306E\u691C\u51FA"));
children.push(p("\u540C\u4E00\u307E\u305F\u306F\u985E\u4F3C\u5185\u5BB9\u306E\u30D5\u30A1\u30A4\u30EB\u304C\u8907\u6570\u691C\u51FA\u3055\u308C\u305F\u5834\u5408\u3001\u6700\u65B0\u7248\u3092\u4E3B\u8CC7\u7523\u3068\u3057\u3066\u63A1\u7528\u3057\u3001\u65E7\u7248\u306F\u5DEE\u5206\u53C2\u7167\u3068\u3057\u3066\u4FDD\u7BA1\u3059\u308B\u3053\u3068\u3092\u63A8\u5968\u3059\u308B\u3002\u63A1\u7528\u3059\u308B\u7248\u306F\u30E6\u30FC\u30B6\u30FC\u304C\u6307\u5B9A\u3059\u308B\u3002"));

// === SECTION 11: PROCESS.md INTEGRATION ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("11. PROCESS.md \u3068\u306E\u9023\u643A [IN-11]"));
children.push(p("トレースID: IN-11 | ↑ SYS-05, SYS-08 | → PR-02", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md 'INGEST完了後のCLAUDE.mdとの連携' (L514-L540)", { italics: true, color: "888888", size: 14 }));

children.push(h2("11.1 INGEST\u5B8C\u4E86\u5F8C\u306E\u30D5\u30ED\u30FC"));
children.push(p("1. INGEST\u5B8C\u4E86\u30B5\u30DE\u30EA\u30FC\u306E\u300C\u63A8\u5968\u30CD\u30AF\u30B9\u30C8\u30A2\u30AF\u30B7\u30E7\u30F3\u300D\u3092\u78BA\u8A8D"));
children.push(p("2. \u30AE\u30E3\u30C3\u30D7\u5206\u6790\u3067\u300C\u65B0\u898F\u4F5C\u6210\u304C\u5FC5\u8981\u300D\u306A\u30D5\u30A7\u30FC\u30BA\u3092PROCESS.md\u3067\u5B9F\u884C"));
children.push(p("3. \u300C\u88DC\u5B8C\u304C\u5FC5\u8981\u300D\u306A\u30D5\u30A7\u30FC\u30BA\u306F\u65E2\u5B58\u8CC7\u7523\u3092\u53C2\u7167\u3057\u306A\u304C\u3089PROCESS.md\u3067\u88DC\u5B8C"));
children.push(p("4. \u300C\u6D41\u7528\u53EF\u80FD\u300D\u306A\u8CC7\u7523\u306Fdocs/\u30FBsrc/\u306B\u30B3\u30D4\u30FC\u6E08\u307F\u306A\u306E\u3067\u305D\u306E\u307E\u307E\u4F7F\u7528"));
children.push(p("5. \u3059\u3079\u3066\u306E\u3010AI\u88DC\u5B8C\u30FB\u8981\u78BA\u8A8D\u3011\u30BF\u30B0\u3092\u89E3\u6D88\u3057\u3066\u304B\u3089\u8A8D\u8A3C\u30E2\u30FC\u30C9\u3078"));

children.push(h2("11.2 project.json \u3078\u306E\u53CD\u6620"));
children.push(p("INGEST\u5B8C\u4E86\u5F8C\u3001project.json \u306E\u5404\u30D5\u30A7\u30FC\u30BA\u306B\u6D41\u7528\u8CC7\u7523\u60C5\u5831\uFF08legacySources\uFF09\u3068AI\u88DC\u5B8C\u4EF6\u6570\uFF08aiSupplementCount\uFF09\u3092\u8FFD\u8A18\u3059\u308B\u3002"));

// === SECTION 12: NOTES ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("12. \u6CE8\u610F\u4E8B\u9805 [IN-12]"));
children.push(p("トレースID: IN-12 | ▲ SYS-07", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: INGEST.md '注意事項' (L544-L550)", { italics: true, color: "888888", size: 14 }));
children.push(p("\u65E2\u5B58\u8CC7\u7523\u306E\u8457\u4F5C\u6A29\u30FB\u6A5F\u5BC6\u6027\u306B\u6CE8\u610F\u3059\u308B\u3053\u3068\u3002\u9867\u5BA2\u56FA\u6709\u60C5\u5831\u306F\u5FC5\u305A\u30DE\u30B9\u30AF\u3059\u308B\u3053\u3068\u3002"));
children.push(p("AI\u306E\u89E3\u6790\u30FB\u88DC\u5B8C\u7D50\u679C\u306F\u3042\u304F\u307E\u3067\u53C2\u8003\u3067\u3042\u308B\u3002\u6280\u8853\u7684\u306A\u5224\u65AD\u306F\u5FC5\u305A\u30A8\u30F3\u30B8\u30CB\u30A2\u304C\u78BA\u8A8D\u3059\u308B\u3053\u3068\u3002"));
children.push(p("\u7279\u306B\u5B89\u5168\u8981\u6C42\u30FBSIL\u30EC\u30D9\u30EB\u30FB\u8A3A\u65AD\u30AB\u30D0\u30EC\u30C3\u30B8\u306E\u88DC\u5B8C\u5024\u306F\u3010AI\u88DC\u5B8C\u30FB\u8981\u78BA\u8A8D\u3011\u30BF\u30B0\u3092\u5FC5\u305A\u4ED8\u4E0E\u3059\u308B\u3053\u3068\u3002"));
children.push(p("\u8A8D\u8A3C\u30E2\u30FC\u30C9\u79FB\u884C\u524D\u306B\u3059\u3079\u3066\u306E\u3010AI\u88DC\u5B8C\u30FB\u8981\u78BA\u8A8D\u3011\u30BF\u30B0\u3092\u89E3\u6D88\u3059\u308B\u3053\u3068\u3002"));

// BUILD DOC
const doc = new Document({
  styles: {
    default: { document: { run: { font: "Arial", size: 20 } } },
    paragraphStyles: [
      { id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 32, bold: true, font: "Arial", color: "2E5D34" },
        paragraph: { spacing: { before: 360, after: 200 }, outlineLevel: 0 } },
      { id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 26, bold: true, font: "Arial", color: "3B8745" },
        paragraph: { spacing: { before: 280, after: 160 }, outlineLevel: 1 } },
      { id: "Heading3", name: "Heading 3", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 22, bold: true, font: "Arial", color: "404040" },
        paragraph: { spacing: { before: 200, after: 120 }, outlineLevel: 2 } },
    ]
  },
  sections: [{
    properties: {
      page: {
        size: { width: 11906, height: 16838 },
        margin: { top: 1440, right: 1440, bottom: 1440, left: 1440 }
      }
    },
    headers: {
      default: new Header({ children: [
        new Paragraph({
          alignment: AlignmentType.RIGHT,
          border: { bottom: { style: BorderStyle.SINGLE, size: 4, color: "2E5D34", space: 1 } },
          children: [new TextRun({ text: "TSDT-IN-001 v1.0  |  \u65E2\u5B58\u8CC7\u7523 \u53D6\u308A\u8FBC\u307F\u30FB\u518D\u6574\u7406\u624B\u9806\u66F8", font: "Arial", size: 16, color: "888888" })]
        })
      ] })
    },
    footers: {
      default: new Footer({ children: [
        new Paragraph({
          alignment: AlignmentType.CENTER,
          children: [new TextRun({ text: "TORASAN \u6A5F\u80FD\u5B89\u5168\u958B\u767A\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8  |  Page ", font: "Arial", size: 16, color: "888888" }),
            new TextRun({ children: [PageNumber.CURRENT], font: "Arial", size: 16, color: "888888" })]
        })
      ] })
    },
    children
  }]
});

const outPath = "/sessions/quirky-friendly-franklin/mnt/TORASAN/INGEST_\u4ED5\u69D8\u66F8.docx";
Packer.toBuffer(doc).then(buf => {
  fs.writeFileSync(outPath, buf);
  console.log("Created: " + outPath);
  // 自動検証
  const { execSync } = require("child_process");
  try {
    execSync(`node validate_docx.js "${outPath}"`, { stdio: "inherit" });
  } catch (e) {
    console.error("⛔ 検証失敗: 誤記またはXML不正が検出されました。修正してください。");
    process.exit(1);
  }
});
