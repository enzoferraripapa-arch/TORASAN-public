# TORASAN -- Functional Safety Framework

A process framework and knowledge toolkit for AI-assisted functional safety development.

## Repository Structure

| Path | Purpose |
|------|---------|
| `.claude/skills/` | All skills (14 general + 27 domain) |
| `.claude/knowledge/` | Knowledge modules (8 general + 14 domain) |
| `PROCESS.md` | ISO 26262 + SPICE process definition |
| `project.json` | Project configuration (Single Source of Truth) |
| `docs/` | Safety templates and design documents |
| `src/` | BLDC motor controller example (C99) |
| `test/` | Unit / Integration / Qualification tests |
| `process_records/` | SPICE process evidence |
| `scripts/` | Automation scripts |
| `install.sh` | Deploy shared skills to ~/.claude/ |

## Getting Started

1. Run `./install.sh` to copy shared skills to `~/.claude/`
2. Use `/session start` to begin a new project session
3. Use `/execute-phase PH-01` to start the V-model process

## Key Commands

| Command | Action |
|---------|--------|
| `/session start` | Initialize project session |
| `/execute-phase PH-XX` | Execute V-model phase |
| `/assess-spice` | Run SPICE assessment |
| `/fmea` | Perform FMEA analysis |
| `/safety-concept` | Develop safety concept |
| `/test-design` | Design test specifications |
| `/trace` | Generate traceability matrix |
| `/generate-docs` | Generate documentation |
| `/health-check` | Check project health |
| `/static-analysis` | Run static analysis |
