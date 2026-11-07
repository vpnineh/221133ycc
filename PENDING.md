# Pending work after v10.29

When every item below is done and verified, delete this file, commit, and push
(the owner asked for this).

v10.29 is built on the owner's v10.28.1. Working tree:
`/tmp/claude-0/-home-user-221133ycc/ab6883ab-a26c-5d5a-8186-47eabea4513b/scratchpad/v1028`
(local git). Never push or deploy the site source; deliver the source zip only.

## Done in v10.29, verified in real browsers

- v10.28.1 regressions:
  - Firefox second-visit hang from awaiting `storage.persist()`.
  - False out-of-memory notice after the user's own reload.
  - Out-of-memory text that ignored the selected model.
  - Join that kept two copies of the file.
  - The `npm run dev` script.
- ONNX Runtime and both ffmpeg cores now download in 1 MiB parts, kept on the
  device as each arrives. The models were moved from 4 MiB to 1 MiB parts.
  - On a network resetting every ~2 MB with Hugging Face blocked, these
    finished: Auto Subtitles in WebKit (iPhone X), Firefox, Chromium and the
    Safari 16.0 and 15.4 engines; Compress Video (HEVC, ffmpeg); Video to MP3
    (AC-3, ffmpeg).
  - With 4 MiB parts the same test stopped at 13 %.
- Screen Wake Lock while a tool works, plus a "keep this page open" note on
  phones.
- The service worker serves `/__preview/` with byte ranges for home-screen
  apps.
- Cache-Control for `/models/*` and `/ort/*`.
- v10.29 changelog in all 6 languages.
- The earlier file-picker, segmented-control and MP3 fixes still pass.

## Still open

1. Verify on a real iPhone. None of these can be tested here:
   - Auto Subtitles "Error Code = 6" on Tiny. The owner saw it on Tiny and
     Base. v10.28 calls code 6 out-of-memory, but that is not proven. If it
     persists on Tiny, try transformers.js v3 with a newer onnxruntime-web, the
     non-SIMD build, or smaller chunks.
   - Home-screen app MP3 preview through the service worker.
   - Wake Lock in a home-screen app (iOS before 18.4 refuses it).
2. Compress Video on the Safari 15.4 engine over the flaky network: 2 of 3
   runs succeeded and 1 ended with "could not be compressed". 2 more runs
   crashed the engine itself at file selection, which looks like Linux
   WPE/GStreamer. Cause of the 1 failure is unknown.
3. Compress Video playback route: v10.28 yields once per frame. Not measured
   whether it drops frames on that route (the direct route is unchanged).
4. First-load speed on slow, high-latency paths (owner's trace: `loc=DE`,
   `colo=LHR`, `http/2`, likely a VPN). Measure what blocks first paint and
   cut it.
5. Ask the owner about approving the hero preview (pending from earlier).

## For the owner (Cloudflare dashboard)

- Security, Bots: turn off Bot Fight Mode / JavaScript Detections. Its injected
  inline script is blocked by the site's CSP on every page.
- Security Events: check whether requests to `/models/`, `/ort/` or `/codecs/`
  from Iran are being challenged or blocked.
