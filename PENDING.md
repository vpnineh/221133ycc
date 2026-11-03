# Pending work after v10.26 (resume when quota refreshes)

Working tree: `/tmp/claude-0/-home-user-221133ycc/ab6883ab-a26c-5d5a-8186-47eabea4513b/scratchpad/v1014`
(local git, HEAD b169cf0). Never push or deploy. Deliver the source zip only.

## Must do, fully

1. Split every large runtime binary into 4 MiB parts and keep each part on the
   device the moment it arrives, the same way the models are handled
   (`src/lib/models/fetch-model.ts`):
   - ONNX Runtime wasm `public/ort/ort-wasm-simd.wasm` and `ort-wasm.wasm`
     (about 10 MB, one request today). Used by Audio to Text, Auto Subtitles
     and Remove Background. See `src/lib/models/ort-runtime.ts`.
   - ffmpeg cores: the SIMD core and the legacy core without SIMD for iOS 15 to
     16.3. Today each is about 16 MB per part. See `scripts/gen-codecs.mjs` and
     `src/workers/ffmpeg.worker.ts`.
   - Any codec wasm over 4 MiB in `public/codecs`.
   - Why it matters: Cloudflare Pages ignores Range requests. On a network that
     resets every 2 to 3 MB, a file over about 3 MB never finishes, so the
     tools still fail on the owner's network in Iran.

2. Run a real flaky-network test through a proxy and let it run to completion.
   Last time the proxy had died and the test was inconclusive.
   - Network conditions: resets every ~2 MB, random stalls, failures before
     headers, Hugging Face blocked.
   - Tools: Auto Subtitles, Audio to Text, Remove Background, Image to Text,
     Compress Video, and Video to MP3 on its converter path.
   - Browsers: current WebKit with the iPhone X profile, Firefox, Chromium, and
     the old Safari engines 15.4, 16.0 and 16.4.
   - Check that progress only moves forward, that reloading mid-download
     resumes, and that no request goes to Hugging Face while this site works.
   - Tools: `scratchpad/flaky.mjs` and `scratchpad/oldwk/livesub.mjs`
     (`BLOCKHF=1`).

3. Video to MP3 on iPhone: confirm the in-page preview plays. If the owner
   still sees the problem, ask for the failing MP3 and its source video.

4. Segmented controls now wrap instead of scrolling. Check them visually on a
   375px phone: Video to MP3 and the Audio to Text language row.

5. Re-run the file-picker `accept` audit after the build
   (`scratchpad/oldwk/accept.mjs`).

6. Write the changelog for v10.26/v10.27 in all 6 languages and regenerate the
   feeds, then package with `node scripts/package-source.mjs v10.xx`.

7. Add `Cache-Control` for `/models/*`; it is missing today.

8. Ask the owner again about approving the hero preview (pending from earlier).

9. First-load speed on slow, high-latency paths. The owner's trace showed
   `loc=DE`, `colo=LHR` and `http/2`, which looks like a VPN exit in Germany
   served from London. Measure the critical path: number of requests and
   bytes before first paint, and the shared JS at 226 KB gzip. Cut what
   blocks first paint.

## For the owner (Cloudflare dashboard)

- Security, Bots: turn off Bot Fight Mode / JavaScript Detections. Its injected
  inline script is blocked by the site's CSP on every page.
- Security Events: check whether requests to `/models/`, `/ort/` or `/codecs/`
  from Iran are being challenged or blocked.
