#!/usr/bin/env node
/**
 * gen_trace_matrix.js — トレーサビリティマトリクス生成 (MD源トレース対応版)
 * TORASAN_トレーサビリティマトリクス.xlsx
 * 
 * 5つのシート:
 * 1. トレースID一覧 (既存)
 * 2. 双方向トレースマトリクス (既存)
 * 3. 関係性詳細 (既存)
 * 4. MDソーストレース (新規 - MD源トレーサビリティ)
 * 5. 凡例 (更新 - MD情報追加)
 */

const fs = require("fs");
const ExcelJS = require("exceljs");

const traceData = JSON.parse(
  fs.readFileSync(
    "/sessions/quirky-friendly-franklin/trace_data.json",
    "utf-8"
  )
);

// Color definitions
const COLORS = {
  claude: "B4C7E7",    // Light blue (CLAUDE.md)
  process: "92D050",   // Light green (PROCESS.md)
  ingest: "FFC7CE",    // Light pink (INGEST.md)
  header: "1A3A5C",    // Dark blue header
  white: "FFFFFF"
};

// Helper: Get trace target section titles
function getTraceTitles(traceIds) {
  if (!traceIds || traceIds.length === 0) return "—";
  return traceIds
    .map(id => traceData.sections[id]?.title || id)
    .join("; ");
}

// Helper: Get file color based on MD source file
function getFileColor(mdSource) {
  if (mdSource.file === "CLAUDE.md") return COLORS.claude;
  if (mdSource.file === "PROCESS.md") return COLORS.process;
  if (mdSource.file === "INGEST.md") return COLORS.ingest;
  return "FFFFFF";
}

// Helper: Create styled cell
function styledCell(value, color, bold = false) {
  return {
    value,
    font: { name: "Arial", size: 11, bold },
    alignment: { horizontal: "left", vertical: "center", wrapText: true },
    border: {
      top: { style: "thin", color: "999999" },
      bottom: { style: "thin", color: "999999" },
      left: { style: "thin", color: "999999" },
      right: { style: "thin", color: "999999" }
    },
    fill: { type: "pattern", pattern: "solid", fgColor: { argb: color } }
  };
}

// Helper: Create header cell
function headerCell(value) {
  return {
    value,
    font: { name: "Arial", size: 11, bold: true, color: COLORS.white },
    alignment: { horizontal: "center", vertical: "center", wrapText: true },
    border: {
      top: { style: "thin", color: "999999" },
      bottom: { style: "thin", color: "999999" },
      left: { style: "thin", color: "999999" },
      right: { style: "thin", color: "999999" }
    },
    fill: { type: "pattern", pattern: "solid", fgColor: { argb: COLORS.header } }
  };
}

async function generateMatrix() {
  const workbook = new ExcelJS.Workbook();

  // ===== Sheet 1: トレースID一覧 =====
  const sheet1 = workbook.addWorksheet("トレースID一覧");
  sheet1.columns = [
    { header: "トレースID", key: "id", width: 15 },
    { header: "文書", key: "doc", width: 25 },
    { header: "セクション番号", key: "sec", width: 15 },
    { header: "タイトル", key: "title", width: 40 },
    { header: "上位トレース先", key: "up", width: 30 },
    { header: "下位/参照トレース先", key: "down", width: 30 }
  ];

  // Add header row styling
  for (let i = 1; i <= 6; i++) {
    sheet1.getCell(1, i).font = { bold: true, color: COLORS.white };
    sheet1.getCell(1, i).fill = {
      type: "pattern",
      pattern: "solid",
      fgColor: { argb: COLORS.header }
    };
  }

  // Add data rows
  Object.entries(traceData.sections).forEach(([id, section]) => {
    // Find references
    const up = [];
    const down = [];
    for (const [src, tgt, type] of traceData.relationships) {
      if (src === id && (type === "上位定義" || type === "参照")) down.push(tgt);
      if (tgt === id && type === "上位定義") up.push(src);
    }

    const docName =
      {
        SYS: "TORASAN_システム仕様書",
        PR: "PROCESS仕様書",
        IN: "INGEST仕様書"
      }[section.doc] || section.doc;

    sheet1.addRow({
      id,
      doc: docName,
      sec: section.sec,
      title: section.title,
      up: up.length > 0 ? up.join(", ") : "—",
      down: down.length > 0 ? down.join(", ") : "—"
    });
  });

  // ===== Sheet 2: 双方向トレースマトリクス =====
  const sheet2 = workbook.addWorksheet("双方向トレースマトリクス");
  const ids = Object.keys(traceData.sections).sort();

  // Create matrix
  const matrix = Array(ids.length + 1)
    .fill(null)
    .map(() => Array(ids.length + 1).fill(""));

  matrix[0][0] = "From \\ To";
  for (let i = 0; i < ids.length; i++) {
    matrix[0][i + 1] = ids[i];
    matrix[i + 1][0] = ids[i];
  }

  // Fill matrix
  for (const [src, tgt, type] of traceData.relationships) {
    const srcIdx = ids.indexOf(src);
    const tgtIdx = ids.indexOf(tgt);
    if (srcIdx >= 0 && tgtIdx >= 0) {
      let symbol = "";
      if (type === "上位定義") symbol = "▼";
      else if (type === "参照") symbol = "→";
      else if (type === "実装") symbol = "▲";
      else if (type === "後続") symbol = "»";

      if (matrix[srcIdx + 1][tgtIdx + 1] === "") {
        matrix[srcIdx + 1][tgtIdx + 1] = symbol;
      } else {
        matrix[srcIdx + 1][tgtIdx + 1] += " " + symbol;
      }
    }
  }

  // Add matrix to sheet
  for (let i = 0; i < matrix.length; i++) {
    for (let j = 0; j < matrix[i].length; j++) {
      const cell = sheet2.getCell(i + 1, j + 1);
      if (i === 0 || j === 0) {
        cell.value = matrix[i][j];
        cell.font = { bold: true, color: COLORS.white };
        cell.fill = {
          type: "pattern",
          pattern: "solid",
          fgColor: { argb: COLORS.header }
        };
      } else {
        cell.value = matrix[i][j] || "";
      }
      cell.alignment = { horizontal: "center", vertical: "center" };
      cell.border = {
        top: { style: "thin", color: "999999" },
        bottom: { style: "thin", color: "999999" },
        left: { style: "thin", color: "999999" },
        right: { style: "thin", color: "999999" }
      };
    }
  }

  // ===== Sheet 3: 関係性詳細 =====
  const sheet3 = workbook.addWorksheet("関係性詳細");
  sheet3.columns = [
    { header: "No.", key: "no", width: 8 },
    { header: "From", key: "from", width: 12 },
    { header: "To", key: "to", width: 12 },
    { header: "関係種別", key: "type", width: 12 },
    { header: "説明", key: "desc", width: 35 },
    { header: "根拠セクション(From)", key: "fromSec", width: 20 },
    { header: "対象セクション(To)", key: "toSec", width: 20 }
  ];

  for (let i = 1; i <= 7; i++) {
    sheet3.getCell(1, i).font = { bold: true, color: COLORS.white };
    sheet3.getCell(1, i).fill = {
      type: "pattern",
      pattern: "solid",
      fgColor: { argb: COLORS.header }
    };
  }

  traceData.relationships.forEach((rel, idx) => {
    const [from, to, type, desc] = rel;
    const fromSec = traceData.sections[from];
    const toSec = traceData.sections[to];

    sheet3.addRow({
      no: idx + 1,
      from,
      to,
      type,
      desc: desc || "",
      fromSec: `${fromSec.doc}仕様書 §${fromSec.sec}`,
      toSec: `${toSec.doc}仕様書 §${toSec.sec}`
    });
  });

  // ===== Sheet 4: MDソーストレース (NEW) =====
  const sheet4 = workbook.addWorksheet("MDソーストレース");
  sheet4.columns = [
    { header: "MDソースID", key: "mdId", width: 15 },
    { header: "ファイル", key: "file", width: 15 },
    { header: "セクション見出し", key: "heading", width: 35 },
    { header: "行範囲", key: "lines", width: 12 },
    { header: "トレース先ID(s)", key: "traceTo", width: 20 },
    { header: "トレース先セクション名", key: "traceToTitle", width: 40 }
  ];

  // Header styling
  for (let i = 1; i <= 6; i++) {
    sheet4.getCell(1, i).font = { bold: true, color: COLORS.white };
    sheet4.getCell(1, i).fill = {
      type: "pattern",
      pattern: "solid",
      fgColor: { argb: COLORS.header }
    };
  }

  // Add MD source data
  Object.entries(traceData.mdSources).forEach(([mdId, mdSource]) => {
    const row = sheet4.addRow({
      mdId,
      file: mdSource.file,
      heading: mdSource.heading,
      lines: mdSource.lines,
      traceTo: mdSource.traceTo.join(", "),
      traceToTitle: getTraceTitles(mdSource.traceTo)
    });

    // Apply color coding
    const color = getFileColor(mdSource);
    for (let col = 1; col <= 6; col++) {
      row.getCell(col).fill = {
        type: "pattern",
        pattern: "solid",
        fgColor: { argb: color }
      };
      row.getCell(col).border = {
        top: { style: "thin", color: "999999" },
        bottom: { style: "thin", color: "999999" },
        left: { style: "thin", color: "999999" },
        right: { style: "thin", color: "999999" }
      };
    }
  });

  // ===== Sheet 5: 凡例 (Updated) =====
  const sheet5 = workbook.addWorksheet("凡例");
  sheet5.columns = [
    { header: "記号/対象", key: "symbol", width: 25 },
    { header: "色", key: "color", width: 15 },
    { header: "種別/ファイル", key: "type", width: 20 },
    { header: "意味", key: "meaning", width: 50 }
  ];

  // Header
  for (let i = 1; i <= 4; i++) {
    sheet5.getCell(1, i).font = { bold: true, color: COLORS.white };
    sheet5.getCell(1, i).fill = {
      type: "pattern",
      pattern: "solid",
      fgColor: { argb: COLORS.header }
    };
  }

  // Add legend entries for relationships
  const legendData = [
    ["▼", "青", "上位定義", "上位文書（SYS）が下位文書（PR/IN）を定義"],
    ["→", "橙", "参照", "横方向の参照関係（PR↔IN間のデータ・フロー）"],
    ["▲", "緑", "実装", "下位文書の項目が上位文書の方針を実装する"],
    ["»", "黄", "後続", "INGEST STEP間のシーケンシャル順序"],
    ["", "", "", ""],
    ["MD源トレース (色分け):", "", "", ""],
    [
      "■ 薄青",
      "薄青",
      "CLAUDE.md",
      "CLAUDE.md内のセクションからのトレース"
    ],
    ["■ 薄緑", "薄緑", "PROCESS.md", "PROCESS.md内のセクションからのトレース"],
    [
      "■ 薄ピンク",
      "薄ピンク",
      "INGEST.md",
      "INGEST.md内のセクションからのトレース"
    ]
  ];

  legendData.forEach((legendRow) => {
    if (legendRow[0] === "" && legendRow[1] === "") {
      sheet5.addRow({
        symbol: legendRow[0],
        color: legendRow[1],
        type: legendRow[2],
        meaning: legendRow[3]
      });
    } else if (legendRow[0].includes("MD源トレース")) {
      const row = sheet5.addRow({
        symbol: legendRow[0],
        color: "",
        type: "",
        meaning: ""
      });
      row.font = { bold: true };
      row.getCell(1).font = { bold: true, size: 12 };
    } else if (
      legendRow[2] === "CLAUDE.md" ||
      legendRow[2] === "PROCESS.md" ||
      legendRow[2] === "INGEST.md"
    ) {
      const row = sheet5.addRow({
        symbol: legendRow[0],
        color: legendRow[1],
        type: legendRow[2],
        meaning: legendRow[3]
      });
      const fileColor =
        legendRow[2] === "CLAUDE.md"
          ? COLORS.claude
          : legendRow[2] === "PROCESS.md"
          ? COLORS.process
          : COLORS.ingest;
      for (let col = 1; col <= 4; col++) {
        row.getCell(col).fill = {
          type: "pattern",
          pattern: "solid",
          fgColor: { argb: fileColor }
        };
      }
    } else {
      sheet5.addRow({
        symbol: legendRow[0],
        color: legendRow[1],
        type: legendRow[2],
        meaning: legendRow[3]
      });
    }
  });

  // Save workbook
  const outputPath =
    "/sessions/quirky-friendly-franklin/mnt/TORASAN/TORASAN_トレーサビリティマトリクス.xlsx";
  await workbook.xlsx.writeFile(outputPath);
  console.log(`Generated: ${outputPath}`);
  console.log(
    `Sheets: ${workbook.worksheets.map((ws) => ws.name).join(", ")}`
  );
}

generateMatrix().catch((err) => {
  console.error("Error generating matrix:", err);
  process.exit(1);
});
