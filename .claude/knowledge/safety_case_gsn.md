# Safety Case / GSN Reference

## 1. GSN Element Definitions

| Element | Symbol | Purpose | Example |
|---------|--------|---------|---------|
| Goal (G) | Rectangle | Claim to be supported | "System is acceptably safe for ASIL B" |
| Strategy (S) | Parallelogram | Argument approach | "Argument over all identified hazards" |
| Context (Ctx) | Rounded rect | Reference/scope info | "Operating environment: IEC 60730 Class B" |
| Solution (Sn) | Circle | Evidence reference | "FMEDA report showing SPFM=92%" |
| Assumption (A) | Oval + 'A' | Assumed true, not proven | "MCU failure rates per SN 29500" |
| Justification (J) | Oval + 'J' | Why strategy is appropriate | "Class B tests cover all Annex H items" |

**Connectors**: SupportedBy (solid arrow), InContextOf (hollow arrow)

## 2. Argument Pattern: Hazard Avoidance

```
G1: System is safe (ASIL B / Class B)
  |-- Ctx1: Hazard list (H1-H7)
  |-- S1: Argue over each hazard
       |-- G1.1: H1 (Motor runaway) mitigated
       |    |-- Sn1: Overspeed detection test report
       |    |-- Sn2: Independent HW shutdown evidence
       |-- G1.2: H2 (Overcurrent) mitigated
       |    |-- Sn3: HW current limiter test
       |    |-- Sn4: SW current monitoring test
       |-- G1.3-G1.7: (remaining hazards)
```

## 3. Argument Pattern: Diagnostic Coverage

```
G2: HW random failures adequately controlled
  |-- Ctx2: ASIL B targets (SPFM>=90%, LFM>=60%)
  |-- S2: Argue by HW metric analysis
       |-- G2.1: SPFM >= 90% achieved
       |    |-- Sn5: FMEDA spreadsheet
       |    |-- A1: Failure rate data source (SN 29500)
       |-- G2.2: LFM >= 60% achieved
       |    |-- Sn6: FMEDA spreadsheet (latent faults)
       |-- G2.3: PMHF < 1E-7/h
            |-- Sn7: PMHF calculation report
```

## 4. Argument Pattern: Process Compliance

```
G3: Development process is adequate
  |-- Ctx3: ISO 26262 + Automotive SPICE Level 2
  |-- S3: Argue by process area
       |-- G3.1: SWE.1 (Requirements) at Level 2
       |    |-- Sn8: SPICE assessment report
       |    |-- Sn9: Traceability matrix
       |-- G3.2: SWE.4 (Unit verification) at Level 2
       |    |-- Sn10: Test coverage report
       |-- G3.3: SUP.8 (Configuration mgmt) at Level 2
            |-- Sn11: CM audit report
```

## 5. IEC 60730 Class B Safety Case Template

```
G-TOP: Washing machine control SW achieves IEC 60730 Class B
  |-- Ctx: IEC 60730 Annex H requirements
  |-- S-TOP: Argue completeness of Class B self-tests
       |
       |-- G-CPU: CPU self-test adequate (H.2.16.5)
       |    |-- Sn: CPU test report, stuck-at fault coverage
       |
       |-- G-RAM: RAM self-test adequate (H.2.19.6)
       |    |-- Sn: March C- test report, coverage proof
       |
       |-- G-FLASH: Flash integrity adequate (H.2.19.8)
       |    |-- Sn: CRC-32 verification report
       |
       |-- G-CLK: Clock monitoring adequate (H.2.18.10)
       |    |-- Sn: Cross-clock monitoring test report
       |
       |-- G-WDT: Watchdog adequate (H.2.14)
       |    |-- Sn: WDT temporal + logical test report
       |
       |-- G-INT: Interrupt handling adequate (H.2.16.7)
       |    |-- Sn: Interrupt timing/sequence test report
       |
       |-- G-IO: I/O integrity adequate (H.2.16.12)
            |-- Sn: I/O plausibility test report
```

## 6. What Assessors Look For

| # | Item | Evidence Required |
|---|------|-------------------|
| 1 | Complete hazard coverage | Every hazard has >=1 goal with supporting evidence |
| 2 | No unsupported claims | Every goal is either decomposed or has a solution |
| 3 | Assumptions are justified | Each assumption has rationale and is trackable |
| 4 | Evidence is current | All solutions reference versioned, reviewed documents |
| 5 | Independence of arguments | Safety mechanisms argued independently from function |

## 7. Safety Case Review Checklist

- [ ] Top-level goal clearly stated with scope
- [ ] All hazards from hazard analysis covered
- [ ] Context elements reference applicable standards
- [ ] Every leaf goal has at least one solution (evidence)
- [ ] No undeveloped goals remain (or explicitly marked)
- [ ] Assumptions are documented and reasonable
- [ ] Traceability from hazard -> safety goal -> requirement -> test
- [ ] Evidence documents are version-controlled and reviewed
- [ ] Argument is free of circular reasoning
- [ ] Independence between primary function and safety mechanism argued

## 8. Common Pitfalls

| # | Pitfall | Fix |
|---|---------|-----|
| 1 | Confusing confidence and safety arguments | Separate "system is safe" from "we trust our evidence" |
| 2 | Undeveloped goals left unmarked | Use diamond symbol for undeveloped; track as open item |
| 3 | Missing context on top goal | Always state operating conditions, standards, scope |
| 4 | Evidence without version/date | Every Sn must reference document ID, version, date |
