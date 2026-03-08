# Contributing to TORASAN

## How to Contribute

### Adding a New Skill

1. Create `.claude/skills/<skill-name>/SKILL.md`
2. Follow the existing skill format (see any skill for reference)
3. Include: Prerequisites, Error Handling, Execution Steps, Output Format
4. Submit a PR with a description of what the skill does and which standard it relates to

### Adding Knowledge

1. Create `.claude/knowledge/<topic>.md`
2. Do NOT copy standard text verbatim (copyright)
3. Focus on implementation patterns and practical guidance
4. Reference specific standard clauses but use your own words

### Improving Existing Content

- Fix errors in process records or skill definitions
- Add missing error handling cases
- Improve cross-references between skills

## Quality Criteria

- Skills must be grounded in published standards
- No proprietary content from any organization
- English required; Japanese and other languages welcome as additions
- Test your skills with Claude Code before submitting

## Language Policy

- English is the primary language
- Japanese documentation is welcome alongside English
- Other languages: PRs welcome for README translations

## PR Process

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a PR with clear description
5. Respond to review comments
