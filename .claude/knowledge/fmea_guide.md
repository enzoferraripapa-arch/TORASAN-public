# FMEA / FMEDA Guide (AIAG/VDA)

## 1. AIAG/VDA 7-Step FMEA Approach

| Step | Name | Key Activity |
|------|------|-------------|
| 1 | Planning & Preparation | Define scope, team, timing, tools |
| 2 | Structure Analysis | System/function tree, block diagram |
| 3 | Function Analysis | Function tree with requirements |
| 4 | Failure Analysis | Failure chain: Effect <- Mode <- Cause |
| 5 | Risk Analysis | Rate S/O/D, determine Action Priority |
| 6 | Optimization | Define and assign actions |
| 7 | Results Documentation | Document results and track actions |

## 2. Severity Scale (Motor Control Context)

| S | Effect | Motor Control Example |
|---|--------|----------------------|
| 10 | Hazardous without warning | Motor runaway at max speed, no detection |
| 9 | Hazardous with warning | Motor runaway detected but cannot stop |
| 8 | Very high | Drum lock during spin, smoke from motor |
| 7 | High | Overheating, water leak from failed pump |
| 6 | Moderate | Incorrect spin speed, excessive vibration |
| 5 | Low | Minor cycle deviation, extended wash time |
| 4 | Very low | Noise increase, cosmetic water marks |
| 3 | Minor | Slight performance degradation |
| 2 | Very minor | Negligible effect, user unaware |
| 1 | None | No discernible effect |

## 3. Occurrence Scale

| O | Probability | Failure Rate | Meaning |
|---|-------------|-------------|---------|
| 10 | Very high | >= 100/1000 | New technology, no history |
| 9 | | 50/1000 | |
| 8 | High | 20/1000 | Frequent failures in similar |
| 7 | | 10/1000 | |
| 6 | Moderate | 2/1000 | Occasional failures in similar |
| 5 | | 0.5/1000 | |
| 4 | | 0.1/1000 | |
| 3 | Low | 0.01/1000 | Isolated failures |
| 2 | Very low | <= 0.001/1000 | Rare, almost identical design |
| 1 | Extremely low | Eliminated | Prevention through proven design |

## 4. Detection Scale

| D | Ability | Method |
|---|---------|--------|
| 10 | Almost impossible | No detection mechanism |
| 9 | Very remote | Indirect/random inspection only |
| 8 | Remote | Visual inspection only |
| 7 | Very low | Manual test/inspection post-assembly |
| 6 | Low | In-process manual measurement |
| 5 | Moderate | SPC-based monitoring |
| 4 | Moderately high | Automated in-process test |
| 3 | High | Multiple automated detections |
| 2 | Very high | Proven detection + error-proofing |
| 1 | Almost certain | Physical prevention (design) |

## 5. Action Priority (AP) - Replaces RPN

| Severity | Occurrence | Detection | AP |
|----------|------------|-----------|-----|
| 9-10 | 4-10 | any | **H** (High) |
| 9-10 | 2-3 | 7-10 | **H** |
| 9-10 | 2-3 | 1-6 | **M** (Medium) |
| 9-10 | 1 | any | **M** |
| 5-8 | 7-10 | any | **H** |
| 5-8 | 4-6 | 7-10 | **H** |
| 5-8 | 4-6 | 1-6 | **M** |
| 5-8 | 2-3 | any | **M** |
| 5-8 | 1 | any | **L** (Low) |
| 2-4 | any | any | **L** |
| 1 | any | any | **L** |

**H** = Must act. **M** = Should act. **L** = May act.

## 6. FMEDA Diagnostic Coverage Calculation

```
DC = lambda_detected / lambda_total

lambda_total = lambda_safe + lambda_detected + lambda_undetected

SPFM = 1 - (lambda_SPF / lambda_total)
LFM  = 1 - (lambda_MPF_latent / lambda_total)
```

Per component: assign failure modes from datasheet, classify each as Safe/Detected/Undetected, sum rates.

## 7. Washing Machine Hazard List Template

| ID | Hazard | Cause | Effect | ASIL/Class | Safety Goal |
|----|--------|-------|--------|------------|-------------|
| H1 | Motor runaway | PWM stuck ON, FET short | Drum overspeed -> structural failure | B | SG1: Prevent overspeed |
| H2 | Overcurrent/fire | FET short, winding fault | Thermal runaway | B | SG2: Current < Imax within 10us |
| H3 | Door open during spin | Lock failure + SW fault | Injury from rotating drum | B | SG3: Interlock integrity |
| H4 | Water overflow | Valve stuck open + sensor fail | Flooding, electrical hazard | B | SG4: Water level bounded |
| H5 | Electric shock | Insulation failure + ground fault | Electrocution | B | SG5: Earth continuity |
| H6 | Motor stall overheating | Rotor lock + no detection | Smoke/fire from winding | B | SG6: Thermal protection |
| H7 | Loss of braking | Brake circuit fail | Drum coasts, door unlock risk | B | SG7: Controlled stop |

## 8. Common Pitfalls

| # | Pitfall | Fix |
|---|---------|-----|
| 1 | Using old RPN instead of AP | Use AIAG/VDA AP table above |
| 2 | Missing failure chain (effect-mode-cause) | Always define complete chain |
| 3 | Assigning D=1 without proven mechanism | D=1 requires physical prevention only |
| 4 | Not updating FMEA after design change | FMEA is living document; update per change |
| 5 | Confusing DFMEA and PFMEA scope | DFMEA=design failure; PFMEA=process failure |
