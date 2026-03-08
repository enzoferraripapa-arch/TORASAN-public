# BLDC Motor Safety Architecture Reference

## 1. Dual-Path Architecture

```
                    +---------------------------+
                    |     MAIN MCU (RL78)       |
                    |  - FOC/sensorless control |
                    |  - Application logic      |
                    |  - SW diagnostics         |
                    +---+---+---+---+-----------+
                        |   |   |   |
                   PWM  | SPI|  ADC | GPIO
                        |   |   |   |
                    +---v---v---v---v-----------+
                    |   GATE DRIVER / PRE-DRIVER |
                    +---+---+---+---+-----------+
                        |   |   |   |
                    +---v---v---v---v-----------+
                    |   3-PHASE INVERTER        |
                    |   (6x FET/IGBT)           |
                    +---+---+---+---+-----------+
                        |       |
                    +---v---+   +---v-----------+
                    | MOTOR |   | CURRENT SENSE |
                    +-------+   +-------+-------+
                                        |
          +-----------------------------+
          |
+---------v------------------+
| INDEPENDENT SAFETY PATH    |
| (HW + Safety MCU/Comparator)|
| - HW overcurrent comparator |
| - Independent WDT           |
| - Voltage supervisor         |
| - Safety relay/contactor     |
+----------------------------+
```

## 2. Independent Safety Path Components

| Component | Function | Response Time | Independence |
|-----------|----------|---------------|-------------|
| HW current comparator | Phase current > Imax | < 10us (analog) | Separate from MCU ADC |
| Shunt + op-amp | Current measurement | Continuous | Dedicated sense resistor |
| Independent WDT | MCU liveness | Configurable (50-500ms) | External IC or separate oscillator |
| Voltage supervisor | VCC monitoring | < 100us | Independent reference (bandgap) |
| Safety relay/contactor | AC mains disconnect | 10-20ms (mechanical) | Separate coil drive from MCU |
| Gate driver DESAT | FET/IGBT short detection | 1-5us | Built into gate driver IC |
| NTC thermistor | Motor/FET temperature | 100ms-1s | Analog path to comparator |

## 3. Overcurrent Protection: HW vs SW

| Parameter | HW Path | SW Path |
|-----------|---------|---------|
| Response time | < 10us | 50-100us (ADC + ISR) |
| Threshold accuracy | +/-5% (comparator) | +/-2% (calibrated ADC) |
| Independence | Fully independent | Same MCU |
| Failure mode | Fail-safe (comparator output) | MCU-dependent |
| Purpose | Catastrophic fault protection | Operational current limit |
| Implementation | Comparator + reference + latch | ADC sampling + SW compare |
| Reset | Manual or timed auto-reset | SW-controlled |

### HW Overcurrent Circuit
```
Ishunt --> Shunt R (e.g., 5 mohm)
       --> Differential Amp (gain=20)
       --> Comparator vs Vref (= Imax * Rshunt * Gain)
       --> Latch (SR flip-flop)
       --> Gate Driver INHIBIT pin (active low)
       --> All FETs OFF within <10us
```

## 4. Overspeed Detection

| Method | Implementation | Accuracy | Latency |
|--------|---------------|----------|---------|
| Back-EMF frequency | Timer capture on zero-crossing | +/-2% | 1 electrical cycle |
| Hall sensor period | Timer capture between Hall edges | +/-1% | 60 deg electrical |
| Encoder pulse rate | Counter over fixed time window | +/-0.5% | Window period |
| SW estimation | FOC observer output | +/-3% | Observer convergence |

### Thresholds (Washing Machine)
| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Max drum speed | 1200-1600 RPM | Structural limit of drum/bearing |
| Overspeed threshold | Max + 10% (1320-1760 RPM) | Detection margin |
| Detection time | < 500ms | Before structural damage |
| Reaction | STO (Safe Torque Off) | Immediate all-FET disable |
| Motor pole pairs | 4-8 (direct drive) | Affects electrical frequency |

## 5. IEC 60335-2-7 Washing Machine Requirements

| Clause | Requirement | Implementation |
|--------|-------------|---------------|
| 8.1.4 | Door lock during spin | Interlock: SW + independent HW |
| 19.1 | Thermal protection | NTC + SW monitor + independent thermal fuse |
| 19.7 | Motor protection | Overcurrent (HW + SW) + thermal cutoff |
| 22.33 | Drum speed limitation | Overspeed detection + STO |
| 22.40 | Water level control | Dual sensor (pressure + electrode) |
| 22.46 | Software Class B | IEC 60730 Annex H compliance |
| 30.2.2 | Earth continuity | < 0.1 ohm ground resistance |

## 6. Safe Torque Off (STO) Implementation

### STO Activation Conditions
| Condition | Source | Action |
|-----------|--------|--------|
| HW overcurrent | Comparator latch | Immediate gate inhibit |
| SW overcurrent | ADC ISR | PWM disable + relay open |
| Overspeed | Timer ISR / observer | PWM disable + relay open |
| MCU watchdog timeout | External WDT IC | Reset + gate inhibit |
| Overtemperature | NTC comparator | PWM disable + relay open |
| Communication loss | Safety MCU timeout | PWM disable |
| Door interlock failure | Safety switch input | PWM disable + relay open |

### STO Sequence
```
1. Disable all PWM outputs (GPIO override to LOW)     [< 1us]
2. Assert gate driver INHIBIT                          [< 1us]
3. Open safety relay (AC mains disconnect)             [< 20ms]
4. Set fault code in NVM                               [< 5ms]
5. Indicate fault to user (LED/display)                [immediate]
6. Require manual reset to restart                     [user action]
```

### STO Verification (Startup)
- Verify all FET gate voltages are LOW before enabling
- Verify safety relay is open before first energization
- Run gate driver diagnostics (DESAT check, bootstrap charge)
- Verify motor is stationary (no back-EMF)

## 7. Washing Machine Hazard List

| ID | Hazard | Severity | Safety Goal | ASIL |
|----|--------|----------|-------------|------|
| H1 | Motor runaway (drum overspeed) | Structural failure, projectile | SG1: Drum speed <= 1200 RPM or STO within 500ms | B |
| H2 | Overcurrent / FET short (fire) | Thermal runaway, fire | SG2: Phase current < Imax, HW cutoff < 10us | B |
| H3 | Door opens during high-speed spin | Limb injury from drum | SG3: Door locked while drum > 50 RPM | B |
| H4 | Water overflow (valve stuck open) | Flooding, electrical hazard | SG4: Water level bounded, valve timeout | B |
| H5 | Electric shock (insulation failure) | Electrocution | SG5: Earth continuity, GFCI | B |
| H6 | Motor stall with continued energization | Winding overheat, smoke/fire | SG6: Stall detected < 2s, STO activated | B |
| H7 | Loss of braking / free-wheeling drum | Coast to high speed, door open risk | SG7: Active braking or mechanical brake verified | B |

## 8. Common Pitfalls

| # | Pitfall | Fix |
|---|---------|-----|
| 1 | SW-only overcurrent protection | Always add HW comparator path (< 10us) |
| 2 | Single-point failure in gate driver | Use gate driver with DESAT + independent inhibit |
| 3 | Safety relay shares MCU GPIO with PWM | Separate safety relay drive from motor control path |
| 4 | No verification of STO state | Read back FET gate voltage and relay state |
| 5 | Overspeed detection only in SW | Add independent speed measurement (Hall/encoder) |
| 6 | Door lock only via SW command | Add independent HW interlock with speed sensor |
