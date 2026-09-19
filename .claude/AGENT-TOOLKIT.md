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
| `AgriciDaniel/claude-seo` | 31 |

Global CLIs: `@playwright/cli`, `ruflo`.

128 skills. Their descriptions plus the routing guide add about 21 KB
(~5,200 tokens) to the start of every session. Trim `BUNDLES` in
`setup-skills.sh` if that budget matters more than the coverage.

## Two collisions the scripts resolve

**`seo-audit`** — marketingskills and claude-seo both ship a skill with that
exact name, and they land on the same path. Whichever installs last silently
wins; marketingskills did, which cost us claude-seo's orchestrator. The skills
hook now renames the marketing one to `marketing-seo-audit` and runs before the
claude-seo hook, so both survive. Do not reorder the hooks.

**`caveman` / `caveman-review`** — already uploaded as account-level skills,
which reach every chat rather than only Claude Code sessions. The hook deletes
the local copies so one trigger does not fire two skills.

## Routing

`setup-skills.sh` writes `~/.claude/CLAUDE.md`, a disambiguation guide for the
overlapping skills — six cover frontend design alone, and two separate SEO
systems are installed side by side. Edit it in the heredoc at the bottom of
the script, not in place; it is regenerated every run.

## Deliberately not installed

**`seoskillsai/seo-skills-ai`** — 24 of its skill names collide exactly with
claude-seo's (`seo-audit`, `seo-schema`, `seo-technical`, `seo-geo`, …).
Different authors, same names, different content. Pick one; this repo picked
claude-seo.

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
