/**
 * Excalidraw compressed-json → SVG エクスポータ
 * Obsidian なしで .excalidraw.md から SVG を生成
 */
const fs = require('fs');
const path = require('path');

// ── LZ-string decompressor (base64 mode) ──
const LZString = (() => {
  const keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
  const baseReverseDic = {};
  function getBaseValue(a, c) {
    if (!baseReverseDic[a]) { baseReverseDic[a] = {}; for (let i = 0; i < a.length; i++) baseReverseDic[a][a[i]] = i; }
    return baseReverseDic[a][c];
  }
  function _decompress(length, resetValue, getNextValue) {
    const dict = []; let enlargeIn = 4, dictSize = 4, numBits = 3, entry = "", w, c;
    const result = []; let bits, maxpower, power, resb;
    const data = { val: getNextValue(0), position: resetValue, index: 1 };
    for (let i = 0; i < 3; i++) dict[i] = i;
    bits = 0; maxpower = 4; power = 1;
    while (power != maxpower) { resb = data.val & data.position; data.position >>= 1; if (!data.position) { data.position = resetValue; data.val = getNextValue(data.index++); } bits |= (resb > 0 ? 1 : 0) * power; power <<= 1; }
    switch (bits) {
      case 0: bits = 0; maxpower = 256; power = 1; while (power != maxpower) { resb = data.val & data.position; data.position >>= 1; if (!data.position) { data.position = resetValue; data.val = getNextValue(data.index++); } bits |= (resb > 0 ? 1 : 0) * power; power <<= 1; } c = String.fromCharCode(bits); break;
      case 1: bits = 0; maxpower = 65536; power = 1; while (power != maxpower) { resb = data.val & data.position; data.position >>= 1; if (!data.position) { data.position = resetValue; data.val = getNextValue(data.index++); } bits |= (resb > 0 ? 1 : 0) * power; power <<= 1; } c = String.fromCharCode(bits); break;
      case 2: return "";
    }
    dict[3] = c; w = c; result.push(c);
    while (true) {
      if (data.index > length) return "";
      bits = 0; maxpower = Math.pow(2, numBits); power = 1;
      while (power != maxpower) { resb = data.val & data.position; data.position >>= 1; if (!data.position) { data.position = resetValue; data.val = getNextValue(data.index++); } bits |= (resb > 0 ? 1 : 0) * power; power <<= 1; }
      switch (c = bits) {
        case 0: bits = 0; maxpower = 256; power = 1; while (power != maxpower) { resb = data.val & data.position; data.position >>= 1; if (!data.position) { data.position = resetValue; data.val = getNextValue(data.index++); } bits |= (resb > 0 ? 1 : 0) * power; power <<= 1; } dict[dictSize++] = String.fromCharCode(bits); c = dictSize - 1; enlargeIn--; break;
        case 1: bits = 0; maxpower = 65536; power = 1; while (power != maxpower) { resb = data.val & data.position; data.position >>= 1; if (!data.position) { data.position = resetValue; data.val = getNextValue(data.index++); } bits |= (resb > 0 ? 1 : 0) * power; power <<= 1; } dict[dictSize++] = String.fromCharCode(bits); c = dictSize - 1; enlargeIn--; break;
        case 2: return result.join('');
      }
      if (enlargeIn == 0) { enlargeIn = Math.pow(2, numBits); numBits++; }
      if (dict[c]) entry = dict[c]; else if (c === dictSize) entry = w + w[0]; else return null;
      result.push(entry); dict[dictSize++] = w + entry[0]; enlargeIn--;
      if (enlargeIn == 0) { enlargeIn = Math.pow(2, numBits); numBits++; }
      w = entry;
    }
  }
  return { decompressFromBase64: (input) => input ? _decompress(input.length, 32, (i) => getBaseValue(keyStr, input[i])) : null };
})();

// ── SVG Renderer ──
const COLORS = {
  '#2E3048': '#2E3048', '#3D405B': '#3D405B', '#E07A5F': '#E07A5F',
  '#81B29A': '#81B29A', '#F4F1DE': '#F4F1DE', '#F2CC8F': '#F2CC8F',
};

function escapeXml(s) { return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;'); }

function colorToHex(c) {
  if (!c || c === 'transparent') return 'none';
  return c;
}

function renderElement(el, padding) {
  const x = (el.x || 0) - padding.minX;
  const y = (el.y || 0) - padding.minY;
  const w = el.width || 0;
  const h = el.height || 0;
  const stroke = colorToHex(el.strokeColor) || '#1e1e1e';
  const fill = colorToHex(el.backgroundColor) || 'none';
  const sw = el.strokeWidth || 2;
  const opacity = el.opacity != null ? el.opacity / 100 : 1;

  let svg = '';

  if (el.type === 'rectangle') {
    const rx = el.roundness ? 8 : 0;
    svg = `<rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${rx}" fill="${fill}" stroke="${stroke}" stroke-width="${sw}" opacity="${opacity}"/>`;
  }
  else if (el.type === 'ellipse') {
    const cx = x + w/2, cy = y + h/2;
    svg = `<ellipse cx="${cx}" cy="${cy}" rx="${w/2}" ry="${h/2}" fill="${fill}" stroke="${stroke}" stroke-width="${sw}" opacity="${opacity}"/>`;
  }
  else if (el.type === 'diamond') {
    const cx = x + w/2, cy = y + h/2;
    const points = `${cx},${y} ${x+w},${cy} ${cx},${y+h} ${x},${cy}`;
    svg = `<polygon points="${points}" fill="${fill}" stroke="${stroke}" stroke-width="${sw}" opacity="${opacity}"/>`;
  }
  else if (el.type === 'arrow' || el.type === 'line') {
    if (el.points && el.points.length >= 2) {
      const pts = el.points.map(p => `${x + p[0]},${y + p[1]}`).join(' ');
      const marker = el.type === 'arrow' ? ' marker-end="url(#arrowhead)"' : '';
      svg = `<polyline points="${pts}" fill="none" stroke="${stroke}" stroke-width="${sw}" opacity="${opacity}"${marker}/>`;
    }
  }
  else if (el.type === 'text') {
    const fontSize = el.fontSize || 14;
    const textColor = stroke;
    const lines = (el.text || '').split('\n');
    const lineHeight = fontSize * 1.3;
    const textAnchor = el.textAlign === 'center' ? 'middle' : el.textAlign === 'right' ? 'end' : 'start';
    const tx = el.textAlign === 'center' ? x + w/2 : el.textAlign === 'right' ? x + w : x;

    svg = `<text font-family="Meiryo, Yu Gothic, sans-serif" font-size="${fontSize}" fill="${textColor}" text-anchor="${textAnchor}" opacity="${opacity}">`;
    lines.forEach((line, i) => {
      svg += `<tspan x="${tx}" y="${y + fontSize + i * lineHeight}">${escapeXml(line)}</tspan>`;
    });
    svg += '</text>';
  }

  return svg;
}

function excalidrawToSvg(data) {
  const elements = (data.elements || []).filter(e => !e.isDeleted);
  if (elements.length === 0) return null;

  // Calculate bounding box
  const PAD = 20;
  let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
  for (const el of elements) {
    const ex = el.x || 0, ey = el.y || 0, ew = el.width || 0, eh = el.height || 0;
    minX = Math.min(minX, ex);
    minY = Math.min(minY, ey);
    if (el.type === 'arrow' || el.type === 'line') {
      for (const p of (el.points || [])) {
        maxX = Math.max(maxX, ex + p[0]);
        maxY = Math.max(maxY, ey + p[1]);
      }
    } else {
      maxX = Math.max(maxX, ex + ew);
      maxY = Math.max(maxY, ey + eh);
    }
  }

  const width = maxX - minX + PAD * 2;
  const height = maxY - minY + PAD * 2;
  const padding = { minX: minX - PAD, minY: minY - PAD };

  // Sort by z-order (shapes first, then text on top)
  const shapes = elements.filter(e => e.type !== 'text');
  const texts = elements.filter(e => e.type === 'text');
  const sorted = [...shapes, ...texts];

  let svgContent = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ${width} ${height}" width="${width}" height="${height}">\n`;
  svgContent += `<rect width="${width}" height="${height}" fill="#ffffff"/>\n`;
  svgContent += `<defs><marker id="arrowhead" markerWidth="10" markerHeight="7" refX="10" refY="3.5" orient="auto"><polygon points="0 0, 10 3.5, 0 7" fill="#1e1e1e"/></marker></defs>\n`;

  for (const el of sorted) {
    const rendered = renderElement(el, padding);
    if (rendered) svgContent += rendered + '\n';
  }

  svgContent += '</svg>';
  return svgContent;
}

// ── Main ──
const diagramDir = path.join(__dirname, '..', 'docs', 'diagrams', 'excalidraw');
const svgOutDir = diagramDir; // Output SVGs alongside sources

const targets = ['02_distribution_model', '03_memory_architecture', '05_tool_resolution'];

for (const name of targets) {
  const mdPath = path.join(diagramDir, `${name}.excalidraw.md`);
  if (!fs.existsSync(mdPath)) { console.log(`${name}: not found`); continue; }

  const content = fs.readFileSync(mdPath, 'utf8');
  const match = content.match(/```compressed-json\n([\s\S]*?)\n```/);
  if (!match) { console.log(`${name}: no compressed-json`); continue; }

  const decompressed = LZString.decompressFromBase64(match[1].trim());
  if (!decompressed) { console.log(`${name}: decompression failed`); continue; }

  const data = JSON.parse(decompressed);
  const svg = excalidrawToSvg(data);
  if (!svg) { console.log(`${name}: no elements`); continue; }

  const svgPath = path.join(svgOutDir, `${name}.excalidraw.svg`);
  fs.writeFileSync(svgPath, svg, 'utf8');
  console.log(`${name}.excalidraw.svg: ${svg.length} chars`);
}
