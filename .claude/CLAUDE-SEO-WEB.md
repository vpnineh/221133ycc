# claude-seo on Claude Code for the web

`.claude/scripts/setup-claude-seo.sh` runs on every session start and provisions
[claude-seo](https://github.com/AgriciDaniel/claude-seo) v2.3.1 into the session
container. Web containers are ephemeral, so this has to re-run each time; a warm
run takes about 45 seconds and an already-provisioned session exits immediately.

## What the script does beyond `install.sh`

The upstream installer tries to download Chromium from `cdn.playwright.dev`,
which the egress policy blocks. The image already ships Chromium under
`$PLAYWRIGHT_BROWSERS_PATH`, but under an older build number and the older
`chrome-linux` directory layout. The script symlinks the shipped build into the
chrome-for-testing layout the installed Playwright expects, links it into
claude-seo's private `ms-playwright/` directory, and flips `browser_ready` in
`runtime-state.json`.

After it runs, `~/.claude/skills/seo/scripts/claude-seo doctor` reports
`Chromium: ready`.

## Known limitation: no URL fetching

Anything in claude-seo that fetches a URL does not work here.

The only egress path in this environment is an HTTP proxy on `127.0.0.1`.
claude-seo's SSRF guard (`scripts/url_safety.py`) refuses any proxy on a
loopback or private address, by design:

```
Error: raw fetch failed: Refusing configured HTTP proxy '127.0.0.1':
blocked hostname 127.0.0.1.
```

There is no environment variable to opt out, and the guard should not be
patched out — it is a real security control, and a local edit would be
reverted by the next version bump anyway.

**Blocked:** `/seo audit`, `/seo page`, `seo-technical`, `seo-sitemap`,
`seo-backlinks`, `seo-drift`, `seo-sxo`, `seo-cluster` — anything that crawls.

**Works:** the advisory and generator skills — `seo-schema`, `seo-plan`,
`seo-content-brief`, `seo-local`, `seo-geo`, `seo-hreflang`, `seo-maps`,
`seo-competitor-pages`, `seo-programmatic` — plus offline generators such as
`schema_generate.py`.

**Workaround:** fetch the page with Claude's own WebFetch tool and hand the
content to the skill, rather than letting the skill fetch it.

For full crawling, run claude-seo on a local machine instead.
