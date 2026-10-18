# Kotlinmania — workspace agent guide

This is the single sharp guide for agents (Claude, Codex, anyone else) doing
work in this workspace. It merges the old workspace-root `CLAUDE.md`, the
embedded hourly runbook, the per-repo `SWIFT_EXPORT_ROLLOUT.md` lessons, and
the long trail of recovered field-feedback into one place. **When this file
disagrees with anything you remember, this file wins.**

> **Source-of-truth.** The workspace-root `AGENTS.md` is canonical.
> The workspace root is the folder that holds every `*-kotlin/` repo;
> paths in this guide are relative to it (from inside a repo, prefix `../`).
> Workspace-root `CLAUDE.md` is a tiny redirect file pointing here
> (plain text — **never a symlink**: this workspace is Kotlin
> Multiplatform and Windows is a first-class target, and Windows git
> does not handle symlinks reliably). Inside each `*-kotlin/` repo
> there is no `CLAUDE.md` at all: per-repo `CLAUDE.md` files were
> removed via `git rm` on 2026-05-24 and must stay removed.
>
> **No symlinks anywhere in this workspace.** Not at the workspace
> root, not in any repo, not under any subdirectory. If you find one,
> replace it with the file it points to (real content) and commit the
> replacement. Symlinks check out as the literal symlink content on
> Windows runners — they break KMP builds in opaque ways.
>
> As of 2026-05-24 every `*-kotlin/AGENTS.md` is a copy of this file
> (blasted out so the next agent in any repo reads the same rules).
> Per-repo specifics live in the repo's `README.md` and any local
> status notes (`NEXT_ACTIONS.md`, `PORT_REPORT.md`, etc.). When a
> repo-local doc disagrees with this file on a workspace-wide rule
> (branch hygiene, Swift Export, build-gate, JS toolchain security,
> Android SDK, test parity, completion contract), this file wins.

## The scheduled-task agent job

The workspace runs scheduled-task automation. The job, briefly:

### Focused-repo priority backlog — GitHub-flagged "JavaScript" projects

These repos have so little Kotlin source ported from their upstream Rust
crates that GitHub still classifies them as JavaScript projects. They are
the highest-priority porting backlog. Pick the stalest one first (§3
"Rotate to the stalest repo") unless `port_priority.json` (§3 "Pick what
to port next") says otherwise.

**Page 1 (top 10):**
tree-sitter-bash-kotlin · tree-sitter-language-kotlin · nucleo-kotlin ·
image-kotlin · itertools-kotlin · indexmap-kotlin · globset-kotlin ·
futures-kotlin · deno-core-icudata-kotlin · tonic-prost-kotlin

**Page 2:** opentelemetry-sdk-kotlin · rama-core-kotlin · tungstenite-kotlin
· csv-kotlin · rama-tcp-kotlin · tokio-tungstenite-kotlin · async-io-kotlin
· vt100-kotlin · encoding-rs-kotlin · env-logger-kotlin ·
constant-time-eq-kotlin · clap-complete-kotlin · assert-cmd-kotlin ·
two-face-kotlin

**Page 3:** gazebo-kotlin · test-log-kotlin · ts-rs-kotlin · toml-kotlin ·
regex-kotlin · winres-kotlin · icu-decimal-kotlin · landlock-kotlin ·
rama-net-kotlin · async-trait-kotlin · icu-locale-core-kotlin

**Page 4:** uuid-kotlin · zstd-kotlin · arboard-kotlin ·
tracing-subscriber-kotlin · url-kotlin · textwrap-kotlin · pathdiff-kotlin
· uds-windows-kotlin · tokio-test-kotlin · tracing-opentelemetry-kotlin

**Page 5:** tonic-kotlin · tokio-stream-kotlin · toml-edit-kotlin ·
test-case-kotlin · wiremock-kotlin · windows-kotlin · quote-kotlin ·
bitflags-kotlin · zip-kotlin · tracing-appender-kotlin ·
ed25519-dalek-kotlin · syntect-kotlin · serde-with-kotlin · rmcp-kotlin

**Page 6:** sqlx-kotlin · socket2-kotlin · axum-kotlin · serde-json-kotlin
· allocative-kotlin · sse-stream-kotlin · windows-sys-kotlin ·
winapi-util-kotlin · tracing-test-kotlin · tracing-kotlin · rand-kotlin ·
rama-socks5-kotlin · tempfile-kotlin · strum-macros-kotlin ·
shared-library-kotlin · serde-yaml-kotlin

**Page 7:** serde-path-to-error-kotlin · sentry-kotlin · rustls-kotlin ·
reqwest-kotlin · rcgen-kotlin · rama-http-kotlin · quick-xml-kotlin ·
pulldown-cmark-kotlin · prost-kotlin · portable-pty-kotlin ·
pkg-config-kotlin · path-absolutize-kotlin · p256-kotlin · owo-colors-kotlin
· opentelemetry-semantic-conventions-kotlin · opentelemetry-kotlin ·
opentelemetry-appender-tracing-kotlin · openssl-sys-kotlin · oauth2-kotlin
· rama-http-backend-kotlin · insta-kotlin

**Page 8 (last):** once-cell-kotlin · notify-kotlin · gix-kotlin · v8-kotlin
· libc-kotlin · rama-tls-rustls-kotlin · tokio-util-kotlin · crypto-box-kotlin
· libwebrtc-kotlin · webbrowser-kotlin · winapi-kotlin

Several of these have memory-recorded blockers that move them down the
working order rather than out of scope — `clap-complete-kotlin` blocked on
`clap-kotlin` publish; `nucleo-kotlin` on `nucleo-matcher-kotlin`;
`indexmap-kotlin` on `hashbrown-kotlin` + `equivalent-kotlin`;
`tonic-prost-kotlin` on tonic + prost; `libwebrtc-kotlin` on `bytes-kotlin`
watchOS slices. See §3 "When you can't port a Rust dependency: check
kotlinmania siblings first" for the decision tree.

### Work order — every session, in this order

Do not treat an earlier item as permission to skip a later one. The §0
completion contract requires the whole chain, and §3 names source porting
as the default next action when infra is clean.

1. **Read this file (`AGENTS.md`) and `SWIFT.md` in full** (you're in `AGENTS.md`, but note that IDE/Antigravity rule injection truncates files after ~21KB, so you MUST explicitly read the workspace-root `AGENTS.md` §3–§12 and `SWIFT.md`). The §0 contract is non-negotiable.
   Read the focused repo's `README.md`, `SWIFT.md`, `RUST_CALLERS.md`, and any `NEXT_ACTIONS.md` /
   `PORT_REPORT.md`. There is no per-repo `CLAUDE.md` anymore (removed
   2026-05-24); the per-repo `AGENTS.md` is a copy of this file.
2. **Audit + sync local main** (§11 step 1). `git fetch --prune`, inspect
   `git status --short --branch`, and if on `main` behind `origin/main`
   run `git pull --ff-only origin main`.
3. **Workflow-shape audit** (§7). Fix or report any `push`/
   `pull_request` regression on `ci.yml` / `codeql.yml` / `publish.yml` /
   platform workflows.
4. **Branch reconciliation** (§2). **PR-only** — never `git push origin
   main`, never local `git switch main && git merge --no-ff <branch>`.
   Reconcile every stranded `[gone]` branch via the documented PR flow.
   Carry dirty work forward (§1 — never stash, never orphan).
5. **Infra defect repair**: build, CI, CodeQL (§8 "CodeQL `java-kotlin`
   extraction"), Dependabot security (§9), JS-toolchain security
   patching (§9), Android SDK normalization (§8), publishing, or
   workflow defect. Local verification first; commit; push the branch;
   `gh pr create` + `gh pr merge --merge --delete-branch`.
6. **Swift Export sweep** (§4 and `SWIFT.md`) if `swiftExport { … }` is configured. The
   five-class sweep, then `./gradlew test` which must include
   `swift test` locally (§6).
7. **Source porting** (§3) is the default next action when infra is
   clean. Use `ast_distance` to inventory gaps — the binary at
   `bin/ast_distance` at the workspace root is **never a
   blocker**; if a repo's `tools/ast_distance` is missing, copy the
   workspace binary in (§3 gives the exact `cp` + `.gitignore` lines).
   Translate from `tmp/` per the translation rules (one Rust file → one
   Kotlin file **except `mod.rs` and `lib.rs`, which must NEVER be ported as
   `Mod.kt` or `Lib.kt` and NEVER re-exported in Kotlin**, top-to-bottom, no
   stubs, no Rust syntax in Kotlin/KDoc/comments, common sense applies). Check
   kotlinmania siblings before declaring a dependency unportable.
8. **Publish** if there's releasable code: bump version in
   `build.gradle.kts` + README install snippets, commit, push, `gh
   release create v<X.Y.Z>` to fire the `release[released]` publish
   workflow.
9. **Session report** (§12) with workflow-shape audit, focused repo,
   branch, constructive outcome OR exact hard blocker, PRs and branches
   inspected/merged/closed, local commands and outcomes, remote workflow
   state, commits, unresolved blockers, and the PR list as full
   clickable GitHub URLs at the very end.

### Hard constraints to remember while doing the above

- **Do NOT port `mod.rs` or `lib.rs` as `Mod.kt` or `Lib.kt`, and NEVER re-export in Kotlin** (§3):
  - **`lib.rs` must be split into focused `.kt` files** (functions, interfaces, and types in their proper package paths). Never create `Lib.kt` or `Mod.kt`, and never attach `// port-lint: source lib.rs`.
  - **`mod.rs` is NOT allowed to re-export.** We do not follow Rust `mod.rs` / `pub use` re-export patterns in Kotlin (no `Mod.kt`, no `typealias` re-export bridges, no wrapper functions). Never attach `// port-lint: source .../mod.rs` (module-root `port-lint` headers trip `ast_distance` stub zeroing). If an upstream `mod.rs` contains real implementation code, split it into focused `.kt` files in the proper package path.
  - **Update callers via `RUST_CALLERS.md`.** When upstream `mod.rs` or `lib.rs` re-exports symbols, inspect `RUST_CALLERS.md` and update callers directly to import the canonical Kotlin package and symbol name.
- **`allWarningsAsErrors = true`** is the workspace default. Every warning
  is a build failure. Treat every warning as a real defect — never scope
  the flag down to silence Swift Export gate noise (§4 forbids it
  explicitly).
- **All current KotlinMania targets build** (§0 condition 3 + §5
  `fullTargetBuildTaskNames`). Never shrink the gate to dodge a CI
  failure. **We do not support 32-bit targets of any kind, and we do not
  support Apple x86_64 targets** (§5.5). Retired: `watchosArm32`,
  `watchosArm64` (arm64_32), `androidNativeArm32`, `androidNativeX86`,
  `linuxArm32Hfp`, `mingwX86`, `iosX64`, `macosX64`, `tvosX64`,
  `watchosX64`. Retained: JVM, Android, JS, Wasm (JS + WASI), Linux
  x64/ARM64, Windows x64, Android Native ARM64/x64, and Apple ARM64 device
  + simulator ARM64 targets. These are workspace-wide retirements, not
  per-repo shrinks.
- **Swift test passes locally** (§0 condition 4). Don't wait for GitHub
  to find Swift Export bugs — by then you've burned a CI run.
- **Test parity with `tmp/` Rust** (§6). Every upstream `#[test]` /
  `tests/*.rs` / `#[cfg(test)]` module gets an answer — port it, or
  leave a one-line honest comment naming the specific semantic that
  doesn't translate. Never write a fake simulation.
- **`port-lint: ignore` is not a real directive** (§3). Strip it when
  you see it. Only `port-lint: source <path>` and `port-lint: tests
  <path>` are recognized.
- **Pure Kotlin Multiplatform only — never a Java library** (§3). No
  `java.*` / `javax.*` imports, no JVM-only or third-party Java artifacts
  (okio, Guava, Bouncy Castle, Jackson, ...). The 200+ Rust ports under
  `kotlinmania/` are the replacement for the Java ecosystem: use them.
- **Workspace artifacts go under**
  `automation-artifacts/` at the workspace root. No
  `/tmp` scratch (§1).

The yarn-resolution security block, the Android SDK installer pattern,
the build-gate task list, the canonical Swift Export rollout recipe, the
de-generification policy, the PR-only reconciliation flow, and every
other detailed recipe are in the sections numbered below — open the
relevant section before acting on a summary line above.

---

## 0. The completion contract — non-negotiable

A session is not done until every focused repo satisfies all of these,
or the run reports the exact hard blocker that makes a condition physically
impossible:

1. **The repo matches its local `CLAUDE.md` contract.** If the repo has no
   `CLAUDE.md`, the workspace `CLAUDE.md` + this file are the contract; say so
   explicitly.
2. **Real source advance.** Port or materially advance Kotlin source from
   upstream Rust in `tmp/`. CI plumbing, branch cleanup, dependency bumps,
   workflow edits, dry-runs, and status reports are *additive*, never a
   substitute for the source-porting requirement.
3. **All current KotlinMania targets build.** "All targets" = the full
   organization target surface required by this workspace — not
   whichever tasks happen to be cheap on the current host, and not a
   repo-selected subset. The `fullTargetBuildTaskNames` wiring (§5) makes
   this enforceable on `./gradlew build`.
4. **Swift test passes locally.** If the repo configures `swiftExport { … }`,
   `./gradlew test` (or `check`) must invoke `swift test` against the
   `embedSwiftExportForXcode`-produced SPM package and treat a non-zero exit
   as a build failure. Do not wait for GitHub to find Swift Export bugs (§4).
5. **Test parity with upstream Rust.** Every `#[test]`, `#[cfg(test)] mod tests
   { … }`, `tests/*.rs`, and `benches/*.rs` under `tmp/` has either (a) a
   faithful Kotlin equivalent in `commonTest`/per-target test sources, or (b)
   an honest one-line comment explaining the *specific* semantic that can't
   port (mem::forget, pin-poll-once, custom Drop). No silent gaps. No fake
   simulations that test a different invariant (§6).
6. **Host-specific targets validated on a host that can run them.** A macOS
   local build is not Windows validation. Windows = the repo's Windows GitHub
   Actions job or another real Windows runner. Same for any other configured
   target the local host can't link.
7. **Clean branch/PR state.** Every dirty file produced in a focused repo is
   committed. Every PR or branch the run touched is either merged through the
   reconciliation flow (§2) or has a documented hard blocker. Stale `[gone]`
   branches with unmerged commits are **undeleted and reconciled** before the
   session ends.
8. **Workflow-shape audit reported.** The session report names every workflow
   file checked, whether `push` / `pull_request` triggers remain, and any
   remote workflow that was disabled/re-enabled (§7).

Things that are **not** a sufficient session outcome on their own:

- "No open PRs, nothing to merge."
- "Remote Actions are disabled / queued / never appeared."
- "Local macOS build passed" while iOS/Android/Windows/Swift/JS targets are
  unverified.
- "I fixed the workflow YAML" with no source advance.
- "Tests passed against unchanged code."
- "I bumped a dependency."

If a session genuinely cannot make a constructive change, the report must say
the session is **blocked**, not complete, and name the exact blocker
(user-owned dirty files in the focused range, missing upstream Rust source,
missing Maven credentials, unpublished dependency, platform runner
unavailable, gate failure with root cause outside the focused repo). A
no-change report without that level of blocker is a failed session.

> **`ast_distance` is never a blocker.** The workspace ships a runnable
> binary at `bin/ast_distance` in the workspace root
> (Mach-O arm64). If a repo requires it and does not ship its own under
> `tools/ast_distance/`, copy the workspace binary into the repo's
> `tools/` folder and add the path to `.gitignore` (it's a local
> developer artifact, not a tracked dependency). Never report "no
> runnable ast_distance" — the runnable is one `cp` away.

---

## 1. Git hygiene — the bright lines

These are non-negotiable. Violating any of them requires immediate repair.

- **No worktrees.** The global pre-commit hook at
  `~/.config/git/hooks/pre-commit` refuses commits from linked worktrees.
  Do not bypass with `--no-verify`. Work in the repo's main checkout.
- **No `/tmp` scratch.** Reports, logs, helper outputs go under the repo's
  own `tmp/` or under
  `automation-artifacts/` at the workspace root. Future agents
  need the raw data, not summaries.
- **Force-push requires explicit confirmation.** Codex `default.rules`
  marks `git push --force`, `--force-with-lease`, and `-f` as `ask`. Do
  not assume a generic "go ahead" covers it — the approval has to name
  the force-push and the branch. Never force-push `main`.
- **`git pull` is fine — but only with `--ff-only` on `main`, and only
  with `--no-ff` on a feature branch.** Two legitimate shapes, no
  others:
  - **On `main`, after a remote PR merged on GitHub:**
    `git pull --ff-only origin main`. There must be no local commits on
    `main` (PR-only rule §2 prevents them). `--ff-only` makes the
    command fail loud if for some reason local `main` *has* diverged,
    so you can't accidentally create a merge commit on `main` that
    didn't go through a PR.
  - **On a feature branch, to bring `main` in before opening/updating
    a PR:** `git pull --no-ff origin main`. This creates the explicit
    merge commit on the feature branch (clean PR diff, no rebase, no
    history rewrite).
  - **Prefer explicit `--ff-only` / `--no-ff` over bare `git pull`.**
    Bare `git pull` is allowed — it's how you keep local in sync — but
    its exact behavior depends on `pull.ff` / `pull.rebase` config and
    on whether the branch has diverged, so it can silently create a
    merge commit or rewrite history. Pass `--ff-only` (on `main`) or
    `--no-ff` (on a feature branch) when the intent matters, so the
    intent is in the command rather than the config.
- **Destructive git is ask-only, not a shortcut.** `git reset --hard`,
  `git branch -D`, `git push --force[-with-lease|-f]`, `git checkout --`,
  `git checkout .`, `git restore`, `git rebase`, and `git clean -f` are
  all marked `ask` in Codex `default.rules` for a reason: each one can
  destroy uncommitted work or rewrite history. Do not reach for them
  to escape a stash/no-ff/branch flow the user asked for. Report the
  stuck state and wait for a confirmation that names the specific
  command.
- **Branch-only, never push to main. PR-merge only.** Push a feature
  branch, open a PR with `gh pr create`, merge the PR with `gh pr merge`.
  **Never `git push origin main`** — even for reconcile merges. **Never
  `git switch main && git merge --no-ff <branch>` to land work on main
  locally.** The "merge happens" event must always be `gh pr merge`, never
  a local commit pushed to `main`. The only local merge that's allowed is
  `git merge --no-ff origin/main` *on a feature branch* (bringing main
  into the branch so the PR is clean) — that doesn't change `main`.
- **No `port/*` branch prefix.** It trips guardrails. Use one neutral
  integration branch per session (e.g. `automation/<short-topic>`,
  `session/<date>-<topic>`); land every commit in the session onto it as
  separate commits; ship as one PR.
- **No broad recursive `find`.** Do not walk
  the workspace root, all `*-kotlin/` repos, or `find .`
  from a repo root without prunes. It monopolizes the workspace and disables
  required tooling. Use `rg --files` with explicit `-g` patterns, or scope
  to small named subtrees (`src/`, `.github/`, `tools/`). If a recursive
  `find` is unavoidable, prune `.git`, `.gradle`, `.android-sdk`,
  `node_modules`, `build`, and any vendored upstream directories first.
- **Inspect `git status --short --branch` (including untracked) before any
  fetch/merge/switch.** Never discard or revert changes you did not make.
- **Commit ALL dirty. Never stash. Never orphan.** This rule exists
  because Sydney has lost an *insane* amount of code to (a) `git stash`
  entries that get forgotten across sessions and (b) commits ending up
  on orphaned refs after a branch switch. The procedure is mandatory:
  - **If dirty AND on a feature branch** → commit it on that branch.
    Push the branch. Open a PR. Merge via `gh pr merge`. Do not switch
    branches first; the dirty work belongs where it was written.
  - **If dirty AND on `main`** → this is the *only* legitimate case for
    switching branches with a dirty tree. Create a new feature branch
    from that dirty state (`git switch -c automation/<topic>`), commit
    it there, push, PR, merge.
  - **Never `git stash`** to "park" work for later. Stashes get
    forgotten; orphaned stashes are dropped silently by garbage
    collection.
  - **Never move dirty work from one feature branch to another.** That
    creates the orphan-ref pattern that has eaten work in this
    workspace. If a feature branch has work that should be elsewhere,
    commit it where it is FIRST, push, then cherry-pick the commit
    onto the other branch and push that too.
  - **Never call it "another agent's WIP" and walk away.** Once dirty
    files are in this tree or this stash, they are yours to land.
  - **End of session = no dirty files anywhere.** `git status --short`
    must be empty before the session report. If a file is genuinely
    not yours to commit (user explicitly told you to leave it alone),
    name it in the session report under "Unresolved blockers."
  - **The branch-must-exist requirement is why we use PR-only merges
    (§2).** Pushing the branch + merging via remote PR is what makes
    the commit recoverable. A local-only branch with un-pushed
    commits is one accidental reset away from the same code loss.

---

## 2. Branch reconciliation — never delete without merging, PR-only

**The rule.** Every branch in every focused repo is yours to manage. Branch
ownership is irrelevant: a stale Dependabot branch, a prior automation
branch, a human-made branch, a `[gone]` upstream tracking ref — all of them.

**Never delete a branch without one of:**

1. **Proving ancestry.** `git merge-base --is-ancestor <branch> main`
   returns true → the branch is already in `main` → safe to delete the
   remote (and local) ref via `gh` or `git push origin --delete`.
2. **Merging through the PR-only reconciliation flow.** For any branch
   with commits not in `main`:

   ```bash
   git fetch --prune
   git switch <branch>
   git merge --no-ff origin/main           # bring main INTO the feature
                                           # branch only — never the reverse
   # Equivalent one-liner: git pull --no-ff origin main
   # resolve conflicts, run local gates (./gradlew test, swift test, etc.)

   # If the original remote ref is gone, push as a fresh ref:
   git push -u origin HEAD:reconcile/<branch>

   gh pr create --base main --head reconcile/<branch> \
       --title "Reconcile stranded <branch>" \
       --body  "Restores remote-deleted branch and merges it into main per workspace rule."

   # Once PR checks are green (or known to be on a manual-CI repo):
   gh pr merge <PR-number> --merge --delete-branch
                                           # or --squash / --rebase per repo style
   ```

3. **NEVER `git switch main && git merge --no-ff <branch> && git push
   origin main`.** That is the forbidden local-merge-to-main flow. Even
   reconcile merges go through a PR. The "main now contains X" event must
   be a `gh pr merge` event, never a local push.

If you find a previously-deleted branch that was **not** an ancestor of
`main` at deletion time (e.g. a `[gone]` local branch pointing at commits
not in `main`'s history): **undelete and reconcile.** Push the branch back
to origin under a fresh name (or its original name if `gh` allows), then
run the PR-only flow above.

For Dependabot PRs, this is normal integration work — `gh pr merge` the
update into `main` as part of the session, preserving the repo's required
lockfile workflow.

### Why PR-only (no local main merges)

- The repo's required CI gates (CodeQL, dependabot, target builds, Swift
  Export, publish dry-run) only fire on the PR event. A local push to
  `main` bypasses them.
- The branch-protection rules on `KotlinMania/*` reject direct pushes to
  `main`. A local `git merge --no-ff <branch> && git push origin main`
  will fail anyway — but worse, it leaves the local `main` ahead of
  `origin/main`, which then either gets force-pushed (forbidden) or has
  to be manually reset (error-prone).
- Audit trail. Every change to `main` corresponds to a PR with a number,
  description, reviewer (even if Claude), and the merge commit message.
  Local merges erase that audit trail.

---

## 3. Porting discipline

These defaults apply unless a repo's own docs explicitly say otherwise.

### Translation rules

- **Kotlin stays Kotlin.** `PascalCase` types, `camelCase` functions/locals,
  lowercase packages. Never `snake_case` Kotlin identifiers to match Rust.
- **No Rust in source.** No Rust syntax in `.kt`, no Rust syntax in KDoc, no
  Rust syntax in inline comments. Translate `Vec<T>` → `List<T>`,
  `Option<&str>` → `String?`, `Self::foo()` → `foo()`, drop lifetimes, lift
  `cfg(...)` / `#[derive(...)]` into prose.
- **Comments are translated upstream content.** Not a place for porting
  notes, "Rust vs Kotlin" rationale, ast_distance strategy, or workaround
  explanations. Those go in commit messages, `NEXT_ACTIONS.md`, or review
  notes. **No deferred language** in any tracked file: no "deferred,"
  "next pass," "out of scope," "blocked on X" written into `CLAUDE.md` /
  `README.md` / `NEXT_ACTIONS.md` / PR descriptions. Finish the work; don't
  inventory what isn't done.
- **No stubs.** No `TODO()`, no placeholder bodies, no `@Suppress(...)`. If
  something is missing, port the dependency. If a Rust test depends on
  semantics Kotlin can't reproduce (mem::forget, pin-poll-once, Drop),
  leave it unported with an honest one-line comment naming the specific
  semantic; never write a "simulation" that tests a different invariant.
- **Translation happens in the main loop.** No subagent-driven `.kt` edits.
- **One Rust file → one Kotlin file — except `mod.rs` and `lib.rs`, which must NEVER be ported as `Mod.kt` or `Lib.kt`.** Translate top-to-bottom in upstream order. Read the whole upstream file before editing the Kotlin file.
- **Do NOT port `mod.rs` or `lib.rs`.**
  - **`lib.rs` must be split into Kotlin functions and types in their proper package paths.** Never create a `Lib.kt` file or attach `// port-lint: source lib.rs`. Place top-level functions, traits/interfaces, and types from `lib.rs` into focused Kotlin files in the appropriate package hierarchy.
  - **`mod.rs` is NOT allowed to be used to re-export.** We do not follow Rust `mod.rs` / `pub use` re-export patterns in Kotlin for a reason (no `Mod.kt`, no `typealias` bridges, no re-export wrappers). If an upstream `mod.rs` contains real implementation code (not module declarations or re-exports), split that implementation into focused `.kt` files in the proper package path (e.g., `Header.kt`, `Decoder.kt`).
  - **Look at `RUST_CALLERS.md` and update the callers instead.** When upstream `mod.rs` or `lib.rs` re-exports symbols from submodules or other crates, do **not** re-export or alias them in Kotlin. Instead, inspect `RUST_CALLERS.md` to identify every caller across the workspace and update the callers directly to import the canonical Kotlin package and symbol name.

### Drift measurement — `ast_distance` is how we measure "how close to done"

There is **no "parity mode"** on/off. Every `*-kotlin/` repo with upstream
Rust under `tmp/` uses `ast_distance` as the way to answer "what's left to
port?" — that's the whole point of the tool, and it applies to every
repo whether or not it ships its own `tools/ast_distance/` binary.

- Prefer the repo-local `tools/ast_distance` binary/script when present.
- If the repo does not ship one, copy the workspace binary into the
  repo's `tools/` folder and add the path to `.gitignore`:
  ```bash
  mkdir -p tools
  cp ../bin/ast_distance tools/
  grep -qxF 'tools/ast_distance' .gitignore 2>/dev/null \
    || echo 'tools/ast_distance' >> .gitignore
  ```
  The binary is a developer artifact, not a tracked dependency.
- **A missing or unrunnable `ast_distance` is NEVER a blocker.**
  Continue transliterating from `tmp/` per the translation rules above
  (read the whole upstream file, one Rust file → one Kotlin file,
  top-to-bottom order, Kotlin-facing comments, no stubs). The measurement
  is for "what gap to attack next" — useful, but absence of measurement
  is not absence of work. Translate. Then run the tool when you can.
- When `ast_distance` *is* runnable, measure before choosing work and
  after completed file/phase boundaries. Don't chase similarity scores
  while half-translating; don't Rustify Kotlin to appease the tool.
- Port-lint provenance headers are required on derived files:
  ```kotlin
  // port-lint: source <path-relative-to-upstream-root>
  package <repo package>
  ```
  The only valid `port-lint:` directives are `source <path>` (for
  ports of Rust source files) and `tests <path>` (for ports of Rust
  test files). **There is no `port-lint: ignore` directive.** Past
  agents have written `// port-lint: ignore — ...` as if it were one;
  it isn't recognized by `ast_distance` or any tooling. Strip those
  lines when you see them; they are noise, not configuration. The
  only legitimate reason a Kotlin file lacks a `port-lint: source`
  header is that it has no upstream Rust counterpart (Kotlin-side
  glue, repo-specific helpers, or new APIs), in which case it simply
  has no header — not a fake "ignore" directive.
- **How `ast_distance` evaluates stubs and contamination (ground truth from the `sydneyrenee/ASTDistance` sources):**
  - **Module-root headers trip stub zeroing:** In `include/ast_parser.hpp` (`has_stub_bodies`), any Kotlin file carrying a port-lint header pointing to a module root (`/mod.rs`, `/lib.rs`, `/main.rs`, or `/__init__.py`) is intentionally flagged as a stub (`has_stub_bodies == true`), which triggers: `"target contains TODO/stub/placeholder markers in function bodies"` and zeroes the score. Upstream `mod.rs` and `lib.rs` files represent Rust module/crate root syntax and must never be ported as `Mod.kt` or `Lib.kt`. Split real implementation from `lib.rs` or `mod.rs` into focused `.kt` files in their package paths, never re-export via `mod.rs`, and consult `RUST_CALLERS.md` to update callers directly.
  - **Zero-tolerance contamination filters:** In `include/codebase.hpp` (`kotlin_contamination_reasons_for_text`), `ast_distance` scans Kotlin code and comments (outside of the exact single-line `// port-lint: (source|tests) <path>` header):
    - **`snake_case` identifiers:** Any lowercase `snake_case` identifier in Kotlin code or comments trips the cheat detector (`snake_case identifier ... in Kotlin code/comments`).
    - **Rust syntax in Kotlin/KDoc/comments:** Never leave Rust keywords or syntax in comments (`fn ...()`, `let (mut) ...`, `pub ...`, `impl ...`, `macro_rules!`, `#[...]`, `assert_eq!`, `format!`, `println!`, `match ... {`, `use ...::`).
    - **Score-padding `@Suppress`:** Annotations like `@Suppress("UNUSED_VARIABLE")`, `@Suppress("unused")`, or `@Suppress("FunctionName")` are detected and zero the score.
    - **Cheat comment patterns:** Phrases like `Kotlin:`, `(from impl ...)`, or Rust lifetime syntax (`'a`, `'static`) are flagged as comment contamination.

A repo that no longer has upstream Rust under `tmp/` and has no
`.ast_distance_config.json` is a mature Kotlin-first repo: it's
optimizing for idiomatic Kotlin, not against an upstream Rust oracle.
That's a state, not a "mode."

### Pure Kotlin Multiplatform only — never import a Java library

Every repo is strictly pure Kotlin on KMP. Never add a Java or JVM-only
dependency, never import `java.*` / `javax.*` in common code, and never reach
for platform crypto or IO when a port exists. The workspace's 200+ Rust ports
(`ls -d ../*-kotlin` from any repo) are a near-complete
Java replacement. Common needs and the port to use:

| Need | Use |
|---|---|
| Hashing, MAC, signatures | `sha1-kotlin`, `sha2-kotlin`, `sha3-kotlin`, `digest-kotlin`, `hmac-kotlin`, `ed25519-dalek-kotlin`, `p256-kotlin`, `crypto-box-kotlin`, `constant-time-eq-kotlin`, `zeroize-kotlin` |
| IO, buffers, files | `km-io` (`io.github.kotlinmania:km-io`; replaces okio and kotlinx-io), `bytes-kotlin`, `tempfile-kotlin`, `memmap2-kotlin`, `walkdir-kotlin`, `glob-kotlin` |
| Encoding, text | `base64-kotlin` (or stdlib `kotlin.io.encoding.Base64`), `encoding-rs-kotlin`, `unicode-*-kotlin`, `regex-kotlin`, `fancy-regex-kotlin` |
| Data formats | `serde-kotlin`, `serde-json-kotlin`, `serde-yaml-kotlin`, `toml-kotlin`, `csv-kotlin`, `quick-xml-kotlin`, `bincode-kotlin`, `prost-kotlin` |
| Compression, archives | `flate2-kotlin`, `zlib-kotlin`, `zstd-kotlin`, `zip-kotlin`, `tar-kotlin` |
| Time, ids, random | `chrono-kotlin`, `time-kotlin`, `uuid-kotlin`, `rand-kotlin`, `getrandom-kotlin` |
| Networking, async | `tokio-kotlin`, `http-kotlin`, `reqwest-kotlin`, `url-kotlin`, `rustls-kotlin`, `tungstenite-kotlin`, `futures-kotlin` |

If a needed capability has no port yet, port it (see below); do not paper
over the gap with a Java library or an `expect`/`actual` that delegates to
the JVM.

### When you can't port a Rust dependency: check kotlinmania siblings first

Before declaring a Rust dependency unportable, **check whether
kotlinmania already ships it.** The workspace has 224 `*-kotlin/`
repos, many of which are Maven Central publications. A "missing"
dependency is often a `*-kotlin/` sibling away. Order of attack:

1. **Look for a `<crate>-kotlin/` sibling repo** in the workspace.
   ```bash
   ls -d ../<crate>-kotlin 2>/dev/null
   ```
2. **Check if that sibling publishes to Maven Central.** Look at its
   `build.gradle.kts` for `publishAndReleaseToMavenCentral` wiring and
   confirm a recent release tag (`gh release list -R
   KotlinMania/<crate>-kotlin`). If yes, declare the Gradle dep:
   ```kotlin
   commonMain.dependencies {
       implementation("io.github.kotlinmania:<crate>-kotlin:<version>")
   }
   ```
3. **If the sibling exists but isn't on Maven Central**, that's the
   real blocker — port progress depends on publishing the sibling
   first. Memory has prior examples: `clap-complete-kotlin` blocked on
   `clap-kotlin` publish, `nucleo-kotlin` blocked on `nucleo-matcher`,
   `indexmap-kotlin` blocked on `hashbrown-kotlin` +
   `equivalent-kotlin`, `tonic-prost-kotlin` blocked on tonic + prost.
4. **If no sibling exists**, you have two options: port it in a new
   `*-kotlin/` repo, or note the genuine impossibility in the current
   repo's session report. Don't stub. Don't write a Kotlin
   "simulation" that tests a different invariant.

### Common sense applies

If something genuinely can't be ported exactly to the letter (mem::forget
semantics, pin-poll-once + Pin behavior `kotlinx.coroutines` can't
reproduce, custom `Drop`, raw pointer layout, `unsafe` UB checks), you
don't port it. Translate what *is* portable; leave a one-line honest
comment in the Kotlin position where the unportable code would have
gone, naming the specific Rust semantic that doesn't translate. Then
move on to the next item.

That's not the same as "I'll skip this." It's: translate as much as is
faithfully translatable, document the exact remainder, ship the rest.
The forbidden pattern is hiding the gap (stubbing, fake simulation,
`@Suppress`, deleting the test). Acknowledging the gap honestly is the
right answer.

### Rotate to the stalest repo

Don't keep gnawing the same handful of projects (regex-syntax-kotlin,
starlark-syntax-kotlin, codex-kotlin). Every `*-kotlin/` repo is in scope.

```bash
for d in ../*-kotlin ../klang; do
    [ -d "$d/.git" ] || continue
    ts=$(git -C "$d" log -1 --format=%ct 2>/dev/null) || continue
    printf '%s\t%s\t%s\n' "$ts" "$(date -r "$ts" +%Y-%m-%d)" "$(basename "$d")"
done | sort -n | head -10
```

Top of the list = where the rotation pointer goes. Skip a candidate only
for a *concrete* reason (dirty tree, hard external blocker, infra issue),
and note the reason in the run report. Don't silently fall back to the
same big repo as last time. Same rule per-file: if you've touched the
same file three runs in a row, pick a different file (or repo) first.

### Pick what to port next — Jira `ordered_priority` artifacts

Two artifacts at the workspace root answer "what should I port next?" without anyone needing to hit Jira directly: `PORT_PRIORITY.md` and `port_priority.json`.

For detailed information on the `ordered_priority` schema, Kahn-style layers, graph analysis metrics, and the catalog of scaffold scripts (`scaffold/analysis/*` and `scaffold/jira/*`), please refer to [JIRA_INTEGRATION.md](JIRA_INTEGRATION.md).

---

## 4. Swift Export rollout — the recipe and the four hazard classes

Every `*-kotlin` repo with `swiftExport { … }` carries a `SWIFT.md` file (which is the renamed `SWIFT_EXPORT_ROLLOUT.md`).

For detailed instructions on the 5-class sweep, the mandatory infrastructure pins, structural rename rules for Swift/Java emitted-name collisions, the strong-typing checklists, and the SAM interface / flat-class patterns, please refer to [SWIFT.md](SWIFT.md).

---

## 5. The build-gate — `build` must compile every current KotlinMania target

The build-gate contract in this file is the source of truth. Do not use a
separate build-gate document as source-of-truth.

### 5.5 Retired targets — workspace-wide, never re-add

Every repo declares exactly the retained target set, and
`fullTargetBuildTaskNames` mirrors the `kotlin { }` target block. Do not
re-add a retired target's declaration, source set, CI task, or workflow
step. Removing a retired target from a repo is policy alignment, not a
gate shrink.

**Retained:** JVM, Android, JS, Wasm JS, Wasm WASI, `linuxX64`,
`linuxArm64`, `mingwX64`, `androidNativeArm64`, `androidNativeX64`,
`iosArm64`, `iosSimulatorArm64`, `macosArm64`, `tvosArm64`,
`tvosSimulatorArm64`, `watchosDeviceArm64`, `watchosSimulatorArm64`.

#### 5.5.1 No 32-bit targets of any kind

`watchosArm32` (armv7k; retired 2026-05-24, Apple Watch armv7k EOL plus
the Mach-O 24-bit scattered relocation limit), `watchosArm64` (arm64_32,
a 32-bit pointer ABI), `linuxArm32Hfp`, and `mingwX86`. Recorded as
Sydney's standing policy on 2026-10-16: no 32-bit targets of any sort.

#### 5.5.2 No Apple x86_64 targets

`iosX64`, `macosX64`, `tvosX64`, `watchosX64`. Apple is dropping Intel
support and JetBrains has deprecated these targets. Apple Silicon device
and simulator targets cover every supported Apple platform.

#### 5.5.3 No 32-bit Android Native

`androidNativeArm32` and `androidNativeX86` (retired 2026-06-25). Android
Native keeps ARM64 and x64 only.

---

## 6. Test parity vs upstream Rust tests under `tmp/`

This is a translation workspace. **Every** upstream test gets an answer:

For each `tmp/**/tests/*.rs`, `tmp/**/benches/*.rs`, and `#[cfg(test)] mod
tests { … }` block:

- **If the test exercises behavior the Kotlin port can faithfully match,
  port it** to `src/commonTest/.../<File>Test.kt` (or per-target test
  source where commonTest can't express it). Use the upstream Rust file's
  organization; preserve test names translated to camelCase.
- **If the test depends on a Rust-specific semantic the Kotlin
  port can't reproduce** (`mem::forget`, pin-poll-once + `Pin` semantics
  that `kotlinx.coroutines` can't reproduce, custom `Drop`, raw pointer
  layout, `unsafe` UB checks), leave it unported with a one-line honest
  comment naming the specific semantic, **in the Kotlin test file at the
  position the port would go**, not in a separate document.
- **Never write a Kotlin "simulation" that tests a different invariant.**
  That's worse than skipping — it passes green while the real behavior
  isn't tested.

Wire `swift test` into the Kotlin `test` task so the Swift Export
boundary is covered by the same gate. Pattern (already in
schemars-kotlin / itertools-kotlin / kasuari-kotlin):

```kotlin
val swiftTest by tasks.registering(Exec::class) {
    dependsOn("embedSwiftExportForXcode")
    workingDir = file("swift-test-harness")
    commandLine("swift", "test", "--package-path", ".")
    // Treat any non-zero exit as a build failure.
    isIgnoreExitValue = false
}

tasks.named("test") {
    dependsOn(swiftTest)
}
```

If `swift-test-harness/` isn't present yet, the per-repo
`SWIFT_EXPORT_ROLLOUT.md` documents how to scaffold it — it's part of the
five canonical rollout changes.

---

## 7. Workflow-shape audit (paid-CI guard)

GitHub Actions across this org runs manual-only after the May 2026 cost
incident. Audit every focused repo's `.github/workflows/*.yml` at the
**start** of every session — even if another agent claims they already
fixed it.

The historical bad shape that must not regress:

```yaml
on:
  push:
    branches: [main]
  pull_request:
    branches: [main]
```

The quiet patterns to enforce:

| File | Triggers |
|---|---|
| `ci.yml` | `workflow_dispatch:` only |
| `codeql.yml` | weekly `schedule` + `workflow_dispatch:` |
| `publish.yml` | `release` `types: [released]` + `workflow_dispatch:` |
| `swift.yml`, `ios.yml`, `linux.yml`, `windows.yml`, `js.yml`, `macos.yml`, `tvos.yml`, `watchos.yml`, `android.yml`, `android-native.yml`, `wasm.yml`, `wasm-wasi.yml` | `workflow_call:` only |

When a workflow still has the old paid trigger AND is enabled remotely,
avoid spending a full CI run on the fix push: use
`gh workflow disable <workflow>`, push the trigger change, then
`gh workflow enable <workflow>`. If `gh workflow disable` is blocked,
report the exact blocker and still make the YAML change locally.

**Every session report must include a workflow-shape audit line per
focused repo:** which files were checked, whether any `push`/
`pull_request` triggers remain, whether any remote workflows were
disabled/re-enabled, and the commit that changed them.

---

## 8. Android SDK normalization (Gradle-backed, no shell)

For repos that target Android, we use a Gradle-backed SDK installer. For complete migration steps, import lists, local.properties logic, CodeQL `java-kotlin` extraction workflows, and verification commands, please refer to [ANDROID.md](ANDROID.md).

---

## 9. JS test-toolchain security patching

For JS npm warning mitigation, resolutions, vendored karma-webpack setups, and Dependabot vulnerability resolutions, please refer to [JS_SECURITY.md](JS_SECURITY.md).

### Automated yarn lock upgrades — do NOT run manually
`kotlinUpgradeYarnLock` and `kotlinWasmUpgradeYarnLock` are wired directly into `build.gradle.kts` as pre-requisite dependencies for all JS and WASM compile/test tasks (`jsTasksNeedingYarnLock`, `wasmTasksNeedingYarnLock`), `hostTests`, and `fullTargetBuildTaskNames` (`./gradlew build`).

**Never run `./gradlew kotlinUpgradeYarnLock` or `./gradlew kotlinWasmUpgradeYarnLock` manually.**
Running `./gradlew test` or `./gradlew build` automatically triggers lockfile upgrades before any compilation or testing begins. Running them manually is redundant and forbidden.


---

## 10. Kotlin/Native + Kotlin/JS gotchas

### `kotlin.native.parallelThreads=0` in `gradle.properties`

Every native compile/link step on `macos-latest` Apple-Silicon free tier
emits `w: The number of threads 4 is more than the number of processors
3`. With `allWarningsAsErrors=true` (which repo build files set),
this is a hard failure waiting on the next runner SKU bump.

```properties
kotlin.native.parallelThreads=0
```

`0` = "one thread per processor core" — adapts automatically; no number
to maintain. **Do not** scope `-Xbackend-threads=0` via `freeCompilerArgs`
on native targets — the Kotlin Gradle plugin appends its own
`-Xbackend-threads=4` to link tasks *after* user `freeCompilerArgs`, so
"last wins" gives 4 and the warning comes back.

### `js` / `wasmJs` source-set split

`browserMain` / `nodeMain` intermediate source sets are the structural
split that keeps Node-only code out of the webpack browser bundle.
Without it, `jsMain` is shared across both runtimes and webpack can't
resolve `fs`.

### `require('fs')` via `(new Function('return require'))()`

For `jsMain` / `wasmJsMain` `actual`s that call Node-only APIs through
`js("…")`, hide `require('fs')` from webpack's static scanner. **The
intuitive `eval('require')` form is WRONG** — webpack's `eval-source-map`
devtool mangles it at runtime with `SyntaxError: Unexpected token ':'`.

```kotlin
// jsMain — dynamic + IIFE return
private fun jsReadFile(path: String): dynamic = js(
    "(function(){ try { var rq = (new Function('return typeof require === \"function\" ? require : null'))(); if (!rq) return undefined; return rq('fs').readFileSync(path, 'utf-8'); } catch (e) { return undefined; } })()",
)

// wasmJsMain — typed return, body wrapped in { } because js(…) compiles to (args) => BODY
private fun jsReadFile(path: String): String? = js(
    "{ try { var rq = (new Function('return typeof require === \"function\" ? require : null'))(); if (!rq) return null; return rq('fs').readFileSync(path, 'utf-8'); } catch (e) { return null; } }",
)
```

Add an inline comment at the call site documenting the trick — it's easy
to re-trip in a "cleanup" commit.

### Parenthesised `DefinePlugin` substitutions in `karma.config.d/*.js`

Unparenthesised `{…}` at statement-start position parses as a block and
explodes `jsBrowserTest` with `Uncaught SyntaxError: Unexpected token ':'`.
Always wrap: `({ KEY: "value" })`.

### `posixMain` — avoid `size_t` in shared signatures

`fread` / similar break `posixMain` metadata compilation because
`size_t` width differs between the 32-bit native targets
(`androidNativeArm32`, `androidNativeX86` — `size_t = UInt`) and the
64-bit targets (`size_t = ULong`). Use `fgetc` / `Int`-returning
helpers or push the call to per-leaf-target Platform files.

*Historical note.* Every 32-bit native target is now retired (§5.5), so
no retained target has a 32-bit `size_t`. Keep shared signatures free of
`size_t` anyway: it keeps `posixMain` metadata portable and stops a
retired target from sneaking back in through a type mismatch.

### `*-sys-kotlin` ports — port real FFI, don't punt

Wire Kotlin/Native `cinterop` and (where applicable) Node N-API in the
same session. Never label FFI work "next pass."

### Default hierarchy template — `nativeMain` covers all native targets

The workspace uses `applyDefaultHierarchyTemplate()` (already in every
repo's `build.gradle.kts`). This auto-creates intermediate source sets
like `nativeMain`, `appleMain`, `iosMain`, `linuxMain`,
`androidNativeMain`, etc. based on which targets are declared.

For `expect`/`actual` declarations:

- An `actual fun` in `nativeMain` satisfies the `expect fun` for **all**
  native targets at once — no need to write one per target.
- If a function's implementation differs between platforms (e.g. Linux
  vs macOS), put that `actual fun` in the appropriate intermediate source
  set (`linuxMain`, `appleMain`, etc.) or the leaf target source set.
- The compiler matches `actual` declarations from the closest source set
  in the hierarchy. A `nativeMain` actual is used by all native targets
  unless a more specific source set overrides it.

This means the FFI layer for a `*-sys-kotlin` port (like `libc-kotlin`)
needs `actual fun` written once in `nativeMain` (delegating to
`platform.posix.*` or a `cinterop` C++ wrapper), not duplicated across
every individual native target.

For JS/WASM targets: use a Node N-API C++ addon (no insecure NPM
libraries). For JVM: JNA or JNI bindings. These are separate source
sets (`jsMain`/`wasmJsMain`/`jvmMain`) and do not share the
`nativeMain` actuals.

---

## 11. Branch + PR ops — the session shape

A normal session is:

1. **Audit + sync local main.** `git fetch --prune`. List local + remote
   branches, open PRs, `git status --short --branch`. If on `main` and
   behind `origin/main` (the common case after Sydney's manual PR sweeps
   landed merges remotely), sync local main:
   ```bash
   git switch main
   git pull --ff-only origin main
   ```
   `--ff-only` makes the command fail loud if local `main` has somehow
   diverged — which it never should under PR-only rules. Inspect every
   focused repo's workflow shape (§7).
2. **Reconcile every non-main branch** through §2: merge to main via
   no-ff, close PRs, delete remote refs proven ancestors of `main`,
   **undelete and reconcile** any `[gone]` branches with unmerged commits.
3. **Pick the focused repo.** Stalest first (§3). Read its `AGENTS.md`,
   `CLAUDE.md`, `README.md`, `SWIFT_EXPORT_ROLLOUT.md` if present.
4. **Workflow-shape, JS-toolchain security, Android SDK, build-gate
   audits** — repair any defect found locally, commit, push.
5. **Swift Export sweep** if `swiftExport { … }` is configured. Run
   `embedSwiftExportForXcode` + `swift test` locally; fix every hazard
   class from §4 in the same session.
6. **Test parity audit** (§6). Port what's missing in this session.
7. **Source-porting advance** per the repo's `CLAUDE.md`. Real Kotlin
   implementation/test changes, not workflow churn.
8. **Local gate.** `./gradlew build` (full) + `./gradlew test` (which
   includes `swift test` after wiring). Fix every failure.
9. **Commit ALL dirty (§1).** `git status --short` must be empty by
   end of session. Never `git stash`. Never orphan work on a switched
   branch. If dirty on `main`, branch first; if dirty on a feature
   branch, commit there.
10. **PR + merge via `gh`, never locally.** Push the branch, open PR
    with `gh pr create` using the canonical body, merge with
    `gh pr merge --merge --delete-branch <PR>` once checks are green.
    Add a one-sentence `gh pr comment` describing what was verified
    locally (build target, swift test, security gate). Never
    `git switch main && git merge` to land work.
11. **Version + release** if there's user-visible advance to publish:
    bump `version` in `build.gradle.kts` + every install snippet in
    `README.md`, commit, push, `gh release create v<X.Y.Z>` to fire
    the `release[released]` publish workflow.
12. **Report** (§12).

---

## 12. Session report — always at the end

Every session ends with a report that includes:

- **Workflow-shape audit** per focused repo (which files checked, what
  triggers remain, anything disabled/re-enabled).
- **Focused repo(s).**
- **Branch used.**
- **Constructive outcome** OR the **exact hard blocker** that prevented
  one (named at the level of §0).
- **PRs and branches** inspected, merged, or closed.
- **Local commands run and outcomes.**
- **Remote workflow state** if checked (run URL + conclusion for any
  platform job that's the only proof for a target the local host can't
  validate).
- **Commits made.**
- **Unresolved blockers.**
- **PR list at the very end** as full clickable GitHub URLs (not bare
  `#N`). If zero PRs, say `PRs: none this session` explicitly.

---

## 13. Quick reference — directories and tools

- Workspace root: the folder holding every `*-kotlin/` repo
- Per-repo: `<repo>-kotlin/`
- Automation artifacts: `automation-artifacts/`
- Canonical Swift Export rollout:
  `automation-artifacts/swift-export-rollout/` (apply.sh, blast.sh,
  PR_BODY.md, SWIFT_EXPORT_ROLLOUT.md, triage docs)
- Canonical Swift Export reference: `schemars-kotlin` (first non-trivial
  repo fully green) and `anstyle-kotlin` (canonical SWIFT_EXPORT_ROLLOUT.md)
- Canonical Android SDK normalization: `serial-test-kotlin` `dc29a78`,
  `anyhow-kotlin` `0fd90ee`
- Port priority: `PORT_PRIORITY.md` / `port_priority.json` (auto-generated;
  don't hand-edit)
- Generator: `python scaffold/analysis/generate_port_priority.py`
- Apply to Jira: `python scaffold/jira/apply_ordered_priority.py`
- Local CodeQL: `codeql` 2.25.4 on `PATH`
- Local Dependabot: `dependabot` 1.86.0 via Homebrew; needs
  `LOCAL_GITHUB_ACCESS_TOKEN=$(gh auth token)` env var

### Repo-state blockers carried forward (from memory)

- `clap-complete-kotlin` blocked on `clap-kotlin` publish (no Maven Central
  artifact yet at 0.1.0/0.1.1/0.1.2).
- `nucleo-kotlin` blocked on `nucleo-matcher-kotlin` (every `src/*.rs`
  imports from sibling crate; infra-only work still valid).
- `tonic-prost-kotlin` blocked on tonic-kotlin + prost-kotlin (both
  effectively empty).
- `indexmap-kotlin` needs `hashbrown-kotlin` + `equivalent-kotlin` (neither
  exists yet).
- `itertools-kotlin` needs `adaptors/{mod,coalesce,map}.rs` + `free.rs` +
  `lib.rs` ported before downstream files (`diff`, `cons_tuples`,
  `multipeek`); smallest leaf files all have unported deps.
- `tonic-prost-kotlin` codec needs Codec/Encoder/Decoder/EncodeBuf/
  DecodeBuf/BufferSettings/Status from tonic-kotlin + Message from
  prost-kotlin.
- `libwebrtc-kotlin` all-target build blocked on `bytes-kotlin` watchOS
  slices (no watchOS publication yet); use `macosArm64Test` as local
  proof until bytes-kotlin publishes watchOS.
- `constant-time-eq-kotlin` `src/lib.rs` is fully ported in `Lib.kt`;
  `tests/count_instructions.rs` and `benches/bench.rs` aren't portable.
  Don't retranslate.
- `env-logger-kotlin` writer module ported 2026-05-22; remaining work
  is `src/lib.rs`, `src/logger.rs`, `fmt/{mod,humantime,kv}.rs`.

### Authority files in this workspace (read order)

1. The workspace-root `AGENTS.md` (this file).
2. The workspace-root `CLAUDE.md` (legacy; this file
   is the merged authority).
3. Per-repo `AGENTS.md`, `CLAUDE.md`, `README.md`,
   `SWIFT_EXPORT_ROLLOUT.md`.
4. `automation-artifacts/swift-export-rollout/` for the rollout script
   bundle.
5. `automation-artifacts/2026-05-19-kotlinmania-ci-hourly-roster/RUNBOOK.md`
   for the persisted hourly runbook.
