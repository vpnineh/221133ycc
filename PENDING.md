# Pending work after v10.31

When every item below is done and verified, delete this file, commit, and push
(the owner asked for this).

v10.31 builds on v10.30.1. Working tree:
`/tmp/claude-0/-home-user-221133ycc/ab6883ab-a26c-5d5a-8186-47eabea4513b/scratchpad/v1030`
(local git). Never push or deploy the site source; deliver the source zip only
(and upload it to x0.at when the owner asks).

## Done in v10.31

- Speech engine ("no available backend … Failed to fetch" on Chrome macOS and
  iPhone): the joined ONNX Runtime file is handed to the engine from memory
  (a same-origin URL the page's fetch answers). No blob: URL, and no 10 MB
  whole-file request when the parts fail; the error names the connection.
- Error cards on phones keep the message full width; buttons wrap below.
  Visitor-facing errors no longer show internal file names or retry counts.
- One progress bar per run: the converter's download and conversion show in
  the tool's own bar (6 tools); Cancel stops the converter; a background
  preview conversion never takes over the bar.
- Audio to Text says "Decoding audio track…" while decoding.
- The keep-the-page-open note sits under the progress bar on phones.
- Transcript lines grow to their text (no clipped third line on phones).

## Done earlier (v10.30 / v10.30.1)

- ffmpeg errors name their stage; playback-route yield budget 50 ms; same-length
  retry for a code-6 window; no redundant last window; problem report button;
  first-load path measured; hero pipe joins the monitor station (approved).

## Still open

1. Verify on a real iPhone and a real Mac (none can be tested here):
   - Auto Subtitles on Tiny and Base after deploying v10.31. If anything
     fails, ask the owner for the copied problem report: it now names how the
     engine file was provided.
   - Home-screen app MP3 preview and Wake Lock (iOS before 18.4 refuses it).
2. Hero motion cost, waiting for the owner's answer (apply only after a yes):
   17 main-thread animations take about 450 ms of every second at 4x CPU
   slowdown. Proposed: two 6 s cycles, then rest; restart on hover, tap or
   when the hero scrolls back into view.
3. Site unreachable from Iran without a VPN (owner report, 2026-11-08). Not a
   code change: the HTML itself does not load. Owner to check DNS answer
   (10.10.34.x means the domain is filtered), cdn-cgi/trace, the pages.dev
   address, and Cloudflare Security Events for country IR.

## For the owner (Cloudflare dashboard)

- Security, Bots: turn off Bot Fight Mode / JavaScript Detections. Its injected
  inline script is blocked by the site's CSP on every page.
- Security Events: check whether requests from Iran are being challenged or
  blocked.
