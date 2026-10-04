#!/usr/bin/env bash
# Installs claude-seo into the session's ~/.claude and wires up the
# pre-installed Chromium, because Claude Code on the web cannot reach
# cdn.playwright.dev to download its own.
#
# Idempotent: re-running on an already-provisioned session is a no-op.
set -euo pipefail

SKILL_DIR="${HOME}/.claude/skills/seo"
REPO_TAG="${CLAUDE_SEO_TAG:-v2.4.1}"
PW_DIR="${PLAYWRIGHT_BROWSERS_PATH:-/opt/pw-browsers}"

log() { echo "[claude-seo] $*"; }

# Healthy is not enough to skip: a resumed session keeps its container, so
# without the version check it would keep the old release forever after the
# pin is bumped. Skip only when the installed version is the pinned one.
installed_ver=$(grep -oP '"version":\s*"\K[^"]+' "${SKILL_DIR}/runtime-plugin.json" 2>/dev/null || true)
# Also require that seo-audit is still ours. Another bundle ships a skill of the
# same name onto the same path; if it has overwritten ours, `doctor` still says
# healthy — it checks the runtime, not the skill files — and we would never put
# the orchestrator back.
ours_audit=0
grep -qE '^[[:space:]]+author:[[:space:]]*"?AgriciDaniel' \
    "${HOME}/.claude/skills/seo-audit/SKILL.md" 2>/dev/null && ours_audit=1
if [ -x "${SKILL_DIR}/scripts/claude-seo" ] && \
   [ "${installed_ver}" = "${REPO_TAG#v}" ] && \
   [ "${ours_audit}" = "1" ] && \
   "${SKILL_DIR}/scripts/claude-seo" doctor 2>/dev/null | grep -q "Chromium: ready"; then
    log "${REPO_TAG} already installed and healthy"
    exit 0
fi
if [ -n "${installed_ver}" ] && [ "${installed_ver}" != "${REPO_TAG#v}" ]; then
    log "upgrading ${installed_ver} -> ${REPO_TAG#v}"
fi

log "installing ${REPO_TAG}..."
TMP=$(mktemp -d)
trap 'rm -rf "${TMP}"' EXIT
git clone --depth 1 --branch "${REPO_TAG}" \
    https://github.com/AgriciDaniel/claude-seo.git "${TMP}/claude-seo" >/dev/null 2>&1

# install.sh re-clones the pinned tag itself; the Chromium step is expected to
# fail here and is repaired below, so a non-zero exit is not fatal.
CLAUDE_SEO_TAG="${REPO_TAG}" bash "${TMP}/claude-seo/install.sh" >/dev/null 2>&1 || true

if [ ! -x "${SKILL_DIR}/scripts/claude-seo" ]; then
    log "ERROR: install failed, skill dir missing" >&2
    exit 1
fi

# Point claude-seo's private browser dir at the image's Chromium. Playwright
# resolves a build number that will not match the image's, so derive the
# expected name from playwright itself rather than hardcoding it.
log "wiring up pre-installed Chromium..."
VENV_PY="${SKILL_DIR}/.venv/bin/python"
# Match the build number by name rather than by path depth: indexing a fixed
# path component only worked while PW_DIR was exactly /opt/pw-browsers, and
# returned the directory name ("pw") anywhere else.
BUILD=$("${VENV_PY}" -c "
import re
from playwright.sync_api import sync_playwright
with sync_playwright() as p:
    m = re.search(r'/chromium(?:_headless_shell)?-(\d+)/', p.chromium.executable_path)
    print(m.group(1) if m else '')
" 2>/dev/null) || BUILD=""

# No pre-installed browser at all (a CI runner, a bare image) is a normal case,
# not an error. Under pipefail an unmatched `ls` exits 2 and kills the script,
# so look for the directory first instead of piping ls.
HAVE=""
for d in "${PW_DIR}"/chromium-*; do
    [ -d "${d}" ] && HAVE="${d##*-}" && break
done

if [ -n "${BUILD}" ] && [ -n "${HAVE}" ]; then
    # Chrome-for-testing layout (what new Playwright expects) mapped onto the
    # older chrome-linux layout the image ships.
    for kind in chromium chromium_headless_shell; do
        src="${PW_DIR}/${kind}-${HAVE}/chrome-linux"
        [ -d "${src}" ] || continue
        if [ "${kind}" = "chromium" ]; then
            dest="${PW_DIR}/${kind}-${BUILD}/chrome-linux64"
        else
            dest="${PW_DIR}/${kind}-${BUILD}/chrome-headless-shell-linux64"
        fi
        mkdir -p "${dest}"
        for f in "${src}"/*; do ln -sfn "${f}" "${dest}/$(basename "${f}")"; done
        [ "${kind}" = "chromium_headless_shell" ] && \
            ln -sfn "${src}/headless_shell" "${dest}/chrome-headless-shell"
        touch "${PW_DIR}/${kind}-${BUILD}/INSTALLATION_COMPLETE" \
              "${PW_DIR}/${kind}-${BUILD}/DEPENDENCIES_VALIDATED"
        mkdir -p "${SKILL_DIR}/ms-playwright"
        ln -sfn "${PW_DIR}/${kind}-${BUILD}" "${SKILL_DIR}/ms-playwright/${kind}-${BUILD}"
    done

    "${VENV_PY}" - <<'PY'
import json, pathlib
p = pathlib.Path.home() / ".claude/skills/seo/runtime-state.json"
if p.is_file():
    s = json.loads(p.read_text())
    s["browser_ready"] = True
    p.write_text(json.dumps(s, indent=2))
PY
fi

log "done: $("${SKILL_DIR}/scripts/claude-seo" doctor | tr '\n' ' ')"
log "NOTE: URL fetching is blocked in this environment (see .claude/CLAUDE-SEO-WEB.md)"
