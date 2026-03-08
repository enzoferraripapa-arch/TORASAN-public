# Git Worktree & Branch Management Best Practices

Reference guide for worktree lifecycle, branch strategy, housekeeping, and multi-worktree workflows.

---

## 1. Git Worktree Lifecycle

### When to Create vs Reuse

| Scenario | Action | Rationale |
|----------|--------|-----------|
| Parallel feature work | Create new worktree | Isolated working directory per branch |
| Quick hotfix while mid-feature | Create temporary worktree | Avoids stash/unstash overhead |
| Long-lived release branch | Keep dedicated worktree | Frequent access justifies persistence |
| One-off code review | Create, then remove after review | Avoid accumulation |
| Same task resumption | Reuse existing worktree | `git worktree list` to find it |

**Rule of thumb**: Create when you need branch isolation; remove when the branch merges. Do not accumulate idle worktrees.

### Creating Worktrees

```bash
# New branch from current HEAD
git worktree add ../feature-login -b feature/login

# Existing remote branch
git worktree add ../hotfix-auth origin/hotfix/auth-bypass

# Detached HEAD (for read-only inspection)
git worktree add --detach ../inspect-v2.0 v2.0.0

# Create and lock immediately (for removable media / network share)
git worktree add --lock --reason "on USB drive" /mnt/usb/worktree -b feature/portable
```

### Safe Cleanup Procedures

**Always prefer `git worktree remove` over manual `rm`.**

```bash
# Standard removal (fails if uncommitted changes exist)
git worktree remove ../feature-login

# Force removal (discards uncommitted changes)
git worktree remove --force ../feature-login

# If worktree was already deleted manually (rm -rf), clean stale refs
git worktree prune
```

**Cleanup workflow after manual deletion**:

```bash
git worktree prune --dry-run -v   # Preview what will be pruned
git worktree prune                 # Actually prune stale entries
git branch -d feature/login        # Delete the orphaned branch if merged
```

### git worktree prune

| Flag | Effect |
|------|--------|
| `--dry-run` / `-n` | Show what would be pruned without acting |
| `--verbose` / `-v` | Report each removal |
| `--expire <time>` | Only prune entries older than `<time>` (e.g., `2.weeks`) |

`git gc` automatically runs `git worktree prune --expire 3.months.ago` internally.

### Common Pitfalls

| Pitfall | Symptom | Fix |
|---------|---------|-----|
| Manual `rm -rf` of worktree dir | `git worktree list` shows stale entry | `git worktree prune` |
| Locked worktree blocks removal | "cannot remove: worktree is locked" | `git worktree unlock <path>` then remove; or `--force --force` |
| Moved worktree directory | Broken `.git` file pointer | `git worktree repair <new-path>` |
| Moved main repo | All linked worktrees broken | `git worktree repair` from new main location |
| Branch already checked out | "fatal: '<branch>' is already checked out at..." | Use `-b new-branch` or `--detach` |

### Lock / Unlock

```bash
# Lock to prevent auto-pruning (e.g., network drive)
git worktree lock --reason "NFS mount, not always available" ../remote-worktree

# Unlock when no longer needed
git worktree unlock ../remote-worktree

# Force-remove a locked worktree (double --force)
git worktree remove --force --force ../locked-worktree
```

---

## 2. Branch Management Strategy

### Naming Conventions

**Format**: `<type>/<ticket-id>-<short-description>`

| Prefix | Use Case | Example |
|--------|----------|---------|
| `feature/` | New functionality | `feature/PROJ-42-user-auth` |
| `bugfix/` | Non-critical bug fix | `bugfix/PROJ-87-null-check` |
| `hotfix/` | Critical production fix | `hotfix/PROJ-99-memory-leak` |
| `release/` | Release preparation | `release/v2.1.0` |
| `chore/` | Maintenance / tooling | `chore/update-ci-pipeline` |
| `refactor/` | Code restructuring | `refactor/extract-parser` |
| `test/` | Test-only changes | `test/add-integration-suite` |

**Rules**:
- Lowercase only, hyphens as separators (no underscores, no camelCase)
- 3-5 words maximum after the prefix
- Include ticket ID when using issue trackers
- Avoid special characters: `~`, `^`, `:`, `?`, `*`, `[`, `\`, spaces

### Merge vs Rebase Decision Matrix

| Condition | Use Rebase | Use Merge |
|-----------|-----------|-----------|
| Local/private branch, not pushed | Yes | -- |
| Shared branch, others have pulled | No | Yes |
| Updating feature branch from main | Yes (`git rebase main`) | OK (`git merge main`) |
| Integrating feature into main | Squash-merge or merge | Yes |
| Preserving full branch history matters | No | Yes |
| Clean linear history preferred | Yes | No |

**The golden rule**: Never rebase commits that have been pushed to a shared branch and pulled by others.

**Recommended hybrid workflow**:

```bash
# Keep feature branch updated (rebase onto main)
git checkout feature/my-work
git rebase main

# Integrate into main (merge commit preserves branch boundary)
git checkout main
git merge --no-ff feature/my-work
```

### Safe Branch Deletion

```bash
# Safe delete (refuses if not fully merged into upstream or HEAD)
git branch -d feature/old-work

# Force delete (use only when you are certain)
git branch -D feature/abandoned-experiment

# Delete remote branch
git push origin --delete feature/old-work

# Delete remote tracking refs for branches deleted on remote
git fetch --prune
# or
git remote prune origin
```

**Always use `-d` first.** Only escalate to `-D` when you deliberately want to discard unmerged work.

### Detecting Stale / Orphaned Branches

```bash
# Branches fully merged into current branch
git branch --merged

# Branches NOT merged into current branch
git branch --no-merged

# Remote branches merged into main
git branch -r --merged main

# Local branches whose upstream is gone (deleted on remote)
git branch -vv | grep ': gone]'

# Automated cleanup of branches with deleted upstream
git fetch -p && git branch -vv | grep ': gone]' | awk '{print $1}' | xargs -r git branch -d
```

**Stale branch audit script**:

```bash
# Show unmerged branches with last commit date
for branch in $(git branch --no-merged main --format='%(refname:short)'); do
  echo "$(git log -1 --format='%ci' "$branch") $branch"
done | sort
```

---

## 3. Git Housekeeping

### git gc Overview

`git gc` is the parent command that orchestrates:

| Sub-command | Purpose |
|-------------|---------|
| `git repack` | Compress objects into pack files |
| `git prune` | Remove unreachable loose objects |
| `git pack-refs` | Pack loose refs into single file |
| `git reflog expire` | Prune old reflog entries |
| `git rerere gc` | Clean rerere metadata |
| `git worktree prune` | Remove stale worktree entries |
| `git commit-graph` | Update commit-graph (if `gc.writeCommitGraph` true) |

### Key Configuration Defaults

| Variable | Default | Description |
|----------|---------|-------------|
| `gc.auto` | 6700 | Loose object count triggering auto-gc |
| `gc.autoPackLimit` | 50 | Pack count triggering consolidation |
| `gc.pruneExpire` | `2 weeks ago` | Grace period before pruning loose objects |
| `gc.reflogExpire` | 90 days | Reflog retention for reachable entries |
| `gc.reflogExpireUnreachable` | 30 days | Reflog retention for unreachable entries |
| `gc.packRefs` | true | Run `git pack-refs` during gc |
| `gc.aggressiveWindow` | 250 | Delta window size for `--aggressive` |
| `gc.aggressiveDepth` | 50 | Delta depth for `--aggressive` |

### Routine Maintenance

```bash
# Normal gc (safe, runs automatically after many git operations)
git gc

# Auto mode (only acts if thresholds exceeded, lightweight)
git gc --auto

# Aggressive mode (slow, use after major history rewrite or large import)
git gc --aggressive --prune=now

# Modern alternative: scheduled maintenance tasks
git maintenance start   # Registers repo for background maintenance
git maintenance run     # Manual one-time execution
```

### Reducing Repo Size After Removing Large Files

**Recommended tool**: `git filter-repo` (officially recommended replacement for `git-filter-branch`).

```bash
# Install
pip install git-filter-repo

# Remove files larger than 50MB from entire history
git filter-repo --strip-blobs-bigger-than 50M

# Remove specific file from all history
git filter-repo --invert-paths --path path/to/large-file.bin

# Remove by glob pattern
git filter-repo --invert-paths --path-glob '*.zip'
```

**Alternative**: BFG Repo-Cleaner (faster for simple cases, 10-720x faster than filter-branch):

```bash
java -jar bfg.jar --strip-blobs-bigger-than 100M repo.git
```

**Post-cleanup** (required after either tool):

```bash
git reflog expire --expire=now --all
git gc --prune=now --aggressive
```

**Warning**: Both tools rewrite history (changing commit hashes). Coordinate with all collaborators before running on shared repos. Force-push will be required.

### Reflog Cleanup

```bash
# View reflog
git reflog

# Expire entries older than 30 days (reachable) / 7 days (unreachable)
git reflog expire --expire=30.days --expire-unreachable=7.days --all

# Nuclear option: expire everything (only after confirming no recovery needed)
git reflog expire --expire=now --all
git gc --prune=now
```

### Finding Large Objects

```bash
# List top 20 largest objects in repo history
git rev-list --objects --all \
  | git cat-file --batch-check='%(objecttype) %(objectname) %(objectsize) %(rest)' \
  | sed -n 's/^blob //p' \
  | sort -rnk2 \
  | head -20
```

---

## 4. Multi-Worktree Workflow

### Architecture

All worktrees share:
- `.git/objects` (object store) -- worktrees are lightweight
- `.git/refs` (most refs)
- `.git/config` (repository config)

Each worktree has its own:
- `HEAD`
- Index (staging area)
- Working directory
- `refs/bisect/*`, `refs/worktree/*`, `refs/rewritten/*`

### Sharing Hooks Across Worktrees

By default, hooks live in `.git/hooks/` of the main repo. Linked worktrees may not automatically find them.

**Solution**: Use `core.hooksPath` with an absolute path.

```bash
# Set shared hooks directory (use absolute path)
git config core.hooksPath /absolute/path/to/repo/.githooks

# Or commit hooks into the repo and configure
git config core.hooksPath .githooks
```

**Caveat**: In a worktree, `.git` is a file (not a directory), so scripts that assume `.git/hooks/` is a directory will fail. Always use `git rev-parse --git-common-dir` to locate shared resources:

```bash
HOOKS_DIR="$(git rev-parse --git-common-dir)/hooks"
```

### Config Inheritance

| Config Layer | Scope | File Location |
|-------------|-------|---------------|
| System | All users | `/etc/gitconfig` |
| Global | Current user | `~/.gitconfig` |
| Repository | All worktrees | `.git/config` |
| Worktree-specific | Single worktree | `.git/worktrees/<id>/config.worktree` |

**Enable per-worktree config**:

```bash
git config extensions.worktreeConfig true

# Set a worktree-specific value
git config --worktree core.sparseCheckout true
git config --worktree user.email "worktree-specific@example.com"
```

**Variables that should NOT be shared across worktrees**:
- `core.worktree` (always worktree-specific)
- `core.bare` (if set to true)
- `core.sparseCheckout` (unless identical for all worktrees)

### Same-Branch Restriction

**Git prevents the same branch from being checked out in multiple worktrees simultaneously.**

```
fatal: 'main' is already checked out at '/path/to/other/worktree'
```

**Workarounds**:

| Approach | Command | Use Case |
|----------|---------|----------|
| New branch from same commit | `git worktree add -b review-main ../review HEAD` | Need to modify |
| Detached HEAD | `git worktree add --detach ../inspect HEAD` | Read-only inspection |

### Useful Worktree-Aware Commands

```bash
# List all worktrees with branch info
git worktree list

# Porcelain output (machine-parseable)
git worktree list --porcelain

# Find which worktree has a branch checked out
git worktree list | grep "feature/login"

# Access another worktree's refs
git log main-worktree/HEAD          # Main worktree's HEAD
git log worktrees/foo/HEAD          # Linked worktree "foo"

# Repair after moving worktrees
git worktree repair ../moved-wt1 ../moved-wt2

# Configure auto-detection of remote tracking branches
git config worktree.guessRemote true
```

### Recommended Directory Layout

**Option A**: Sibling directories (most common)

```
project/              # Main worktree (main branch)
project-feature-a/    # Worktree for feature/a
project-hotfix-x/     # Worktree for hotfix/x
```

**Option B**: Nested under main repo (add to .gitignore)

```
project/
  .worktrees/         # Add to .gitignore
    feature-a/
    hotfix-x/
```

**Option C**: Bare repo as hub (advanced, used by some CI systems)

```
project.git/          # Bare repository (no working tree)
project-main/         # Worktree for main
project-feature-a/    # Worktree for feature/a
```

---

## 5. Quick Reference Commands

| Task | Command |
|------|---------|
| Create worktree | `git worktree add <path> [-b <branch>]` |
| List worktrees | `git worktree list` |
| Remove worktree | `git worktree remove <path>` |
| Prune stale entries | `git worktree prune --dry-run -v` |
| Lock worktree | `git worktree lock --reason "<why>" <path>` |
| Repair connections | `git worktree repair [<paths>]` |
| Merged branches | `git branch --merged [<target>]` |
| Unmerged branches | `git branch --no-merged [<target>]` |
| Stale remote refs | `git fetch --prune` |
| Orphaned locals | `git branch -vv \| grep ': gone]'` |
| Run gc | `git gc --auto` |
| Aggressive gc | `git gc --aggressive --prune=now` |
| Expire reflog | `git reflog expire --expire=now --all` |
| Background maintenance | `git maintenance start` |
| Find large blobs | `git rev-list --objects --all \| git cat-file --batch-check=...` |

---

*Compiled 2026-03-02 from Git official documentation and current community best practices.*
*Sources: git-scm.com/docs, Atlassian Git Tutorials, Graphite, GitKraken, DevToolbox.*
