// Excalidraw compressed-json デコーダ
const fs = require('fs');

// LZ-string inline implementation (decompressFromBase64 only)
const LZString = (function() {
  const keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
  const baseReverseDic = {};
  function getBaseValue(alphabet, character) {
    if (!baseReverseDic[alphabet]) {
      baseReverseDic[alphabet] = {};
      for (let i = 0; i < alphabet.length; i++) {
        baseReverseDic[alphabet][alphabet.charAt(i)] = i;
      }
    }
    return baseReverseDic[alphabet][character];
  }
  function _decompress(length, resetValue, getNextValue) {
    const dictionary = [];
    let enlargeIn = 4, dictSize = 4, numBits = 3;
    let entry = "", result = [], w, c;
    let bits = 0, maxpower, power, resb;
    const data = { val: getNextValue(0), position: resetValue, index: 1 };
    for (let i = 0; i < 3; i++) dictionary[i] = i;
    maxpower = Math.pow(2, 2); power = 1;
    while (power != maxpower) {
      resb = data.val & data.position;
      data.position >>= 1;
      if (data.position == 0) { data.position = resetValue; data.val = getNextValue(data.index++); }
      bits |= (resb > 0 ? 1 : 0) * power;
      power <<= 1;
    }
    switch (bits) {
      case 0: bits = 0; maxpower = Math.pow(2, 8); power = 1;
        while (power != maxpower) {
          resb = data.val & data.position; data.position >>= 1;
          if (data.position == 0) { data.position = resetValue; data.val = getNextValue(data.index++); }
          bits |= (resb > 0 ? 1 : 0) * power; power <<= 1;
        }
        c = String.fromCharCode(bits); break;
      case 1: bits = 0; maxpower = Math.pow(2, 16); power = 1;
        while (power != maxpower) {
          resb = data.val & data.position; data.position >>= 1;
          if (data.position == 0) { data.position = resetValue; data.val = getNextValue(data.index++); }
          bits |= (resb > 0 ? 1 : 0) * power; power <<= 1;
        }
        c = String.fromCharCode(bits); break;
      case 2: return "";
    }
    dictionary[3] = c; w = c; result.push(c);
    while (true) {
      if (data.index > length) return "";
      bits = 0; maxpower = Math.pow(2, numBits); power = 1;
      while (power != maxpower) {
        resb = data.val & data.position; data.position >>= 1;
        if (data.position == 0) { data.position = resetValue; data.val = getNextValue(data.index++); }
        bits |= (resb > 0 ? 1 : 0) * power; power <<= 1;
      }
      switch (c = bits) {
        case 0: bits = 0; maxpower = Math.pow(2, 8); power = 1;
          while (power != maxpower) {
            resb = data.val & data.position; data.position >>= 1;
            if (data.position == 0) { data.position = resetValue; data.val = getNextValue(data.index++); }
            bits |= (resb > 0 ? 1 : 0) * power; power <<= 1;
          }
          dictionary[dictSize++] = String.fromCharCode(bits); c = dictSize - 1; enlargeIn--; break;
        case 1: bits = 0; maxpower = Math.pow(2, 16); power = 1;
          while (power != maxpower) {
            resb = data.val & data.position; data.position >>= 1;
            if (data.position == 0) { data.position = resetValue; data.val = getNextValue(data.index++); }
            bits |= (resb > 0 ? 1 : 0) * power; power <<= 1;
          }
          dictionary[dictSize++] = String.fromCharCode(bits); c = dictSize - 1; enlargeIn--; break;
        case 2: return result.join('');
      }
      if (enlargeIn == 0) { enlargeIn = Math.pow(2, numBits); numBits++; }
      if (dictionary[c]) { entry = dictionary[c]; }
      else if (c === dictSize) { entry = w + w.charAt(0); }
      else { return null; }
      result.push(entry);
      dictionary[dictSize++] = w + entry.charAt(0);
      enlargeIn--;
      if (enlargeIn == 0) { enlargeIn = Math.pow(2, numBits); numBits++; }
      w = entry;
    }
  }
  return {
    decompressFromBase64: function(input) {
      if (input == null || input == "") return null;
      return _decompress(input.length, 32, function(index) {
        return getBaseValue(keyStr, input.charAt(index));
      });
    }
  };
})();

// Process all .excalidraw.md files
const dir = 'docs/diagrams/excalidraw';
const files = fs.readdirSync(dir).filter(f => f.endsWith('.excalidraw.md'));

for (const file of files) {
  const content = fs.readFileSync(`${dir}/${file}`, 'utf8');
  const match = content.match(/```compressed-json\n([\s\S]*?)\n```/);
  if (!match) { console.log(`${file}: no compressed-json`); continue; }

  const decompressed = LZString.decompressFromBase64(match[1].trim());
  if (!decompressed) { console.log(`${file}: decompression failed`); continue; }

  const data = JSON.parse(decompressed);
  console.log(`\n=== ${file} ===`);
  console.log(`Elements: ${data.elements.length}`);

  // Print text elements
  const texts = data.elements.filter(e => e.type === 'text');
  texts.forEach(t => console.log(`  TEXT: "${t.text}"`));
}
