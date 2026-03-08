#!/bin/bash
# num_check.sh — PROCESS.md §5.3 数値整合チェック
# 使用方法: bash scripts/num_check.sh
# 終了コード: 0=全PASS, 1=矛盾あり

FAIL=0
GREP_OPTS="-rn --include=*.md --include=*.h --include=*.c --exclude-dir=archive* --exclude-dir=node_modules --exclude-dir=.git --exclude-dir=app --exclude-dir=docs/archive_v1"

echo "====== 数値整合チェック (PROCESS.md §5) ======"
echo ""

echo "--- RAM容量 (正: 5.5KB / 5632B) ---"
if grep $GREP_OPTS '12KB\|12 KB\|8KB\|8 KB' . src 2>/dev/null | grep -v 'archive'; then
    echo "❌ FAIL: 誤ったRAM容量"
    FAIL=1
else
    echo "✅ PASS"
fi
echo ""

echo "--- ROM容量 (正: 64KB) ---"
if grep $GREP_OPTS '256KB\|256 KB\|128KB\|128 KB' . src 2>/dev/null | grep -v 'archive'; then
    echo "❌ FAIL: 誤ったROM容量"
    FAIL=1
else
    echo "✅ PASS"
fi
echo ""

echo "--- ADCサンプリング周波数 (正: 100Hz/10Hz) ---"
if grep $GREP_OPTS '10kHz\|10 kHz\|1kHz\|1 kHz\|10000Hz\|1000Hz' . src 2>/dev/null | grep -v 'archive\|1000000'; then
    echo "❌ FAIL: 誤ったADC周波数"
    FAIL=1
else
    echo "✅ PASS"
fi
echo ""

echo "--- ADC分解能 (正: 10bit) ---"
if grep $GREP_OPTS '12ビット\|12bit\|12-bit' . src 2>/dev/null | grep -v 'archive'; then
    echo "❌ FAIL: 誤ったADC分解能"
    FAIL=1
else
    echo "✅ PASS"
fi
echo ""

echo "--- クロック公差 (正: ±4%) ---"
if grep $GREP_OPTS '±5%' . src 2>/dev/null | grep -v 'archive'; then
    echo "❌ FAIL: 誤ったクロック公差"
    FAIL=1
else
    echo "✅ PASS"
fi
echo ""

echo "--- CRCアルゴリズム (正: CRC32) ---"
if grep $GREP_OPTS 'CRC-16\|CRC16' . src 2>/dev/null | grep -v 'archive'; then
    echo "❌ FAIL: 誤ったCRCアルゴリズム"
    FAIL=1
else
    echo "✅ PASS"
fi
echo ""

echo "======================================"
if [ $FAIL -eq 0 ]; then
    echo "✅ 数値整合チェック: 全項目PASS"
else
    echo "❌ 数値整合チェック: 矛盾あり"
fi
exit $FAIL
