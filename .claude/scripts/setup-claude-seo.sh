#!/usr/bin/env bash
# Installs claude-seo into the session's ~/.claude and wires up the
# pre-installed Chromium, because Claude Code on the web cannot reach
# cdn.playwright.dev to download its own.
#
# Idempotent: re-running on an already-provisioned session is a no-op.
set -euo pipefail

SKILL_DIR="${HOME}/.claude/skills/seo"
REPO_TAG="${CLAUDE_SEO_TAG:-v2.3.1}"
PW_DIR="${PLAYWRIGHT_BROWSERS_PATH:-/opt/pw-browsers}"

log() { echo "[claude-seo] $*"; }

if [ -x "${SKILL_DIR}/scripts/claude-seo" ] && \
   "${SKILL_DIR}/scripts/claude-seo" doctor 2>/dev/null | grep -q "Chromium: ready"; then
    log "already installed and healthy"
    exit 0
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
BUILD=$("${VENV_PY}" -c "
from playwright.sync_api import sync_playwright
import pathlib
with sync_playwright() as p:
    print(pathlib.Path(p.chromium.executable_path).parts[3].split('-')[-1])
" 2>/dev/null) || BUILD=""

HAVE=$(ls -d "${PW_DIR}"/chromium-* 2>/dev/null | head -1 | sed 's/.*-//')

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
