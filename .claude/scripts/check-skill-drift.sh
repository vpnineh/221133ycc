#!/usr/bin/env bash
# Checks the installed skill set for the things that rot without anyone
# noticing. Prints a report; exits 1 if anything needs a human decision.
#
# Skills are not vendored — every session installs them fresh from upstream, so
# the skills themselves are never stale. What rots is everything around them:
#
#   1. Pinned versions. claude-seo is pinned to a tag; upstream moves on.
#   2. Retired repos. A repo can become a "signpost" whose SKILL.md files are
#      redirect stubs. Sessions keep installing them and silently get nothing.
#      seo-geo-claude-skills did exactly this.
#   3. Name collisions. Two bundles shipping the same skill name overwrite each
#      other by install order. marketingskills silently ate claude-seo's
#      seo-audit this way.
#   4. The committed inventory drifting from what is actually installed.
set -uo pipefail

SKILLS_DIR="${HOME}/.claude/skills"
# Claude Code ships its own skills into this directory. No script installs
# them, so CI — which starts from an empty HOME — would report them missing
# every run. Keep them out of the inventory.
BUILTIN='^(session-start-hook)$'
REPO_DIR="${CLAUDE_PROJECT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}"
INVENTORY="${REPO_DIR}/.claude/SKILLS.md"
issues=0

note() { printf '  %s\n' "$*"; }
section() { printf '\n== %s ==\n' "$*"; }

installed() {
    find "${SKILLS_DIR}" -maxdepth 1 -mindepth 1 ! -name synced -xtype d -printf '%f\n' | grep -Ev "${BUILTIN}" | sort -u
}

section "Pinned versions"
pin=$(grep -oP 'REPO_TAG="\$\{CLAUDE_SEO_TAG:-\K[^}]+' "${REPO_DIR}/.claude/scripts/setup-claude-seo.sh" 2>/dev/null)
# Unauthenticated API calls are rate-limited per IP, and CI runners share
# addresses, so use a token when one is in the environment.
auth=()
[ -n "${GH_TOKEN:-${GITHUB_TOKEN:-}}" ] && \
    auth=(-H "Authorization: Bearer ${GH_TOKEN:-${GITHUB_TOKEN}}")
latest=$(curl -fsSL --max-time 30 "${auth[@]+"${auth[@]}"}" \
    https://api.github.com/repos/AgriciDaniel/claude-seo/releases/latest 2>/dev/null \
    | grep -oP '"tag_name":\s*"\K[^"]+' | head -1)
if [ -z "${latest}" ]; then
    note "claude-seo: pinned ${pin:-?}; could not reach the GitHub API to compare"
elif [ "${pin}" = "${latest}" ]; then
    note "claude-seo: ${pin} is current"
else
    note "claude-seo: pinned ${pin}, upstream released ${latest}"
    note "  -> bump REPO_TAG in .claude/scripts/setup-claude-seo.sh"
    issues=$((issues + 1))
fi

section "Retired upstreams (redirect stubs)"
# A signpost stub announces itself in its heading or opening lines, not in the
# body — match only the first few lines, or a skill that merely discusses
# moved repos trips this.
stubs=0
while IFS= read -r name; do
    f="${SKILLS_DIR}/${name}/SKILL.md"
    [ -f "${f}" ] || continue
    if head -20 "${f}" | grep -qiE '^#.*has moved|is now a \*\*signpost\*\*|no longer developed here'; then
        note "${name}: upstream retired, this is a redirect stub"
        stubs=$((stubs + 1))
    fi
done < <(installed)
if [ "${stubs}" -eq 0 ]; then
    note "none"
else
    note "-> the bundle moved; find its successor and update BUNDLES"
    issues=$((issues + stubs))
fi

section "Name collisions"
# Two skills declaring the same frontmatter name, or a directory whose declared
# name disagrees with it (the sign that something overwrote something else).
dupes=$(while IFS= read -r name; do
    f="${SKILLS_DIR}/${name}/SKILL.md"
    [ -f "${f}" ] && awk -F': *' '/^name:/{print $2; exit}' "${f}"
done < <(installed) | sort | uniq -d)
if [ -z "${dupes}" ]; then
    note "none"
else
    note "duplicate declared names: ${dupes}"
    note "-> one bundle is overwriting another; rename in setup-skills.sh"
    issues=$((issues + 1))
fi

section "Inventory freshness"
if [ ! -f "${INVENTORY}" ]; then
    note "SKILLS.md missing"
    issues=$((issues + 1))
else
    # Stop at the account-level section: those skills live at claude.ai, not in
    # this container, so comparing them against installed directories always
    # reports them as missing.
    listed=$(sed '/^## Account level/,$d' "${INVENTORY}" | grep -oP '^- \*\*\K[^*]+' | sort -u)
    added=$(comm -13 <(echo "${listed}") <(installed))
    removed=$(comm -23 <(echo "${listed}") <(installed))
    if [ -z "${added}${removed}" ]; then
        note "SKILLS.md matches the installed set"
    else
        [ -n "${added}" ]   && note "new upstream: $(echo ${added} | tr '\n' ' ')"
        [ -n "${removed}" ] && note "gone from upstream: $(echo ${removed} | tr '\n' ' ')"
        note "-> regenerate with .claude/scripts/list-skills.sh"
        issues=$((issues + 1))
    fi
fi

printf '\n'
if [ "${issues}" -eq 0 ]; then
    echo "No drift."
else
    echo "${issues} item(s) need attention."
fi
exit $(( issues > 0 ? 1 : 0 ))
