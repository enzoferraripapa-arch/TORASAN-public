# Safety Diagnostics Reference (IEC 60730 / ISO 26262)

## 1. IEC 60730 Annex H Table H.1 - Class B Measures

| Component | Annex H Ref | Class B Measure | Test Method | DC |
|-----------|-------------|-----------------|-------------|-----|
| CPU registers | H.2.16.5 | Stuck-at test (SW) | Checkerboard pattern (0x55/0xAA) per register | Medium |
| Program Counter | H.2.16.6 | PC stuck-at test | Jump to known addresses, verify execution | Medium |
| Interrupt handling | H.2.16.7 | Interrupt monitor | Verify occurrence within time window | Medium |
| Clock | H.2.18.10 | Frequency monitor | Cross-reference independent clock source | Medium |
| Volatile memory (RAM) | H.2.19.6 | March C- or Checkerboard | March C- algorithm on all safety-relevant RAM | Medium |
| Non-volatile memory (Flash) | H.2.19.8 | CRC or checksum | CRC-32 over entire Flash at startup + periodic | Medium |
| Watchdog | H.2.14 | Temporal + logical | Time window WDT with program flow check | Medium |
| Stack overflow | H.2.16.9 | Stack monitoring | Pattern at stack boundary, periodic check | Low |
| I/O peripherals | H.2.16.12 | Plausibility check | Read-back output, cross-check input pairs | Low |
| Voltage | H.2.18.11 | OV/UV monitoring | ADC or comparator, independent reference | Medium |
| Communication | H.2.18.14 | Protocol check | CRC + sequence counter + timeout | Medium |

## 2. March C-Minus Algorithm (6 Steps)

**Notation**: r0=read expect 0, r1=read expect 1, w0=write 0, w1=write 1

| Step | Direction | Operation | Detects |
|------|-----------|-----------|---------|
| 1 | Forward (addr++) | w0 | Initialize all to 0 |
| 2 | Forward | r0, w1 | SAF-0, TF-0->1, AF |
| 3 | Forward | r1, w0 | SAF-1, TF-1->0, AF |
| 4 | Backward (addr--) | r0, w1 | SAF-0, TF-0->1, AF, CF |
| 5 | Backward | r1, w0 | SAF-1, TF-1->0, AF, CF |
| 6 | Forward | r0 | Final verify all zeros |

**Fault coverage**: Stuck-At (SAF), Transition (TF), Address decoder (AF), Coupling (CF)

**Runtime complexity**: 10n (n = number of cells)

### Runtime Implementation Notes
- Test in small blocks (e.g., 32-64 bytes) to limit interrupt latency
- Save/restore block contents if testing used RAM (destructive test)
- For startup: test full RAM before variable initialization
- For periodic: rotate tested block each cycle, complete full RAM within FTTI

## 3. CRC-32 Implementation for 8/16-bit MCU

| Strategy | Flash Usage | RAM Usage | Speed | Best For |
|----------|------------|-----------|-------|----------|
| Byte-wise (no table) | ~100 bytes | 4 bytes | Slow | Minimal resource MCU |
| Table 256-entry | ~1 KB table | 4 bytes | Fast | RL78, most 16-bit |
| Nibble (16-entry table) | ~80 bytes | 4 bytes | Medium | Balance speed/size |
| HW CRC peripheral | 0 | 0 | Fastest | If MCU has CRC unit |

**Polynomial**: 0x04C11DB7 (IEEE 802.3)
**Initial value**: 0xFFFFFFFF
**Final XOR**: 0xFFFFFFFF

### Execution Strategy
- **Startup**: Full Flash CRC before entering main loop (<500ms target)
- **Periodic**: Chunk-based (e.g., 4KB per cycle), complete within diagnostic interval
- **Store reference CRC**: In dedicated Flash section (not included in CRC calculation)

## 4. CPU Register Test Patterns (RL78 Context)

| Register Set | Test Pattern | Method |
|-------------|-------------|--------|
| General (R0-R7) | 0x55, 0xAA, 0xFF, 0x00 | Write pattern, read-back, compare |
| PSW (flags) | Set/clear each flag | Arithmetic ops, verify flag state |
| SP (Stack Pointer) | Save, load test value, verify, restore | Must disable interrupts |
| CS/ES (segment) | Save, write pattern, verify, restore | Test each segment register |
| Bank registers | Test each bank independently | Switch banks, write/verify |

**Key constraints on RL78:**
- Disable interrupts during SP test
- Save/restore context for bank switching tests
- Test within interrupt-safe window

## 5. Clock Monitoring Strategy

| Method | Implementation | Detection Time |
|--------|---------------|---------------|
| Main vs Sub-clock cross-check | Timer on main clock, capture on sub-clock, compare ratio | 1-10ms |
| External crystal WDT | Independent oscillator feeds WDT | WDT timeout period |
| PLL lock detection | Monitor PLL lock status register | Immediate (HW flag) |
| Frequency counter | Count main clock edges in fixed sub-clock window | Window period |

### Thresholds
- Acceptable frequency deviation: +/-2% (typical)
- Detection: Compare measured ratio against expected +/- tolerance
- Reaction: Switch to safe clock or enter safe state

## 6. Voltage Monitoring via ADC

### Implementation Parameters

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Sampling rate | 1 kHz (min) | Capture transients within FTTI |
| Filter | Moving average (N=4-8) | Reject noise, keep response <10ms |
| Hysteresis | +/-50mV around threshold | Prevent oscillation at boundary |
| Debounce | 3-5 consecutive samples | Confirm sustained fault |
| OV threshold | Vnom + 10% | Component absolute max rating |
| UV threshold | Vnom - 10% | Minimum operating voltage |
| Reference | Bandgap (internal) or ext ref | Must be independent of supply |

### State Machine
```
NORMAL -> (sample > OV_thresh for debounce_count) -> OV_FAULT -> SAFE_STATE
NORMAL -> (sample < UV_thresh for debounce_count) -> UV_FAULT -> SAFE_STATE
FAULT  -> (sample in range for recovery_count) -> NORMAL (if recovery allowed)
```

## 7. Self-Test Timing Table

| Test | Startup | Periodic | Duration | Interruptible |
|------|---------|----------|----------|---------------|
| CPU registers | Yes (full) | Yes (partial/rotating) | 50-200us | No |
| RAM March C- | Yes (full) | Yes (block rotation) | 5-50ms (full) | Yes (between blocks) |
| Flash CRC-32 | Yes (full) | Yes (chunk rotation) | 100-500ms (full) | Yes (between chunks) |
| Clock monitor | Yes | Continuous | N/A (HW) | N/A |
| WDT | Yes (verify) | Continuous | N/A | N/A |
| Stack overflow | Yes (init pattern) | Every 1-10ms | <10us | No |
| Voltage monitor | Yes | Continuous (1kHz ADC) | N/A | N/A |
| I/O plausibility | Yes | Every 10-100ms | 10-50us | No |

### Timing Budget Example (10ms main loop)
```
CPU register test:   200us (rotating subset)
RAM block test:      500us (64-byte block)
Flash CRC chunk:     500us (4KB chunk)
I/O plausibility:     50us
WDT refresh:          10us
Margin:             ~8.7ms for application
```

## 8. Common Pitfalls

| # | Pitfall | Fix |
|---|---------|-----|
| 1 | March test corrupts live data | Use save/restore or test at startup before init |
| 2 | CRC reference stored in CRC range | Place reference CRC in separate section |
| 3 | Clock monitor uses same clock source | Use truly independent oscillator |
| 4 | Voltage ADC uses same rail as reference | Use bandgap or separate reference |
| 5 | Self-test blocks interrupts too long | Break into small blocks, test between ISR windows |
