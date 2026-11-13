# DonchianProp research progress (v13.13 -> v13.14)
Recovery: `sh dp/setup.sh` (downloads HistData + Dropbox US30, builds bins and harness). Then from dp/bt:
`export DATADIR=$PWD/data/bin/ OUTDIR=$PWD/out`.

## Data
- HistData 2013..2026-11 (8 symbols) + holdout zips 2009..2012 in data/zips_h (NAS100/SPX500 start 2010).
- US30 from user Dropbox export -> sha256 identical to MANIFEST.
- Other bins differ slightly from MANIFEST (HistData revised the 2026 monthly files; reference ended 2026-10-09).

## Baseline validation (CALR, 100k funded, 2016-01..2026-10)
| run | %/mo | Sharpe | MaxDD | worst day | README |
|---|---|---|---|---|---|
| v13.13 | 2.28 | 2.26 | 8.37 | -3.84 | 2.30 / 8.37 / -3.84 |
| v13.12 | 2.21 | 2.17 | 9.92 | -3.65 | 2.23 / 9.92 / -3.65 |
| v13.13 VF=0 OVR=P.7=0.10,SLK.3=2.0 | identical to v13.12 | | | | |

## More baselines on our data (v13.13)
- LH 2010-06..2015-12 100k: v13.13 +122.6% (ref 222,616 end), v13.12 +114.1%.
- Challenge 39 starts (100k, abandon -6%): median 93 d, P(<=90d) 47.5%, pass/attempt 77%.
- Fresh funded 39 x 24 mo, monthly payout: 2.12 %/mo, worst day -3.58, min eq p0 91.6k.
- Funded 2024-26 payouts: 10k 2.24 / 25k 2.99 / 100k 3.11 %/mo.
- 100k 10y monthly: mean 2.27, std 3.62, 32/130 negative months, worst -7.2% (2022-01).
- Offline month-lock: profit lock does not cut negative months (32 -> 28-30) and costs 0.3-1.0 %/mo. Loss brake (MTD<=-2% -> x0.5):
  worst month -7.2 -> -5.3, DD 8.0 -> 7.0, mean -0.05.
## Experiments (budget ~40 configs)
- S2 lot dithering for small accounts: REJECTED (see prereg/PREREG_v13.14.txt). ~12 configs used.
- S2b lot dithering (fixed one-draw-per-signal): REJECTED (10k 2024-26 2.24 -> 3.17 but worst day -4.08; 10y +6% only, worst month -9.3).
- T-series graded daily trim + funded room scaler + higher funded risk: REJECTED (best P1 +12% on fresh funded, P(year<0) 1.3 -> 5.3%).
- SC-series scalping (shock fade, shock continuation, round-number fade, round-number breakout) x 9 symbols: all FAIL, also gross.
  FundedNext round trip = 10% (NAS) .. 64% (EURGBP) of the median 15-min range.
- Challenge: P(funded <= 60 d) = 10.8% (100k). Running: 10k/25k challenge baselines, NH (NAS H4-55 trend, challenge-only).
- v1314 = v1313 + inputs off by default (InpLotDither, InpDayTrimPct, InpFundedRoomExp); defaults deal-for-deal identical to v13.13.
