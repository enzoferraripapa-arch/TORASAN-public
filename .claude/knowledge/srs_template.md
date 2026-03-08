# SRS Template (IEEE 29148 + EARS)

## 1. IEEE 29148 SRS Structure

| Section | Content |
|---------|---------|
| 1. Introduction | 1.1 Purpose, 1.2 Scope, 1.3 Definitions, 1.4 References |
| 2. System Overview | Context diagram, system boundaries, interfaces |
| 3. System Context | Operating environment, user classes, constraints |
| 4. Functional Requirements | Organized by feature/mode/state |
| 5. Safety Requirements | ASIL-tagged, traced to safety goals |
| 6. Performance Requirements | Timing, throughput, resource budgets |
| 7. Interface Requirements | HW/SW/comm interfaces |
| 8. Verification Requirements | Test criteria per requirement |

## 2. EARS Requirement Patterns

### Pattern Templates

| Pattern | Template | Use When |
|---------|----------|----------|
| Ubiquitous | The [system] shall [action]. | Always true, no condition |
| Event-Driven | When [trigger], the [system] shall [action]. | Response to event |
| State-Driven | While [state], the [system] shall [action]. | Behavior in a state |
| Unwanted | If [condition], then the [system] shall [action]. | Error handling/safety |
| Optional | Where [feature], the [system] shall [action]. | Configurable features |
| Complex | While [state], when [trigger], the [system] shall [action]. | Combined conditions |

### Motor Control Examples

**Ubiquitous:**
> REQ-F-001: The motor controller shall limit phase current to 15A peak.

**Event-Driven:**
> REQ-S-010: When overcurrent is detected (Iphase > 15A), the motor controller shall disable all PWM outputs within 10 microseconds.

**State-Driven:**
> REQ-F-020: While in SPIN mode, the motor controller shall maintain drum speed within +/-5% of the target RPM.

**Unwanted (Safety):**
> REQ-S-030: If communication with the main MCU is lost for >100ms, the safety MCU shall activate Safe Torque Off.

**Optional:**
> REQ-F-040: Where vibration sensor is installed, the motor controller shall reduce spin speed when vibration exceeds 5g.

**Complex:**
> REQ-S-050: While in SPIN mode, when drum speed exceeds 1200 RPM, the motor controller shall verify door lock status every 100ms.

## 3. Requirement Categorization

| Category | Prefix | Description |
|----------|--------|-------------|
| Functional | REQ-F-xxx | Normal operating behavior |
| Safety | REQ-S-xxx | ASIL-tagged, traced to safety goals |
| Performance | REQ-P-xxx | Timing, throughput, accuracy |
| Timing | REQ-T-xxx | Deadlines, periods, response times |
| Interface | REQ-I-xxx | HW/SW/communication interfaces |
| Diagnostic | REQ-D-xxx | Self-test, monitoring, fault detection |

## 4. Requirement Template

```
ID:          REQ-{category}-{number}
Title:       {short descriptive name}
Description: {EARS pattern sentence}
Rationale:   {why this requirement exists}
Source:       {SG-xx / stakeholder / standard clause}
ASIL:        {QM / A / B / C / D}
Category:    {Functional / Safety / Performance / Timing / Interface / Diagnostic}
Priority:    {Must / Should / May}
Verification: {Test / Analysis / Inspection / Review}
Trace Up:    {parent requirement or safety goal ID}
Trace Down:  {design element or test case ID}
Status:      {Draft / Reviewed / Approved / Implemented / Verified}
```

## 5. Quality Attributes Checklist

| Attribute | Check | Bad Example | Good Example |
|-----------|-------|-------------|--------------|
| Atomic | One requirement = one testable statement | "shall monitor and control" | Split into REQ-D-001 (monitor) and REQ-F-002 (control) |
| Unambiguous | Single interpretation | "fast response" | "within 10 microseconds" |
| Measurable | Quantified criteria | "sufficient accuracy" | "+/-0.5% of full scale" |
| Traceable | Has source and target | No ID or trace | REQ-S-010 -> SG1, -> TC-S-010 |
| Consistent | No conflicts with others | REQ-001 says 10ms, REQ-002 says 50ms for same event | Aligned timing |
| Complete | Covers all states/modes | Only normal mode | Normal + fault + startup + shutdown |
| Feasible | Implementable on target HW | "zero latency" | "<1 microsecond latency" |
| Necessary | Traces to a need | Feature creep | Traced to safety goal or stakeholder |

## 6. Verification Criteria Template

```
Verification Method: {Test / Analysis / Inspection}
Pass Criteria:       {measurable condition}
Test Environment:    {target HW / simulator / model}
Input Conditions:    {stimulus description}
Expected Output:     {specific measurable response}
Coverage:            {statement / branch / MC/DC as required by ASIL}
```

## 7. Common Pitfalls

| # | Pitfall | Fix |
|---|---------|-----|
| 1 | Compound requirements (and/or) | Split into atomic requirements |
| 2 | Missing "shall" keyword | Every requirement must use "shall" |
| 3 | No verification criteria | Add pass/fail criteria at creation time |
| 4 | Vague timing ("quickly") | Specify exact time with units |
| 5 | Missing negative/fault requirements | Add "Unwanted" EARS patterns for every hazard |
| 6 | No ASIL tag on safety requirements | Tag every safety req with ASIL level |
