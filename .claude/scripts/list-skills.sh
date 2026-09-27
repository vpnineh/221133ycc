#!/usr/bin/env bash
# Writes .claude/SKILLS.md: every skill installed in this session's container,
# grouped by the bundle it came from, with each skill's own one-line summary.
#
# Reads ~/.claude/skills-provenance.tsv, which setup-skills.sh records as it
# installs. Skills with no provenance row (claude-seo's, graphify's, anything
# installed by hand) land under "Other".
#
# Usage: bash .claude/scripts/list-skills.sh [output-path]
set -euo pipefail

SKILLS_DIR="${HOME}/.claude/skills"
# Claude Code ships its own skills into this directory. No script installs
# them, so CI — which starts from an empty HOME — would report them missing
# every run. Keep them out of the inventory.
BUILTIN='^(session-start-hook)$'
PROV="${HOME}/.claude/skills-provenance.tsv"
OUT="${1:-${CLAUDE_PROJECT_DIR:-.}/.claude/SKILLS.md}"

# First sentence of the skill's description, flattened to one line. Frontmatter
# descriptions are often a paragraph or a long trigger list, so cut at the first
# sentence end and cap the length.
summarize() {
    awk '
        /^---[[:space:]]*$/ { d++; next }
        d == 1 && /^description:/ {
            sub(/^description:[[:space:]]*/, ""); gsub(/^[>|][-+]?[[:space:]]*/, "")
            gsub(/^"/, ""); buf = $0; collecting = 1; next
        }
        d == 1 && collecting && /^[[:space:]]+[^[:space:]]/ { sub(/^[[:space:]]+/, " "); buf = buf $0; next }
        d == 1 && collecting { collecting = 0 }
        END { print buf }
    ' "$1" | sed 's/"$//' | cut -c1-200 | sed 's/\([.!?]\) [A-Z].*/\1/' | sed 's/[[:space:]]*$//'
}

count=$(find "${SKILLS_DIR}" -maxdepth 1 -mindepth 1 ! -name synced -xtype d -printf '%f\n' | grep -Evc "${BUILTIN}")

{
    echo "# Installed skills"
    echo
    echo "${count} skills, provisioned into every session on this repo by the"
    echo "SessionStart hooks in \`.claude/settings.json\`. Regenerate with"
    echo "\`bash .claude/scripts/list-skills.sh\`."
    echo
    echo "Account-level skills (uploaded at claude.ai, so they reach every chat"
    echo "rather than only Claude Code) are listed separately at the end."
    echo

    # Bundles in the order setup-skills.sh installs them, then the rest.
    sources=$(awk -F'\t' '{print $2}' "${PROV}" 2>/dev/null | awk '!seen[$0]++')

    for src in ${sources}; do
        members=$(awk -F'\t' -v s="${src}" '$2 == s {print $1}' "${PROV}" | sort -u)
        [ -z "${members}" ] && continue
        printf '## %s\n\n' "${src}"
        for m in ${members}; do
            f="${SKILLS_DIR}/${m}/SKILL.md"
            [ -f "${f}" ] || continue
            printf -- '- **%s** — %s\n' "${m}" "$(summarize "${f}")"
        done
        echo
    done

    # Anything without a provenance row.
    others=$(comm -23 \
        <(find "${SKILLS_DIR}" -maxdepth 1 -mindepth 1 ! -name synced -xtype d -printf '%f\n' | grep -Ev "${BUILTIN}" | sort -u) \
        <(awk -F'\t' '{print $1}' "${PROV}" 2>/dev/null | sort -u))
    if [ -n "${others}" ]; then
        printf '## Other\n\n'
        printf 'claude-seo (its own hook), graphify, and anything installed by hand.\n\n'
        for m in ${others}; do
            f="${SKILLS_DIR}/${m}/SKILL.md"
            [ -f "${f}" ] || continue
            printf -- '- **%s** — %s\n' "${m}" "$(summarize "${f}")"
        done
        echo
    fi

    cat <<'ACCOUNT'
## Account level (every chat, not just Claude Code)

Uploaded as skills at claude.ai, so they load in plain chats too. Not managed
by these hooks.

- **typesafe-ai** — building with TypeSafe's System One models
- **graphify** — codebase knowledge graph
- **karpathy-guidelines** — behavioural guardrails against common LLM coding mistakes
- **caveman** — ultra-compressed output mode
- **caveman-review** — one-line-per-finding code review
ACCOUNT
} > "${OUT}"

echo "wrote ${OUT} (${count} skills)"
