# Agent toolkit provisioning

Two SessionStart hooks provision every session opened on this repo. The
container is ephemeral and rebuilt each time, so both re-run on every start.
They run in this order, which matters — see the `seo-audit` note below.

1. `scripts/setup-skills.sh` — skill bundles, global CLIs, routing guide
2. `scripts/setup-claude-seo.sh` — claude-seo (see `CLAUDE-SEO-WEB.md`)

Both are idempotent. Measured: a warm session is ~7ms, a cold one 2m6s.

## Installed

| Source | Skills |
| --- | --- |
| `ibelick/ui-skills` | 7 |
| `vercel-labs/agent-skills` | 9 |
| `pbakaus/impeccable` | 1 |
| `nextlevelbuilder/ui-ux-pro-max-skill` | 7 |
| `upstash/context7` | 3 |
| `anthropics/skills` (`frontend-design`) | 1 |
| `coreyhaines31/marketingskills` | 50 |
| `JuliusBrussee/caveman` | 17 |
| `emilkowalski/skills` | 13 |
| `aaron-he-zhu/aaron-marketing-skills` (seo-geo arm only) | 16 |
| `AgriciDaniel/claude-seo` (v2.4.0) | 33 |
| `Graphify-Labs/graphify` (PyPI `graphifyy`) | 1 |

Global CLIs: `@playwright/cli`, `ruflo`, `graphify`.

159 skills, listed with summaries in `SKILLS.md`. Their descriptions plus the
routing guide add about 30 KB (~7,400 tokens) to the start of every session. Trim `BUNDLES` in
`setup-skills.sh` if that budget matters more than the coverage.

## Two collisions the scripts resolve

**`seo-audit`** — marketingskills and claude-seo both ship a skill with that
exact name, and they land on the same path. Whichever installs last silently
wins; marketingskills did, which cost us claude-seo's orchestrator. The skills
hook now renames the marketing one to `marketing-seo-audit` and runs before the
claude-seo hook, so both survive. Do not reorder the hooks.

The two are told apart by the `author: AgriciDaniel` metadata field, never by
description text. The first version grepped for "subagent delegation"; claude-seo
v2.4.0 rewrote that description, and on the next resume the hook took
claude-seo's orchestrator for the marketing one, deleted the real
`marketing-seo-audit` and moved claude-seo's into its place. Authorship is
stable across releases; prose is not.

The claude-seo hook also treats a `seo-audit` it does not own as unhealthy and
reinstalls, so an overwrite heals on the next session instead of persisting —
`claude-seo doctor` checks the runtime, not the skill files, and would report
healthy either way.

**`caveman` / `caveman-review`** — already uploaded as account-level skills,
which reach every chat rather than only Claude Code sessions. The hook deletes
the local copies so one trigger does not fire two skills.

## graphify — always on

`setup-skills.sh` installs the `graphify` CLI (`uv tool install graphifyy`),
then runs `graphify install` **after** writing the routing guide, because the
guide's heredoc rewrites `~/.claude/CLAUDE.md` and would drop graphify's
always-on block. It then builds a code graph of the session's project
(`graphify update .`, tree-sitter, no LLM, a few seconds) into
`graphify-out/`, which is gitignored.

The PreToolUse hooks that make Claude use the graph are in
`.claude/settings.json`, not in the script: Claude Code reads hooks once at
startup, before SessionStart hooks run. `Bash|Grep` gets a nudge to run
`graphify query` first; `Read|Glob` runs in strict mode, which blocks the
first raw source read of a session until one query has run, then only nudges.
Set `GRAPHIFY_HOOK_STRICT=0` to keep only the nudge. Both hooks are silent
no-ops when graphify is missing or the project has no graph.

## Inventory

`scripts/list-skills.sh` regenerates `SKILLS.md`: every installed skill grouped
by the bundle it came from, with its own one-line summary. It reads
`~/.claude/skills-provenance.tsv`, which the bundle loop records as it installs,
so the grouping is observed rather than hardcoded.

`npx skills add` installs skills as symlinks, not directories — the generator
uses `-xtype d`, and anything walking that tree must too.

## Routing

`setup-skills.sh` writes `~/.claude/CLAUDE.md`, a disambiguation guide for the
overlapping skills — six cover frontend design alone, and two separate SEO
systems are installed side by side. Edit it in the heredoc at the bottom of
the script, not in place; it is regenerated every run.

## CI-only failure modes

The drift workflow starts from an empty HOME without Claude Code installed.
Three things that never happen in a session break there, all now handled:

- `npx skills add` picks target agents by detecting installed binaries. With no
  `claude` on PATH it installs everywhere except `~/.claude/skills`. Hence
  `-a claude-code -y` on every call — the first CI run landed 1 skill of 125.
- The CLI exits 0 even on `Invalid agents`, so its status proves nothing. The
  hook checks that each bundle's marker skill exists afterwards, and with
  `SKILLS_STRICT=1` (set in CI) a missing one fails the job.
- `ls` exits 2 on a missing path, and under `pipefail` that kills a script from
  inside an innocent-looking `$(ls … | head)`. Neither script pipes `ls` any
  more.

## Daily drift check

`.github/workflows/skills-update.yml` runs at 04:17 UTC and opens one PR only
when something changed.

Skills are not vendored, so they are never stale — every session installs them
fresh. What the workflow catches is everything around them, via
`scripts/check-skill-drift.sh`:

- the claude-seo version pin, which it bumps when upstream tags a release
- upstreams that retired into redirect stubs, the `seo-geo-claude-skills`
  failure mode, where sessions keep installing and silently get nothing
- new name collisions between bundles, the `seo-audit` failure mode
- `SKILLS.md` drifting from what actually installs

The drift script's inventory check stops at the `## Account level` heading:
those skills live at claude.ai, not in the container, so comparing them against
installed directories reports them missing every time.

## Session-start token cost

About 4,400 tokens, and it does not scale with how big the skills are — only
with how many there are. Claude Code loads each skill's frontmatter
(name plus description) so it knows what exists, and loads a body only when
that skill is actually invoked. Measured here: 17 KB of frontmatter against
358 KB of SKILL.md bodies and 703 MB of the whole tree.

So vendoring the skills into this repo would not lower the cost — the same
descriptions would still load, from a different path. The only lever is
installing fewer skills: trim `BUNDLES`.

## Deliberately not installed

**`seoskillsai/seo-skills-ai`** — 24 skill names collide exactly with
claude-seo's, and on every shared skill its SKILL.md is far thinner:
`seo-technical` 1.6 KB vs 17.3 KB, `seo-geo` 1.1 KB vs 19.4 KB, `seo-local`
1.1 KB vs 17.2 KB. Same repo layout as claude-seo (install.sh, agents/,
extensions/, hooks/, schema/), so it reads as a lighter reimplementation.
Nothing to gain by swapping.

**`aaron-he-zhu/seo-geo-claude-skills`** — retired by its author. All 20 of its
SKILL.md files are redirect stubs ("this repo is now a signpost") pointing at
`aaron-he-zhu/aaron-marketing-skills`. Installing it would add 20 stubs. The
successor's seo-geo arm is installed instead.

**The other 104 skills in `aaron-marketing-skills`** — ad, email, influencer,
launch, narrative and social arms. Substantive, but they cover the same ground
as the 50 marketingskills already installed, and taking them would roughly
double the session-start cost. Add individual ones with
`npx skills add aaron-he-zhu/aaron-marketing-skills -s <name> -g`.

**`podo/design-agent-skills`** — a catalogue of 151 skills. From its README:
"Skills install on demand — the catalogue is a lightweight index, not a bulk
download." Browse with `npx design-agent-skills` and add individual skills.

**`usestrix/strix`** — needs a running Docker daemon. Docker is in the image
but the daemon is not running, so Strix cannot start its sandbox. It also
needs `STRIX_LLM` and `LLM_API_KEY`.

## Blocked by the egress policy

`context7.com` and `supabase.com` both return 403 at the proxy, so the Context7
and Supabase MCP servers cannot reach their backends. The Context7 skills are
installed and describe the workflow, but `find-docs` fails at the fetch step.
Both work normally on a local machine.

MCP servers are configured per-client anyway (claude.ai connector settings, or
a `.mcp.json`), not by these scripts.

## Other repos

These hooks only cover sessions opened on this repo. To get the same setup in
every web session regardless of repository, paste the combined script into the
environment setup script field at claude.ai/code — build it by concatenating
the two scripts below their shared header.
