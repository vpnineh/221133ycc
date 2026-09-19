#!/usr/bin/env bash
# Installs the agent skill bundles and global CLIs into the session container.
# Web containers are ephemeral, so this re-runs on every session start.
#
# Idempotent: bundles already present are skipped, so a warm session is fast.
#
# Deliberately NOT installed, see .claude/AGENT-TOOLKIT.md for the reasoning:
#   seoskillsai/seo-skills-ai   - 24 skill-name collisions with claude-seo
#   podo/design-agent-skills    - a 151-skill catalogue, meant to be on-demand
#   usestrix/strix              - needs a running Docker daemon
set -euo pipefail

SKILLS_DIR="${HOME}/.claude/skills"

log() { echo "[skills] $*"; }

# bundle spec: "marker-skill-dir|npx-target|extra-args"
BUNDLES=(
  "improve-ui|ibelick/ui-skills|"
  "web-design-guidelines|vercel-labs/agent-skills|"
  "impeccable|pbakaus/impeccable|"
  "ui-ux-pro-max|nextlevelbuilder/ui-ux-pro-max-skill|"
  "find-docs|upstash/context7|"
  "frontend-design|anthropics/skills|--skill frontend-design"
  "copywriting|coreyhaines31/marketingskills|"
  "caveman-explore|JuliusBrussee/caveman|"
)

for spec in "${BUNDLES[@]}"; do
    IFS='|' read -r marker target extra <<< "${spec}"
    if [ -d "${SKILLS_DIR}/${marker}" ]; then
        continue
    fi
    log "installing ${target}..."
    # shellcheck disable=SC2086
    npx -y skills add "${target}" ${extra} -g >/dev/null 2>&1 || \
        log "WARN: ${target} did not install cleanly"
done

# marketingskills ships a skill literally named seo-audit, which lands on the
# same path as claude-seo's orchestrator of the same name. Whichever installs
# last silently wins. Rename the marketing one so both survive; this hook runs
# before the claude-seo hook, so claude-seo then installs its own cleanly.
MK="${SKILLS_DIR}/seo-audit"
if [ -f "${MK}/SKILL.md" ] && ! grep -q "subagent delegation" "${MK}/SKILL.md"; then
    log "renaming marketingskills' seo-audit -> marketing-seo-audit"
    rm -rf "${SKILLS_DIR}/marketing-seo-audit"
    mv "${MK}" "${SKILLS_DIR}/marketing-seo-audit"
    sed -i '0,/^name: seo-audit$/s//name: marketing-seo-audit/' \
        "${SKILLS_DIR}/marketing-seo-audit/SKILL.md"
fi

# caveman's core `caveman` and `caveman-review` are already uploaded as
# account-level skills, which reach every chat rather than just Claude Code
# sessions. Drop the local copies so the same trigger does not fire twice.
for dup in caveman caveman-review; do
    rm -rf "${SKILLS_DIR:?}/${dup}"
done

# Global CLIs. playwright-cli ships its own skill; ruflo is an agent
# orchestration CLI.
for pkg in "@playwright/cli:playwright-cli" "ruflo:ruflo"; do
    name="${pkg%%:*}"; bin="${pkg##*:}"
    command -v "${bin}" >/dev/null 2>&1 && continue
    log "installing ${name}..."
    npm install -g "${name}@latest" >/dev/null 2>&1 || log "WARN: ${name} failed"
done

# Routing guidance. With ~110 skills installed, several cover overlapping
# ground and the description text alone does not separate them. This lands at
# user scope so it applies to every project in the container, not just this
# repo. Regenerated each run; the container is ephemeral anyway.
cat > "${HOME}/.claude/CLAUDE.md" <<'ROUTING'
# Skill routing

Roughly 110 skills are installed here and several overlap. Pick by the shape of
the task, not by keyword match on the skill name. When two fit, prefer the
narrower one.

## Frontend and UI

| Task | Skill |
| --- | --- |
| Build a site or app surface from scratch, production quality | `web-pro` |
| Aesthetic direction for new UI, avoiding templated defaults | `frontend-design` |
| Broad design critique or redesign of an existing interface | `impeccable` |
| Audit an existing surface, read-only, produce a plan | `improve-ui` |
| Fast polish pass: spacing, hierarchy, typography | `baseline-ui` |
| Pick palettes, font pairings, styles from a local dataset | `ui-ux-pro-max` |
| Implement with shadcn/ui + Tailwind | `ui-styling` |
| Review against the Web Interface Guidelines checklist | `web-design-guidelines` |
| Accessibility: ARIA, keyboard, focus, contrast | `fixing-accessibility` |
| Janky animation, layout thrashing, scroll-linked motion | `fixing-motion-performance` |
| Titles, meta descriptions, Open Graph, canonical, JSON-LD | `fixing-metadata` |

`web-pro` is the widest. Reach for it when building; reach for the specific
audit skills when reviewing.

## React and Next.js

`vercel-react-best-practices` for performance patterns,
`vercel-composition-patterns` for component API design,
`vercel-react-view-transitions` for transition animations,
`vercel-react-native-skills` for Expo and React Native,
`vercel-optimize` for Vercel cost and runtime metrics on a deployed project.

## SEO — two separate systems, do not mix

**claude-seo** (`seo`, `seo-audit`, `seo-technical`, `seo-schema`, `seo-geo`,
`seo-page`, …) is the tooling system: it runs Python, crawls, and delegates to
`seo-*` subagents. Use it for analysis of a real site.

**marketingskills** is advisory prose with no tooling. Its SEO entries are
`marketing-seo-audit` (renamed to avoid clobbering claude-seo's `seo-audit`),
`schema`, `ai-seo`, `programmatic-seo`, `site-architecture`.

Analysing a live URL → claude-seo. Deciding strategy, or no crawling possible
→ marketingskills.

In this environment claude-seo cannot fetch URLs (the SSRF guard refuses the
loopback egress proxy). Fetch the page with WebFetch and hand the content over,
or fall back to the advisory skills.

## Marketing

About 50 skills, each narrow and well-named — `pricing`, `cold-email`, `cro`,
`churn-prevention`, `ads`, `ad-creative`, and so on. Their descriptions
cross-reference each other; follow those pointers. Start at
`product-marketing` on a new project: it writes the shared context file the
others read. `marketing-council` gives several expert takes on one question.

## Caveman — token compression

`caveman` and `caveman-review` are installed at account level, so they work in
every chat. Trigger them on request ("caveman mode", "be brief", "fewer
tokens"), not unprompted — compressed prose is a deliberate trade, not a
default.

The repo's other skills are Claude Code only and mostly need its CLI or cloud
proxy: `caveman-setup`, `caveman-stats`, `caveman-manage`, `cavecrew`. The
pure-prose ones — `investigate-first`, `safe-refactor`, `surgical-patch`,
`verify-and-stop`, `lean-build` — say much the same as
`karpathy-guidelines`; one of them is enough, do not stack them.

## Docs lookup

`find-docs` / `context7-mcp` fetch current library documentation. Prefer them
over training data for API signatures and config. In this environment
`context7.com` is blocked by the egress policy, so they fail at the fetch step
— say so rather than answering from memory.
ROUTING

log "done: $(ls "${SKILLS_DIR}" | grep -cv '^synced$') skills available, routing guide written"
