# Agent toolkit provisioning

Two SessionStart hooks provision this session container, which is ephemeral and
rebuilt on every web session:

- `scripts/setup-claude-seo.sh` — claude-seo (see `CLAUDE-SEO-WEB.md`)
- `scripts/setup-skills.sh` — skill bundles and global CLIs

Both are idempotent. A warm session costs milliseconds; a cold one is about a
minute for claude-seo and a few minutes for the bundles.

## Installed

| Source | Skills | Notes |
| --- | --- | --- |
| `ibelick/ui-skills` | 7 | UI audit and cleanup passes |
| `vercel-labs/agent-skills` | 9 | React, Next.js, Vercel deploy |
| `pbakaus/impeccable` | 1 | Frontend design review |
| `nextlevelbuilder/ui-ux-pro-max-skill` | 7 | Design systems, brand, slides |
| `upstash/context7` | 3 | Library docs lookup |
| `anthropics/skills` | 1 | `frontend-design` only |
| `coreyhaines31/marketingskills` | 50 | Marketing playbooks |
| `AgriciDaniel/claude-seo` | 30 | Via the other hook |

Global CLIs: `@playwright/cli`, `ruflo`.

Roughly 109 skills. Their descriptions add about 17 KB to the start of every
session — real, but acceptable. Trim `BUNDLES` in `setup-skills.sh` if that
budget matters more than the coverage.

## Deliberately not installed

**`seoskillsai/seo-skills-ai`** — 24 of its skill names collide exactly with
claude-seo's (`seo-audit`, `seo-schema`, `seo-technical`, `seo-geo`, …).
Different authors, same names, different content. Installing both makes which
one answers a coin flip. Pick one; this repo picked claude-seo.

**`podo/design-agent-skills`** — a catalogue of 151 skills, from its README:
"Skills install on demand — the catalogue is a lightweight index, not a bulk
download." Browse it with `npx design-agent-skills` and add individual skills.

**`usestrix/strix`** — needs a running Docker daemon. Docker is installed in
this image but the daemon is not running, so Strix cannot start its sandbox.
It also needs `STRIX_LLM` and `LLM_API_KEY`.

## Blocked by the egress policy

`context7.com` and `supabase.com` both return 403 at the proxy, so the Context7
and Supabase MCP servers cannot reach their backends from here. The Context7
*skills* are installed and describe the workflow, but `find-docs` will fail at
the fetch step. Both work normally on a local machine.

MCP servers are configured per-client anyway (claude.ai connector settings, or
a `.mcp.json`), not by these scripts.
