#!/usr/bin/env python3
"""
wsl-spice-improver.py — WSL マルチエージェント SPICE 改善オーケストレーター

4つの WSL エージェントが並行して process_records を改善し、
5つ目のエージェントがプロセス横断整合性チェックを実施する。

Usage:
    python scripts/wsl-spice-improver.py --dry-run
    python scripts/wsl-spice-improver.py --max-iterations 3
    python scripts/wsl-spice-improver.py --agents swe-improver,sup-improver
"""

import argparse
import atexit
import json
import os
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime, timezone
from pathlib import Path

# ===========================================================
# 定数
# ===========================================================
PROJECT_DIR = Path(__file__).resolve().parent.parent
WSL_AGENT_SH = PROJECT_DIR / "scripts" / "wsl-agent.sh"
PROJECT_JSON = PROJECT_DIR / "project.json"
PROGRESS_LOG = PROJECT_DIR / "process_records" / "spice_improvement_log.json"
WSL_CWD = "/mnt/c/Users/" + os.environ.get("USER", os.environ.get("USERNAME", "user")) + "/Documents/TORASAN"

PA_ORDER = {"N": 0, "P": 1, "L": 2, "F": 3}
PA_LABELS = {0: "N", 1: "P", 2: "L", 3: "F"}

AGENT_TIMEOUT = 900  # 15 minutes per agent
AGENT_MAX_TURNS = 40
AGENT_MODEL = "sonnet"

# エージェント定義: name → 担当プロセス
AGENT_DEFS = {
    "sys-improver": {
        "processes": ["SYS.3", "SYS.4", "SYS.5"],
        "focus": "システム設計・統合・テスト記録の改善",
    },
    "swe-improver": {
        "processes": ["SWE.1", "SWE.2", "SWE.3", "SWE.4", "SWE.5", "SWE.6"],
        "focus": "SW要件〜適格性テスト記録の改善",
    },
    "man-improver": {
        "processes": ["MAN.3"],
        "focus": "プロジェクト管理記録の改善",
    },
    "sup-improver": {
        "processes": ["SUP.1", "SUP.8", "SUP.9", "SUP.10"],
        "focus": "品質保証・構成管理・問題管理記録の改善",
    },
}


# ===========================================================
# ProgressTracker
# ===========================================================
class ProgressTracker:
    """イテレーション毎の PA 値変化を JSON に記録"""

    def __init__(self, log_path: Path):
        self.log_path = log_path
        self.data = self._load()

    def _load(self) -> dict:
        if self.log_path.exists():
            with open(self.log_path, "r", encoding="utf-8") as f:
                return json.load(f)
        return {"iterations": [], "started_at": None, "finished_at": None}

    def save(self):
        try:
            self.log_path.parent.mkdir(parents=True, exist_ok=True)
            with open(self.log_path, "w", encoding="utf-8") as f:
                json.dump(self.data, f, indent=2, ensure_ascii=False)
        except Exception as e:
            print(f"  WARNING: Failed to save progress log: {e}")

    def start_run(self):
        self.data["started_at"] = _now_iso()

    def finish_run(self):
        self.data["finished_at"] = _now_iso()
        self.save()

    def record_iteration(self, iteration: int, before: dict, after: dict,
                         agent_results: dict, cross_check_result: str):
        entry = {
            "iteration": iteration,
            "timestamp": _now_iso(),
            "pa_before": before,
            "pa_after": after,
            "improvements": _calc_improvements(before, after),
            "agent_results": agent_results,
            "cross_check_summary": cross_check_result[:2000] if cross_check_result else None,
        }
        self.data["iterations"].append(entry)
        self.save()


# ===========================================================
# WorktreeManager
# ===========================================================
class WorktreeManager:
    """Git worktree の作成・マージ・削除"""

    def __init__(self, project_dir: Path):
        self.project_dir = project_dir
        self.worktree_base = project_dir / ".claude" / "worktrees"
        # 初期化時にベースディレクトリ作成（race condition 防止）
        self.worktree_base.mkdir(parents=True, exist_ok=True)

    def create(self, agent_name: str) -> Path:
        """worktree を作成し、パスを返す"""
        wt_path = self.worktree_base / agent_name
        branch_name = f"spice/{agent_name}"

        # 既存の worktree + ブランチを確実に削除
        self.cleanup(agent_name)

        # 新しいブランチで worktree 作成
        _git(self.project_dir, "worktree", "add", "-b", branch_name,
             str(wt_path), "HEAD")
        print(f"  [worktree] Created: {wt_path} (branch: {branch_name})")
        return wt_path

    def commit_worktree_changes(self, agent_name: str) -> bool:
        """worktree 内の未コミット変更を自動 commit。変更があれば True を返す"""
        wt_path = self.worktree_base / agent_name
        if not wt_path.exists():
            return False

        # worktree 内の未コミット変更を確認
        status = _git_output(wt_path, "status", "--porcelain")
        if not status.strip():
            print(f"  [worktree] {agent_name}: no uncommitted changes")
            return False

        # process_records/ 配下のみ stage & commit
        try:
            _git(wt_path, "add", "process_records/")
            _git(wt_path, "commit", "-m",
                 f"spice: {agent_name} improvements\n\n"
                 f"Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>")
            print(f"  [worktree] {agent_name}: committed changes in worktree")
            return True
        except subprocess.CalledProcessError:
            print(f"  [worktree] {agent_name}: nothing to commit after staging")
            return False

    def merge(self, agent_name: str) -> tuple[bool, str]:
        """worktree の変更をメインブランチにマージ。(success, message) を返す"""
        branch_name = f"spice/{agent_name}"
        current = _git_output(self.project_dir, "branch", "--show-current").strip()

        # マージ前の diff を確認
        diff = _git_output(self.project_dir, "diff", f"{current}..{branch_name}",
                           "--stat")
        if not diff.strip():
            return True, f"No changes from {agent_name}"

        # process_records 以外の変更がないか確認（品質ゲート）
        changed_files = _git_output(
            self.project_dir, "diff", f"{current}..{branch_name}",
            "--name-only"
        ).strip().split("\n")

        unexpected = [f for f in changed_files
                      if f and not f.startswith("process_records/")]
        if unexpected:
            return False, (f"BLOCKED: {agent_name} modified files outside "
                          f"process_records/: {unexpected}")

        # マージ実行
        try:
            _git(self.project_dir, "merge", branch_name, "--no-ff",
                 "-m", f"spice: merge {agent_name} improvements")
            return True, f"Merged {agent_name}: {diff.strip()}"
        except subprocess.CalledProcessError as e:
            # 競合時はマージ中断（abort 自体の失敗も catch）
            try:
                _git(self.project_dir, "merge", "--abort")
            except subprocess.CalledProcessError as abort_e:
                print(f"  WARNING: merge --abort failed: {abort_e}")
            return False, f"CONFLICT merging {agent_name}: {e}"

    def cleanup(self, agent_name: str):
        """worktree とブランチを削除"""
        wt_path = self.worktree_base / agent_name
        branch_name = f"spice/{agent_name}"

        if wt_path.exists():
            try:
                _git(self.project_dir, "worktree", "remove", str(wt_path),
                     "--force")
            except subprocess.CalledProcessError:
                pass  # already removed

        # ブランチ削除（マージ済みなら -d で十分）
        try:
            _git(self.project_dir, "branch", "-D", branch_name)
        except subprocess.CalledProcessError:
            pass  # already deleted

    def cleanup_all(self):
        """全エージェントの worktree をクリーンアップ"""
        for name in AGENT_DEFS:
            self.cleanup(name)
        # cross-checker は worktree 不使用


# ===========================================================
# AgentRunner
# ===========================================================
class AgentRunner:
    """WSL エージェントの実行"""

    def __init__(self, dry_run: bool = False):
        self.dry_run = dry_run

    def run(self, agent_name: str, prompt: str,
            cwd: str = WSL_CWD, timeout: int = AGENT_TIMEOUT) -> dict:
        """単一エージェントを実行し、結果 dict を返す"""
        if self.dry_run:
            print(f"\n{'='*60}")
            print(f"[DRY-RUN] Agent: {agent_name}")
            print(f"[DRY-RUN] CWD: {cwd}")
            print(f"[DRY-RUN] Timeout: {timeout}s")
            print(f"[DRY-RUN] Prompt ({len(prompt)} chars):")
            print(f"{prompt[:500]}...")
            print(f"{'='*60}\n")
            return {"ok": True, "result": "[DRY-RUN] Skipped", "agent": agent_name}

        print(f"  [agent] Starting {agent_name} (timeout={timeout}s)...")
        start = time.time()

        try:
            result = subprocess.run(
                ["bash", str(WSL_AGENT_SH), prompt,
                 "--timeout", str(timeout),
                 "--cwd", cwd,
                 "--model", AGENT_MODEL,
                 "--max-turns", str(AGENT_MAX_TURNS)],
                capture_output=True, text=True,
                timeout=timeout + 30,  # buffer
                cwd=str(PROJECT_DIR),
            )
            elapsed = time.time() - start

            try:
                data = json.loads(result.stdout.strip())
                data["agent"] = agent_name
                data["elapsed_s"] = round(elapsed, 1)
                print(f"  [agent] {agent_name} finished in {elapsed:.0f}s "
                      f"(ok={data.get('ok')})")
                return data
            except json.JSONDecodeError:
                return {
                    "ok": False, "agent": agent_name,
                    "error": f"JSON parse failed: {result.stdout[:500]}",
                    "stderr": result.stderr[:500],
                    "elapsed_s": round(elapsed, 1),
                }
        except subprocess.TimeoutExpired:
            return {
                "ok": False, "agent": agent_name,
                "error": f"Timeout after {timeout}s",
            }
        except Exception as e:
            return {
                "ok": False, "agent": agent_name,
                "error": str(e),
            }

    def run_parallel(self, tasks: list[tuple[str, str, str]]) -> dict[str, dict]:
        """複数エージェントを並行実行。tasks = [(name, prompt, cwd), ...]"""
        results = {}
        with ThreadPoolExecutor(max_workers=len(tasks)) as executor:
            futures = {
                executor.submit(self.run, name, prompt, cwd): name
                for name, prompt, cwd in tasks
            }
            for future in as_completed(futures):
                name = futures[future]
                try:
                    results[name] = future.result()
                except Exception as e:
                    results[name] = {"ok": False, "agent": name, "error": str(e)}
        return results


# ===========================================================
# SpiceImprover (メインオーケストレーター)
# ===========================================================
class SpiceImprover:
    def __init__(self, args):
        self.max_iterations = args.max_iterations
        self.dry_run = args.dry_run
        self.agents = (args.agents.split(",") if args.agents
                       else list(AGENT_DEFS.keys()))

        # 入力バリデーション
        if self.max_iterations < 1:
            print("ERROR: --max-iterations must be >= 1")
            sys.exit(1)

        if not self.agents:
            print("ERROR: --agents must specify at least one agent")
            sys.exit(1)

        for a in self.agents:
            if a not in AGENT_DEFS:
                print(f"ERROR: Unknown agent '{a}'. "
                      f"Available: {list(AGENT_DEFS.keys())}")
                sys.exit(1)

        self.worktree = WorktreeManager(PROJECT_DIR)
        self.runner = AgentRunner(dry_run=self.dry_run)
        self.tracker = ProgressTracker(PROGRESS_LOG)

        # 異常終了時のクリーンアップ登録
        atexit.register(self._emergency_cleanup)

    def _emergency_cleanup(self):
        """atexit: worktree 残存防止"""
        try:
            self.worktree.cleanup_all()
        except Exception:
            pass

    # ----- Assess -----
    def assess(self) -> dict:
        """project.json から現在の PA 値を読み取る"""
        try:
            with open(PROJECT_JSON, "r", encoding="utf-8") as f:
                pj = json.load(f)
        except FileNotFoundError:
            print(f"ERROR: {PROJECT_JSON} not found")
            sys.exit(1)
        except json.JSONDecodeError as e:
            print(f"ERROR: Invalid JSON in {PROJECT_JSON}: {e}")
            sys.exit(1)

        processes = pj.get("spice_assessment", {}).get("processes", {})
        if not processes:
            print("ERROR: No spice_assessment.processes in project.json")
            sys.exit(1)

        snapshot = {}
        for pid, info in processes.items():
            if not isinstance(info, dict):
                print(f"WARNING: Process {pid} is not a dict, skipping")
                continue
            snapshot[pid] = {
                "pa_1_1": info.get("pa_1_1", "N"),
                "pa_2_1": info.get("pa_2_1", "N"),
                "pa_2_2": info.get("pa_2_2", "N"),
                "level": info.get("level", 0),
            }
        return snapshot

    # ----- Improve -----
    def improve(self, current_pa: dict) -> dict[str, dict]:
        """4エージェント並行で process_records を改善"""
        tasks = []
        for agent_name in self.agents:
            defn = AGENT_DEFS[agent_name]

            # worktree 作成
            if not self.dry_run:
                wt_path = self.worktree.create(agent_name)
                wsl_cwd = _windows_to_wsl_path(wt_path)
            else:
                wsl_cwd = _windows_to_wsl_path(
                    PROJECT_DIR / ".claude" / "worktrees" / agent_name)

            prompt = self._build_improve_prompt(agent_name, defn, current_pa)
            tasks.append((agent_name, prompt, wsl_cwd))

        return self.runner.run_parallel(tasks)

    # ----- Merge -----
    def merge(self, successful_agents: list[str]) -> list[tuple[bool, str]]:
        """成功エージェントのみマージ（失敗エージェントはスキップ）"""
        results = []
        for agent_name in self.agents:
            if agent_name not in successful_agents:
                print(f"  [merge] {agent_name}: SKIPPED (agent failed)")
                self.worktree.cleanup(agent_name)
                results.append((True, f"Skipped {agent_name} (agent failed)"))
                continue
            # worktree 内の未コミット変更を commit してからマージ
            self.worktree.commit_worktree_changes(agent_name)
            ok, msg = self.worktree.merge(agent_name)
            print(f"  [merge] {agent_name}: {'OK' if ok else 'FAIL'} — {msg}")
            results.append((ok, msg))
            self.worktree.cleanup(agent_name)
        return results

    # ----- CrossCheck -----
    def cross_check(self) -> tuple[bool, str]:
        """プロセス横断整合性チェック。(success, result_text) を返す"""
        prompt = self._build_crosscheck_prompt()
        result = self.runner.run("cross-checker", prompt, WSL_CWD)
        ok = result.get("ok", False)
        text = result.get("result", result.get("error", "unknown"))
        return ok, text

    # ----- Verify -----
    def verify(self, before: dict, after: dict) -> bool:
        """PA 値が劣化していないことを確認"""
        degraded = []
        for pid in before:
            if pid not in after:
                continue
            for pa in ["pa_1_1", "pa_2_1", "pa_2_2"]:
                b = PA_ORDER.get(before[pid][pa], 0)
                a = PA_ORDER.get(after[pid][pa], 0)
                if a < b:
                    degraded.append(f"{pid}.{pa}: {before[pid][pa]} → {after[pid][pa]}")

        if degraded:
            print(f"  [verify] DEGRADATION DETECTED:")
            for d in degraded:
                print(f"    - {d}")
            return False

        print("  [verify] No degradation detected. PASS")
        return True

    # ----- Main Loop -----
    def run(self):
        print(f"\n{'#'*60}")
        print(f"# WSL SPICE Improver — {_now_iso()}")
        print(f"# Mode: {'DRY-RUN' if self.dry_run else 'LIVE'}")
        print(f"# Max iterations: {self.max_iterations}")
        print(f"# Agents: {self.agents}")
        print(f"{'#'*60}\n")

        # WSL プリフライトチェック
        if not self.dry_run:
            print("  [preflight] Checking WSL availability...")
            _check_wsl_available()
            print("  [preflight] WSL OK")

        self.tracker.start_run()
        current_branch = _git_output(PROJECT_DIR, "branch", "--show-current").strip()

        for i in range(1, self.max_iterations + 1):
            print(f"\n{'='*60}")
            print(f"  ITERATION {i}/{self.max_iterations}")
            print(f"{'='*60}")

            # --- Safety tag ---
            tag_name = f"spice-iter-{i}-start"
            if not self.dry_run:
                try:
                    _git(PROJECT_DIR, "tag", "-f", tag_name)
                    print(f"  [safety] Tagged: {tag_name}")
                except subprocess.CalledProcessError:
                    pass

            # --- Phase 1: Assess ---
            print("\n  [Phase 1] Assess...")
            before = self.assess()
            target_processes = []
            for agent_name in self.agents:
                for pid in AGENT_DEFS[agent_name]["processes"]:
                    if pid in before and before[pid]["level"] < 2:
                        target_processes.append(pid)

            if not target_processes:
                print("  All target processes at Level 2! Stopping.")
                break

            print(f"  Target processes (< Level 2): {target_processes}")
            _print_pa_table(before, target_processes)

            # --- Phase 2: Improve ---
            print("\n  [Phase 2] Improve (parallel agents)...")
            agent_results = self.improve(before)

            # 品質ゲート: エージェント正常終了チェック
            successful_agents = [n for n, r in agent_results.items()
                                 if r.get("ok")]
            failed_agents = [n for n, r in agent_results.items()
                             if not r.get("ok")]
            if failed_agents:
                print(f"  [gate] WARNING: Failed agents: {failed_agents}")
                for n in failed_agents:
                    print(f"    {n}: {agent_results[n].get('error', 'unknown')}")

            if not successful_agents:
                print("  [gate] ALL agents failed. Skipping merge.")
                if not self.dry_run:
                    self.worktree.cleanup_all()
                continue

            # --- Phase 3: Merge ---
            if not self.dry_run:
                print("\n  [Phase 3] Merge...")
                merge_results = self.merge(successful_agents)
                merge_failures = [msg for ok, msg in merge_results if not ok]
                if merge_failures:
                    print(f"  [gate] MERGE FAILURES: {merge_failures}")
                    print(f"  [rollback] Rolling back to {tag_name}...")
                    _git(PROJECT_DIR, "reset", "--hard", tag_name)
                    self.worktree.cleanup_all()
                    continue
            else:
                print("\n  [Phase 3] Merge... [DRY-RUN] skipped")

            # --- Phase 4: CrossCheck ---
            print("\n  [Phase 4] CrossCheck (cross-process consistency)...")
            cc_ok, crosscheck_result = self.cross_check()
            if not cc_ok and not self.dry_run:
                print(f"  [gate] CrossCheck agent failed: {crosscheck_result[:200]}")
                print("  Continuing with Verify (CrossCheck is advisory)...")

            # --- Phase 5: Verify ---
            if not self.dry_run:
                print("\n  [Phase 5] Verify...")
                after = self.assess()
                passed = self.verify(before, after)

                if not passed:
                    print(f"  [rollback] Degradation detected. "
                          f"Rolling back to {tag_name}...")
                    _git(PROJECT_DIR, "reset", "--hard", tag_name)
                    self.worktree.cleanup_all()
                    self.tracker.record_iteration(
                        i, before, before, agent_results,
                        f"ROLLED BACK: {crosscheck_result[:500]}")
                    continue

                _print_pa_table(after, target_processes)
                improvements = _calc_improvements(before, after)
                if improvements:
                    print(f"\n  Improvements this iteration:")
                    for imp in improvements:
                        print(f"    {imp}")

                self.tracker.record_iteration(
                    i, before, after, agent_results, crosscheck_result)

                # コミット
                _git(PROJECT_DIR, "add", "process_records/")
                _git(PROJECT_DIR, "commit", "-m",
                     f"spice: iteration {i} improvements\n\n"
                     f"Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>")
                print(f"  [commit] Iteration {i} committed.")
            else:
                print("\n  [Phase 5] Verify... [DRY-RUN] skipped")
                self.tracker.record_iteration(
                    i, before, before,
                    {n: {"ok": True, "result": "[DRY-RUN]"} for n in self.agents},
                    "[DRY-RUN]")

        # --- Finish ---
        self.tracker.finish_run()
        self.worktree.cleanup_all()

        print(f"\n{'='*60}")
        print(f"  FINISHED — {_now_iso()}")
        print(f"  Progress log: {PROGRESS_LOG}")
        print(f"{'='*60}\n")

        # 最終レポート
        if not self.dry_run:
            final = self.assess()
            print("\n  Final PA values:")
            _print_pa_table(final, list(final.keys()))

    # ----- Prompt Builders -----
    def _build_improve_prompt(self, agent_name: str, defn: dict,
                              current_pa: dict) -> str:
        procs = defn["processes"]
        pa_info = "\n".join(
            f"  {pid}: PA1.1={current_pa[pid]['pa_1_1']}, "
            f"PA2.1={current_pa[pid]['pa_2_1']}, "
            f"PA2.2={current_pa[pid]['pa_2_2']}, Level={current_pa[pid]['level']}"
            for pid in procs if pid in current_pa
        )

        record_files = "\n".join(
            f"  - process_records/{pid.replace('.', '.')}*.md"
            for pid in procs
        )

        return f"""You are a SPICE process improvement agent ({agent_name}).
Your task: Improve the process records for the assigned processes to raise their
Automotive SPICE capability levels toward Level 2.

== ASSIGNED PROCESSES ==
{pa_info}

== TARGET ==
All processes must reach Level 2:
  - PA 1.1 = F (fully achieved, 86-100%)
  - PA 2.1 >= L (largely achieved, 51-85%)
  - PA 2.2 >= L (largely achieved, 51-85%)

== REFERENCE FILES (READ FIRST) ==
1. PROCESS.md — BP/WP definitions for each process
2. .claude/knowledge/automotive_spice.md — PA rating criteria, GP evidence requirements
3. .claude/skills/assess-spice/SKILL.md — Assessment methodology

== PROCESS RECORD FILES TO IMPROVE ==
{record_files}

== IMPROVEMENT INSTRUCTIONS ==
For each assigned process record:

1. READ the current process record file completely
2. READ PROCESS.md for that process's BP/WP/Outcome definitions
3. READ .claude/knowledge/automotive_spice.md for PA rating criteria

4. IMPROVE §1 (PA 1.1 - Process Performance):
   - Ensure ALL BPs have concrete implementation records with dates
   - Ensure ALL Outcomes show clear achievement evidence
   - Add specific metrics, test results, document references

5. IMPROVE §2 (BP Implementation Records):
   - Fill in any missing BP rows with concrete implementation details
   - Add specific dates, evidence references, responsible parties
   - Ensure each BP links to concrete artifacts

6. IMPROVE §3 (PA 2.2 - Work Product Management):
   - Ensure ALL work products have version numbers, review status, storage location
   - Add any missing work products from PROCESS.md WP definitions
   - Record review/approval status for each WP

7. IMPROVE §4 (PA 2.1 - Performance Management):
   - Ensure goal/plan/monitoring/adjustment are ALL documented
   - Link to MAN.3 milestones
   - Add specific monitoring results and adjustment decisions

== CRITICAL CONSTRAINTS ==
- Do NOT edit project.json (orchestrator handles PA updates separately)
- Do NOT delete any existing content — only ADD or ENHANCE
- PRESERVE the §1-§5 structure exactly
- Be SPECIFIC — no vague statements like "概ね達成" without evidence
- Every claim must reference a concrete artifact (file path, section, test ID)
- When adding dates, use existing dates from the record or project timeline
- Focus on quality over quantity — one well-documented BP is better than five vague ones
"""

    def _build_crosscheck_prompt(self) -> str:
        return """You are a SPICE cross-process consistency checker.
Your task: After individual process improvements, verify cross-process consistency.

== REFERENCE FILES (READ FIRST) ==
1. PROCESS.md — Process definitions and relationships
2. .claude/knowledge/automotive_spice.md — PA criteria

== CHECK ALL process_records/*.md FILES ==
Read ALL 16 process record files and verify the following 6 consistency aspects:

1. REQUIREMENTS TRACEABILITY (SYS.2 → SWE.1 → SWE.3):
   - System requirements traced to SW requirements?
   - SW requirements traced to detailed design?
   - Any broken links or missing references?

2. V-MODEL CORRESPONDENCE (SYS.2↔SYS.5, SWE.1↔SWE.6):
   - Left-side (design) processes have matching right-side (test) processes?
   - Test specifications reference the corresponding design artifacts?

3. PLAN-EXECUTION ALIGNMENT (MAN.3 → all processes):
   - MAN.3 milestones referenced in each process §4?
   - Dates in process records consistent with MAN.3 timeline?

4. CONFIGURATION MANAGEMENT CONSISTENCY (SUP.8 → all):
   - SUP.8 configuration items cover all process work products?
   - Version numbers in SUP.8 match those in individual process §3?

5. QA COVERAGE (SUP.1 → all):
   - SUP.1 QA activities cover all process work products?
   - QA findings referenced in relevant process records?

6. CHANGE MANAGEMENT FLOW (SUP.10 → SUP.9 → SUP.8):
   - Change requests linked to problem reports?
   - Configuration changes tracked for resolved problems?

== OUTPUT ==
For each inconsistency found:
1. FIX it directly in the process_records file
2. Report: [FIXED] {process} — {description}

For items that are consistent:
- Report: [OK] {check aspect} — {brief confirmation}

== CONSTRAINTS ==
- Do NOT edit project.json
- Do NOT delete existing content — only ADD or FIX
- PRESERVE §1-§5 structure
"""


# ===========================================================
# Utility functions
# ===========================================================
def _now_iso() -> str:
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _windows_to_wsl_path(win_path: Path) -> str:
    """Windows パスを WSL パスに変換 (C:\\Users\\... → /mnt/c/Users/...)"""
    posix = win_path.resolve().as_posix()
    if len(posix) >= 2 and posix[1] == ":":
        drive = posix[0].lower()
        return f"/mnt/{drive}{posix[2:]}"
    return posix


def _check_wsl_available():
    """WSL が利用可能か事前チェック"""
    try:
        result = subprocess.run(
            ["wsl.exe", "-e", "true"],
            capture_output=True, timeout=10,
        )
        if result.returncode != 0:
            print("ERROR: WSL is not available (wsl.exe -e true failed)")
            sys.exit(1)
    except FileNotFoundError:
        print("ERROR: wsl.exe not found")
        sys.exit(1)
    except subprocess.TimeoutExpired:
        print("ERROR: WSL check timed out")
        sys.exit(1)


def _git(cwd: Path, *args):
    """git コマンドを実行（例外スロー）"""
    subprocess.run(
        ["git", *args], cwd=str(cwd),
        check=True, capture_output=True, text=True,
    )


def _git_output(cwd: Path, *args) -> str:
    """git コマンドを実行し、stdout を返す"""
    result = subprocess.run(
        ["git", *args], cwd=str(cwd),
        capture_output=True, text=True,
    )
    return result.stdout


def _calc_improvements(before: dict, after: dict) -> list[str]:
    """PA 値の改善一覧を返す"""
    improvements = []
    for pid in before:
        if pid not in after:
            continue
        for pa in ["pa_1_1", "pa_2_1", "pa_2_2"]:
            b = before[pid][pa]
            a = after[pid][pa]
            if PA_ORDER.get(a, 0) > PA_ORDER.get(b, 0):
                improvements.append(f"{pid}.{pa}: {b} → {a}")
        bl = before[pid]["level"]
        al = after[pid]["level"]
        if al > bl:
            improvements.append(f"{pid}.level: {bl} → {al}")
    return improvements


def _print_pa_table(pa_data: dict, processes: list[str]):
    """PA 値のテーブルを表示"""
    print(f"\n  {'Process':<8} {'PA1.1':<6} {'PA2.1':<6} {'PA2.2':<6} {'Level':<6}")
    print(f"  {'-'*32}")
    for pid in sorted(processes):
        if pid in pa_data:
            d = pa_data[pid]
            print(f"  {pid:<8} {d['pa_1_1']:<6} {d['pa_2_1']:<6} "
                  f"{d['pa_2_2']:<6} {d['level']:<6}")


# ===========================================================
# CLI
# ===========================================================
def main():
    parser = argparse.ArgumentParser(
        description="WSL Multi-Agent SPICE Improvement Orchestrator")
    parser.add_argument("--max-iterations", type=int, default=3,
                        help="Maximum improvement iterations (default: 3)")
    parser.add_argument("--agents", type=str, default=None,
                        help="Comma-separated agent names (default: all)")
    parser.add_argument("--dry-run", action="store_true",
                        help="Print prompts and skip execution")
    args = parser.parse_args()

    improver = SpiceImprover(args)
    improver.run()


if __name__ == "__main__":
    main()
