const fs = require("fs");
const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell,
  Header, Footer, AlignmentType, LevelFormat,
  TableOfContents, HeadingLevel, BorderStyle, WidthType, ShadingType,
  PageNumber, PageBreak, ImageRun
} = require("docx");

// ====== STYLING CONSTANTS ======
const border = { style: BorderStyle.SINGLE, size: 1, color: "999999" };
const borders = { top: border, bottom: border, left: border, right: border };
const cellMargins = { top: 60, bottom: 60, left: 100, right: 100 };
const hdrShade = { fill: "1A3A5C", type: ShadingType.CLEAR };
const altShade = { fill: "EDF2F7", type: ShadingType.CLEAR };
const noShade = { fill: "FFFFFF", type: ShadingType.CLEAR };
const accentShade = { fill: "FFF8E1", type: ShadingType.CLEAR };
const TW = 9026; // A4 content width (1" margins)

// ====== HELPER FUNCTIONS ======
function hc(text, width, shade) {
  return new TableCell({
    borders, width: { size: width, type: WidthType.DXA }, shading: shade || hdrShade, margins: cellMargins,
    verticalAlign: "center",
    children: [new Paragraph({ alignment: AlignmentType.CENTER, children: [new TextRun({ text, bold: true, font: "Arial", size: 20, color: "FFFFFF" })] })]
  });
}
function c(text, width, opts = {}) {
  return new TableCell({
    borders, width: { size: width, type: WidthType.DXA },
    shading: opts.accent ? accentShade : opts.shaded ? altShade : noShade, margins: cellMargins,
    columnSpan: opts.span || undefined,
    children: Array.isArray(opts.children) ? opts.children : [new Paragraph({
      alignment: opts.center ? AlignmentType.CENTER : AlignmentType.LEFT,
      children: [new TextRun({ text, font: "Arial", size: 20, bold: !!opts.bold })]
    })]
  });
}
function h1(t) { return new Paragraph({ heading: HeadingLevel.HEADING_1, spacing: { before: 400, after: 220 }, children: [new TextRun({ text: t, font: "Arial", size: 36, bold: true, color: "1A3A5C" })] }); }
function h2(t) { return new Paragraph({ heading: HeadingLevel.HEADING_2, spacing: { before: 300, after: 180 }, children: [new TextRun({ text: t, font: "Arial", size: 28, bold: true, color: "2E6B9E" })] }); }
function h3(t) { return new Paragraph({ heading: HeadingLevel.HEADING_3, spacing: { before: 220, after: 120 }, children: [new TextRun({ text: t, font: "Arial", size: 24, bold: true, color: "404040" })] }); }
function p(t, opts = {}) { return new Paragraph({ spacing: { after: 140, line: 340 }, indent: opts.indent ? { left: opts.indent } : undefined, children: [new TextRun({ text: t, font: "Arial", size: 21, ...opts })] }); }
function pMulti(runs) { return new Paragraph({ spacing: { after: 140, line: 340 }, children: runs.map(r => new TextRun({ font: "Arial", size: 21, ...r })) }); }
function pBold(label, text) { return pMulti([{ text: label, bold: true }, { text }]); }
function makeTable(headers, rows, colWidths) {
  return new Table({
    width: { size: colWidths.reduce((a, b) => a + b, 0), type: WidthType.DXA }, columnWidths: colWidths,
    rows: [
      new TableRow({ children: headers.map((h, i) => hc(h, colWidths[i])) }),
      ...rows.map((row, ri) => new TableRow({
        children: row.map((txt, ci) => c(txt, colWidths[ci], { shaded: ri % 2 === 1, center: ci === 0 }))
      }))
    ]
  });
}
function spacer(pts) { return new Paragraph({ spacing: { before: pts || 200 }, children: [] }); }

const children = [];

// ===========================
// COVER PAGE
// ===========================
children.push(spacer(2400));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 80 },
  children: [new TextRun({ text: "TORASAN", font: "Arial", size: 36, color: "1A3A5C", bold: true })] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 300 },
  children: [new TextRun({ text: "\u6A5F\u80FD\u5B89\u5168\u958B\u767A\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8", font: "Arial", size: 26, color: "666666" })] }));

children.push(new Paragraph({ alignment: AlignmentType.CENTER, border: { top: { style: BorderStyle.SINGLE, size: 8, color: "1A3A5C", space: 1 } }, spacing: { before: 100 }, children: [] }));

children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 400, after: 120 },
  children: [new TextRun({ text: "AI\u652F\u63F4\u6A5F\u80FD\u5B89\u5168\u958B\u767A\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF", font: "Arial", size: 52, bold: true, color: "1A3A5C" })] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 80 },
  children: [new TextRun({ text: "\u30B7\u30B9\u30C6\u30E0\u4ED5\u69D8\u66F8", font: "Arial", size: 36, bold: true, color: "2E6B9E" })] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 200 },
  children: [new TextRun({ text: "\u2014 \u30B3\u30F3\u30BB\u30D7\u30C8\u30FB\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3\u30FB\u6587\u66F8\u4F53\u7CFB \u2014", font: "Arial", size: 24, color: "888888", italics: true })] }));

children.push(new Paragraph({ alignment: AlignmentType.CENTER, border: { bottom: { style: BorderStyle.SINGLE, size: 8, color: "1A3A5C", space: 1 } }, spacing: { after: 400 }, children: [] }));

const infoW = [2800, 6226];
const infoData = [
  ["\u6587\u66F8\u756A\u53F7", "TSDT-SYS-001"],
  ["\u7248\u6570", "1.0"],
  ["\u4F5C\u6210\u65E5", "2026-02-27"],
  ["\u30B9\u30C6\u30FC\u30BF\u30B9", "\u8349\u6848"],
  ["\u4F5C\u6210\u8005", "TORASAN"],
  ["\u627F\u8A8D\u8005", "TBD"],
  ["\u5206\u985E", "\u793E\u5185\u6280\u8853\u6587\u66F8\uFF08\u30A8\u30F3\u30B8\u30CB\u30A2\u5411\u3051\uFF09"],
];
children.push(new Table({
  width: { size: TW, type: WidthType.DXA }, columnWidths: infoW,
  rows: infoData.map(r => new TableRow({
    children: [c(r[0], infoW[0], { bold: true, shaded: true }), c(r[1], infoW[1])]
  }))
}));

children.push(new Paragraph({ children: [new PageBreak()] }));

// ===========================
// REVISION HISTORY
// ===========================
children.push(h1("\u6539\u7248\u5C65\u6B74"));
children.push(makeTable(
  ["\u7248\u6570", "\u65E5\u4ED8", "\u5909\u66F4\u5185\u5BB9", "\u4F5C\u6210\u8005"],
  [["1.0", "2026-02-27", "\u521D\u7248\u4F5C\u6210", "TORASAN"]],
  [1200, 1800, 4226, 1800]
));
children.push(new Paragraph({ children: [new PageBreak()] }));

// ===========================
// TOC
// ===========================
children.push(h1("\u76EE\u6B21"));
children.push(new TableOfContents("Table of Contents", { hyperlink: true, headingStyleRange: "1-3" }));
children.push(new Paragraph({ children: [new PageBreak()] }));

// ===========================
// 1. INTRODUCTION
// ===========================
children.push(h1("1. \u306F\u3058\u3081\u306B [SYS-01]"));
children.push(p("トレースID: SYS-01 | ↓ PR-01, IN-01", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md \'このファイルについて\' (L1-L21) / INGEST.md \'このファイルについて\' (L1-L26) / CLAUDE.md 全体", { italics: true, color: "888888", size: 14 }));

children.push(h2("1.1 \u672C\u6587\u66F8\u306E\u76EE\u7684\u3068\u4F4D\u7F6E\u3065\u3051"));
children.push(p("\u672C\u6587\u66F8\u306F\u3001TORASAN\u6A5F\u80FD\u5B89\u5168\u958B\u767A\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u306B\u304A\u3051\u308BAI\u652F\u63F4\u958B\u767A\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u306E\u6700\u4E0A\u4F4D\u6587\u66F8\u3067\u3042\u308B\u3002\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u5168\u4F53\u306E\u30B3\u30F3\u30BB\u30D7\u30C8\u3001\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3\u3001\u6587\u66F8\u4F53\u7CFB\u3001\u304A\u3088\u3073\u5404\u6587\u66F8\u9593\u306E\u95A2\u4FC2\u6027\u3092\u5B9A\u7FA9\u3057\u3001\u958B\u767A\u30C1\u30FC\u30E0\u304C\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u5168\u4F53\u3092\u7406\u89E3\u3059\u308B\u305F\u3081\u306E\u57FA\u76E4\u3068\u306A\u308B\u3002"));
children.push(p("\u672C\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u306F\u3001ISO 26262\u3092\u4E2D\u5FC3\u3068\u3059\u308B\u6A5F\u80FD\u5B89\u5168\u898F\u683C\u306B\u6E96\u62E0\u3057\u305F\u7D44\u307F\u8FBC\u307F\u88FD\u54C1\u958B\u767A\u3092\u3001Anthropic Claude Code\uFF08AI\u30B3\u30FC\u30C9\u751F\u6210\u30FB\u6587\u66F8\u751F\u6210\u30C4\u30FC\u30EB\uFF09\u3067\u52B9\u7387\u7684\u306B\u652F\u63F4\u3059\u308B\u3053\u3068\u3092\u76EE\u7684\u3068\u3059\u308B\u3002"));

children.push(h2("1.2 \u5BFE\u8C61\u8AAD\u8005"));
children.push(p("\u672C\u6587\u66F8\u306E\u4E3B\u305F\u308B\u5BFE\u8C61\u8AAD\u8005\u306F\u4EE5\u4E0B\u306E\u793E\u5185\u30A8\u30F3\u30B8\u30CB\u30A2\u3067\u3042\u308B\u3002"));
children.push(makeTable(
  ["\u5F79\u5272", "\u672C\u6587\u66F8\u306E\u6D3B\u7528\u65B9\u6CD5"],
  [
    ["\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u30EA\u30FC\u30C0\u30FC", "\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u5168\u4F53\u50CF\u306E\u628A\u63E1\u30FB\u30C1\u30FC\u30E0\u3078\u306E\u5C55\u958B"],
    ["SW\u30A8\u30F3\u30B8\u30CB\u30A2", "AI\u652F\u63F4\u958B\u767A\u306E\u5177\u4F53\u7684\u306A\u904B\u7528\u30D5\u30ED\u30FC\u306E\u7406\u89E3"],
    ["HW\u30A8\u30F3\u30B8\u30CB\u30A2", "HW/SW\u5206\u62C5\u3068AI\u62C5\u5F53\u7BC4\u56F2\u306E\u78BA\u8A8D"],
    ["\u54C1\u8CEA\u4FDD\u8A3C\u30FB\u5B89\u5168\u62C5\u5F53", "\u30C4\u30FC\u30EB\u9069\u683C\u6027\u30FB\u30EC\u30D3\u30E5\u30FC\u4F53\u5236\u306E\u78BA\u8A8D"],
    ["\u65B0\u898F\u53C2\u52A0\u30E1\u30F3\u30D0\u30FC", "\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u306E\u30AA\u30F3\u30DC\u30FC\u30C7\u30A3\u30F3\u30B0\u8CC7\u6599"],
  ],
  [2800, 6226]
));

children.push(h2("1.3 \u7528\u8A9E\u5B9A\u7FA9"));
children.push(makeTable(
  ["\u7528\u8A9E", "\u5B9A\u7FA9"],
  [
    ["Claude Code", "Anthropic\u793E\u306EAI\u30B3\u30FC\u30C9\u751F\u6210\u30FB\u6587\u66F8\u751F\u6210\u30C4\u30FC\u30EB\u3002\u672C\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u306E\u5B9F\u884C\u57FA\u76E4"],
    ["V\u30E2\u30C7\u30EB", "ISO 26262\u306B\u57FA\u3065\u304F\u958B\u767A\u30E9\u30A4\u30D5\u30B5\u30A4\u30AF\u30EB\u30E2\u30C7\u30EB\u3002\u5DE6\u8FBA=\u8A2D\u8A08\u30FB\u8981\u6C42\u3001\u53F3\u8FBA=\u691C\u8A3C\u30FB\u30C6\u30B9\u30C8"],
    ["\u63A2\u7D22\u30E2\u30FC\u30C9", "\u30D7\u30ED\u30C8\u30BF\u30A4\u30D7\u30FB\u30B3\u30F3\u30BB\u30D7\u30C8\u691C\u8A0E\u6BB5\u968E\u306E\u67D4\u8EDF\u306A\u904B\u7528\u30E2\u30FC\u30C9"],
    ["\u8A8D\u8A3C\u30E2\u30FC\u30C9", "\u898F\u683C\u8A8D\u8A3C\u53D6\u5F97\u306B\u5411\u3051\u305F\u53B3\u5BC6\u306A\u904B\u7528\u30E2\u30FC\u30C9"],
    ["ASIL", "Automotive Safety Integrity Level\u3002ISO 26262\u306E\u5B89\u5168\u5EA6\u6C34\u6E96\uFF08A\u301CD\uFF09"],
    ["TCL", "Tool Confidence Level\u3002\u30C4\u30FC\u30EB\u306E\u4FE1\u983C\u6027\u30EC\u30D9\u30EB\uFF08TCL1\u301CTCL3\uFF09"],
    ["INGEST", "\u65E2\u5B58\u8CC7\u7523\u306E\u53D6\u308A\u8FBC\u307F\u30FB\u518D\u6574\u7406\u30D7\u30ED\u30BB\u30B9"],
    ["\u30C8\u30EC\u30FC\u30B9", "\u8981\u4EF6\u9593\u306E\u53CC\u65B9\u5411\u8FFD\u8DE1\u53EF\u80FD\u6027\uFF08Traceability\uFF09"],
  ],
  [2400, 6626]
));

// ===========================
// 2. CONCEPT
// ===========================
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("2. \u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u30B3\u30F3\u30BB\u30D7\u30C8 [SYS-02]"));
children.push(p("トレースID: SYS-02 | ↓ PR-01, IN-01", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md '設計思想' (L13-L21) / INGEST.md '設計思想' (L19-L26)", { italics: true, color: "888888", size: 14 }));

children.push(h2("2.1 \u89E3\u6C7A\u3059\u308B\u8AB2\u984C"));
children.push(p("\u6A5F\u80FD\u5B89\u5168\u898F\u683C\u306B\u6E96\u62E0\u3057\u305F\u7D44\u307F\u8FBC\u307F\u88FD\u54C1\u958B\u767A\u306F\u3001\u8986\u5927\u306A\u6587\u66F8\u4F5C\u6210\u30FB\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3\u7BA1\u7406\u30FB\u5909\u66F4\u5F71\u97FF\u5206\u6790\u7B49\u3001\u4EBA\u7684\u30EA\u30BD\u30FC\u30B9\u3078\u306E\u8CA0\u8377\u304C\u6975\u3081\u3066\u5927\u304D\u3044\u3002\u7279\u306B\u4EE5\u4E0B\u306E\u8AB2\u984C\u304C\u6DF1\u523B\u3067\u3042\u308B\u3002"));

children.push(pBold("(1) \u6587\u66F8\u4F5C\u6210\u30B3\u30B9\u30C8\uFF1A", "\u898F\u683C\u304C\u8981\u6C42\u3059\u308B\u6210\u679C\u7269\u306F15\u30D5\u30A7\u30FC\u30BA\u4EE5\u4E0A\u306B\u53CA\u3073\u3001\u5404\u30D5\u30A7\u30FC\u30BA\u3067\u6B63\u5F0F\u306A\u6587\u66F8\u304C\u5FC5\u8981\u3002\u624B\u4F5C\u696D\u3067\u306F\u5DE5\u6570\u304C\u81A8\u5927\u3059\u308B\u3002"));
children.push(pBold("(2) \u30C8\u30EC\u30FC\u30B9\u7BA1\u7406\u306E\u8907\u96D1\u3055\uFF1A", "\u5B89\u5168\u76EE\u6A19\u304B\u3089\u30C6\u30B9\u30C8\u30B1\u30FC\u30B9\u307E\u3067\u306E\u53CC\u65B9\u5411\u30C8\u30EC\u30FC\u30B9\u3092\u7DAD\u6301\u3057\u7D9A\u3051\u308B\u306E\u306F\u6975\u3081\u3066\u56F0\u96E3\u3002"));
children.push(pBold("(3) \u5909\u66F4\u5F71\u97FF\u306E\u8FFD\u8DE1\uFF1A", "MCU\u5909\u66F4\u3084ASIL\u30EC\u30D9\u30EB\u5909\u66F4\u6642\u3001\u5F71\u97FF\u7BC4\u56F2\u306E\u7279\u5B9A\u3068\u5168\u6587\u66F8\u306E\u6574\u5408\u6027\u7DAD\u6301\u304C\u56F0\u96E3\u3002"));
children.push(pBold("(4) \u65E2\u5B58\u8CC7\u7523\u306E\u6D3B\u7528\uFF1A", "\u904E\u53BB\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u306E\u30B3\u30FC\u30C9\u3084\u6587\u66F8\u304C\u6563\u5728\u3057\u3001\u4F53\u7CFB\u7684\u306A\u6D41\u7528\u304C\u3067\u304D\u3066\u3044\u306A\u3044\u3002"));

children.push(h2("2.2 \u30BD\u30EA\u30E5\u30FC\u30B7\u30E7\u30F3\u30A2\u30D7\u30ED\u30FC\u30C1"));
children.push(p("\u672C\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u306F\u3001Claude Code\uFF08AI\uFF09\u3092\u300C\u30C4\u30FC\u30EB\u300D\u3068\u3057\u3066\u6D3B\u7528\u3057\u3001\u4E0A\u8A18\u8AB2\u984C\u3092\u89E3\u6C7A\u3059\u308B\u3002\u305F\u3060\u3057\u3001AI\u306F\u3042\u304F\u307E\u3067\u300C\u652F\u63F4\u300D\u3067\u3042\u308A\u3001\u6700\u7D42\u7684\u306A\u6280\u8853\u5224\u65AD\u30FB\u627F\u8A8D\u306F\u5FC5\u305A\u4EBA\u9593\uFF08\u30A8\u30F3\u30B8\u30CB\u30A2\uFF09\u304C\u884C\u3046\u3002"));

children.push(h3("AI\u3068\u4EBA\u9593\u306E\u5F79\u5272\u5206\u62C5\u539F\u5247"));
children.push(makeTable(
  ["\u9818\u57DF", "AI\uFF08Claude Code\uFF09\u304C\u62C5\u5F53", "\u4EBA\u9593\u304C\u62C5\u5F53"],
  [
    ["\u6587\u66F8\u751F\u6210", "\u30C6\u30F3\u30D7\u30EC\u30FC\u30C8\u306B\u57FA\u3065\u304F\u6210\u679C\u7269\u306E\u81EA\u52D5\u751F\u6210", "\u5185\u5BB9\u306E\u6280\u8853\u7684\u6B63\u78BA\u6027\u78BA\u8A8D\u30FB\u627F\u8A8D"],
    ["\u30C8\u30EC\u30FC\u30B9\u7BA1\u7406", "\u8981\u4EF6ID\u306E\u81EA\u52D5\u4ED8\u4E0E\u30FB\u53CC\u65B9\u5411\u30EA\u30F3\u30AF\u306E\u7DAD\u6301", "\u8981\u4EF6\u306E\u59A5\u5F53\u6027\u5224\u65AD"],
    ["\u5909\u66F4\u5F71\u97FF\u5206\u6790", "\u5F71\u97FF\u30D5\u30A7\u30FC\u30BA\u30FB\u5F71\u97FF\u8981\u4EF6\u306E\u5373\u6642\u7279\u5B9A", "\u5909\u66F4\u306E\u627F\u8A8D\u30FB\u5BFE\u5FDC\u65B9\u91DD\u306E\u6C7A\u5B9A"],
    ["\u30B3\u30FC\u30C9\u751F\u6210", "\u5B89\u5168\u30BF\u30B0\u4ED8\u304D\u30B3\u30FC\u30C9\u306E\u751F\u6210\u30FB\u88DC\u5B8C", "\u30B3\u30FC\u30C9\u30EC\u30D3\u30E5\u30FC\u30FB\u5B9F\u6A5F\u30C6\u30B9\u30C8"],
    ["\u30C6\u30B9\u30C8\u4ED5\u69D8", "\u30C6\u30B9\u30C8\u30B1\u30FC\u30B9\u306E\u81EA\u52D5\u5C0E\u51FA\u30FB\u4ED5\u69D8\u66F8\u751F\u6210", "\u30C6\u30B9\u30C8\u5B9F\u65BD\u30FB\u5B9F\u6A5F\u691C\u8A3C\u30FB\u5224\u5B9A"],
    ["\u65E2\u5B58\u8CC7\u7523\u6574\u7406", "\u30B9\u30AD\u30E3\u30F3\u30FB\u5206\u985E\u30FB\u30AE\u30E3\u30C3\u30D7\u5206\u6790\u30FB\u518D\u6587\u66F8\u5316", "\u6D41\u7528\u5224\u65AD\u30FB\u6280\u8853\u78BA\u8A8D\u30FB\u6A5F\u5BC6\u7BA1\u7406"],
  ],
  [1600, 3713, 3713]
));

children.push(h2("2.3 \u8A2D\u8A08\u539F\u5247"));
children.push(pBold("(1) \u898F\u683C\u9A86\u683C\u306E\u4FDD\u6301\uFF1A", "ISO 26262 V\u30E2\u30C7\u30EB\u3092\u9AA8\u683C\u3068\u3057\u3001\u4ED6\u898F\u683C\u306F\u5DEE\u5206\u3068\u3057\u3066\u5BFE\u5FDC\u3059\u308B\u3002AI\u5C0E\u5165\u306B\u3088\u308A\u898F\u683C\u8981\u6C42\u3092\u7701\u7565\u3057\u306A\u3044\u3002"));
children.push(pBold("(2) \u6BB5\u968E\u7684\u54C1\u8CEA\u5411\u4E0A\uFF1A", "\u300C\u63A2\u7D22\u30E2\u30FC\u30C9\u300D\u3067\u7D20\u65E9\u304F\u524D\u9032\u3057\u3001\u300C\u8A8D\u8A3C\u30E2\u30FC\u30C9\u300D\u3067\u54C1\u8CEA\u3092\u78BA\u5B9A\u3055\u305B\u308B2\u6BB5\u968E\u30A2\u30D7\u30ED\u30FC\u30C1\u3002"));
children.push(pBold("(3) AI\u51FA\u529B\u306E\u5168\u6570\u691C\u8A3C\uFF1A", "Claude Code\u306FTCL3\u6271\u3044\u3002\u3059\u3079\u3066\u306E\u751F\u6210\u7269\u306F\u4EBA\u9593\u306B\u3088\u308B\u30EC\u30D3\u30E5\u30FC\u304C\u5FC5\u9808\u3002"));
children.push(pBold("(4) \u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3\u512A\u5148\uFF1A", "\u3059\u3079\u3066\u306E\u8981\u4EF6\u30FB\u8A2D\u8A08\u30FB\u30C6\u30B9\u30C8\u306B\u53CC\u65B9\u5411\u30C8\u30EC\u30FC\u30B9\u3092\u5E38\u6642\u7DAD\u6301\u3059\u308B\u3002"));
children.push(pBold("(5) \u65E2\u5B58\u8CC7\u7523\u306E\u6709\u52B9\u6D3B\u7528\uFF1A", "\u65B0\u898F\u958B\u767A\u3060\u3051\u3067\u306A\u304F\u3001\u904E\u53BB\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u306E\u8CC7\u7523\u3092\u4F53\u7CFB\u7684\u306B\u53D6\u308A\u8FBC\u307F\u6D41\u7528\u3059\u308B\u3002"));

// ===========================
// 3. SYSTEM ARCHITECTURE
// ===========================
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("3. \u30B7\u30B9\u30C6\u30E0\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3 [SYS-03]"));
children.push(p("トレースID: SYS-03 | ↓ PR-10, IN-02", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md 'フォルダ構成' (L868-L905) / INGEST.md 'フォルダ構成' (L29-L49)", { italics: true, color: "888888", size: 14 }));

children.push(h2("3.1 \u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u5168\u4F53\u69CB\u6210"));
children.push(p("\u672C\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u306F\u30013\u5C64\u69CB\u9020\u3067\u69CB\u6210\u3055\u308C\u308B\u3002"));

children.push(makeTable(
  ["\u5C64", "\u69CB\u6210\u8981\u7D20", "\u5F79\u5272"],
  [
    ["\u5236\u5FA1\u5C64", "CLAUDE.md", "\u30A8\u30F3\u30C8\u30EA\u30DD\u30A4\u30F3\u30C8\u30FB\u30EB\u30FC\u30C6\u30A3\u30F3\u30B0\u30FB\u72B6\u614B\u7BA1\u7406"],
    ["\u30D7\u30ED\u30BB\u30B9\u5C64", "PROCESS.md / INGEST.md", "\u958B\u767A\u30D7\u30ED\u30BB\u30B9\u5B9A\u7FA9\u30FB\u8CC7\u7523\u6574\u7406\u30D7\u30ED\u30BB\u30B9\u5B9A\u7FA9"],
    ["\u30C7\u30FC\u30BF\u5C64", "project.json / ingest.json / docs/ / src/", "\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u72B6\u614B\u30FB\u6210\u679C\u7269\u30FB\u30BD\u30FC\u30B9\u30B3\u30FC\u30C9"],
  ],
  [1400, 3313, 4313]
));

children.push(h2("3.2 \u30B3\u30F3\u30DD\u30FC\u30CD\u30F3\u30C8\u95A2\u4FC2\u56F3"));
children.push(p("\u4EE5\u4E0B\u306B\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u306E\u5168\u30B3\u30F3\u30DD\u30FC\u30CD\u30F3\u30C8\u3068\u305D\u306E\u95A2\u4FC2\u3092\u793A\u3059\u3002"));

// Component diagram as a table-based layout
const dBorder = { style: BorderStyle.SINGLE, size: 2, color: "1A3A5C" };
const dBorders = { top: dBorder, bottom: dBorder, left: dBorder, right: dBorder };
const dPad = { top: 100, bottom: 100, left: 140, right: 140 };

function diagramCell(title, body, width, fillColor) {
  return new TableCell({
    borders: dBorders, width: { size: width, type: WidthType.DXA },
    shading: { fill: fillColor, type: ShadingType.CLEAR }, margins: dPad,
    children: [
      new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 60 },
        children: [new TextRun({ text: title, font: "Arial", size: 20, bold: true, color: "1A3A5C" })] }),
      new Paragraph({ alignment: AlignmentType.CENTER,
        children: [new TextRun({ text: body, font: "Arial", size: 18, color: "555555" })] })
    ]
  });
}

// Row 1: System Spec (this doc)
children.push(new Table({
  width: { size: TW, type: WidthType.DXA }, columnWidths: [TW],
  rows: [new TableRow({ children: [diagramCell("\u25C6 \u672C\u6587\u66F8\uFF1A\u30B7\u30B9\u30C6\u30E0\u4ED5\u69D8\u66F8 (TSDT-SYS-001)", "\u30B3\u30F3\u30BB\u30D7\u30C8 / \u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3 / \u6587\u66F8\u4F53\u7CFB / \u95A2\u4FC2\u6027\u5B9A\u7FA9", TW, "E8F0FE")] })]
}));
children.push(spacer(80));

// Row 2: CLAUDE.md
children.push(new Table({
  width: { size: TW, type: WidthType.DXA }, columnWidths: [TW],
  rows: [new TableRow({ children: [diagramCell("CLAUDE.md\uFF08\u5236\u5FA1\u5C64\uFF09", "\u8D77\u52D5\u8A2D\u5B9A / \u6307\u793A\u632F\u308A\u5206\u3051 / project.json\u8AAD\u307F\u8FBC\u307F / \u9032\u6357\u30C0\u30C3\u30B7\u30E5\u30DC\u30FC\u30C9\u8868\u793A", TW, "FFF8E1")] })]
}));
children.push(spacer(80));

// Row 3: PROCESS.md and INGEST.md side by side
const halfW = Math.floor(TW / 2);
children.push(new Table({
  width: { size: TW, type: WidthType.DXA }, columnWidths: [halfW, TW - halfW],
  rows: [new TableRow({ children: [
    diagramCell("PROCESS.md\uFF08\u30D7\u30ED\u30BB\u30B9\u5C64\uFF09", "V\u30E2\u30C7\u30EB 15\u30D5\u30A7\u30FC\u30BA\n\u898F\u683C\u5BFE\u5FDC / \u30C8\u30EC\u30FC\u30B9 / \u30EC\u30D3\u30E5\u30FC", halfW, "E1F5E9"),
    diagramCell("INGEST.md\uFF08\u30D7\u30ED\u30BB\u30B9\u5C64\uFF09", "\u65E2\u5B58\u8CC7\u7523\u306E\u53D6\u308A\u8FBC\u307F\n\u30B9\u30AD\u30E3\u30F3 \u2192 \u5206\u985E \u2192 \u518D\u6587\u66F8\u5316", TW - halfW, "F3E5F5"),
  ] })]
}));
children.push(spacer(80));

// Row 4: Data layer
const thirdW = Math.floor(TW / 3);
children.push(new Table({
  width: { size: TW, type: WidthType.DXA }, columnWidths: [thirdW, thirdW, TW - thirdW * 2],
  rows: [new TableRow({ children: [
    diagramCell("project.json", "\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u72B6\u614B\n\u9032\u6357\u30FB\u524D\u63D0\u6761\u4EF6", thirdW, "F5F5F5"),
    diagramCell("docs/", "\u5168\u30D5\u30A7\u30FC\u30BA\u306E\u6210\u679C\u7269\n\u30EC\u30D3\u30E5\u30FC\u8A18\u9332\u30FB\u30C8\u30EC\u30FC\u30B9", thirdW, "F5F5F5"),
    diagramCell("src/", "\u30BD\u30FC\u30B9\u30B3\u30FC\u30C9\n\u5B89\u5168\u30BF\u30B0\u4ED8\u304D", TW - thirdW * 2, "F5F5F5"),
  ] })]
}));

// ===========================
// 4. CLAUDE.md SPECIFICATION
// ===========================
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("4. CLAUDE.md \u306E\u4ED5\u69D8 [SYS-04]"));
children.push(p("トレースID: SYS-04 | ↓ PR-01, IN-03", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: CLAUDE.md '必ず最初に実行すること' (L6-L10) / CLAUDE.md '指示別の動作' (L14-L23)", { italics: true, color: "888888", size: 14 }));

children.push(h2("4.1 \u5F79\u5272\u3068\u8CAC\u52D9"));
children.push(p("CLAUDE.md \u306F\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u306E\u300C\u5236\u5FA1\u5C64\u300D\u3068\u3057\u3066\u6A5F\u80FD\u3059\u308B\u3002Claude Code \u304C\u8D77\u52D5\u3055\u308C\u305F\u969B\u306B\u6700\u521D\u306B\u8AAD\u307F\u8FBC\u307E\u308C\u308B\u30D5\u30A1\u30A4\u30EB\u3067\u3042\u308A\u3001\u30E6\u30FC\u30B6\u30FC\u306E\u6307\u793A\u3092\u9069\u5207\u306A\u30D7\u30ED\u30BB\u30B9\u6587\u66F8\u306B\u632F\u308A\u5206\u3051\u308B\u30EB\u30FC\u30BF\u30FC\u306E\u5F79\u5272\u3092\u62C5\u3046\u3002"));

children.push(h3("CLAUDE.md \u81EA\u4F53\u306B\u306F\u30D7\u30ED\u30BB\u30B9\u30ED\u30B8\u30C3\u30AF\u3092\u542B\u307E\u306A\u3044\u3002"));
children.push(p("\u3059\u3079\u3066\u306E\u5177\u4F53\u7684\u306A\u958B\u767A\u30D7\u30ED\u30BB\u30B9\u306F PROCESS.md \u307E\u305F\u306F INGEST.md \u306B\u59D4\u8B72\u3055\u308C\u308B\u3002"));

children.push(h2("4.2 \u6307\u793A\u30EB\u30FC\u30C6\u30A3\u30F3\u30B0\u30C6\u30FC\u30D6\u30EB"));
children.push(makeTable(
  ["\u30E6\u30FC\u30B6\u30FC\u306E\u6307\u793A", "\u30EB\u30FC\u30C6\u30A3\u30F3\u30B0\u5148", "\u8AAC\u660E"],
  [
    ["\u300C\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u3092\u958B\u59CB\u3057\u3066\u304F\u3060\u3055\u3044\u300D", "PROCESS.md STEP 1\u301C5", "\u65B0\u898F\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u306E\u521D\u671F\u8A2D\u5B9A"],
    ["\u300C\u8CC7\u7523\u6574\u7406\u3092\u958B\u59CB\u3057\u3066\u304F\u3060\u3055\u3044\u300D", "INGEST.md", "\u65E2\u5B58\u8CC7\u7523\u306E\u53D6\u308A\u8FBC\u307F\u30FB\u518D\u6574\u7406"],
    ["\u300C\u9032\u6357\u3092\u6559\u3048\u3066\u304F\u3060\u3055\u3044\u300D", "project.json \u8AAD\u307F\u8FBC\u307F", "\u9032\u6357\u30C0\u30C3\u30B7\u30E5\u30DC\u30FC\u30C9\u8868\u793A"],
    ["\u300CPH-XX \u3092\u5B9F\u884C\u3057\u3066\u304F\u3060\u3055\u3044\u300D", "PROCESS.md \u8A72\u5F53\u30D5\u30A7\u30FC\u30BA", "\u7279\u5B9A\u30D5\u30A7\u30FC\u30BA\u306E\u5B9F\u884C"],
    ["\u300C\u30B3\u30FC\u30C7\u30A3\u30F3\u30B0GL\u3092\u4F5C\u6210\u3057\u3066\u300D", "PROCESS.md \u898F\u683C\u8A2D\u5B9A\u53C2\u7167", "\u30B3\u30FC\u30C7\u30A3\u30F3\u30B0\u30AC\u30A4\u30C9\u30E9\u30A4\u30F3\u751F\u6210"],
    ["\u300C\u5916\u90E8\u30E9\u30A4\u30D6\u30E9\u30EA\u7BA1\u7406\u3092\u4F5C\u6210\u3057\u3066\u300D", "PROCESS.md \u8A2D\u5B9A\u53C2\u7167", "\u5916\u90E8\u30E9\u30A4\u30D6\u30E9\u30EA\u7BA1\u7406\u30D5\u30A1\u30A4\u30EB\u751F\u6210"],
  ],
  [3200, 2413, 3413]
));

children.push(h2("4.3 \u8D77\u52D5\u30B7\u30FC\u30B1\u30F3\u30B9"));
children.push(p("Claude Code \u304C\u8D77\u52D5\u3055\u308C\u308B\u3068\u3001\u4EE5\u4E0B\u306E\u9806\u5E8F\u3067\u51E6\u7406\u304C\u5B9F\u884C\u3055\u308C\u308B\u3002"));
children.push(p("1. CLAUDE.md \u3092\u8AAD\u307F\u8FBC\u3080\uFF08\u5FC5\u9808\uFF09"));
children.push(p("2. PROCESS.md \u3092\u8AAD\u307F\u8FBC\u3080\uFF08\u5FC5\u9808\uFF09"));
children.push(p("3. project.json \u304C\u5B58\u5728\u3059\u308B\u5834\u5408\u306F\u8AAD\u307F\u8FBC\u307F\u3001\u9032\u6357\u30FB\u524D\u63D0\u6761\u4EF6\u3092\u78BA\u8A8D"));
children.push(p("4. \u9032\u6357\u30C0\u30C3\u30B7\u30E5\u30DC\u30FC\u30C9\u3092\u8868\u793A\u3057\u3066\u30E6\u30FC\u30B6\u30FC\u306B\u73FE\u72B6\u3092\u5831\u544A"));
children.push(p("5. \u30E6\u30FC\u30B6\u30FC\u306E\u6307\u793A\u3092\u5F85\u6A5F"));

// ===========================
// 5. PROCESS/INGEST RELATIONSHIP
// ===========================
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("5. PROCESS.md \u3068 INGEST.md \u306E\u95A2\u4FC2\u6027 [SYS-05]"));
children.push(p("トレースID: SYS-05 | ↓ PR-02, PR-03, IN-07, IN-11", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: CLAUDE.md '指示別の動作' (L14-L23)", { italics: true, color: "888888", size: 14 }));

children.push(h2("5.1 \u4E21\u6587\u66F8\u306E\u4F4D\u7F6E\u3065\u3051"));
children.push(p("PROCESS.md \u3068 INGEST.md \u306F\u300C\u30D7\u30ED\u30BB\u30B9\u5C64\u300D\u306E2\u3064\u306E\u67F1\u3067\u3042\u308A\u3001\u305D\u308C\u305E\u308C\u7570\u306A\u308B\u5165\u53E3\u304B\u3089\u540C\u3058\u6210\u679C\u7269\uFF08docs/ / src/\uFF09\u3078\u5408\u6D41\u3059\u308B\u3002"));

children.push(makeTable(
  ["\u9805\u76EE", "PROCESS.md", "INGEST.md"],
  [
    ["\u76EE\u7684", "\u65B0\u898F\u958B\u767A\u30D7\u30ED\u30BB\u30B9\u306E\u5B9A\u7FA9\u30FB\u5B9F\u884C", "\u65E2\u5B58\u8CC7\u7523\u306E\u53D6\u308A\u8FBC\u307F\u30FB\u518D\u6574\u7406"],
    ["\u5165\u529B", "\u30E6\u30FC\u30B6\u30FC\u306E\u8981\u4EF6\u30FB\u6280\u8853\u60C5\u5831", "legacy/ \u30D5\u30A9\u30EB\u30C0\u306E\u65E2\u5B58\u30D5\u30A1\u30A4\u30EB"],
    ["\u51FA\u529B", "docs/ / src/ \u306E\u6210\u679C\u7269", "docs/ / src/ \u306E\u6210\u679C\u7269\uFF08\u518D\u6587\u66F8\u5316\u6E08\u307F\uFF09"],
    ["\u72B6\u614B\u7BA1\u7406", "project.json", "ingest.json"],
    ["\u30D5\u30A7\u30FC\u30BA\u69CB\u6210", "PH-01\u301CPH-15\uFF08V\u30E2\u30C7\u30EB\uFF09", "STEP 1\u301C6\uFF08\u7DDA\u5F62\u30D1\u30A4\u30D7\u30E9\u30A4\u30F3\uFF09"],
    ["\u5B9F\u884C\u30BF\u30A4\u30DF\u30F3\u30B0", "\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u958B\u59CB\u6642\u304B\u3089\u7D42\u4E86\u307E\u3067", "\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u521D\u671F\uFF08PROCESS\u524D or \u4E26\u884C\uFF09"],
    ["\u6587\u66F8\u756A\u53F7", "TSDT-PR-001", "TSDT-IN-001"],
  ],
  [2000, 3513, 3513]
));

children.push(h2("5.2 \u5B9F\u884C\u30D5\u30ED\u30FC\u306E\u95A2\u4FC2"));
children.push(p("INGEST \u3068 PROCESS \u306F\u4EE5\u4E0B\u306E3\u3064\u306E\u30D1\u30BF\u30FC\u30F3\u3067\u9023\u643A\u3059\u308B\u3002"));

children.push(h3("\u30D1\u30BF\u30FC\u30F3A\uFF1AINGEST\u5148\u884C\u578B\uFF08\u63A8\u5968\uFF09"));
children.push(p("\u65E2\u5B58\u8CC7\u7523\u304C\u3042\u308B\u5834\u5408\u306E\u6A19\u6E96\u30D1\u30BF\u30FC\u30F3\u3002\u307E\u305AINGEST\u3067\u8CC7\u7523\u3092\u6574\u7406\u3057\u3001\u30AE\u30E3\u30C3\u30D7\u3092\u7279\u5B9A\u3057\u305F\u5F8C\u3001PROCESS\u3067\u4E0D\u8DB3\u5206\u3092\u65B0\u898F\u4F5C\u6210\u3059\u308B\u3002"));
children.push(p("INGEST STEP 1\u301C6 \u2192 \u30AE\u30E3\u30C3\u30D7\u5206\u6790 \u2192 PROCESS PH-01\u301C \u3067\u4E0D\u8DB3\u30D5\u30A7\u30FC\u30BA\u3092\u88DC\u5B8C", { italics: true, color: "555555" }));

children.push(h3("\u30D1\u30BF\u30FC\u30F3B\uFF1APROCESS\u5358\u72EC\u578B"));
children.push(p("\u65E2\u5B58\u8CC7\u7523\u304C\u306A\u3044\u5B8C\u5168\u65B0\u898F\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u306E\u5834\u5408\u3002INGEST\u306F\u4F7F\u7528\u305B\u305A\u3001PROCESS PH-01\u304B\u3089\u9806\u6B21\u5B9F\u884C\u3059\u308B\u3002"));

children.push(h3("\u30D1\u30BF\u30FC\u30F3C\uFF1A\u4E26\u884C\u578B"));
children.push(p("INGEST\u3068PROCESS\u3092\u4E26\u884C\u3067\u9032\u3081\u308B\u30D1\u30BF\u30FC\u30F3\u3002INGEST\u3067\u9AD8\u4FA1\u5024\u8CC7\u7523\u304B\u3089\u9806\u6B21\u518D\u6587\u66F8\u5316\u3057\u306A\u304C\u3089\u3001PROCESS\u3067\u4E0D\u8DB3\u30D5\u30A7\u30FC\u30BA\u3082\u540C\u6642\u306B\u9032\u3081\u308B\u3002\u63A2\u7D22\u30E2\u30FC\u30C9\u3067\u306F\u3053\u306E\u30D1\u30BF\u30FC\u30F3\u3082\u8A31\u5BB9\u3055\u308C\u308B\u3002"));

children.push(h2("5.3 \u30C7\u30FC\u30BF\u306E\u5408\u6D41\u30DD\u30A4\u30F3\u30C8"));
children.push(p("INGEST \u3068 PROCESS \u306E\u6210\u679C\u7269\u306F\u4EE5\u4E0B\u306E\u5171\u901A\u30C7\u30FC\u30BF\u30B9\u30C8\u30A2\u306B\u5408\u6D41\u3059\u308B\u3002"));
children.push(makeTable(
  ["\u5171\u901A\u30C7\u30FC\u30BF", "\u5F79\u5272", "INGEST\u304B\u3089\u306E\u5165\u529B", "PROCESS\u304B\u3089\u306E\u5165\u529B"],
  [
    ["docs/", "\u6210\u679C\u7269\u683C\u7D0D", "\u518D\u6587\u66F8\u5316\u6E08\u307F\u8CC7\u7523", "\u65B0\u898F\u4F5C\u6210\u6210\u679C\u7269"],
    ["src/", "\u30B3\u30FC\u30C9\u683C\u7D0D", "\u30BF\u30B0\u4ED8\u4E0E\u6E08\u307F\u65E2\u5B58\u30B3\u30FC\u30C9", "\u65B0\u898F\u751F\u6210\u30B3\u30FC\u30C9"],
    ["project.json", "\u72B6\u614B\u7BA1\u7406", "legacySources\u30FBaiSupplementCount", "\u30D5\u30A7\u30FC\u30BA\u72B6\u614B\u30FB\u30C8\u30EC\u30FC\u30B9\u60C5\u5831"],
    ["docs/\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3.md", "\u53CC\u65B9\u5411\u30C8\u30EC\u30FC\u30B9", "\u65E2\u5B58\u8981\u4EF6ID\u306E\u7D44\u307F\u8FBC\u307F", "\u65B0\u898F\u8981\u4EF6ID\u306E\u767B\u9332"],
  ],
  [2200, 1600, 2613, 2613]
));

children.push(h2("5.4 \u30E2\u30FC\u30C9\u3068\u306E\u95A2\u4FC2"));
children.push(makeTable(
  ["\u30E2\u30FC\u30C9", "PROCESS.md", "INGEST.md"],
  [
    ["\u63A2\u7D22\u30E2\u30FC\u30C9", "\u30D5\u30A7\u30FC\u30BA\u9806\u5E8F\u81EA\u7531 / TBD\u8A31\u5BB9 / \u7C21\u6613\u30EC\u30D3\u30E5\u30FC", "\u5168STEP\u5B9F\u884C\u53EF / \u3010AI\u88DC\u5B8C\u3011\u30BF\u30B0\u8A31\u5BB9"],
    ["\u8A8D\u8A3C\u30E2\u30FC\u30C9", "\u53B3\u5BC6\u306A\u30D5\u30A7\u30FC\u30BA\u9806 / TBD\u30BC\u30ED / \u6B63\u5F0F\u30EC\u30D3\u30E5\u30FC", "\u5168\u3010AI\u88DC\u5B8C\u3011\u89E3\u6D88\u5FC5\u9808 / \u79FB\u884C\u524D\u306B\u5B8C\u4E86"],
  ],
  [2000, 3513, 3513]
));

// ===========================
// 6. DOCUMENT HIERARCHY
// ===========================
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("6. \u6587\u66F8\u4F53\u7CFB [SYS-06]"));
children.push(p("トレースID: SYS-06 | ↓ PR-01, IN-01", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: CLAUDE.md 'プロジェクト管理ファイル' (L27-L34)", { italics: true, color: "888888", size: 14 }));

children.push(h2("6.1 \u6587\u66F8\u968E\u5C64\u69CB\u9020"));
children.push(makeTable(
  ["\u968E\u5C64", "\u6587\u66F8", "\u6587\u66F8\u756A\u53F7", "\u6982\u8981"],
  [
    ["L0\uFF08\u6700\u4E0A\u4F4D\uFF09", "\u672C\u6587\u66F8\uFF1A\u30B7\u30B9\u30C6\u30E0\u4ED5\u69D8\u66F8", "TSDT-SYS-001", "\u30B3\u30F3\u30BB\u30D7\u30C8\u30FB\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3\u30FB\u95A2\u4FC2\u6027"],
    ["L1", "CLAUDE.md", "\u2014", "\u8D77\u52D5\u8A2D\u5B9A\u30FB\u6307\u793A\u30EB\u30FC\u30C6\u30A3\u30F3\u30B0"],
    ["L2", "PROCESS.md \u4ED5\u69D8\u66F8", "TSDT-PR-001", "V\u30E2\u30C7\u30EB\u958B\u767A\u30D7\u30ED\u30BB\u30B9\u5B9A\u7FA9"],
    ["L2", "INGEST.md \u4ED5\u69D8\u66F8", "TSDT-IN-001", "\u65E2\u5B58\u8CC7\u7523\u53D6\u308A\u8FBC\u307F\u30D7\u30ED\u30BB\u30B9\u5B9A\u7FA9"],
    ["L3", "project.json", "\u2014", "\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u72B6\u614B\u30C7\u30FC\u30BF"],
    ["L3", "ingest.json", "\u2014", "INGEST\u72B6\u614B\u30C7\u30FC\u30BF"],
    ["L4", "docs/01\u301C15 \u5404\u30D5\u30A7\u30FC\u30BA\u6210\u679C\u7269", "\u2014", "V\u30E2\u30C7\u30EB\u5404\u30D5\u30A7\u30FC\u30BA\u306E\u6210\u679C\u7269"],
    ["L4", "docs/\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3.md", "\u2014", "\u53CC\u65B9\u5411\u30C8\u30EC\u30FC\u30B9\u30DE\u30C8\u30EA\u30AF\u30B9"],
    ["L4", "docs/\u30EC\u30D3\u30E5\u30FC\u8A18\u9332/", "\u2014", "\u30D5\u30A7\u30FC\u30BA\u5225\u30EC\u30D3\u30E5\u30FC\u8A18\u9332"],
    ["L4", "docs/\u30C4\u30FC\u30EB\u9069\u683C\u6027\u8A18\u9332.md", "\u2014", "\u30C4\u30FC\u30EB\u306ETCL\u5206\u985E\u30FB\u691C\u8A3C\u8A18\u9332"],
    ["L4", "src/", "\u2014", "\u5B89\u5168\u30BF\u30B0\u4ED8\u304D\u30BD\u30FC\u30B9\u30B3\u30FC\u30C9"],
  ],
  [1200, 3000, 1813, 3013]
));

children.push(h2("6.2 \u6587\u66F8\u9593\u306E\u53C2\u7167\u95A2\u4FC2"));
children.push(makeTable(
  ["\u53C2\u7167\u5143", "\u53C2\u7167\u5148", "\u53C2\u7167\u5185\u5BB9"],
  [
    ["\u672C\u6587\u66F8 (L0)", "CLAUDE.md / PROCESS.md / INGEST.md", "\u5168\u6587\u66F8\u306E\u4F4D\u7F6E\u3065\u3051\u3068\u95A2\u4FC2\u6027"],
    ["CLAUDE.md (L1)", "PROCESS.md / INGEST.md", "\u6307\u793A\u306E\u30EB\u30FC\u30C6\u30A3\u30F3\u30B0\u5148"],
    ["CLAUDE.md (L1)", "project.json", "\u8D77\u52D5\u6642\u306E\u72B6\u614B\u8AAD\u307F\u8FBC\u307F"],
    ["PROCESS.md (L2)", "docs/ \u5404\u30D5\u30A7\u30FC\u30BA\u6210\u679C\u7269", "\u6210\u679C\u7269\u306E\u751F\u6210\u30FB\u66F4\u65B0"],
    ["PROCESS.md (L2)", "docs/\u30B3\u30FC\u30C7\u30A3\u30F3\u30B0GL.md", "PH-09\u524D\u306E\u524D\u63D0\u6761\u4EF6"],
    ["PROCESS.md (L2)", "docs/\u5916\u90E8\u30E9\u30A4\u30D6\u30E9\u30EA\u7BA1\u7406.md", "PH-09\u524D\u306E\u524D\u63D0\u6761\u4EF6"],
    ["INGEST.md (L2)", "ingest_work/ \u4E2D\u9593\u30D5\u30A1\u30A4\u30EB", "\u6574\u7406\u4F5C\u696D\u306E\u4E2D\u9593\u7D50\u679C"],
    ["INGEST.md (L2)", "docs/ / src/", "\u518D\u6587\u66F8\u5316\u6E08\u307F\u6210\u679C\u7269\u306E\u51FA\u529B\u5148"],
    ["INGEST.md (L2)", "project.json", "legacySources / aiSupplementCount \u306E\u8FFD\u8A18"],
  ],
  [2200, 3413, 3413]
));

// ===========================
// 7. QUALITY & SAFETY
// ===========================
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("7. \u54C1\u8CEA\u30FB\u5B89\u5168\u4FDD\u8A3C\u306E\u8003\u3048\u65B9 [SYS-07]"));
children.push(p("トレースID: SYS-07 | ↓ PR-04, PR-06, PR-07, PR-08, IN-08 | ▲ PR-11, IN-12", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md 'ツール適格性確認' (L413-L434) / 'レビュープロセス' (L353-L410) / '要件トレーサビリティ管理' (L254-L309)", { italics: true, color: "888888", size: 14 }));

children.push(h2("7.1 AI\u751F\u6210\u7269\u306E\u54C1\u8CEA\u4FDD\u8A3C\u30E2\u30C7\u30EB"));
children.push(p("Claude Code\u306ETCL3\u5206\u985E\u306B\u57FA\u3065\u304D\u3001\u300C\u65B9\u6CD5A\uFF1A\u30C4\u30FC\u30EB\u51FA\u529B\u306E\u5B8C\u5168\u691C\u8A3C\u300D\u3092\u63A1\u7528\u3059\u308B\u3002\u3053\u308C\u306F\u3001AI\u304C\u751F\u6210\u3057\u305F\u3059\u3079\u3066\u306E\u6210\u679C\u7269\u3092\u4EBA\u9593\u304C\u30EC\u30D3\u30E5\u30FC\u30FB\u627F\u8A8D\u3059\u308B\u3053\u3068\u3092\u610F\u5473\u3059\u308B\u3002"));

children.push(h3("\u54C1\u8CEA\u4FDD\u8A3C\u306E\u6D41\u308C"));
children.push(p("1. AI\u304C\u6210\u679C\u7269\u3092\u751F\u6210\uFF08\u30B9\u30C6\u30FC\u30BF\u30B9\uFF1A\u300C\u66AB\u5B9A\u300D\uFF09"));
children.push(p("2. \u30A8\u30F3\u30B8\u30CB\u30A2\u304C\u30BB\u30EB\u30D5\u30EC\u30D3\u30E5\u30FC\u307E\u305F\u306F\u72EC\u7ACB\u30EC\u30D3\u30E5\u30FC\u3092\u5B9F\u65BD"));
children.push(p("3. \u30EC\u30D3\u30E5\u30FC\u8A18\u9332\u3092 docs/\u30EC\u30D3\u30E5\u30FC\u8A18\u9332/ \u306B\u4FDD\u5B58"));
children.push(p("4. \u627F\u8A8D\u5F8C\u306B\u30B9\u30C6\u30FC\u30BF\u30B9\u3092\u300C\u627F\u8A8D\u6E08\u307F\u300D\u306B\u5909\u66F4"));
children.push(p("5. \u8A8D\u8A3C\u30E2\u30FC\u30C9\u3067\u306F\u300C\u627F\u8A8D\u6E08\u307F\u300D\u306E\u307F\u6B21\u30D5\u30A7\u30FC\u30BA\u306B\u79FB\u884C\u53EF\u80FD"));

children.push(h2("7.2 \u3010AI\u88DC\u5B8C\u30FB\u8981\u78BA\u8A8D\u3011\u30BF\u30B0\u306E\u904B\u7528"));
children.push(p("AI\u304C\u63A8\u6E2C\u307E\u305F\u306F\u88DC\u5B8C\u3057\u305F\u5185\u5BB9\u306B\u306F\u5FC5\u305A\u3010AI\u88DC\u5B8C\u30FB\u8981\u78BA\u8A8D\u3011\u30BF\u30B0\u3092\u4ED8\u4E0E\u3059\u308B\u3002\u7279\u306B\u5B89\u5168\u8981\u6C42\u30FBSIL\u30EC\u30D9\u30EB\u30FB\u8A3A\u65AD\u30AB\u30D0\u30EC\u30C3\u30B8\u306E\u88DC\u5B8C\u5024\u306F\u53B3\u683C\u306B\u7BA1\u7406\u3059\u308B\u3002\u8A8D\u8A3C\u30E2\u30FC\u30C9\u79FB\u884C\u524D\u306B\u3059\u3079\u3066\u306E\u30BF\u30B0\u3092\u30BC\u30ED\u306B\u3059\u308B\u3053\u3068\u304C\u5FC5\u9808\u3067\u3042\u308B\u3002"));

children.push(h2("7.3 \u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3\u306B\u3088\u308B\u5B89\u5168\u4FDD\u8A3C"));
children.push(p("\u5B89\u5168\u76EE\u6A19\uFF08SG\uFF09\u304B\u3089\u30C6\u30B9\u30C8\u30B1\u30FC\u30B9\uFF08TC\uFF09\u307E\u3067\u306E\u53CC\u65B9\u5411\u30C8\u30EC\u30FC\u30B9\u3092\u5E38\u6642\u7DAD\u6301\u3059\u308B\u3053\u3068\u3067\u3001\u3059\u3079\u3066\u306E\u5B89\u5168\u8981\u6C42\u304C\u8A2D\u8A08\u30FB\u5B9F\u88C5\u30FB\u691C\u8A3C\u3055\u308C\u3066\u3044\u308B\u3053\u3068\u3092\u4FDD\u8A3C\u3059\u308B\u3002"));
children.push(p("SG \u2192 FSR \u2192 TSR \u2192 SR \u2192 SA \u2192 UT \u2192 TC\uFF08\u4E0B\u6D41\uFF09", { italics: true, color: "2E6B9E" }));
children.push(p("TC \u2192 SR \u2192 TSR \u2192 FSR \u2192 SG\uFF08\u4E0A\u6D41\uFF09", { italics: true, color: "2E6B9E" }));

// ===========================
// 8. OPERATIONAL GUIDELINES
// ===========================
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("8. \u904B\u7528\u30AC\u30A4\u30C9\u30E9\u30A4\u30F3 [SYS-08]"));
children.push(p("トレースID: SYS-08 | ↓ PR-02, IN-11", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md '運用モード' (L38-L63) / '起動時の動作' (L166-L251) / INGEST.md 'PROCESS.mdとの連携' (L514-L540)", { italics: true, color: "888888", size: 14 }));

children.push(h2("8.1 \u65B0\u898F\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u958B\u59CB\u6642"));
children.push(p("1. CLAUDE.md / PROCESS.md / INGEST.md \u3092\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u30D5\u30A9\u30EB\u30C0\u306B\u914D\u7F6E"));
children.push(p("2. Claude Code \u3092\u8D77\u52D5\u3057\u3001\u300C\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u3092\u958B\u59CB\u3057\u3066\u304F\u3060\u3055\u3044\u300D\u3068\u6307\u793A"));
children.push(p("3. \u521D\u671F\u8A2D\u5B9A\uFF08\u88FD\u54C1\u540D\u30FB\u30AB\u30C6\u30B4\u30EA\u30FB\u898F\u683C\u30FBASIL\u7B49\uFF09\u3092\u5165\u529B"));
children.push(p("4. project.json \u304C\u81EA\u52D5\u751F\u6210\u3055\u308C\u3001\u30D5\u30A7\u30FC\u30BA\u5B9F\u884C\u304C\u53EF\u80FD\u306B\u306A\u308B"));

children.push(h2("8.2 \u65E2\u5B58\u8CC7\u7523\u304C\u3042\u308B\u5834\u5408"));
children.push(p("1. legacy/ \u30D5\u30A9\u30EB\u30C0\u306B\u65E2\u5B58\u8CC7\u7523\u3092\u914D\u7F6E"));
children.push(p("2. \u300C\u8CC7\u7523\u6574\u7406\u3092\u958B\u59CB\u3057\u3066\u304F\u3060\u3055\u3044\u300D\u3068\u6307\u793A"));
children.push(p("3. INGEST STEP 1\u301C6 \u3092\u5B9F\u884C\u3057\u3001\u30AE\u30E3\u30C3\u30D7\u5206\u6790\u3067\u4E0D\u8DB3\u3092\u7279\u5B9A"));
children.push(p("4. \u300C\u6574\u7406\u304C\u7D42\u308F\u3063\u305F\u306E\u3067PROCESS\u306EPH-01\u3092\u958B\u59CB\u3057\u3066\u304F\u3060\u3055\u3044\u300D\u3068\u6307\u793A"));

children.push(h2("8.3 \u8A8D\u8A3C\u30E2\u30FC\u30C9\u79FB\u884C\u6642"));
children.push(p("\u4EE5\u4E0B\u306E\u5168\u6761\u4EF6\u3092\u6E80\u305F\u3057\u305F\u5834\u5408\u306B\u306E\u307F\u3001\u63A2\u7D22\u30E2\u30FC\u30C9\u304B\u3089\u8A8D\u8A3C\u30E2\u30FC\u30C9\u3078\u79FB\u884C\u53EF\u80FD\u3068\u3059\u308B\u3002"));
children.push(makeTable(
  ["No", "\u79FB\u884C\u6761\u4EF6", "\u78BA\u8A8D\u65B9\u6CD5"],
  [
    ["1", "\u5168\u30D5\u30A7\u30FC\u30BA\u306E TBD \u4E8B\u9805\u304C\u30BC\u30ED", "project.json \u306E tbdCount \u5168\u30D5\u30A7\u30FC\u30BA\u5408\u8A08 = 0"],
    ["2", "\u672A\u7D10\u4ED8\u3051\u8981\u4EF6\u304C\u30BC\u30ED", "project.json \u306E unlinkedReqs \u5168\u30D5\u30A7\u30FC\u30BA\u5408\u8A08 = 0"],
    ["3", "\u5168\u3010AI\u88DC\u5B8C\u30FB\u8981\u78BA\u8A8D\u3011\u30BF\u30B0\u304C\u89E3\u6D88\u6E08\u307F", "docs/ / src/ \u5185\u306E\u30BF\u30B0\u691C\u7D22\u7D50\u679C = 0\u4EF6"],
    ["4", "\u5168\u30D5\u30A7\u30FC\u30BA\u306E\u30EC\u30D3\u30E5\u30FC\u8A18\u9332\u304C\u5B58\u5728", "docs/\u30EC\u30D3\u30E5\u30FC\u8A18\u9332/ \u306B\u5168PH\u306E\u8A18\u9332\u304C\u5B58\u5728"],
    ["5", "\u30C4\u30FC\u30EB\u9069\u683C\u6027\u8A18\u9332\u304C\u5B8C\u4E86", "docs/\u30C4\u30FC\u30EB\u9069\u683C\u6027\u8A18\u9332.md \u304C\u5B58\u5728\u30FB\u78BA\u8A8D\u6E08\u307F"],
    ["6", "FMEA/FTA \u304CASIL\u306B\u5FDC\u3058\u3066\u5B8C\u4E86", "ASIL C/D: FTA\u5FC5\u9808 / ASIL B: FMEA\u63A8\u5968"],
  ],
  [600, 4213, 4213]
));

// ===========================
// 9. SCOPE & LIMITATIONS
// ===========================
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("9. \u9069\u7528\u7BC4\u56F2\u3068\u5236\u7D04 [SYS-09]"));
children.push(p("トレースID: SYS-09 | ↓ PR-01, IN-01", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md '注意事項・免責' (L968-L974) / INGEST.md '注意事項' (L544-L550)", { italics: true, color: "888888", size: 14 }));

children.push(h2("9.1 \u9069\u7528\u7BC4\u56F2"));
children.push(makeTable(
  ["\u9805\u76EE", "\u7BC4\u56F2"],
  [
    ["\u5BFE\u8C61\u88FD\u54C1", "\u6A5F\u80FD\u5B89\u5168\u304C\u6C42\u3081\u3089\u308C\u308B\u7D44\u307F\u8FBC\u307F\u88FD\u54C1\uFF08\u8ECA\u8F09\u30FB\u7523\u696D\u30FB\u5BB6\u96FB\u30FB\u533B\u7642\u30FBIoT\uFF09"],
    ["\u5BFE\u5FDC\u898F\u683C", "ISO 26262 / IEC 61508 / IEC 60730 / IEC 62061 / JIS B 9705 / EU\u6A5F\u68B0\u898F\u5247"],
    ["AI\u62C5\u5F53\u7BC4\u56F2", "PH-01\u301CPH-15\u306E\u8A2D\u8A08\u30FB\u4ED5\u69D8\u66F8\u751F\u6210\u30FB\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3\u7BA1\u7406"],
    ["\u4EBA\u9593\u5FC5\u9808\u7BC4\u56F2", "\u30C6\u30B9\u30C8\u5B9F\u65BD\u30FB\u5B9F\u6A5F\u691C\u8A3C\u30FB\u59A5\u5F53\u6027\u78BA\u8A8D\u30FB\u30EC\u30D3\u30E5\u30FC\u627F\u8A8D"],
  ],
  [2400, 6626]
));

children.push(h2("9.2 \u5236\u7D04\u4E8B\u9805"));
children.push(p("\u672C\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u306FAI\u306B\u3088\u308B\u652F\u63F4\u30C4\u30FC\u30EB\u3067\u3042\u308A\u3001\u6210\u679C\u7269\u306E\u6700\u7D42\u78BA\u8A8D\u30FB\u627F\u8A8D\u306F\u5FC5\u305A\u30A8\u30F3\u30B8\u30CB\u30A2\u304C\u5B9F\u65BD\u3059\u308B\u3053\u3068\u3002"));
children.push(p("Claude Code\u306E\u751F\u6210\u7269\u306FTCL3\u6271\u3044\u3067\u3042\u308A\u3001\u3059\u3079\u3066\u306E\u6210\u679C\u7269\u306F\u4EBA\u9593\u306B\u3088\u308B\u30EC\u30D3\u30E5\u30FC\u304C\u5FC5\u9808\u3067\u3042\u308B\u3002"));
children.push(p("\u6A5F\u80FD\u5B89\u5168\u898F\u683C\u3078\u306E\u9069\u5408\u6027\u306E\u6700\u7D42\u5224\u65AD\u306F\u8A8D\u5B9A\u6A5F\u95A2\u30FB\u5BE9\u67FB\u54E1\u304C\u884C\u3046\u3002"));
children.push(p("API\u30AD\u30FC\u30FB\u6A5F\u5BC6\u60C5\u5831\u3092\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF\u306E\u30D5\u30A1\u30A4\u30EB\u306B\u8A18\u8F09\u3057\u306A\u3044\u3053\u3068\u3002"));

// ===========================
// APPENDIX
// ===========================
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("\u4ED8\u9332A\uFF1A\u30D5\u30A1\u30A4\u30EB\u4E00\u89A7\u3068\u30D0\u30FC\u30B8\u30E7\u30F3\u7BA1\u7406"));
children.push(makeTable(
  ["\u30D5\u30A1\u30A4\u30EB", "\u7248\u6570", "\u30B9\u30C6\u30FC\u30BF\u30B9", "\u6700\u7D42\u66F4\u65B0"],
  [
    ["\u30B7\u30B9\u30C6\u30E0\u4ED5\u69D8\u66F8\uFF08\u672C\u6587\u66F8\uFF09", "1.0", "\u8349\u6848", "2026-02-27"],
    ["CLAUDE.md", "1.0", "\u8349\u6848", "2026-02-27"],
    ["PROCESS.md", "1.0", "\u8349\u6848", "2026-02-27"],
    ["PROCESS_\u4ED5\u69D8\u66F8.docx", "1.0", "\u8349\u6848", "2026-02-27"],
    ["INGEST.md", "1.0", "\u8349\u6848", "2026-02-27"],
    ["INGEST_\u4ED5\u69D8\u66F8.docx", "1.0", "\u8349\u6848", "2026-02-27"],
  ],
  [3200, 1200, 1413, 3213]
));

// ===========================
// BUILD DOCUMENT
// ===========================
const doc = new Document({
  styles: {
    default: { document: { run: { font: "Arial", size: 21 } } },
    paragraphStyles: [
      { id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 36, bold: true, font: "Arial", color: "1A3A5C" },
        paragraph: { spacing: { before: 400, after: 220 }, outlineLevel: 0 } },
      { id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 28, bold: true, font: "Arial", color: "2E6B9E" },
        paragraph: { spacing: { before: 300, after: 180 }, outlineLevel: 1 } },
      { id: "Heading3", name: "Heading 3", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 24, bold: true, font: "Arial", color: "404040" },
        paragraph: { spacing: { before: 220, after: 120 }, outlineLevel: 2 } },
    ]
  },
  sections: [{
    properties: {
      page: {
        size: { width: 11906, height: 16838 }, // A4
        margin: { top: 1440, right: 1440, bottom: 1440, left: 1440 }
      }
    },
    headers: {
      default: new Header({ children: [
        new Paragraph({
          alignment: AlignmentType.RIGHT,
          border: { bottom: { style: BorderStyle.SINGLE, size: 4, color: "1A3A5C", space: 1 } },
          children: [new TextRun({ text: "TSDT-SYS-001 v1.0  |  AI\u652F\u63F4\u6A5F\u80FD\u5B89\u5168\u958B\u767A\u30D5\u30EC\u30FC\u30E0\u30EF\u30FC\u30AF \u30B7\u30B9\u30C6\u30E0\u4ED5\u69D8\u66F8", font: "Arial", size: 16, color: "888888" })]
        })
      ] })
    },
    footers: {
      default: new Footer({ children: [
        new Paragraph({
          alignment: AlignmentType.CENTER,
          children: [
            new TextRun({ text: "TORASAN \u6A5F\u80FD\u5B89\u5168\u958B\u767A\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8  |  \u793E\u5185\u6280\u8853\u6587\u66F8  |  Page ", font: "Arial", size: 16, color: "888888" }),
            new TextRun({ children: [PageNumber.CURRENT], font: "Arial", size: 16, color: "888888" }),
          ]
        })
      ] })
    },
    children
  }]
});

const outPath = "/sessions/quirky-friendly-franklin/mnt/TORASAN/TORASAN_\u30B7\u30B9\u30C6\u30E0\u4ED5\u69D8\u66F8.docx";
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
