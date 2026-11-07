# Pending work after v10.30.1

When every item below is done and verified, delete this file, commit, and push
(the owner asked for this).

v10.30.1 is the owner's v10.30 plus review fixes. Working tree:
`/tmp/claude-0/-home-user-221133ycc/ab6883ab-a26c-5d5a-8186-47eabea4513b/scratchpad/v1030`
(local git). Never push or deploy the site source; deliver the source zip only.

## Done in v10.30 / v10.30.1

- Compress Video: ffmpeg errors name the failed stage (download, start, run).
- Playback route: yields only after 50 ms of work. v10.30 shipped the budget
  at 0 (a leftover A/B setting); v10.30.1 sets it to 50.
- Speech windows: a window that fails with a memory-pattern error (code 6)
  runs once more at the same length. v10.30 shrank the window to 15 s and
  10 s, which saves no memory because Whisper pads every window to 30 s.
- `computeChunks` no longer adds a last window that lies wholly inside the
  overlap.
- "Copy problem report" on the speech tools' error cards, cleared per run.
- First-load path measured: FCP 1556 ms at 400 ms RTT / 1.6 Mbps.
- The test file `public/theora-probe.ogv` was removed from the export.
- The golden gate runs `npm run check` again.
- Hero preview: the owner keeps the current hero.

## Still open

1. Verify on a real iPhone. None of these can be tested here:
   - Auto Subtitles "Error Code = 6" on Tiny. Ask the owner for the copied
     problem report if it happens again. If it persists, try transformers.js
     v3 with a newer onnxruntime-web, or the non-SIMD build.
   - Home-screen app MP3 preview through the service worker.
   - Wake Lock in a home-screen app (iOS before 18.4 refuses it).
2. Hero, waiting for the owner's approval (apply only after a yes):
   - The front pipe ends in the air; proposed: it drops into a collar on the
     monitor station (screenshots sent).
   - Motion cost: 17 main-thread animations take about 450 ms of every
     second at 4x CPU slowdown (2 ms paused). The two pipe dash flows are
     about 100 ms each. Proposed: run two 6 s cycles, then rest on the same
     frame; restart on hover, tap or when the hero scrolls back into view.

## For the owner (Cloudflare dashboard)

- Security, Bots: turn off Bot Fight Mode / JavaScript Detections. Its injected
  inline script is blocked by the site's CSP on every page.
- Security Events: check whether requests to `/models/`, `/ort/` or `/codecs/`
  from Iran are being challenged or blocked.
