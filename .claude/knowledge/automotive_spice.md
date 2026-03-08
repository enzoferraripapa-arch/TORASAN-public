# Automotive SPICE Assessment Reference

## 1. PA Rating Scale

| Rating | Range | Meaning |
|--------|-------|---------|
| N (Not achieved) | 0-15% | Little/no evidence of achievement |
| P (Partially) | 16-50% | Some evidence, significant weakness |
| L (Largely) | 51-85% | Systematic approach, some weakness |
| F (Fully) | 86-100% | Complete, systematic, no significant weakness |

## 2. Capability Level Achievement

| Level | Criteria | Meaning |
|-------|----------|---------|
| Level 0 | PA 1.1 < L | Incomplete process |
| Level 1 | PA 1.1 >= L | Performed process |
| Level 2 | PA 1.1 = F AND PA 2.1 >= L AND PA 2.2 >= L | Managed process |
| Level 3 | Level 2 + PA 3.1 = F AND PA 3.2 >= L | Established process |

## 3. PA 2.1 Performance Management (GP Evidence)

| GP | Practice | Required Evidence |
|----|----------|-------------------|
| GP 2.1.1 | Identify objectives | Documented process objectives per project |
| GP 2.1.2 | Plan the process | Process plan with schedule, resources, responsibilities |
| GP 2.1.3 | Monitor the process | Progress tracking records, status reports |
| GP 2.1.4 | Adjust the process | Change records, corrective actions taken |
| GP 2.1.5 | Define responsibilities | RACI or role assignments documented |
| GP 2.1.6 | Identify resources | Resource allocation records |
| GP 2.1.7 | Manage interfaces | Interface agreements, communication records |

## 4. PA 2.2 Work Product Management (GP Evidence)

| GP | Practice | Required Evidence |
|----|----------|-------------------|
| GP 2.2.1 | Define WP requirements | Templates, naming conventions, content standards |
| GP 2.2.2 | Define WP documentation | Document structure, metadata requirements |
| GP 2.2.3 | Identify, document, control WPs | CM plan, version control, baselines |
| GP 2.2.4 | Review WPs | Review records with findings and closure |

## 5. SWE Process Key BPs

### SWE.1 Software Requirements Analysis
| BP | Practice |
|----|----------|
| BP1 | Specify software requirements |
| BP2 | Structure software requirements |
| BP3 | Analyze software requirements |
| BP4 | Analyze impact on operating environment |
| BP5 | Develop verification criteria |
| BP6 | Ensure consistency (bidirectional tracing to system) |

### SWE.2 Software Architectural Design
| BP | Practice |
|----|----------|
| BP1 | Develop SW architecture |
| BP2 | Allocate SW requirements to elements |
| BP3 | Define interfaces between elements |
| BP4 | Describe dynamic behavior |
| BP5 | Define resource consumption objectives |
| BP6 | Evaluate alternative architectures |
| BP7 | Ensure consistency and bidirectional tracing |

### SWE.3 Software Detailed Design
| BP | Practice |
|----|----------|
| BP1 | Develop detailed design |
| BP2 | Define interfaces of SW units |
| BP3 | Describe dynamic behavior |
| BP4 | Evaluate detailed design |
| BP5 | Ensure consistency and bidirectional tracing |

### SWE.4 Software Unit Verification
| BP | Practice |
|----|----------|
| BP1 | Develop unit verification strategy |
| BP2 | Develop unit test specification (from detailed design) |
| BP3 | Test SW units |
| BP4 | Achieve test coverage per strategy |
| BP5 | Ensure consistency and bidirectional tracing |

### SWE.5 Software Integration and Integration Test
| BP | Practice |
|----|----------|
| BP1 | Develop integration strategy |
| BP2 | Develop integration test spec |
| BP3 | Integrate and test SW elements |
| BP4 | Achieve regression/coverage per strategy |
| BP5 | Ensure consistency and bidirectional tracing |

### SWE.6 Software Qualification Test
| BP | Practice |
|----|----------|
| BP1 | Develop qualification test strategy |
| BP2 | Develop qualification test spec (from SW requirements) |
| BP3 | Test integrated software |
| BP4 | Achieve regression/coverage per strategy |
| BP5 | Ensure consistency and bidirectional tracing |

## 6. Common Assessment Gaps

| # | Gap | Fix |
|---|-----|-----|
| 1 | Missing bidirectional traceability | Ensure every requirement traces up (source) and down (design/test) |
| 2 | No review records for WPs | Add review checklists, findings logs, sign-off |
| 3 | Test cases not traced to requirements | Add requirement ID to every test case |
| 4 | No verification criteria in SRS | Add testable acceptance criteria per requirement |
| 5 | Missing process plan or objectives | Create per-project process plan with objectives |
| 6 | No change management evidence | Record all changes with rationale and impact |
| 7 | Inconsistent naming/versioning | Define and enforce naming convention + version scheme |
| 8 | No resource/interface management | Document resource needs and stakeholder interfaces |

## 7. Assessment Preparation Checklist

- [ ] All process records (process_records/) up to date
- [ ] Bidirectional traceability matrix complete
- [ ] Review records exist for all work products
- [ ] Version control history clean and traceable
- [ ] Process plans documented per project
- [ ] Change log maintained with rationale
- [ ] Test evidence archived with pass/fail status
- [ ] Templates defined and consistently used
- [ ] Roles and responsibilities documented
- [ ] Corrective actions tracked to closure
