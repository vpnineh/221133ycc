> Note (package v13.3): this calibration note was written in the research tree. `rd/r20/runn.sh hcal tcal ...` corresponds to `harness/run.sh v131 NAME @CAL ...` here, and `fn3/work/rep.sh` to `tools/reps.sh`. Paths below are historical.

# Harness cost profiles CAL / CALR (calibrated to the FundedNext MT5 tester)

Harness: `rd/r20/hcal` (a copy of `h131`, binary `tcal`). It runs the same EA source (v13.1, `ea.cpp`). With no new env keys set, `tcal` produces the same deals as `h131/t131` (checked byte for byte on a 2024-26 run). `h131` itself is untouched.

Rebuild: `cd rd/r20/hcal && g++ -O2 -std=c++17 -w -DEA_SRC='"ea.cpp"' -DINPUTS_SRC='"inputs119.h"' t114_run.cpp -o tcal`

## Invocation (copy and paste)

```sh
R=<research tree>/rd/r20
SY=XAUUSD,XAGUSD,US30,NAS100,AUDJPY,USDJPY,EURGBP,SPX500,GBPUSD     # GBPUSD = EURGBP P/L conversion, as in G131

# CAL  : costs as the FundedNext tester charges them (10y runs: swaps tied to the Fed funds rate)
$R/runn.sh hcal tcal NAME $SY PROFILE=CAL  SWAPM=4 DSHIFT=1 TZS=3 TZW=2 TIMER4=1
# CALR : CAL + slippage of 0.5 x spread on market entries and on stop exits
$R/runn.sh hcal tcal NAME $SY PROFILE=CALR SWAPM=4 DSHIFT=1 TZS=3 TZW=2 TIMER4=1

# Tester-exact swaps (fixed USD per lot per night, as the broker charges today). Use these for 2024-26 comparisons:
$R/runn.sh hcal tcal NAME $SY PROFILE=CAL  SWAPM=3 DSHIFT=1 TZS=3 TZW=2 TIMER4=1
$R/runn.sh hcal tcal NAME $SY PROFILE=CALR SWAPM=3 DSHIFT=1 TZS=3 TZW=2 TIMER4=1
```

`runn.sh` sets `PRESET=2 TZS=4 TZW=2 SWAPM=1 FROM=2016-01-01 PREDAYS=1100` before your arguments. A later `KEY=value` overrides an earlier one, so the strings above replace those defaults. Every other key still works: PRESET, OVR, SLV, NOGUARD, RISK, and so on.

Results: `out/CAL131*` and `out/CALR131*` are the 10y v13.1 runs (SWAPM=4). Run stats with `cd rd/r20 && python3 pstat2.py CAL131 CALR131`.

To replicate the three tester runs (window 2024-01 or 2025-01 to 2026-10), run `fn3/work/rep.sh TAG <env...>`, then `python3 -I fn3/work/summ.py TAG` for the match statistics. `rep.sh` reproduces the tester's history gaps: PREDAYS=0, except AUDJPY at 365 days for the 2024 starts, and PREDAYS=366 for the 2025 start.

## What each key does

| Key | Effect |
|---|---|
| `PROFILE=CAL` | Per-symbol spread, commission, contract size and swap table below. All slippage set to 0, because the tester fills market orders at bid/ask and stops exactly at the SL price. |
| `PROFILE=CALR` | CAL, plus entry slippage and stop-exit slippage of `CALR_K` x spread (default 0.5). Market and time exits get `CALR_KX` x spread (default 0). |
| `SWAPM=3` | Swap = fixed USD per lot per night (`swl`/`sws` below). Triple on Wednesday for metals and FX, Friday for indices. This is the broker spec seen in the tester deals for 2024-26. |
| `SWAPM=4` | Swap = notional x (Fed funds rate + `ml`)/365 for longs, or notional x (`ms` − FFR)/365 for shorts. Applies to metals and indices only; FX = 0. `ml` and `ms` are fitted so that 2025-26 matches the broker. Use this for multi-year runs: fixed 2025 dollar swaps on a 2016 gold or silver price would overstate cost by 3–4x. |
| `DSHIFT=1` | The `dnew/bin` M1 files are stamped in **UK local time (GMT/BST)**, which was found from the weekly open times in all years. DSHIFT converts them to true UTC, so the harness clock equals the broker server clock (UTC+2/+3, US DST). This removes the "+1 h" offset that the h131 runs show from late March to late October, and puts D1 and H4 bar cuts where the broker has them. **Must be used with `TZS=3 TZW=2`.** |
| `TIMER4=1` | Also calls `OnTimer` on the open, low/high and high/low ticks of each minute, not only after the close tick. Entries then fill at the open of the signal minute, as in MT5 (1 s timer), instead of 59 s later at the close. |
| `PREDAYS_<SYM>=d` | Per-symbol history before FROM (used to mimic the tester's data start). |
| `SPEC_<SYM>="k=v:k=v"` | Per-symbol override. Keys: `spread`, `comm` (USD per lot per side), `commpct`, `contract`, `se` / `sx` / `ss` (entry / market-exit / stop slippage in price units), `swl` / `sws` (USD per lot per night), `ml` / `ms` (annual % margins). `SHOWSPEC=1` prints the effective spec to stderr. |

## Calibrated values (FundedNext-Server 3, from three tester runs and their logs)

| Symbol | spread (price) | commission | contract | swap long / short, USD per lot per night (SWAPM=3) | ml / ms % (SWAPM=4) | h131 base for comparison |
|---|---|---|---|---|---|---|
| XAUUSD | 0.44 | 1.5 per side (3.0 RT, charged at exit) | 100 | −58.0 / −15.8 | 1.13 / 5.10 | spread 0.25, 0.0016% per side (~4.2/lot), slip 0.05, swap FFR+2.5% |
| XAGUSD | 0.037 (effective; quoted mean is 0.050) | 1.5 per side | 5000 | −100.0 / +50.8 | 14.5 / −1.1 | 0.025, 0.0016%, slip 0.003 |
| US30 | 3.9 | 0 | **10** | −92.75 / 0* | 3.24 / 2.5* | 2.0, contract 1, slip 0.5 |
| NAS100 (=NDX100) | 1.70 (effective; quoted mean is 2.26) | 0 | **10** | −33.0 / 0* | 0.95 / 2.5* | 1.5, contract 1, slip 0.5 |
| SPX500 | 0.65 | 0 | **10** | −10.46 / 0* | 1.87 / 2.5* | 0.6, contract 1, slip 0.15 |
| USDJPY | 0.0097 | 1.5 per side | 100k | 0 / 0 | – | 0.010, 3.5 per side, slip 0.002, swap 1%/y |
| AUDJPY | 0.0115 | 1.5 per side | 100k | 0 / 0 | – | 0.016, 3.0 per side, slip 0.003 |
| EURGBP | 0.00010 | 1.5 per side | 100k | 0 / 0 | – | 0.00010, 3.0 per side, slip 0.00002 |

\* Index shorts are never held overnight in the tester runs, so the short swap is not observed. The value shown is an assumption.

CALR slippage (0.5 x spread) works out to: XAU 0.22, XAG 0.0185, US30 1.95, NAS 0.85, SPX 0.325, USDJPY 0.0049 (≈0.5 pip), AUDJPY 0.0058, EURGBP 0.00005. It applies to market entries and stop exits.

How each value was derived:
- **Spread**: from the `(bid / ask / last)` part of every `market buy/sell` line in the three logs, n=670 for 2025-06 → 2026-10. Two values were then corrected against matched fills: for XAG and NAS the quoted spread over-charged the harness relative to the tester's realised fills on the same trades. The harness has one constant spread; the tester uses the spread of each minute bar.
- **Commission**: from the deal records. 3.0 USD per lot round turn, booked on the exit deal; the indices have none.
- **Swaps**: from the deal swap fields of single-exit positions, divided by lots x nights. Scatter is under 1% (p10/p90 almost equal), so these are fixed money per lot ("points" mode), not a percentage. FX swaps are 0 at this broker in all three runs; check this on the live account.
- **Contract**: deal profit ÷ (price move x lots) gives 100 / 5000 / 10 / 10 / 10. Indices are 10 per lot, so the minimum lot step is 0.1 index unit. h131 assumed 1.
- **Clock**: the tester matches the harness at the same minute after `DSHIFT=1`. Without it, the h131 label is server time + 1 h during UK BST. That is 0 h in the US/EU DST gap weeks, so it is not a constant 1 h.
- The data feed itself differs from FundedNext's. US30/NDX opening prints differ by up to ±50 points at 02:00 server, and XAU by about ±0.5. The calibration cannot remove this noise, and it causes most of the remaining unmatched trades (NAS IBS, SPX RSI2, EURGBP MR, AUDJPY BB).

## Runs produced (rd/r20/out)
- `CAL131`, `CALR131`: 10y v13.1, SWAPM=4 (main results). `CAL131f`, `CALR131f`: same with SWAPM=3 (fixed 2025 swaps in every year, an upper bound on swap cost). `CALR131g`: CALR plus `SPEC_USDJPY=se=0.015:sx=0.015:ss=0.015` (GOTOBI stress test).
- `R23_*`, `R24_*`, `R25_*`: replications of the tester runs (tags BASE, CAL, CAL4, CALR, CALR4 are final; DS, DST, CAL3, CALt and CALn are intermediate steps).
