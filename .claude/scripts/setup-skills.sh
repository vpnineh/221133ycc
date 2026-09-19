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

# Global CLIs. playwright-cli ships its own skill; ruflo is an agent
# orchestration CLI.
for pkg in "@playwright/cli:playwright-cli" "ruflo:ruflo"; do
    name="${pkg%%:*}"; bin="${pkg##*:}"
    command -v "${bin}" >/dev/null 2>&1 && continue
    log "installing ${name}..."
    npm install -g "${name}@latest" >/dev/null 2>&1 || log "WARN: ${name} failed"
done

log "done: $(ls "${SKILLS_DIR}" | grep -cv '^synced$') skills available"
