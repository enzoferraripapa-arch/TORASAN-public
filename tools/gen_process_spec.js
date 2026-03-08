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
const headerShading = { fill: "1F4E79", type: ShadingType.CLEAR };
const altShading = { fill: "F2F7FB", type: ShadingType.CLEAR };
const noShading = { fill: "FFFFFF", type: ShadingType.CLEAR };

const TW = 9360; // table width = content width (US Letter 1" margins)

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

function h1(text) { return new Paragraph({ heading: HeadingLevel.HEADING_1, spacing: { before: 360, after: 200 }, children: [new TextRun({ text, font: "Arial", size: 32, bold: true, color: "1F4E79" })] }); }
function h2(text) { return new Paragraph({ heading: HeadingLevel.HEADING_2, spacing: { before: 280, after: 160 }, children: [new TextRun({ text, font: "Arial", size: 26, bold: true, color: "2E75B6" })] }); }
function h3(text) { return new Paragraph({ heading: HeadingLevel.HEADING_3, spacing: { before: 200, after: 120 }, children: [new TextRun({ text, font: "Arial", size: 22, bold: true, color: "404040" })] }); }
function p(text, opts = {}) { return new Paragraph({ spacing: { after: 120 }, children: [new TextRun({ text, font: "Arial", size: 20, ...opts })] }); }
function pBold(label, text) { return new Paragraph({ spacing: { after: 120 }, children: [new TextRun({ text: label, font: "Arial", size: 20, bold: true }), new TextRun({ text, font: "Arial", size: 20 })] }); }

function makeTable(headers, rows, colWidths) {
  const tw = colWidths.reduce((a, b) => a + b, 0);
  return new Table({
    width: { size: tw, type: WidthType.DXA }, columnWidths: colWidths,
    rows: [
      new TableRow({ children: headers.map((h, i) => headerCell(h, colWidths[i])) }),
      ...rows.map((row, ri) => new TableRow({
        children: row.map((c, ci) => cell(c, colWidths[ci], { shaded: ri % 2 === 1, center: ci === 0 }))
      }))
    ]
  });
}

// ============ BUILD DOCUMENT ============
const children = [];

// --- COVER PAGE ---
children.push(new Paragraph({ spacing: { before: 3000 }, children: [] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 200 },
  children: [new TextRun({ text: "TORASAN", font: "Arial", size: 28, color: "1F4E79", bold: true })] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 100 },
  children: [new TextRun({ text: "機能安全開発プロジェクト", font: "Arial", size: 24, color: "666666" })] }));
children.push(new Paragraph({ spacing: { before: 400 }, alignment: AlignmentType.CENTER,
  children: [new TextRun({ text: "製品開発プロセス手順書", font: "Arial", size: 52, bold: true, color: "1F4E79" })] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 100, after: 100 },
  children: [new TextRun({ text: "ISO 26262\u30D9\u30FC\u30B9 \u30A2\u30B8\u30E3\u30A4\u30EB\u5BFE\u5FDC\u7248", font: "Arial", size: 28, color: "2E75B6" })] }));
children.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 600 }, border: { top: { style: BorderStyle.SINGLE, size: 6, color: "1F4E79", space: 1 } }, children: [] }));

// Document info table on cover
const infoW = [2800, 6560];
const infoRows = [
  ["\u6587\u66F8\u756A\u53F7", "TSDT-PR-001"],
  ["\u7248\u6570", "1.0"],
  ["\u4F5C\u6210\u65E5", "2026-02-27"],
  ["\u30B9\u30C6\u30FC\u30BF\u30B9", "\u8349\u6848"],
  ["\u4F5C\u6210\u8005", "TORASAN"],
  ["\u627F\u8A8D\u8005", "TBD"],
];
children.push(new Paragraph({ spacing: { before: 400 }, children: [] }));
children.push(new Table({
  width: { size: TW, type: WidthType.DXA }, columnWidths: infoW,
  rows: infoRows.map((r, i) => new TableRow({
    children: [
      cell(r[0], infoW[0], { bold: true, shaded: true }),
      cell(r[1], infoW[1], { shaded: false }),
    ]
  }))
}));

children.push(new Paragraph({ children: [new PageBreak()] }));

// --- TOC ---
children.push(h1("\u76EE\u6B21"));
children.push(new TableOfContents("Table of Contents", { hyperlink: true, headingStyleRange: "1-3" }));
children.push(new Paragraph({ children: [new PageBreak()] }));

// === SECTION 1: OVERVIEW ===
children.push(h1("1. \u6982\u8981 [PR-01]"));
children.push(p("トレースID: PR-01 | ↑ SYS-01, SYS-02, SYS-04, SYS-06, SYS-09", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md 'このファイルについて / 設計思想' (L1-L21) / PROCESS.md '参照ファイル一覧' (L24-L35) / PROCESS.md '起動時の動作 (STEP 1-5)' (L166-L251) / PROCESS.md 'コーディングガイドライン管理' (L470-L496) / PROCESS.md '外部ライブラリ管理' (L499-L513) / PROCESS.md '共通実行ルール' (L950-L964)", { italics: true, color: "888888", size: 14 }));

children.push(h2("1.1 \u6587\u66F8\u306E\u76EE\u7684"));
children.push(p("\u672C\u6587\u66F8\u306F\u3001TORASAN\u6A5F\u80FD\u5B89\u5168\u958B\u767A\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u306B\u304A\u3051\u308BISO 26262\u30D9\u30FC\u30B9\u306E\u88FD\u54C1\u958B\u767A\u30D7\u30ED\u30BB\u30B9\u624B\u9806\u66F8\u3067\u3042\u308B\u3002Claude Code\uFF08AI\u30B3\u30FC\u30C9\u751F\u6210\u30FB\u6587\u66F8\u751F\u6210\u30C4\u30FC\u30EB\uFF09\u306B\u3088\u308B\u958B\u767A\u652F\u63F4\u3092\u524D\u63D0\u3068\u3057\u3001V\u30E2\u30C7\u30EB\u306B\u57FA\u3065\u304F\u30D5\u30A7\u30FC\u30BA\u69CB\u6210\u3001\u6210\u679C\u7269\u5B9A\u7FA9\u3001\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3\u7BA1\u7406\u3001\u5909\u66F4\u7BA1\u7406\u3001\u30EC\u30D3\u30E5\u30FC\u30D7\u30ED\u30BB\u30B9\u3092\u5B9A\u7FA9\u3059\u308B\u3002"));

children.push(h2("1.2 \u8A2D\u8A08\u601D\u60F3"));
children.push(p("ISO 26262 V\u30E2\u30C7\u30EB\u3092\u9AA8\u683C\u3068\u3057\u3001\u4ED6\u898F\u683C\uFF08IEC 61508 / IEC 60730 / IEC 62061 / JIS B 9705 / EU\u6A5F\u68B0\u898F\u5247\u7B49\uFF09\u306F\u5DEE\u5206\u3068\u3057\u3066\u5BFE\u5FDC\u3059\u308B\u3002"));
children.push(p("\u30A2\u30B8\u30E3\u30A4\u30EB\u7684\u904B\u7528\u304C\u524D\u63D0\u3067\u3042\u308A\u3001\u30D5\u30A7\u30FC\u30BA\u306F\u53CD\u5FA9\u30FB\u524D\u5F8C\u81EA\u7531\u3067\u5B9F\u884C\u53EF\u80FD\u3002MCU\u30FB\u90E8\u54C1\u30FBSIL\u30EC\u30D9\u30EB\u7B49\u306E\u524D\u63D0\u5909\u66F4\u306F\u968F\u6642\u5BFE\u5FDC\u3059\u308B\u3002"));
children.push(p("AI\u306E\u62C5\u5F53\u7BC4\u56F2\u306FSW\u30B3\u30F3\u30DD\u30FC\u30CD\u30F3\u30C8\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8\u307E\u3067\u3068\u3057\u3001\u5B9F\u6A5F\u30C6\u30B9\u30C8\u30FB\u59A5\u5F53\u6027\u78BA\u8A8D\u306F\u4EBA\u9593\u304C\u62C5\u5F53\u3059\u308B\u3002"));
children.push(p("\u6210\u679C\u7269\u306F\u300C\u63A2\u7D22\u30E2\u30FC\u30C9\uFF08\u66AB\u5B9A\uFF09\u300D\u2192\u300C\u8A8D\u8A3C\u30E2\u30FC\u30C9\uFF08\u78BA\u5B9A\uFF09\u300D\u306E2\u6BB5\u968E\u3067\u7BA1\u7406\u3059\u308B\u3002"));

children.push(h2("1.3 \u9069\u7528\u7BC4\u56F2"));
children.push(p("\u672C\u30D7\u30ED\u30BB\u30B9\u306F\u3001\u8ECA\u8F09\u30FB\u7523\u696D\u6A5F\u68B0\u30FB\u5BB6\u96FB\u30FB\u533B\u7642\u30FBIoT\u7B49\u306E\u6A5F\u80FD\u5B89\u5168\u304C\u6C42\u3081\u3089\u308C\u308B\u7D44\u307F\u8FBC\u307F\u88FD\u54C1\u958B\u767A\u306B\u9069\u7528\u3055\u308C\u308B\u3002"));

children.push(h2("1.4 \u53C2\u7167\u6587\u66F8\u4E00\u89A7"));
children.push(makeTable(
  ["\u30D5\u30A1\u30A4\u30EB", "\u5185\u5BB9", "\u4F5C\u6210\u30BF\u30A4\u30DF\u30F3\u30B0"],
  [
    ["docs/\u30B3\u30FC\u30C7\u30A3\u30F3\u30B0\u30AC\u30A4\u30C9\u30E9\u30A4\u30F3.md", "MISRA-C\u6E96\u62E0\u30EB\u30FC\u30EB\u30FB\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u56FA\u6709\u898F\u7D04", "PH-09\u958B\u59CB\u524D"],
    ["docs/\u5916\u90E8\u30E9\u30A4\u30D6\u30E9\u30EA\u7BA1\u7406.md", "OSS\u30FBRTOS\u30FB\u30B5\u30FC\u30C9\u30D1\u30FC\u30C6\u30A3\u306E\u5B89\u5168\u6027\u8A55\u4FA1\u8A18\u9332", "PH-09\u958B\u59CB\u524D"],
    ["docs/\u30C4\u30FC\u30EB\u9069\u683C\u6027\u8A18\u9332.md", "Claude Code\u7B49\u306E\u958B\u767A\u30C4\u30FC\u30EB\u306ETCL\u5206\u985E\u30FB\u691C\u8A3C\u8A18\u9332", "PH-01\u3068\u540C\u6642"],
    ["docs/\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3.md", "\u5168\u30D5\u30A7\u30FC\u30BA\u6A2A\u65AD\u306E\u53CC\u65B9\u5411\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3\u30DE\u30C8\u30EA\u30AF\u30B9", "\u5404\u30D5\u30A7\u30FC\u30BA\u66F4\u65B0\u6642"],
    ["docs/FMEA.md", "\u6545\u969C\u30E2\u30FC\u30C9\u5F71\u97FF\u89E3\u6790", "PH-07\u4EE5\u964D"],
    ["docs/FTA.md", "\u30D5\u30A9\u30FC\u30EB\u30C8\u30C4\u30EA\u30FC\u5206\u6790", "ASIL C/D\u6642\u5FC5\u9808"],
  ],
  [3000, 4000, 2360]
));

// === SECTION 2: OPERATION MODES ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("2. \u904B\u7528\u30E2\u30FC\u30C9 [PR-02]"));
children.push(p("トレースID: PR-02 | ↑ SYS-05, SYS-08 | → IN-11", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md '運用モード' (L38-L63)", { italics: true, color: "888888", size: 14 }));

children.push(h2("2.1 \u63A2\u7D22\u30E2\u30FC\u30C9\uFF08EXPLORE\uFF09"));
children.push(p("\u8981\u7D20\u958B\u767A\u30FB\u30B3\u30F3\u30BB\u30D7\u30C8\u691C\u8A0E\u30FB\u30D7\u30ED\u30C8\u30BF\u30A4\u30D7\u30D5\u30A7\u30FC\u30BA\u3067\u4F7F\u7528\u3059\u308B\u30C7\u30D5\u30A9\u30EB\u30C8\u30E2\u30FC\u30C9\u3002"));
children.push(makeTable(
  ["\u9805\u76EE", "\u5185\u5BB9"],
  [
    ["\u6210\u679C\u7269\u30B9\u30C6\u30FC\u30BF\u30B9", "\u300C\u66AB\u5B9A\u300D/ \u300CTBD\u300D\u3067\u9032\u3081\u3066\u3088\u3044"],
    ["\u30D5\u30A7\u30FC\u30BA\u9806\u5E8F", "\u81EA\u7531\uFF08\u30B9\u30AD\u30C3\u30D7\u30FB\u9006\u623B\u308AOK\uFF09"],
    ["\u524D\u63D0\u5909\u66F4\u5BFE\u5FDC", "\u5F71\u97FF\u7BC4\u56F2\u3092\u5373\u5EA7\u306B\u63D0\u793A"],
    ["\u30EC\u30D3\u30E5\u30FC", "\u7C21\u6613\u7248\uFF08\u30BB\u30EB\u30D5\u30EC\u30D3\u30E5\u30FC\uFF09"],
    ["\u5909\u66F4\u7BA1\u7406", "\u7C21\u6613\u7248\uFF08changeLog\u8A18\u9332\u306E\u307F\uFF09"],
  ],
  [2800, 6560]
));

children.push(h2("2.2 \u8A8D\u8A3C\u30E2\u30FC\u30C9\uFF08CERTIFY\uFF09"));
children.push(p("\u88FD\u54C1\u5316\u30FB\u898F\u683C\u8A8D\u8A3C\u53D6\u5F97\u30D5\u30A7\u30FC\u30BA\u3002"));
children.push(makeTable(
  ["\u9805\u76EE", "\u5185\u5BB9"],
  [
    ["\u30D5\u30A7\u30FC\u30BA\u9806\u5E8F", "ISO 26262 V\u30E2\u30C7\u30EB\u306B\u5F93\u3044\u53B3\u5BC6\u306B\u7BA1\u7406"],
    ["\u6210\u679C\u7269\u30B9\u30C6\u30FC\u30BF\u30B9", "\u300C\u627F\u8A8D\u6E08\u307F\u300D\u306B\u683C\u4E0A\u3052\u5F8C\u306B\u6B21\u30D5\u30A7\u30FC\u30BA\u3078"],
    ["\u5909\u66F4\u7BA1\u7406", "\u5909\u66F4\u7533\u8ACB\u2192\u5F71\u97FF\u8A55\u4FA1\u2192\u627F\u8A8D\u2192\u5B9F\u65BD\u2192\u518D\u30EC\u30D3\u30E5\u30FC"],
    ["TBD\u30FB\u66AB\u5B9A\u4E8B\u9805", "\u3059\u3079\u3066\u30BC\u30ED\u306B\u3059\u308B\u3053\u3068"],
    ["\u30EC\u30D3\u30E5\u30FC\u8A18\u9332", "\u5FC5\u9808\uFF08\u30C4\u30FC\u30EB\u9069\u683C\u6027\u78BA\u8A8D\u542B\u3080\uFF09"],
  ],
  [2800, 6560]
));

// === SECTION 3: PHASE STRUCTURE ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("3. V\u30E2\u30C7\u30EB \u30D5\u30A7\u30FC\u30BA\u69CB\u6210 [PR-03]"));
children.push(p("トレースID: PR-03 | ↑ SYS-05 | → IN-05, IN-07, IN-08", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md '起動時の動作 (STEP 1-5)' (L166-L251) / PROCESS.md 'Vモデルフェーズ構成とAI担当範囲' (L516-L536) / PROCESS.md '各フェーズ詳細 (PH-01〜PH-15)' (L566-L848)", { italics: true, color: "888888", size: 14 }));

children.push(h2("3.1 \u30D5\u30A7\u30FC\u30BA\u4E00\u89A7\u3068AI\u62C5\u5F53\u7BC4\u56F2"));
children.push(p("V\u5B57\u5DE6\u8FBA\uFF08\u8A2D\u8A08\u30FB\u8981\u6C42\uFF09\u3068V\u5B57\u53F3\u8FBA\uFF08\u691C\u8A3C\u30FB\u30C6\u30B9\u30C8\u4ED5\u69D8\uFF09\u306E\u5BFE\u5FDC\u95A2\u4FC2\u3092\u4EE5\u4E0B\u306B\u793A\u3059\u3002"));
children.push(makeTable(
  ["V\u5B57\u5DE6\u8FBA\uFF08\u8A2D\u8A08\uFF09", "V\u5B57\u53F3\u8FBA\uFF08\u691C\u8A3C\uFF09", "AI\u62C5\u5F53"],
  [
    ["PH-01 \u5B89\u5168\u8A08\u753B", "PH-15 \u6A5F\u80FD\u5B89\u5168\u30A2\u30BB\u30B9\u30E1\u30F3\u30C8", "\u25CE"],
    ["PH-02 \u30A2\u30A4\u30C6\u30E0\u5B9A\u7FA9", "\u2014", "\u25CE"],
    ["PH-03 HARA", "PH-14 \u8ECA\u4E21\u7D71\u5408\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8", "\u25CE"],
    ["PH-04 \u6A5F\u80FD\u5B89\u5168\u30B3\u30F3\u30BB\u30D7\u30C8", "PH-13 \u30B7\u30B9\u30C6\u30E0\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8", "\u25CE"],
    ["PH-05 \u6280\u8853\u5B89\u5168\u30B3\u30F3\u30BB\u30D7\u30C8", "PH-12 HW\u7D71\u5408\u30C6\u30B9\u30C8\u4ED5\u69D8\uFF08\u67A0\uFF09", "\u25CE / \u25B3"],
    ["PH-06 \u30B7\u30B9\u30C6\u30E0\u8A2D\u8A08", "\u2014", "\u25CE"],
    ["PH-07 HW\u8981\u6C42\u30FB\u8A2D\u8A08\uFF08\u67A0\uFF09", "\u2014", "\u25B3\uFF08\u67A0\u306E\u307F\uFF09"],
    ["PH-08 SW\u5B89\u5168\u8981\u6C42(SRS)", "PH-11 SW\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8\u7FA4", "\u25CE"],
    ["PH-09 SW\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3\u8A2D\u8A08", "\u2014", "\u25CE"],
    ["PH-10 SW\u30E6\u30CB\u30C3\u30C8\u8A2D\u8A08\u30FB\u5B9F\u88C5", "\u2014", "\u25CE"],
  ],
  [3500, 3860, 2000]
));
children.push(p("\u25CE = AI\u62C5\u5F53\uFF08\u8A2D\u8A08\u30FB\u4ED5\u69D8\u66F8\u751F\u6210\uFF09\u3001\u25B3 = AI\u67A0\u306E\u307F\uFF08\u8A73\u7D30\u306FHW\u62C5\u5F53\u8005\uFF09\u3001\u270B = \u4EBA\u9593\u304C\u62C5\u5F53\uFF08\u30C6\u30B9\u30C8\u5B9F\u65BD\u30FB\u5B9F\u6A5F\u691C\u8A3C\u30FB\u30EC\u30D3\u30E5\u30FC\u627F\u8A8D\uFF09"));

// === SECTION 3.2-3.16: EACH PHASE ===
const phases = [
  { id: "PH-01", name: "\u5B89\u5168\u8A08\u753B\uFF08Safety Plan\uFF09", ref: "Part 2 cl.7 / Part 6 cl.5", output: "docs/01_\u5B89\u5168\u8A08\u753B.md",
    items: ["\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u6982\u8981\u3068\u5B89\u5168\u8A08\u753B\u306E\u9069\u7528\u7BC4\u56F2", "\u5B89\u5168\u7D44\u7E54\u3068\u8CAC\u4EFB\u5206\u62C5", "\u9069\u7528\u898F\u683C\u30FB\u5B89\u5168\u30EC\u30D9\u30EB\u76EE\u6A19\uFF08TBD\u53EF\uFF09", "\u5B89\u5168\u6D3B\u52D5\u30EA\u30B9\u30C8\u3068\u30B9\u30B1\u30B8\u30E5\u30FC\u30EB\u6982\u8981", "\u5B89\u5168\u30C4\u30FC\u30EB\u30FB\u958B\u767A\u74B0\u5883\uFF08\u30C4\u30FC\u30EB\u9069\u683C\u6027\u8A18\u9332\u3092\u53C2\u7167\uFF09", "\u30EC\u30D3\u30E5\u30FC\u8A08\u753B\uFF08\u5404\u30D5\u30A7\u30FC\u30BA\u306E\u30EC\u30D3\u30E5\u30FC\u65B9\u5F0F\uFF09", "\u5909\u66F4\u7BA1\u7406\u65B9\u91DD", "\u524D\u63D0\u6761\u4EF6\u30FBTBD\u4E8B\u9805\u30EA\u30B9\u30C8"] },
  { id: "PH-02", name: "\u30A2\u30A4\u30C6\u30E0\u5B9A\u7FA9\uFF08Item Definition\uFF09", ref: "Part 3 cl.5", output: "docs/02_\u30A2\u30A4\u30C6\u30E0\u5B9A\u7FA9.md",
    items: ["\u30A2\u30A4\u30C6\u30E0\u306E\u6A5F\u80FD\u8AAC\u660E", "\u30B7\u30B9\u30C6\u30E0\u5883\u754C\u5B9A\u7FA9\uFF08\u30D6\u30ED\u30C3\u30AF\u56F3\uFF09", "\u52D5\u4F5C\u74B0\u5883\u30FB\u4F7F\u7528\u6761\u4EF6\u30FB\u60F3\u5B9A\u30E6\u30FC\u30B6\u30FC", "\u65E2\u77E5\u306E\u30CF\u30B6\u30FC\u30C9\u30FB\u5B89\u5168\u95A2\u9023\u6CD5\u898F", "\u5916\u90E8\u30A4\u30F3\u30BF\u30D5\u30A7\u30FC\u30B9\u4E00\u89A7", "\u524D\u63D0\u6761\u4EF6\u30FBTBD\u4E8B\u9805\u30EA\u30B9\u30C8"] },
  { id: "PH-03", name: "HARA\uFF08\u30CF\u30B6\u30FC\u30C9\u5206\u6790\u30FB\u30EA\u30B9\u30AF\u8A55\u4FA1\uFF09", ref: "Part 3 cl.6", output: "docs/03_HARA.md",
    items: ["\u4F7F\u7528\u30B7\u30CA\u30EA\u30AA\u30FB\u52D5\u4F5C\u30E2\u30FC\u30C9\u5B9A\u7FA9", "\u30CF\u30B6\u30FC\u30C9\u540C\u5B9A\u30EA\u30B9\u30C8", "\u30EA\u30B9\u30AF\u8A55\u4FA1\u8868\uFF08SG-ID\u3092\u30C8\u30EC\u30FC\u30B9ID\u3068\u3057\u3066\u4ED8\u4E0E\uFF09", "\u5B89\u5168\u76EE\u6A19\uFF08SG-XXX\uFF09\u4E00\u89A7", "\u6B8B\u7559\u30EA\u30B9\u30AF\u30FBTBD\u4E8B\u9805"] },
  { id: "PH-04", name: "\u6A5F\u80FD\u5B89\u5168\u30B3\u30F3\u30BB\u30D7\u30C8\uFF08FSC\uFF09", ref: "Part 3 cl.7", output: "docs/04_FSC.md",
    items: ["SG \u2192 FSR \u30DE\u30C3\u30D4\u30F3\u30B0", "\u6A5F\u80FD\u5B89\u5168\u8981\u6C42\uFF08FSR-XXX / ASIL / \u4E0A\u4F4DSG-ID\u660E\u8A18\uFF09", "\u4E88\u5099\u7684\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3", "\u5B89\u5168\u6A5F\u69CB\u306E\u6982\u5FF5", "\u5916\u90E8\u30EA\u30B9\u30AF\u4F4E\u6E1B\u624B\u6BB5\u306E\u524D\u63D0", "TBD\u4E8B\u9805"] },
  { id: "PH-05", name: "\u6280\u8853\u5B89\u5168\u30B3\u30F3\u30BB\u30D7\u30C8\uFF08TSC\uFF09", ref: "Part 4 cl.7", output: "docs/05_TSC.md",
    items: ["TSR\u4E00\u89A7\uFF08TSR-XXX / ASIL / HW or SW\u5272\u308A\u5F53\u3066 / \u4E0A\u4F4DFSR-ID\u660E\u8A18\uFF09", "\u30B7\u30B9\u30C6\u30E0\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3", "ASIL\u5206\u89E3\uFF08\u5FC5\u8981\u306A\u5834\u5408\uFF09", "\u5B89\u5168\u6A5F\u69CB\u306E\u6280\u8853\u7684\u5B9F\u73FE\u65B9\u91DD", "\u8A3A\u65AD\u30AB\u30D0\u30EC\u30C3\u30B8\uFF08DC\uFF09\u76EE\u6A19\u5024", "TBD\u4E8B\u9805"] },
  { id: "PH-06", name: "\u30B7\u30B9\u30C6\u30E0\u8A2D\u8A08", ref: "Part 4 cl.7", output: "docs/06_\u30B7\u30B9\u30C6\u30E0\u8A2D\u8A08.md",
    items: ["\u30B7\u30B9\u30C6\u30E0\u69CB\u6210\u8A73\u7D30", "HW/SW\u30A4\u30F3\u30BF\u30D5\u30A7\u30FC\u30B9\u5B9A\u7FA9", "\u901A\u4FE1\u30FB\u4FE1\u53F7\u4E00\u89A7", "\u5B89\u5168\u6A5F\u69CB\u306E\u8A73\u7D30\u8A2D\u8A08", "\u30B7\u30B9\u30C6\u30E0\u8A2D\u8A08 \u2192 TSR \u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3", "TBD\u4E8B\u9805"] },
  { id: "PH-07", name: "HW\u8981\u6C42\u30FB\u8A2D\u8A08\uFF08\u67A0\u306E\u307F\uFF09", ref: "Part 5", output: "docs/07_HW\u8A2D\u8A08.md",
    items: ["HW\u5B89\u5168\u8981\u6C42\uFF08TSR\u306EHW\u5272\u308A\u5F53\u3066\u5206\u3092\u8EE2\u8A18\uFF09", "HW\u69CB\u6210\u6982\u8981\uFF08TBD\u53EF\uFF09", "\u5B89\u5168\u95A2\u9023HW\u8981\u7D20\u30EA\u30B9\u30C8", "HW\u8A3A\u65AD\u8981\u6C42\uFF08FMEDA\u53C2\u7167\uFF09", "\u3010\u8981HW\u62C5\u5F53\u8A18\u8F09\u3011\u56DE\u8DEF\u8A2D\u8A08\u30FB\u90E8\u54C1\u9078\u5B9A\u30FBFMEDA\u7D50\u679C"] },
  { id: "PH-08", name: "SW\u5B89\u5168\u8981\u6C42\u4ED5\u69D8\uFF08SRS\uFF09", ref: "Part 6 cl.6", output: "docs/08_SW\u5B89\u5168\u8981\u6C42.md",
    items: ["\u6587\u66F8\u7BA1\u7406\u60C5\u5831\u30FB\u9069\u7528\u898F\u683C", "SW\u5B89\u5168\u8981\u6C42\uFF08SR-XXX\uFF09", "\u8A3A\u65AD\u8981\u6C42\uFF08DR-XXX / DC\u76EE\u6A19\u5024\u660E\u8A18\uFF09", "SRS \u2192 TSR \u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3", "\u672A\u89E3\u6C7A\u4E8B\u9805\u30FBTBD"] },
  { id: "PH-09", name: "SW\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3\u8A2D\u8A08", ref: "Part 6 cl.7", output: "docs/09_SW\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3.md",
    items: ["SW\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3\u6982\u8981\uFF08\u30E2\u30B8\u30E5\u30FC\u30EB\u69CB\u6210\uFF09", "\u30E2\u30B8\u30E5\u30FC\u30EB\u4E00\u89A7\uFF08SA-ID\u4ED8\u4E0E / ASIL / \u4E0A\u4F4DSR-ID\u7D10\u4ED8\uFF09", "\u30BF\u30B9\u30AF\u30FB\u5272\u308A\u8FBC\u307F\u69CB\u6210", "\u30E2\u30B8\u30E5\u30FC\u30EB\u9593\u30A4\u30F3\u30BF\u30D5\u30A7\u30FC\u30B9\u5B9A\u7FA9", "\u5B89\u5168\u6A5F\u69CB\u306E\u5B9F\u88C5\u65B9\u91DD", "\u30E1\u30E2\u30EA\u30DE\u30C3\u30D7\u6982\u8981", "\u30B3\u30FC\u30C7\u30A3\u30F3\u30B0\u30AC\u30A4\u30C9\u30E9\u30A4\u30F3\u9069\u7528\u65B9\u91DD", "\u5916\u90E8\u30E9\u30A4\u30D6\u30E9\u30EA\u4F7F\u7528\u7B87\u6240", "TBD\u4E8B\u9805"] },
  { id: "PH-10", name: "SW\u30E6\u30CB\u30C3\u30C8\u8A2D\u8A08\u30FB\u5B9F\u88C5", ref: "Part 6 cl.8 / cl.9", output: "docs/10_SW\u30E6\u30CB\u30C3\u30C8\u8A2D\u8A08.md + src/",
    items: ["\u30E6\u30CB\u30C3\u30C8\u4E00\u89A7\uFF08UT-ID\u4ED8\u4E0E / \u4E0A\u4F4DSA-ID / SR-ID\u7D10\u4ED8\uFF09", "\u5404\u30E6\u30CB\u30C3\u30C8\u306E\u8A73\u7D30\u8A2D\u8A08\uFF08IF\u30FB\u72B6\u614B\u9077\u79FB\uFF09", "\u5B89\u5168\u95A2\u9023\u30E6\u30CB\u30C3\u30C8\u306E\u8A2D\u8A08\u6839\u62E0", "\u30B3\u30FC\u30C9\u5185\u5B89\u5168\u30BF\u30B0\uFF08@req / @asil / @test\uFF09"] },
  { id: "PH-11", name: "SW\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8\u7FA4", ref: "Part 6 cl.9/10/11", output: "docs/11_\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8/",
    items: ["PH-11a: SW\u30E6\u30CB\u30C3\u30C8\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8", "PH-11b: SW\u7D50\u5408\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8", "PH-11c: SW\u30B3\u30F3\u30DD\u30FC\u30CD\u30F3\u30C8\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8", "\u5404\u30C6\u30B9\u30C8\u30B1\u30FC\u30B9\u306BTC-ID\u4ED8\u4E0E\u30FB\u4E0A\u4F4D\u8981\u4EF6\u7D10\u4ED8", "\u30AB\u30D0\u30EC\u30C3\u30B8\u76EE\u6A19\uFF08ASIL\u5225\uFF09", "\u30C6\u30B9\u30C8\u5B8C\u4E86\u57FA\u6E96\uFF08Exit Criteria\uFF09"] },
  { id: "PH-12", name: "HW\u7D71\u5408\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8\uFF08\u67A0\u306E\u307F\uFF09", ref: "Part 5", output: "docs/12_HW\u7D71\u5408\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8.md",
    items: ["\u30C6\u30B9\u30C8\u65B9\u91DD\u30FB\u74B0\u5883", "\u30C6\u30B9\u30C8\u5BFE\u8C61HW\u6A5F\u80FD\u30FB\u5B89\u5168\u6A5F\u69CB", "\u3010\u8981HW\u62C5\u5F53\u8A18\u8F09\u3011\u30C6\u30B9\u30C8\u30B1\u30FC\u30B9\u8A73\u7D30", "\u30C6\u30B9\u30C8\u5B8C\u4E86\u57FA\u6E96"] },
  { id: "PH-13", name: "\u30B7\u30B9\u30C6\u30E0\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8", ref: "Part 4 cl.8", output: "docs/13_\u30B7\u30B9\u30C6\u30E0\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8.md",
    items: ["\u30C6\u30B9\u30C8\u65B9\u91DD\uFF08\u5B9F\u6A5F\u5FC5\u8981\u306A\u65E8\u3092\u660E\u8A18\uFF09", "\u30B7\u30B9\u30C6\u30E0\u5B89\u5168\u6A5F\u80FD\u306E\u691C\u8A3C\u9805\u76EE\uFF08\u4E0A\u4F4DFSR/SG\u7D10\u4ED8\uFF09", "\u30C6\u30B9\u30C8\u30B1\u30FC\u30B9\uFF08TC-ID\u4ED8\u4E0E / SG\u30FBFSR\u7D10\u4ED8\uFF09", "\u30D5\u30A9\u30FC\u30EB\u30C8\u30A4\u30F3\u30B8\u30A7\u30AF\u30B7\u30E7\u30F3\u30C6\u30B9\u30C8\u9805\u76EE", "\u30C6\u30B9\u30C8\u5B8C\u4E86\u57FA\u6E96"] },
  { id: "PH-14", name: "\u8ECA\u4E21\u7D71\u5408\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8", ref: "Part 4 cl.8\uFF08ISO 26262\u9078\u629E\u6642\u306E\u307F\uFF09", output: "docs/14_\u8ECA\u4E21\u7D71\u5408\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8.md",
    items: ["\u30C6\u30B9\u30C8\u65B9\u91DD\u30FB\u8ECA\u4E21\u30C6\u30B9\u30C8\u74B0\u5883", "\u8ECA\u4E21\u30EC\u30D9\u30EB\u3067\u306E\u5B89\u5168\u76EE\u6A19\u306E\u691C\u8A3C\u9805\u76EE\uFF08SG\u7D10\u4ED8\uFF09", "\u30C6\u30B9\u30C8\u30B1\u30FC\u30B9\uFF08TC-ID\u4ED8\u4E0E / SG\u7D10\u4ED8\uFF09", "\u30C6\u30B9\u30C8\u5B8C\u4E86\u57FA\u6E96"] },
  { id: "PH-15", name: "\u6A5F\u80FD\u5B89\u5168\u30A2\u30BB\u30B9\u30E1\u30F3\u30C8", ref: "Part 2 cl.15", output: "docs/15_\u5B89\u5168\u30A2\u30BB\u30B9\u30E1\u30F3\u30C8.md",
    items: ["\u5B89\u5168\u30B1\u30FC\u30B9\u30B5\u30DE\u30EA\u30FC", "\u5B89\u5168\u76EE\u6A19\u306E\u9054\u6210\u78BA\u8A8D\uFF08SG\u5225\u30FBC\u5168\u4EF6\u30FBTC\u7D10\u4ED8\u304D\u78BA\u8A8D\uFF09", "\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3\u5B8C\u7D50\u78BA\u8A8D", "\u898F\u683C\u5225\u30C1\u30A7\u30C3\u30AF\u30EA\u30B9\u30C8", "\u30EC\u30D3\u30E5\u30FC\u5B8C\u4E86\u78BA\u8A8D\u30FB\u30C4\u30FC\u30EB\u9069\u683C\u6027\u8A18\u9332\u306E\u78BA\u8A8D", "FMEA/FTA\u7D50\u679C\u306E\u78BA\u8A8D", "\u672A\u89E3\u6C7A\u4E8B\u9805\u30FB\u6B8B\u7559\u30EA\u30B9\u30AF\u4E00\u89A7", "\u78BA\u8A8D\u30FB\u7F72\u540D\u6B04"] },
];

phases.forEach((ph, idx) => {
  children.push(h2(`3.${idx + 2} ${ph.id}\uFF1A${ph.name}`));
  children.push(pBold("ISO 26262\u53C2\u7167\uFF1A", ph.ref));
  children.push(pBold("\u51FA\u529B\u30D5\u30A1\u30A4\u30EB\uFF1A", ph.output));
  children.push(p("\u6210\u679C\u7269\u306E\u69CB\u6210\u9805\u76EE\uFF1A"));
  const itemCols = [600, 8760];
  children.push(new Table({
    width: { size: TW, type: WidthType.DXA }, columnWidths: itemCols,
    rows: ph.items.map((item, i) => new TableRow({
      children: [
        cell(`${i + 1}`, itemCols[0], { center: true, shaded: i % 2 === 1 }),
        cell(item, itemCols[1], { shaded: i % 2 === 1 }),
      ]
    }))
  }));
});

// === SECTION 4: REQUIREMENT TRACEABILITY ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("4. \u8981\u4EF6\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3\u7BA1\u7406 [PR-04]"));
children.push(p("トレースID: PR-04 | ↑ SYS-07 | → IN-08", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md '作業トレース' (L67-L121) / PROCESS.md '進捗ダッシュボード' (L124-L162) / PROCESS.md '要件トレーサビリティ管理' (L254-L309)", { italics: true, color: "888888", size: 14 }));

children.push(h2("4.1 \u8981\u4EF6ID\u4F53\u7CFB"));
children.push(makeTable(
  ["ID\u4F53\u7CFB", "\u30D5\u30A7\u30FC\u30BA", "\u8AAC\u660E"],
  [
    ["SG-XXX", "PH-03", "\u5B89\u5168\u76EE\u6A19 Safety Goal"],
    ["FSR-XXX", "PH-04", "\u6A5F\u80FD\u5B89\u5168\u8981\u6C42 Functional Safety Requirement"],
    ["TSR-XXX", "PH-05", "\u6280\u8853\u5B89\u5168\u8981\u6C42 Technical Safety Requirement"],
    ["SR-XXX", "PH-08", "SW\u5B89\u5168\u8981\u6C42 Software Safety Requirement"],
    ["DR-XXX", "PH-08", "\u8A3A\u65AD\u8981\u6C42 Diagnostic Requirement"],
    ["SA-XXX", "PH-09", "SW\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3\u8981\u7D20"],
    ["UT-XXX", "PH-10", "SW\u30E6\u30CB\u30C3\u30C8"],
    ["TC-XXX", "PH-11", "\u30C6\u30B9\u30C8\u30B1\u30FC\u30B9"],
  ],
  [2000, 1500, 5860]
));

children.push(h2("4.2 \u53CC\u65B9\u5411\u30C8\u30EC\u30FC\u30B9\u306E\u6D41\u308C"));
children.push(p("\u4E0B\u6D41\u30C8\u30EC\u30FC\u30B9\uFF1ASG \u2192 FSR \u2192 TSR \u2192 SR \u2192 SA \u2192 UT \u2192 TC"));
children.push(p("\u4E0A\u6D41\u30C8\u30EC\u30FC\u30B9\uFF1ATC \u2192 SR \u2192 TSR \u2192 FSR \u2192 SG"));
children.push(p("\u5404\u30D5\u30A7\u30FC\u30BA\u5B8C\u4E86\u5F8C\u306B docs/\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3.md \u3092\u81EA\u52D5\u66F4\u65B0\u3059\u308B\u3002\u8A8D\u8A3C\u30E2\u30FC\u30C9\u79FB\u884C\u524D\u306B\u672A\u7D10\u4ED8\u3051\u3092\u30BC\u30ED\u306B\u3059\u308B\u3053\u3068\u3002"));

// === SECTION 5: CHANGE MANAGEMENT ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("5. \u5909\u66F4\u7BA1\u7406 [PR-05]"));
children.push(pBold("ISO 26262\u53C2\u7167\uFF1A", "Part 8 cl.7"));

children.push(h2("5.1 \u5909\u66F4\u7BA1\u7406\u30D7\u30ED\u30BB\u30B9"));
children.push(makeTable(
  ["\u30E2\u30FC\u30C9", "\u5909\u66F4\u30D5\u30ED\u30FC"],
  [
    ["\u63A2\u7D22\u30E2\u30FC\u30C9", "\u5909\u66F4\u767A\u751F \u2192 changeLog\u8A18\u9332 \u2192 \u5F71\u97FF\u7BC4\u56F2\u78BA\u8A8D \u2192 \u6210\u679C\u7269\u66F4\u65B0"],
    ["\u8A8D\u8A3C\u30E2\u30FC\u30C9", "\u5909\u66F4\u7533\u8ACB \u2192 \u5F71\u97FF\u8A55\u4FA1 \u2192 \u627F\u8A8D \u2192 \u5B9F\u65BD \u2192 \u518D\u30EC\u30D3\u30E5\u30FC \u2192 changeLog\u8A18\u9332"],
  ],
  [2200, 7160]
));

children.push(h2("5.2 \u524D\u63D0\u5909\u66F4\u5F71\u97FF\u30DE\u30C8\u30EA\u30AF\u30B9"));
children.push(makeTable(
  ["\u5909\u66F4\u9805\u76EE", "\u5F71\u97FF\u30D5\u30A7\u30FC\u30BA", "\u5F71\u97FF\u8981\u4EF6\u4F53\u7CFB"],
  [
    ["MCU\u5909\u66F4", "PH-07/08/09/10/11", "SR\u30FBSA\u30FBUT\u30FBTC"],
    ["ASIL\u30EC\u30D9\u30EB\u5909\u66F4", "PH-03/04/05/08/09/11", "SG\u30FBFSR\u30FBTSR\u30FBSR\u30FBTC"],
    ["\u9069\u7528\u898F\u683C\u8FFD\u52A0", "\u5168\u30D5\u30A7\u30FC\u30BA", "\u5168\u8981\u4EF6ID\u306B\u5DEE\u5206\u8FFD\u8A18"],
    ["\u30B7\u30B9\u30C6\u30E0\u5883\u754C\u5909\u66F4", "PH-02/03/04/05/06", "SG\u30FBFSR\u30FBTSR"],
    ["OS/RTOS\u5909\u66F4", "PH-09/10/11", "SA\u30FBUT\u30FBTC"],
    ["\u5B89\u5168\u6A5F\u69CB\u306E\u8FFD\u52A0\u30FB\u524A\u9664", "PH-05/06/08/09/11", "TSR\u30FBSR\u30FBSA\u30FBTC"],
    ["\u5916\u90E8\u30E9\u30A4\u30D6\u30E9\u30EA\u5909\u66F4", "PH-09/10/11", "SA\u30FBUT\u30FBTC"],
  ],
  [2600, 3380, 3380]
));

children.push(h2("5.3 \u30D0\u30FC\u30B8\u30E7\u30F3\u7BA1\u7406\u30EB\u30FC\u30EB"));
children.push(makeTable(
  ["\u30D0\u30FC\u30B8\u30E7\u30F3\u5909\u5316", "\u6761\u4EF6"],
  [
    ["\u30DE\u30A4\u30CA\u30FC\uFF08v1.0\u2192v1.1\uFF09", "\u5185\u5BB9\u306E\u8FFD\u8A18\u30FB\u8EFD\u5FAE\u306A\u4FEE\u6B63"],
    ["\u30E1\u30B8\u30E3\u30FC\uFF08v1.x\u2192v2.0\uFF09", "\u5B89\u5168\u8981\u6C42\u30FB\u5B89\u5168\u76EE\u6A19\u306E\u5909\u66F4\u30FB\u5927\u5E45\u4FEE\u6B63"],
    ["\u30D9\u30FC\u30B9\u30E9\u30A4\u30F3\u8A2D\u5B9A", "\u8A8D\u8A3C\u30E2\u30FC\u30C9\u79FB\u884C\u6642\u30FB\u5916\u90E8\u30EC\u30D3\u30E5\u30FC\u63D0\u51FA\u6642\u30FB\u898F\u683C\u8A8D\u8A3C\u7533\u8ACB\u6642"],
  ],
  [3200, 6160]
));

// === SECTION 6: REVIEW ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("6. \u30EC\u30D3\u30E5\u30FC\u30D7\u30ED\u30BB\u30B9 [PR-06]"));
children.push(p("トレースID: PR-06 | ↑ SYS-07", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md 'レビュープロセス' (L353-L410)", { italics: true, color: "888888", size: 14 }));
children.push(pBold("ISO 26262\u53C2\u7167\uFF1A", "Part 2 cl.6 / Part 6 cl.6\u301C11"));

children.push(h2("6.1 \u30EC\u30D3\u30E5\u30FC\u65B9\u5F0F"));
children.push(makeTable(
  ["\u30E2\u30FC\u30C9", "\u65B9\u5F0F", "\u30BF\u30A4\u30DF\u30F3\u30B0"],
  [
    ["\u63A2\u7D22\u30E2\u30FC\u30C9", "\u30BB\u30EB\u30D5\u30EC\u30D3\u30E5\u30FC\uFF08\u78BA\u8A8D\u30EA\u30B9\u30C8\u3067\u5B9F\u65BD\uFF09", "\u4EFB\u610F"],
    ["\u8A8D\u8A3C\u30E2\u30FC\u30C9", "\u6B63\u5F0F\u30EC\u30D3\u30E5\u30FC\uFF08\u8A18\u9332\u5FC5\u9808\uFF09", "\u6B21\u30D5\u30A7\u30FC\u30BA\u79FB\u884C\u524D\u306B\u5FC5\u9808"],
  ],
  [2200, 4560, 2600]
));

children.push(h2("6.2 \u72EC\u7ACB\u30EC\u30D3\u30E5\u30FC\u306E\u8981\u5426\uFF08ISO 26262\uFF09"));
children.push(makeTable(
  ["ASIL", "SW\u8981\u6C42", "SW\u8A2D\u8A08", "\u30B3\u30FC\u30C9", "\u30C6\u30B9\u30C8\u4ED5\u69D8"],
  [
    ["ASIL A/B", "\u30BB\u30EB\u30D5\u53EF", "\u30BB\u30EB\u30D5\u53EF", "\u30BB\u30EB\u30D5\u53EF", "\u30BB\u30EB\u30D5\u53EF"],
    ["ASIL C", "\u72EC\u7ACB\u63A8\u5968", "\u72EC\u7ACB\u63A8\u5968", "\u72EC\u7ACB\u5FC5\u9808", "\u72EC\u7ACB\u63A8\u5968"],
    ["ASIL D", "\u72EC\u7ACB\u5FC5\u9808", "\u72EC\u7ACB\u5FC5\u9808", "\u72EC\u7ACB\u5FC5\u9808", "\u72EC\u7ACB\u5FC5\u9808"],
  ],
  [1500, 1965, 1965, 1965, 1965]
));

// === SECTION 7: TOOL QUALIFICATION ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("7. \u30C4\u30FC\u30EB\u9069\u683C\u6027\u78BA\u8A8D [PR-07]"));
children.push(p("トレースID: PR-07 | ↑ SYS-07", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md 'ツール適格性確認' (L413-L434)", { italics: true, color: "888888", size: 14 }));
children.push(pBold("ISO 26262\u53C2\u7167\uFF1A", "Part 8 cl.11"));
children.push(makeTable(
  ["\u9805\u76EE", "\u5185\u5BB9"],
  [
    ["\u30C4\u30FC\u30EB\u540D", "Claude Code\uFF08Anthropic\uFF09"],
    ["\u30C4\u30FC\u30EB\u7A2E\u5225", "AI\u30B3\u30FC\u30C9\u751F\u6210\u30FB\u6587\u66F8\u751F\u6210\u30C4\u30FC\u30EB"],
    ["TCL\u5206\u985E", "TCL3\uFF08\u751F\u6210\u7269\u304CSW\u5B89\u5168\u8981\u6C42\u30FB\u8A2D\u8A08\u30FB\u30B3\u30FC\u30C9\u306B\u76F4\u7D50\uFF09"],
    ["\u5BFE\u5FDC\u65B9\u6CD5", "\u65B9\u6CD5A\uFF1A\u30C4\u30FC\u30EB\u51FA\u529B\u306E\u5B8C\u5168\u691C\u8A3C\uFF08\u5168\u6210\u679C\u7269\u3092\u4EBA\u9593\u304C\u30EC\u30D3\u30E5\u30FC\u30FB\u627F\u8A8D\uFF09"],
  ],
  [2800, 6560]
));

// === SECTION 8: FMEA/FTA ===
children.push(h1("8. FMEA / FTA\uFF08\u6545\u969C\u89E3\u6790\uFF09 [PR-08]"));
children.push(p("トレースID: PR-08 | ↑ SYS-07", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md 'FMEA / FTA' (L438-L467)", { italics: true, color: "888888", size: 14 }));
children.push(pBold("ISO 26262\u53C2\u7167\uFF1A", "Part 5\uFF08HW-FMEDA\uFF09/ Part 6\uFF08SW FMEA\u53C2\u8003\uFF09"));
children.push(makeTable(
  ["\u5BFE\u8C61", "\u63A2\u7D22\u30E2\u30FC\u30C9", "\u8A8D\u8A3C\u30E2\u30FC\u30C9"],
  [
    ["HW-FMEDA", "\u8EFD\u91CF\u7248\uFF08\u4E3B\u8981\u6545\u969C\u30E2\u30FC\u30C9\u306E\u307F\uFF09", "\u5FC5\u9808\uFF08Part 5\u8981\u6C42\uFF09"],
    ["SW-FMEA", "\u4EFB\u610F", "ASIL C/D\u63A8\u5968\u30FBASIL D\u5FC5\u9808"],
    ["FTA", "\u4EFB\u610F", "ASIL C/D\u5FC5\u9808"],
  ],
  [2200, 3580, 3580]
));

// === SECTION 9: MULTI-STANDARD MATRIX ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("9. \u898F\u683C\u5225\u30D5\u30A7\u30FC\u30BA\u30DE\u30C8\u30EA\u30AF\u30B9 [PR-09]"));
children.push(p("\u25CE=\u5FC5\u9808\u3001\u25CB=\u63A8\u5968\u3001\u25B3=\u4EFB\u610F\u3001\u2014=\u5BFE\u8C61\u5916\u3001\u2245=26262\u3068\u985E\u4F3C\u30FB\u8AAD\u307F\u66FF\u3048"));
const stdCols = [2200, 1150, 1150, 1150, 1150, 1150, 1410];
children.push(makeTable(
  ["\u30D5\u30A7\u30FC\u30BA", "ISO 26262", "IEC 61508", "IEC 60730", "IEC 62061", "JIS B 9705", "EU\u6A5F\u68B0\u898F\u5247"],
  [
    ["PH-01 \u5B89\u5168\u8A08\u753B", "\u25CE", "\u25CE", "\u25CB", "\u25CE", "\u25CB", "\u25CB"],
    ["PH-02 \u30A2\u30A4\u30C6\u30E0\u5B9A\u7FA9", "\u25CE", "\u2245", "\u25B3", "\u2245", "\u2245", "\u25CB"],
    ["PH-03 HARA", "\u25CE", "\u2245HAZOP", "\u25B3", "\u2245HAZOP", "\u2245ISO12100", "\u25CE"],
    ["PH-04 FSC", "\u25CE", "\u2245\u5B89\u5168\u6A5F\u80FD\u8981\u6C42", "\u2014", "\u2245SFS", "\u2014", "\u2014"],
    ["PH-05 TSC", "\u25CE", "\u2245SRS", "\u25CB", "\u2245SRS", "\u25CB", "\u2014"],
    ["PH-06 \u30B7\u30B9\u30C6\u30E0\u8A2D\u8A08", "\u25CE", "\u25CE", "\u25CB", "\u25CE", "\u25CB", "\u25CB"],
    ["PH-08 SW\u5B89\u5168\u8981\u6C42", "\u25CE", "\u25CE", "\u25CB", "\u25CE", "\u25CB", "\u25CB"],
    ["PH-09 SW\u30A2\u30FC\u30AD\u30C6\u30AF\u30C1\u30E3", "\u25CE", "\u25CE", "\u25CB", "\u25CE", "\u25CB", "\u25CB"],
    ["PH-10 SW\u5B9F\u88C5", "\u25CE", "\u25CE", "\u25CE", "\u25CE", "\u25CE", "\u25CE"],
    ["PH-11 SW\u30C6\u30B9\u30C8\u4ED5\u69D8", "\u25CE", "\u25CE", "\u25CE", "\u25CE", "\u25CE", "\u25CE"],
    ["PH-13 \u30B7\u30B9\u30C6\u30E0\u30C6\u30B9\u30C8", "\u25CE", "\u25CE", "\u25CB", "\u25CE", "\u25CB", "\u25CB"],
    ["PH-14 \u8ECA\u4E21\u7D71\u5408\u30C6\u30B9\u30C8", "\u25CE", "\u2014", "\u2014", "\u2014", "\u2014", "\u2014"],
    ["PH-15 \u5B89\u5168\u30A2\u30BB\u30B9\u30E1\u30F3\u30C8", "\u25CE", "\u25CE", "\u2014", "\u25CB", "\u2014", "\u2014"],
    ["FMEA / FTA", "\u25CB", "\u25CE", "\u25B3", "\u25CB", "\u25CB", "\u25CB"],
  ],
  stdCols
));

// === SECTION 10: FOLDER STRUCTURE ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("10. \u30D5\u30A9\u30EB\u30C0\u69CB\u6210 [PR-10]"));
children.push(p("トレースID: PR-10 | ↑ SYS-03 | → IN-02", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md 'フォルダ構成' (L868-L905)", { italics: true, color: "888888", size: 14 }));
children.push(p("\u6210\u679C\u7269\u306F\u4EE5\u4E0B\u306E\u30D5\u30A9\u30EB\u30C0\u69CB\u6210\u306B\u5F93\u3063\u3066\u7BA1\u7406\u3059\u308B\u3002"));

const folderRows = [
  ["CLAUDE.md", "\u8D77\u52D5\u8A2D\u5B9A\u30D5\u30A1\u30A4\u30EB"],
  ["PROCESS.md", "\u958B\u767A\u30D7\u30ED\u30BB\u30B9\u624B\u9806\u66F8\uFF08\u672C\u4F53\uFF09"],
  ["project.json", "\u30D7\u30ED\u30B8\u30A7\u30AF\u30C8\u72B6\u614B\u30FB\u9032\u6357\uFF08\u81EA\u52D5\u66F4\u65B0\uFF09"],
  ["docs/01_\u5B89\u5168\u8A08\u753B.md", "PH-01 \u6210\u679C\u7269"],
  ["docs/02_\u30A2\u30A4\u30C6\u30E0\u5B9A\u7FA9.md", "PH-02 \u6210\u679C\u7269"],
  ["docs/03_HARA.md", "PH-03 \u6210\u679C\u7269"],
  ["docs/04_FSC.md \u301C 15_\u5B89\u5168\u30A2\u30BB\u30B9\u30E1\u30F3\u30C8.md", "PH-04\u301CPH-15 \u6210\u679C\u7269"],
  ["docs/11_\u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8/", "PH-11 \u30C6\u30B9\u30C8\u4ED5\u69D8\u66F8\u7FA4\uFF0811a/11b/11c\uFF09"],
  ["docs/\u30C8\u30EC\u30FC\u30B5\u30D3\u30EA\u30C6\u30A3.md", "\u5168\u30D5\u30A7\u30FC\u30BA\u6A2A\u65AD\u30C8\u30EC\u30FC\u30B9\u30DE\u30C8\u30EA\u30AF\u30B9"],
  ["docs/\u30EC\u30D3\u30E5\u30FC\u8A18\u9332/", "\u30D5\u30A7\u30FC\u30BA\u5225\u30EC\u30D3\u30E5\u30FC\u8A18\u9332"],
  ["src/", "\u30BD\u30FC\u30B9\u30B3\u30FC\u30C9\uFF08\u5B89\u5168\u30BF\u30B0\u4ED8\u304D\uFF09"],
];
children.push(makeTable(["\u30D5\u30A1\u30A4\u30EB / \u30D5\u30A9\u30EB\u30C0", "\u5F79\u5272"], folderRows, [4680, 4680]));

// === SECTION 11: NOTES ===
children.push(new Paragraph({ children: [new PageBreak()] }));
children.push(h1("11. \u6CE8\u610F\u4E8B\u9805\u30FB\u514D\u8CAC [PR-11]"));
children.push(p("トレースID: PR-11 | ▲ SYS-07", { italics: true, color: "999999", size: 16 }));
children.push(p("MDソース: PROCESS.md '注意事項・免責' (L968-L974)", { italics: true, color: "888888", size: 14 }));
children.push(p("\u672C\u624B\u9806\u66F8\u306FAI\u306B\u3088\u308B\u652F\u63F4\u30C4\u30FC\u30EB\u3067\u3042\u308B\u3002\u6210\u679C\u7269\u306E\u6700\u7D42\u78BA\u8A8D\u30FB\u627F\u8A8D\u306F\u5FC5\u305A\u30A8\u30F3\u30B8\u30CB\u30A2\u304C\u5B9F\u65BD\u3059\u308B\u3053\u3068\u3002"));
children.push(p("Claude Code\u306E\u751F\u6210\u7269\u306FTCL3\u6271\u3044\u3067\u3042\u308A\u3001\u3059\u3079\u3066\u306E\u6210\u679C\u7269\u306F\u4EBA\u9593\u306B\u3088\u308B\u30EC\u30D3\u30E5\u30FC\u304C\u5FC5\u9808\u3067\u3042\u308B\u3002"));
children.push(p("\u6A5F\u80FD\u5B89\u5168\u898F\u683C\u3078\u306E\u9069\u5408\u6027\u306E\u6700\u7D42\u5224\u65AD\u306F\u8A8D\u5B9A\u6A5F\u95A2\u30FB\u5BE9\u67FB\u54E1\u304C\u884C\u3046\u3002"));
children.push(p("API\u30AD\u30FC\u30FB\u6A5F\u5BC6\u60C5\u5831\u3092\u672C\u30D5\u30A1\u30A4\u30EB\u306B\u8A18\u8F09\u3057\u306A\u3044\u3053\u3068\u3002"));

// build doc
const doc = new Document({
  styles: {
    default: { document: { run: { font: "Arial", size: 20 } } },
    paragraphStyles: [
      { id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 32, bold: true, font: "Arial", color: "1F4E79" },
        paragraph: { spacing: { before: 360, after: 200 }, outlineLevel: 0 } },
      { id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 26, bold: true, font: "Arial", color: "2E75B6" },
        paragraph: { spacing: { before: 280, after: 160 }, outlineLevel: 1 } },
      { id: "Heading3", name: "Heading 3", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 22, bold: true, font: "Arial", color: "404040" },
        paragraph: { spacing: { before: 200, after: 120 }, outlineLevel: 2 } },
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
          border: { bottom: { style: BorderStyle.SINGLE, size: 4, color: "1F4E79", space: 1 } },
          children: [new TextRun({ text: "TSDT-PR-001 v1.0  |  \u88FD\u54C1\u958B\u767A\u30D7\u30ED\u30BB\u30B9\u624B\u9806\u66F8", font: "Arial", size: 16, color: "888888" })]
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

const outPath = "/sessions/quirky-friendly-franklin/mnt/TORASAN/PROCESS_仕様書.docx";
Packer.toBuffer(doc).then(buf => {
  fs.writeFileSync(outPath, buf);
  // 自動検証
  const { execSync } = require("child_process");
  try {
    execSync(`node validate_docx.js "${outPath}"`, { stdio: "inherit" });
  } catch (e) {
    console.error("⛔ 検証失敗: 誤記またはXML不正が検出されました。修正してください。");
    process.exit(1);
  }
  console.log("Created: " + outPath);
});
