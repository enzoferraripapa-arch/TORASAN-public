# MISRA C:2012 Reference

## 1. Rule Classification Summary

| Category | Count | Compliance |
|----------|-------|-----------|
| Mandatory | 10 rules | Must comply, no deviation permitted |
| Required | 133 rules + 17 directives | Must comply, formal deviation process |
| Advisory | 32 rules | Should comply, deviation optional |

## 2. Mandatory Rules (Must Never Violate)

| Rule | Description |
|------|-------------|
| 1.3 | No undefined or critical unspecified behavior |
| 9.1 | Auto variables shall be initialized before use |
| 12.5 | sizeof operator shall not have side effects |
| 13.1 | Initializer lists shall not contain persistent side effects |
| 13.2 | Evaluation order shall not affect value of expression |
| 13.6 | sizeof operand shall not contain side effects |
| 17.3 | Function shall not be declared implicitly |
| 17.4 | Function return type shall not be void if caller uses it |
| 17.6 | Array parameter shall not be used for sizeof |
| 21.13 | ctype.h functions: argument shall be unsigned char or EOF |

## 3. Top 10 Most Violated Required Rules

| Rule | Description | Common Violation |
|------|-------------|-----------------|
| 8.4 | Compatible declaration shall be visible | Missing extern declaration in header |
| 10.3 | No implicit narrowing conversion | int assigned to uint8_t without cast |
| 10.4 | Operands of same essential type | Mixing signed/unsigned in arithmetic |
| 11.3 | No cast between pointer and integer type | Register access via hardcoded address |
| 14.4 | Controlling expression shall be boolean | `if (x)` where x is int |
| 15.7 | Switch shall have default clause | Missing default in switch |
| 17.7 | Return value of non-void function shall be used | Ignoring return of library calls |
| 18.4 | Pointer arithmetic only +/- with integer | Complex pointer operations |
| 20.1 | #include preceded only by preprocessor directives | Code before #include |
| 21.3 | No stdlib memory allocation (malloc/free) | Dynamic allocation in safety code |

## 4. Key Directives

| Directive | Description | Impact |
|-----------|-------------|--------|
| Dir 1.1 | Implementation-defined behavior shall be documented | Compiler/MCU specific |
| Dir 2.1 | All source code shall be traceable to design | Traceability matrix |
| Dir 4.1 | Run-time failures shall be minimized | Defensive programming |
| Dir 4.3 | Assembly language shall be encapsulated/documented | Isolate asm in dedicated functions |
| Dir 4.6 | typedefs that indicate size and signedness shall be used | Use stdint.h types |
| Dir 4.7 | Error information shall be checked immediately | Check return values |
| Dir 4.8 | Pointer to struct should be opaque if possible | Information hiding |
| Dir 4.9 | Function should be used instead of function-like macro | Prefer inline functions |
| Dir 4.12 | Dynamic memory allocation shall not be used | No malloc/free in safety code |

## 5. Deviation Permit Template

```
Deviation ID:    DEV-{project}-{number}
Rule:            Rule {X.Y} / Directive {X.Y}
Category:        Required / Advisory
Location:        {file}:{line} or module-wide
Description:     {What the deviation is}
Justification:   {Why the rule cannot be followed}
Risk Assessment: {Impact on safety, what mitigates the risk}
Mitigation:      {Alternative measures: review, test, static analysis}
Approved By:     {Name, Role}
Date:            {YYYY-MM-DD}
Review Date:     {next review date}
```

## 6. cppcheck MISRA Configuration

### Command Line
```bash
cppcheck --addon=misra.json --enable=all --inconclusive \
  --suppress=missingIncludeSystem \
  --template="{file}:{line}: [{severity}] {id}: {message}" \
  --output-file=misra_report.txt \
  src/
```

### misra.json addon config
```json
{
  "script": "misra",
  "args": [
    "--rule-texts-file=misra_rules.txt",
    "--suppress-rules=1.1,1.2"
  ]
}
```

### Suppression File (misra_suppressions.txt)
```
# Format: rule_number:file:line
# Only for formally deviated rules
11.3:src/hal/reg_access.h:*
```

### Integration with Build
```makefile
misra-check:
	cppcheck --addon=misra.json --error-exitcode=1 src/
	@echo "MISRA check passed"
```

## 7. Amendment 1 (2016) Key Additions

| Rule | Description |
|------|-------------|
| 2.6 | Function should not contain unreachable code |
| 2.7 | No unused parameters in function |
| 8.2 | Function types shall be explicitly declared |
| 21.13 | (Promoted to Mandatory) ctype.h argument safety |
| 21.14-21.20 | Additional stdlib restrictions |
| 22.1-22.6 | Resource management rules (files, memory) |

## 8. Amendment 2 (2020) Key Additions

| Rule | Description |
|------|-------------|
| 1.4 | Emergent language features shall not be used |
| 1.5 | Implementation-defined suffixes on integer constants |
| 7.5 | Array elements shall not overlap |
| 21.21 | System() function shall not be used |
| 21.22-21.24 | Additional tgmath.h and signal restrictions |

## 9. Coding Style Quick Rules

| Topic | Rule |
|-------|------|
| Types | Use stdint.h: uint8_t, int16_t, uint32_t, etc. |
| Booleans | Use stdbool.h: bool, true, false |
| Constants | Use enum or const, not #define for values |
| NULL | Use NULL macro, not 0, for null pointer |
| Casts | Minimize; document with comment when required |
| Goto | Prohibited (Rule 15.1) |
| Recursion | Prohibited (Rule 17.2) |
| Dynamic memory | Prohibited (Dir 4.12, Rule 21.3) |
| Variable-length arrays | Prohibited (Rule 18.8) |

## 10. Common Pitfalls

| # | Pitfall | Fix |
|---|---------|-----|
| 1 | Using int instead of fixed-width types | Always use stdint.h types (uint8_t, etc.) |
| 2 | Implicit boolean conversion | Use explicit comparison: `if (x != 0)` |
| 3 | Missing return value check | Check every non-void function return |
| 4 | Deviation without formal record | Always create deviation permit before suppressing |
| 5 | Running cppcheck without MISRA addon | Always include --addon=misra.json |
