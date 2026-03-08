# TORASAN

**T**oolkit for **O**rganized **R**isk **A**nalysis and **S**afety **A**ssurance **N**avigation

A process framework and knowledge toolkit for AI-assisted functional safety development.
Built on ISO 26262, IEC 61508, IEC 60730, Automotive SPICE, and MISRA-C:2012.

> **Vision**: To preserve and democratize functional safety engineering knowledge
> before it is lost to the global labor shortage crisis.

---

## What is this?

TORASAN is a collection of:

- **41 Skills** (`.claude/skills/`) -- Structured prompts for Claude Code that encode domain expertise in functional safety, FMEA, HARA, SRS generation, test design, SPICE assessment, and more
- **22 Knowledge modules** (`.claude/knowledge/`) -- Reference material covering ISO 26262, IEC 60730, MISRA-C, Automotive SPICE, safety diagnostics, and BLDC motor control
- **Process definition** (`PROCESS.md`) -- A complete V-model process mapped to Automotive SPICE process areas (MAN.3, SYS.1-5, SWE.1-6, SUP.1/8/9/10)
- **Working example** -- A full BLDC motor controller (IEC 60730 Class B) with safety requirements, architecture, source code, unit tests, FMEA, traceability, and SPICE records

This is not a product. This is a process toolkit. You bring the product; TORASAN provides the safety engineering structure.

## Who is this for?

- Embedded safety engineers dealing with ISO 26262 / IEC 61508 / IEC 60730
- Engineers at small/mid-size suppliers where "the safety person" is one person
- Anyone exploring AI-assisted development for safety-critical systems
- Automotive SPICE practitioners who want structured process records

## Quick Start

1. Clone this repo
2. Install [Claude Code](https://docs.anthropic.com/en/docs/claude-code)
3. Copy shared skills to your global config:
   ```bash
   ./install.sh
   ```
4. Open your project, run skills:
   ```
   /session start
   /execute-phase PH-01
   ```

## Repository Structure

```
TORASAN/
├── .claude/skills/       41 skills (14 general + 27 domain)
├── .claude/knowledge/    22 knowledge modules
├── PROCESS.md            V-model process definition (ISO 26262 + SPICE)
├── project.json          Project configuration (Single Source of Truth)
├── docs/                 Safety plan, HARA, FSC, SRS, test specs, ...
├── src/                  BLDC motor controller (C99, MISRA-C compliant)
├── test/                 Unit / Integration / Qualification tests (Unity)
├── process_records/      SPICE process evidence (18 processes)
├── scripts/              Automation (doc gen, SPICE improvement, WSL bridge)
├── templates/            Project templates
├── platforms/            MCU platform definitions
└── install.sh            Deploy shared skills to ~/.claude/
```

## Skills Overview

| Category | Count | Examples |
|----------|-------|---------|
| Safety Analysis | 7 | safety-concept, fmea, safety-diag, safety-verify, srs-generate |
| Development | 6 | sw-design, system-design, driver-gen, mcu-config, motor-control |
| Testing | 4 | test-design, test-coverage, systest-design, static-analysis |
| Process | 6 | execute-phase, assess-spice, trace, validate, update-record |
| Management | 8 | session, health-check, skill-manage, skill-evolve, repo-manage |
| Documentation | 4 | generate-docs, md2pptx, ingest, manage-tbd |
| Platform | 6 | platform-info, env-check, config-audit, worktree-cleanup |

## The Working Example

The included BLDC motor controller demonstrates a complete IEC 60730 Class B development:

- 15-phase V-model execution (PH-01 through PH-15)
- Safety goals, HARA, FMEA, fault tree analysis
- Software safety requirements (SR-001 to SR-018)
- Architecture with safety monitoring, failsafe, diagnostic manager
- Unit + integration + qualification tests
- Traceability matrix
- SPICE Level 2 achieved on 14/16 processes

## AI-Assisted Development

This entire framework was developed with Claude Code (Anthropic).
That is the point. Safety engineering knowledge can be encoded as AI skills
and reused across projects.

Every skill follows a consistent structure:
- Prerequisites and error handling
- Step-by-step execution procedure
- Output format specification
- Cross-references to project.json and process records

## License

GPLv3. See [LICENSE](LICENSE).

Take it. Use it. Improve it. Share it back.

## Disclaimer

See [DISCLAIMER.md](DISCLAIMER.md). TORASAN does not certify products.
All safety decisions are yours.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

Good first issues:
- Add IEC 62304 (medical device) safety skills
- Add EN ISO 13849-1 (machinery) safety patterns
- Translate README to German / Korean / Chinese
- Add STM32 HAL platform definition
- Improve FMEA templates for your domain

## About

Built by a Japanese embedded systems engineer with functional safety experience.
Not a corporation. Not a startup. Just an engineer who thinks this knowledge
should not be locked behind expensive consulting fees.

If you improve it -- share it back.
