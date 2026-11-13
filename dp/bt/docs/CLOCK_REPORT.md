# CLOCK_REPORT: true clock of the HistData M1 files (17 symbols, 2013-2026) and of the DJIUSD.prop MT5 export

All numbers below were measured on the supplied files (not copied from the earlier agent). Notation: "label" = the timestamp string in the file; offset = label minus UTC in hours (so UTC-5 means label = UTC - 5h). NY = America/New_York. Cells used everywhere: W = both US and EU on winter time, S = both on summer time, M = March weeks where the US is already on DST but London/EU is not (2nd Sun Mar to last Sun Mar), O = late Oct / early Nov weeks where the EU is back on winter time but the US is still on DST (last Sun Oct to 1st Sun Nov). Cells M and O are the discriminating weeks: they tell whether the clock follows US or EU dates.

## 1. Verified rule (HistData, all 17 symbols identical)

| Period (label dates) | Rule | label - UTC | Switch dates (all on a Sunday while the market is closed) |
|---|---|---|---|
| 2013-01-01 .. 2018-11-03 | US Eastern local time with US DST (America/New_York) | -5h winter, -4h summer | summer = 2nd Sunday March .. 1st Sunday November (e.g. 2013-03-10/2013-11-03 ... 2018-03-11/2018-11-04) |
| 2018-11-04 .. 2026-10-09 (end of data) | UTC-5 plus EU (London) summer-time shift | -5h winter, -4h in EU summer | summer = last Sunday March .. last Sunday October (2019-03-31/10-27, 2020-03-29/10-25, 2021-03-28/10-31, 2022-03-27/10-30, 2023-03-26/10-29, 2024-03-31/10-27, 2025-03-30/10-26, 2026-03-29; the 2026-10-25 end is not yet in the data) |

Notes: (1) The regime change cannot be dated more precisely than 2018-11-04 .. 2019-03-10, because both rules give UTC-5 between the US fall-back (2018-11-04) and the first spring week where they differ (US 2019-03-10 vs EU 2019-03-31). The last week that follows the US rule is the week of 2018-10-28 (O cell, label UTC-4 while London was already on winter time); the first that follows the EU rule is the week of 2019-03-10 (M cell, label UTC-5 although NY was already on EDT). (2) The market itself always follows US dates (NY 17:00 daily rollover, 08:30 ET releases, 09:30 ET cash open); from 2019 only the label follows EU dates. So in the M and O weeks of 2019-2026 the label is one hour BEHIND New York local time (the weekly open appears at 16:00 label instead of 17:00). (3) Conversion used: utc = label + 5h (winter) or + 4h (summer) with the boundary at 00:00 label on the switch Sunday (no bar exists between Friday close and Sunday evening, so any boundary inside the weekend is equivalent; checked: 0 bars on label Saturday, 0 on Sunday before 16:00, 0 after Friday 17:05 in all 17 files). Code: src/clockrule.py.

## 2. Probes (all computed per symbol, per year, per cell)

* P1, weekly structure. For every week the first bar after the weekend gap (Sunday open) and the last bar before it (Friday close) are taken (FX 17:00 NY; XAU/XAG/NAS100/SPX500 open 18:00 NY and close 17:00 NY; DAX open Monday 08:00 CET, close 22:00 CET). The label time-of-day in cell S/M/O is compared with the same year's winter value; the implied shift is expressed as label-UTC using the true NY offset (DAX: EU offset). Value shown = median implied offset (hours) and share of weeks within 0.5h.
* P2, absolute 08:30 ET / 10:00 ET spike. Mean absolute 1-minute return by label minute on Thursdays (08:30 jobless claims) and first Fridays (NFP, day 3-7). The offset (-8..+2) whose label minutes (08:30 ET and 10:00 ET converted with the true NY UTC offset) have the highest mean |return| relative to the median minute. Shown: offset, ratio of best to median, and ratio of the runner-up offset. A good detection has ratio clearly above runner-up.
* P3, indices only. NAS100/SPX500: joint 09:30 ET cash open + 16:00 ET close spike (confounded in some years by the stronger 08:30 spike, runner-up often close); DAX: 09:00 CET Xetra open spike (EU anchored, independent of the NY probes).
* P4, daily break (XAUUSD, XAGUSD, NAS100, SPX500): the 1-hour break around 17:00 NY, located day by day (Mon-Thu); gives exact switch dates (section 5).
* P5, cross-check after conversion: weekly open in true UTC is 22:00 (NY winter) / 21:00 (NY summer) for all 12 FX pairs in every cell and year (section 7).

## 3. Weekly open/close, pooled over the 12 FX pairs: share of weekly events (open and close, 1426 symbol-week consensus values 2013-2026) consistent with each rule

Consensus per week = robust quantile over the 12 pairs (open: 20th percentile of implied offset, close: 80th; holes in the data only delay opens / advance closes). Rule NY = label UTC-5/-4 on US dates; rule EU = UTC-5/-4 on EU dates.

Only weeks where the two rules predict different offsets (M and O cells) can discriminate:

| Era | weekly events in M/O cells | share consistent with NY(US-date) rule | share consistent with EU-date rule |
|---|---|---|---|
| 2013-2018-10 | 41 | 1.000 | 0.000 |
| 2018-11-2026 | 57 | 0.018 | 0.982 |

Of 1426 consensus events only 8 deviate from the piecewise rule above; all are single weeks around holidays or hole weeks: close week 2018-11-04 (implied offset -4.0); close week 2020-12-27 (implied offset -19.0); close week 2023-02-26 (implied offset -6.0); open week 2014-06-01 (implied offset -6.0); open week 2016-12-25 (implied offset -4.5); open week 2017-01-01 (implied offset -20.0); open week 2019-05-26 (implied offset -16.8); open week 2023-12-24 (implied offset -4.0).

Per-year, per-cell pooled over all 15 non-DAX symbols (share of symbol-week events consistent with each rule):

| Year | W: NY / EU | S: NY / EU | M: NY / EU (n) | O: NY / EU (n) |
|---|---|---|---|---|
| 2013 | 0.99 / 0.99 | 1.00 / 1.00 | 0.98 / 0.01 (94) | 1.00 / 0.00 (32) |
| 2014 | 0.99 / 0.99 | 0.98 / 0.98 | 1.00 / 0.00 (96) | 1.00 / 0.00 (32) |
| 2015 | 0.99 / 0.99 | 0.99 / 0.99 | 1.00 / 0.00 (96) | 1.00 / 0.00 (32) |
| 2016 | 0.97 / 0.97 | 1.00 / 1.00 | 1.00 / 0.00 (60) | 1.00 / 0.00 (32) |
| 2017 | 0.97 / 0.97 | 1.00 / 1.00 | 1.00 / 0.00 (64) | 1.00 / 0.00 (32) |
| 2018 | 0.98 / 0.98 | 1.00 / 1.00 | 1.00 / 0.00 (64) | 1.00 / 0.00 (32) |
| 2019 | 0.93 / 0.93 | 0.92 / 0.92 | 0.05 / 0.89 (96) | 0.00 / 0.94 (32) |
| 2020 | 0.92 / 0.92 | 0.94 / 0.94 | 0.00 / 0.94 (96) | 0.00 / 0.94 (32) |
| 2021 | 0.93 / 0.93 | 0.94 / 0.94 | 0.00 / 0.94 (64) | 0.00 / 0.94 (32) |
| 2022 | 0.93 / 0.93 | 0.93 / 0.93 | 0.00 / 0.94 (64) | 0.03 / 0.91 (32) |
| 2023 | 0.81 / 0.81 | 0.66 / 0.66 | 0.19 / 0.52 (64) | 0.00 / 0.94 (32) |
| 2024 | 0.92 / 0.92 | 0.93 / 0.93 | 0.00 / 0.96 (92) | 0.03 / 0.88 (32) |
| 2025 | 0.93 / 0.93 | 0.93 / 0.93 | 0.00 / 0.94 (96) | 0.00 / 0.94 (32) |
| 2026 | 0.93 / 0.93 | 0.93 / 0.93 | 0.00 / 0.94 (96) | - |

Reading: 2013-2018 the M/O weeks are 98-100% consistent with the NY rule and 0-1% with the EU rule; 2019-2026 the picture flips (EU 89-96%, NY 0-5%). 2023 is lower in all cells (S 0.66, W 0.81) because HistData has many missing whole hours in 2023 (see section 8), not because of a clock change.

## 4. Evidence table per symbol-year (winter W and summer S; expected offsets -5 and -4)

P1 = weekly structure offset (share of weeks agreeing, n events). P2 = 08:30/10:00 ET spike offset (ratio/runner-up). P3 = index probe offset (ratio/runner-up). OK = offset equals the rule; "weak" = probe ratio not clearly above runner-up or the best offset is one of the confounded ones. Verdict: CONFIRMED when P1 matches and at least one of P2/P3 also matches with ratio >= 1.15 x runner-up; P1-ONLY when P1 matches but P2/P3 are weak or disagree; AMBIGUOUS when P1 agreement is below 0.85.

| Symbol | Year | W: P1 | W: P2 | W: P3 | S: P1 | S: P2 | S: P3 | Verdict W / S |
|---|---|---|---|---|---|---|---|---|
| XAUUSD | 2013 | -5.0 (0.97, 36) | -5 (7.5/2.3) | - | -4.0 (1.00, 60) | -4 (9.6/2.0) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2014 | -5.0 (0.97, 36) | -5 (8.9/2.3) | - | -4.0 (0.95, 58) | -4 (8.4/3.0) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2015 | -5.0 (0.97, 34) | -5 (8.2/2.5) | - | -4.0 (0.97, 60) | -4 (7.5/2.6) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2016 | -5.0 (0.97, 35) | -5 (3.7/2.0) | - | -4.0 (1.00, 61) | -4 (6.2/2.3) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2017 | -5.0 (0.97, 36) | -5 (5.2/2.2) | - | -4.0 (1.00, 60) | -4 (6.9/2.3) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2018 | -5.0 (0.97, 37) | -5 (4.4/2.4) | - | -4.0 (1.00, 60) | -4 (5.9/2.7) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2019 | -5.0 (0.97, 36) | -5 (8.8/2.7) | - | -4.0 (0.98, 58) | -4 (6.4/2.7) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2020 | -5.0 (0.97, 34) | -5 (3.9/2.0) | - | -4.0 (0.98, 58) | -4 (4.2/2.6) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2021 | -5.0 (0.97, 33) | -5 (5.9/2.2) | - | -4.0 (1.00, 60) | -4 (6.9/2.6) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2022 | -5.0 (0.97, 36) | -5 (6.5/2.0) | - | -4.0 (1.00, 60) | -4 (6.2/2.3) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2023 | -5.0 (0.86, 36) | -5 (6.9/2.2) | - | -4.0 (0.70, 60) | -4 (7.2/3.1) | - | CONFIRMED / ok (holes) |
| XAUUSD | 2024 | -5.0 (0.95, 37) | -5 (6.7/2.1) | - | -4.0 (1.00, 59) | -4 (8.8/2.7) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2025 | -5.0 (0.94, 36) | -5 (3.4/2.5) | - | -4.0 (0.98, 58) | -4 (4.2/1.9) | - | CONFIRMED / CONFIRMED |
| XAUUSD | 2026 | -5.0 (1.00, 19) | -5 (3.0/2.3) | - | -4.0 (0.96, 51) | -4 (5.6/2.3) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2013 | -5.0 (0.97, 36) | -5 (5.4/2.0) | - | -4.0 (1.00, 60) | -4 (8.8/2.5) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2014 | -5.0 (0.97, 36) | -5 (6.1/2.5) | - | -4.0 (0.95, 58) | -4 (6.4/2.6) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2015 | -5.0 (0.94, 34) | -5 (5.9/3.0) | - | -4.0 (0.88, 60) | -4 (7.9/3.4) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2016 | -5.0 (0.97, 35) | -5 (4.0/2.4) | - | -4.0 (0.98, 61) | -4 (5.8/2.4) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2017 | -5.0 (0.97, 36) | -5 (4.4/2.3) | - | -4.0 (1.00, 60) | -4 (5.5/2.4) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2018 | -5.0 (0.97, 37) | -5 (3.7/2.2) | - | -4.0 (1.00, 60) | -4 (4.8/2.5) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2019 | -5.0 (0.97, 36) | -5 (5.8/2.3) | - | -4.0 (0.98, 58) | -4 (5.2/2.4) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2020 | -5.0 (0.97, 34) | -5 (4.2/1.9) | - | -4.0 (0.98, 58) | -4 (3.8/2.4) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2021 | -5.0 (0.97, 33) | -5 (4.5/2.2) | - | -4.0 (1.00, 60) | -4 (5.8/3.2) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2022 | -5.0 (0.97, 36) | -5 (6.0/2.1) | - | -4.0 (1.00, 60) | -4 (6.2/3.0) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2023 | -5.0 (0.92, 36) | -5 (5.5/2.1) | - | -4.0 (0.80, 60) | -4 (5.4/2.8) | - | CONFIRMED / ok (holes) |
| XAGUSD | 2024 | -5.0 (0.97, 37) | -5 (5.9/2.7) | - | -4.0 (1.00, 59) | -4 (6.6/3.0) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2025 | -5.0 (0.97, 36) | -5 (3.2/2.4) | - | -4.0 (0.98, 58) | -4 (3.3/2.8) | - | CONFIRMED / CONFIRMED |
| XAGUSD | 2026 | -5.0 (1.00, 19) | -5 (2.6/2.1) | - | -4.0 (0.96, 51) | -4 (5.3/3.2) | - | CONFIRMED / CONFIRMED |
| NAS100 | 2013 | -5.0 (0.97, 36) | -5 (5.8/4.2) | -5 (4.8/3.3) | -4.0 (1.00, 59) | -4 (5.0/4.1) | -4 (4.4/2.8) | CONFIRMED / CONFIRMED |
| NAS100 | 2014 | -5.0 (0.97, 36) | -5 (4.4/3.4) | -5 (3.7/3.0) | -4.0 (0.97, 58) | -4 (4.9/3.3) | -4 (3.8/2.9) | CONFIRMED / CONFIRMED |
| NAS100 | 2015 | -5.0 (0.97, 34) | -5 (4.7/3.6) | -5 (3.8/2.5) | -4.0 (0.97, 60) | -4 (3.2/3.2) | -4 (3.8/2.3) | CONFIRMED / CONFIRMED |
| NAS100 | 2016 | -5.0 (0.97, 35) | -4 (3.6/3.5) | -5 (3.3/2.1) | -4.0 (1.00, 61) | -2 (3.0/2.9) | -4 (3.8/3.0) | CONFIRMED / CONFIRMED |
| NAS100 | 2017 | -5.0 (0.94, 34) | -4 (4.6/3.6) | -5 (5.2/3.1) | -4.0 (1.00, 60) | -4 (4.4/4.0) | -4 (4.1/3.1) | CONFIRMED / CONFIRMED |
| NAS100 | 2018 | -5.0 (0.89, 37) | +1 (3.6/2.9) | -5 (3.9/2.7) | -4.0 (1.00, 60) | -4 (4.6/3.3) | -4 (4.0/2.5) | CONFIRMED / CONFIRMED |
| NAS100 | 2019 | -5.0 (0.94, 36) | -5 (4.4/3.2) | -5 (3.6/2.3) | -4.0 (0.98, 58) | -4 (4.3/3.5) | -4 (3.5/2.5) | CONFIRMED / CONFIRMED |
| NAS100 | 2020 | -5.0 (0.97, 34) | -4 (3.2/2.7) | -5 (3.7/1.7) | -4.0 (0.98, 58) | -3 (3.7/3.4) | -4 (3.5/2.2) | CONFIRMED / CONFIRMED |
| NAS100 | 2021 | -5.0 (0.94, 33) | -4 (3.0/2.7) | -5 (3.8/2.7) | -4.0 (0.98, 62) | -3 (4.5/4.4) | -4 (4.4/3.0) | CONFIRMED / CONFIRMED |
| NAS100 | 2022 | -5.0 (0.97, 36) | -5 (5.4/3.1) | -6 (4.2/3.3) | -4.0 (1.00, 60) | -4 (7.9/4.1) | -5 (4.9/3.8) | CONFIRMED / CONFIRMED |
| NAS100 | 2023 | -5.0 (0.92, 36) | -5 (9.0/5.0) | -5 (5.1/4.8) | -4.0 (0.90, 60) | -4 (7.6/4.9) | -4 (4.7/4.6) | CONFIRMED / CONFIRMED |
| NAS100 | 2024 | -5.0 (0.95, 37) | -5 (6.1/4.4) | -5 (4.5/3.7) | -4.0 (0.97, 59) | -4 (8.1/4.3) | -5 (4.8/4.1) | CONFIRMED / CONFIRMED |
| NAS100 | 2025 | -5.0 (0.97, 36) | -5 (5.6/5.0) | -5 (4.3/3.9) | -4.0 (0.98, 58) | -4 (4.0/3.8) | -4 (4.1/2.9) | P1-ONLY / CONFIRMED |
| NAS100 | 2026 | -5.0 (1.00, 19) | -5 (4.2/3.9) | -5 (4.6/2.8) | -4.0 (0.96, 51) | -4 (4.0/3.4) | -4 (3.7/2.4) | CONFIRMED / CONFIRMED |
| SPX500 | 2013 | -5.0 (0.97, 36) | -5 (3.5/2.2) | -6 (2.6/2.2) | -4.0 (1.00, 59) | -4 (4.1/2.1) | -4 (2.6/2.3) | CONFIRMED / CONFIRMED |
| SPX500 | 2014 | -5.0 (0.97, 36) | -5 (4.4/2.5) | -6 (2.9/2.5) | -4.0 (0.97, 58) | -4 (2.9/2.0) | -5 (2.7/2.2) | CONFIRMED / CONFIRMED |
| SPX500 | 2015 | -5.0 (0.97, 34) | -5 (3.6/2.4) | -5 (2.7/2.2) | -4.0 (0.97, 60) | -4 (3.0/2.3) | -4 (2.7/1.9) | CONFIRMED / CONFIRMED |
| SPX500 | 2016 | -5.0 (0.97, 35) | -5 (3.3/2.4) | -5 (2.1/1.9) | -4.0 (1.00, 61) | -4 (2.7/2.4) | -4 (2.8/1.8) | CONFIRMED / CONFIRMED |
| SPX500 | 2017 | -5.0 (0.94, 34) | -4 (2.8/2.7) | -5 (3.3/1.6) | -4.0 (1.00, 60) | -4 (3.1/2.3) | -4 (2.6/1.6) | CONFIRMED / CONFIRMED |
| SPX500 | 2018 | -5.0 (0.89, 37) | +1 (3.3/2.6) | -5 (3.3/2.6) | -4.0 (1.00, 60) | -4 (3.6/2.7) | -4 (3.1/2.3) | CONFIRMED / CONFIRMED |
| SPX500 | 2019 | -5.0 (0.94, 36) | -5 (4.2/2.8) | -5 (2.9/2.3) | -4.0 (0.98, 58) | -4 (3.9/2.7) | -4 (2.7/2.4) | CONFIRMED / CONFIRMED |
| SPX500 | 2020 | -5.0 (0.97, 34) | +1 (2.5/2.3) | -5 (3.0/1.7) | -4.0 (0.97, 58) | -4 (3.6/2.9) | -4 (3.0/2.3) | CONFIRMED / CONFIRMED |
| SPX500 | 2021 | -5.0 (0.94, 33) | +1 (2.7/2.6) | -5 (3.3/2.4) | -4.0 (0.98, 62) | -4 (3.6/3.6) | -4 (3.8/2.7) | CONFIRMED / CONFIRMED |
| SPX500 | 2022 | -5.0 (0.97, 36) | -5 (6.5/2.9) | -6 (4.4/2.8) | -4.0 (1.00, 60) | -4 (7.0/3.3) | -5 (4.6/3.3) | CONFIRMED / CONFIRMED |
| SPX500 | 2023 | -5.0 (0.92, 36) | -5 (9.8/4.0) | -6 (5.1/4.1) | -4.0 (0.77, 60) | -4 (6.4/3.9) | -5 (4.0/4.0) | CONFIRMED / ok (holes) |
| SPX500 | 2024 | -5.0 (0.92, 37) | -5 (6.5/4.0) | -5 (4.4/4.2) | -4.0 (1.00, 59) | -4 (9.3/4.7) | -5 (5.1/4.3) | CONFIRMED / CONFIRMED |
| SPX500 | 2025 | -5.0 (0.97, 36) | -5 (5.5/3.6) | -6 (4.2/3.8) | -4.0 (0.98, 58) | -4 (4.1/3.3) | -4 (3.9/3.0) | CONFIRMED / CONFIRMED |
| SPX500 | 2026 | -5.0 (1.00, 19) | -5 (5.0/3.5) | -5 (4.3/3.1) | -4.0 (0.94, 51) | -4 (4.0/3.1) | -4 (3.5/2.7) | CONFIRMED / CONFIRMED |
| DAX | 2013 | -5.0 (1.00, 36) | -5 (4.0/1.8) | +2 (2.6/2.4) | -4.0 (1.00, 59) | -4 (3.2/1.8) | -4 (2.9/2.2) | CONFIRMED / CONFIRMED |
| DAX | 2014 | -5.0 (1.00, 34) | -5 (2.7/2.1) | -5 (2.8/2.6) | -4.0 (0.98, 56) | -4 (2.7/1.7) | -4 (2.6/1.8) | CONFIRMED / CONFIRMED |
| DAX | 2015 | -5.0 (1.00, 34) | -5 (2.0/1.7) | -5 (2.7/2.0) | -4.0 (0.98, 54) | -4 (2.1/1.5) | -4 (2.7/1.7) | CONFIRMED / CONFIRMED |
| DAX | 2016 | -5.0 (1.00, 33) | -5 (2.6/1.7) | -5 (3.2/2.5) | -4.0 (1.00, 57) | -4 (2.7/1.9) | -4 (4.0/2.4) | CONFIRMED / CONFIRMED |
| DAX | 2017 | -5.0 (1.00, 31) | -5 (2.0/1.6) | -5 (3.3/1.9) | -4.0 (1.00, 56) | -4 (2.8/1.9) | -4 (3.5/1.8) | CONFIRMED / CONFIRMED |
| DAX | 2018 | -5.0 (0.97, 32) | -5 (1.8/1.8) | -6 (3.8/3.3) | -4.0 (1.00, 58) | -3 (1.7/1.6) | -4 (3.1/1.6) | P1-ONLY / CONFIRMED |
| DAX | 2019 | -5.0 (0.97, 36) | -5 (3.2/2.2) | -5 (4.2/3.9) | -4.0 (0.98, 58) | -4 (3.6/2.1) | -4 (4.4/2.9) | CONFIRMED / CONFIRMED |
| DAX | 2020 | -5.0 (0.74, 34) | -4 (1.8/1.7) | -5 (4.0/2.3) | -4.0 (0.83, 58) | -4 (2.4/1.9) | -4 (3.5/3.3) | ok (holes) / ok (holes) |
| DAX | 2021 | -5.0 (1.00, 33) | -5 (1.7/1.6) | -5 (3.9/1.6) | -4.0 (1.00, 60) | -4 (2.2/1.5) | -4 (4.1/1.7) | CONFIRMED / CONFIRMED |
| DAX | 2022 | -5.0 (1.00, 34) | -5 (2.8/1.6) | -5 (3.2/2.0) | -4.0 (1.00, 60) | -4 (3.3/1.7) | -4 (3.0/1.5) | CONFIRMED / CONFIRMED |
| DAX | 2023 | -5.0 (0.88, 34) | -5 (4.8/2.5) | -5 (4.5/3.0) | -4.0 (0.84, 58) | -4 (2.9/1.6) | -4 (3.3/1.6) | CONFIRMED / ok (holes) |
| DAX | 2024 | -5.0 (0.97, 37) | -5 (3.1/2.2) | -5 (6.3/2.8) | -4.0 (0.95, 59) | -4 (5.7/2.1) | -4 (5.5/3.2) | CONFIRMED / CONFIRMED |
| DAX | 2025 | -5.0 (1.00, 36) | -5 (3.1/1.9) | -5 (4.7/2.6) | -4.0 (1.00, 58) | -4 (2.9/1.8) | -4 (4.7/2.6) | CONFIRMED / CONFIRMED |
| DAX | 2026 | -5.0 (1.00, 19) | -5 (3.8/2.2) | -5 (4.8/2.8) | -4.0 (1.00, 51) | -4 (2.4/1.9) | -4 (5.1/2.5) | CONFIRMED / CONFIRMED |
| EURUSD | 2013 | -5.0 (1.00, 36) | -5 (3.8/2.2) | - | -4.0 (1.00, 60) | -4 (9.2/2.4) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2014 | -5.0 (1.00, 36) | -5 (8.3/3.5) | - | -4.0 (0.98, 60) | -4 (8.7/2.4) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2015 | -5.0 (1.00, 34) | -5 (8.4/2.8) | - | -4.0 (1.00, 60) | -4 (9.1/2.2) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2016 | -5.0 (0.97, 35) | -5 (4.4/2.5) | - | -4.0 (1.00, 62) | -4 (5.6/2.2) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2017 | -5.0 (0.97, 36) | -5 (4.2/2.5) | - | -4.0 (1.00, 62) | -4 (5.2/2.7) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2018 | -5.0 (1.00, 37) | -5 (4.0/3.2) | - | -4.0 (1.00, 62) | -4 (5.0/1.9) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2019 | -5.0 (1.00, 36) | -5 (6.5/2.0) | - | -4.0 (0.98, 60) | -4 (6.5/2.4) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2020 | -5.0 (0.97, 36) | -5 (3.7/2.1) | - | -4.0 (1.00, 60) | -4 (2.8/2.3) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2021 | -5.0 (1.00, 35) | -5 (5.0/2.8) | - | -4.0 (1.00, 62) | -4 (6.4/2.1) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2022 | -5.0 (1.00, 36) | -5 (6.5/2.3) | - | -4.0 (1.00, 62) | -4 (5.9/2.3) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2023 | -5.0 (0.86, 36) | -5 (10.0/2.3) | - | -4.0 (0.72, 60) | -4 (8.6/2.3) | - | CONFIRMED / ok (holes) |
| EURUSD | 2024 | -5.0 (1.00, 37) | -5 (8.8/2.3) | - | -4.0 (1.00, 60) | -4 (14.3/2.6) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2025 | -5.0 (1.00, 36) | -5 (4.9/2.1) | - | -4.0 (1.00, 60) | -4 (7.2/1.9) | - | CONFIRMED / CONFIRMED |
| EURUSD | 2026 | -5.0 (1.00, 19) | -5 (4.8/2.1) | - | -4.0 (0.98, 53) | -4 (6.2/2.0) | - | CONFIRMED / CONFIRMED |
| GBPUSD | 2013 | -5.0 (1.00, 36) | -5 (4.5/3.4) | - | -4.0 (1.00, 60) | -4 (8.9/6.0) | - | CONFIRMED / CONFIRMED |
| GBPUSD | 2014 | -5.0 (1.00, 36) | -5 (6.3/2.8) | - | -4.0 (0.98, 60) | -4 (6.3/4.3) | - | CONFIRMED / CONFIRMED |
| GBPUSD | 2015 | -5.0 (1.00, 34) | -5 (7.5/4.0) | - | -4.0 (1.00, 60) | -4 (8.9/3.8) | - | CONFIRMED / CONFIRMED |
| GBPUSD | 2016 | -5.0 (1.00, 35) | -5 (3.5/2.7) | - | -4.0 (1.00, 62) | -4 (3.7/3.0) | - | CONFIRMED / CONFIRMED |
| GBPUSD | 2017 | -5.0 (0.97, 36) | -5 (3.4/2.4) | - | -4.0 (1.00, 62) | -8 (4.6/4.1) | - | CONFIRMED / P1-ONLY |
| GBPUSD | 2018 | -5.0 (1.00, 37) | -5 (3.1/2.7) | - | -4.0 (1.00, 62) | -4 (4.1/3.3) | - | P1-ONLY / CONFIRMED |
| GBPUSD | 2019 | -5.0 (1.00, 36) | +2 (6.5/3.7) | - | -4.0 (0.98, 60) | -4 (3.5/2.3) | - | P1-ONLY / CONFIRMED |
| GBPUSD | 2020 | -5.0 (0.97, 36) | -5 (2.9/2.4) | - | -4.0 (1.00, 60) | -4 (2.5/2.3) | - | CONFIRMED / P1-ONLY |
| GBPUSD | 2021 | -5.0 (1.00, 35) | -8 (4.0/3.5) | - | -4.0 (1.00, 62) | -4 (4.6/2.9) | - | P1-ONLY / CONFIRMED |
| GBPUSD | 2022 | -5.0 (1.00, 36) | -5 (6.4/2.9) | - | -4.0 (1.00, 62) | -4 (5.1/2.9) | - | CONFIRMED / CONFIRMED |
| GBPUSD | 2023 | -5.0 (0.81, 36) | -5 (8.7/3.5) | - | -4.0 (0.63, 60) | -4 (7.2/3.3) | - | ok (holes) / ok (holes) |
| GBPUSD | 2024 | -5.0 (1.00, 37) | -5 (7.9/3.1) | - | -4.0 (1.00, 60) | -4 (12.5/3.0) | - | CONFIRMED / CONFIRMED |
| GBPUSD | 2025 | -5.0 (1.00, 36) | -5 (4.3/3.5) | - | -4.0 (1.00, 60) | -4 (6.0/2.4) | - | CONFIRMED / CONFIRMED |
| GBPUSD | 2026 | -5.0 (1.00, 19) | -5 (5.0/3.2) | - | -4.0 (1.00, 53) | -4 (5.2/1.9) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2013 | -5.0 (1.00, 36) | -5 (5.1/1.5) | - | -4.0 (1.00, 60) | -4 (9.0/1.7) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2014 | -5.0 (1.00, 36) | -5 (9.8/2.1) | - | -4.0 (0.98, 60) | -4 (9.3/2.0) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2015 | -5.0 (1.00, 34) | -5 (8.6/1.9) | - | -4.0 (1.00, 60) | -4 (9.1/1.7) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2016 | -5.0 (0.97, 35) | -5 (4.0/1.8) | - | -4.0 (1.00, 62) | -4 (4.4/1.5) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2017 | -5.0 (0.97, 36) | -5 (4.6/2.2) | - | -4.0 (1.00, 62) | -4 (6.5/2.1) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2018 | -5.0 (1.00, 37) | -5 (3.7/2.0) | - | -4.0 (1.00, 62) | -4 (3.8/1.9) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2019 | -5.0 (1.00, 36) | -5 (7.7/2.1) | - | -4.0 (0.98, 60) | -4 (7.2/1.8) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2020 | -5.0 (0.97, 36) | -5 (3.6/2.3) | - | -4.0 (1.00, 60) | -4 (3.5/2.1) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2021 | -5.0 (1.00, 35) | -5 (4.3/1.9) | - | -4.0 (1.00, 62) | -4 (7.5/2.0) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2022 | -5.0 (1.00, 36) | -5 (7.1/2.0) | - | -4.0 (0.98, 62) | -4 (5.3/1.7) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2023 | -5.0 (0.92, 36) | -5 (9.6/2.3) | - | -4.0 (0.62, 60) | -4 (8.5/1.9) | - | CONFIRMED / ok (holes) |
| USDJPY | 2024 | -5.0 (1.00, 37) | -5 (8.6/2.2) | - | -4.0 (1.00, 60) | -4 (12.5/2.5) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2025 | -5.0 (1.00, 36) | -5 (4.8/1.9) | - | -4.0 (1.00, 60) | -4 (7.7/2.2) | - | CONFIRMED / CONFIRMED |
| USDJPY | 2026 | -5.0 (1.00, 19) | -5 (5.2/1.7) | - | -4.0 (0.98, 53) | -4 (4.9/1.9) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2013 | -5.0 (1.00, 36) | -5 (7.3/2.5) | - | -4.0 (1.00, 60) | -4 (8.7/2.1) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2014 | -5.0 (1.00, 36) | -5 (12.6/2.1) | - | -4.0 (0.98, 60) | -4 (9.9/1.9) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2015 | -5.0 (1.00, 34) | -5 (7.9/3.1) | - | -4.0 (1.00, 60) | -4 (7.0/2.4) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2016 | -5.0 (0.97, 35) | -5 (5.0/2.8) | - | -4.0 (1.00, 62) | -4 (5.8/2.6) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2017 | -5.0 (0.97, 36) | -5 (7.6/2.6) | - | -4.0 (1.00, 62) | -4 (7.1/2.0) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2018 | -5.0 (1.00, 37) | -5 (6.8/2.2) | - | -4.0 (1.00, 62) | -4 (6.4/2.2) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2019 | -5.0 (1.00, 36) | -5 (6.4/3.1) | - | -4.0 (0.98, 60) | -4 (6.5/2.7) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2020 | -5.0 (0.97, 36) | -5 (3.3/2.3) | - | -4.0 (1.00, 60) | -4 (3.0/2.2) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2021 | -5.0 (1.00, 35) | -5 (3.6/2.2) | - | -4.0 (1.00, 62) | -4 (4.3/2.2) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2022 | -5.0 (1.00, 36) | -5 (5.7/1.9) | - | -4.0 (0.98, 62) | -4 (5.5/2.0) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2023 | -5.0 (0.83, 36) | -5 (7.5/2.7) | - | -4.0 (0.75, 60) | -4 (4.6/2.1) | - | ok (holes) / ok (holes) |
| USDCAD | 2024 | -5.0 (0.95, 37) | -5 (6.5/2.6) | - | -4.0 (1.00, 60) | -4 (9.7/3.6) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2025 | -5.0 (1.00, 36) | -5 (4.6/2.2) | - | -4.0 (1.00, 60) | -4 (4.3/1.9) | - | CONFIRMED / CONFIRMED |
| USDCAD | 2026 | -5.0 (1.00, 19) | -5 (4.2/2.8) | - | -4.0 (1.00, 53) | -4 (6.2/2.2) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2013 | -5.0 (1.00, 36) | -5 (4.4/2.0) | - | -4.0 (1.00, 60) | -4 (10.2/2.4) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2014 | -5.0 (1.00, 36) | -5 (8.9/3.1) | - | -4.0 (0.98, 60) | -4 (8.6/2.6) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2015 | -5.0 (1.00, 34) | -5 (5.7/2.5) | - | -4.0 (1.00, 60) | -4 (6.9/2.2) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2016 | -5.0 (0.97, 35) | -5 (4.6/2.5) | - | -4.0 (1.00, 62) | -4 (5.0/2.4) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2017 | -5.0 (0.97, 36) | -5 (4.7/2.5) | - | -4.0 (1.00, 62) | -4 (5.1/2.3) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2018 | -5.0 (1.00, 37) | -5 (3.5/2.3) | - | -4.0 (1.00, 62) | -4 (4.6/1.9) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2019 | -5.0 (1.00, 36) | -5 (6.5/2.1) | - | -4.0 (0.98, 60) | -4 (6.2/2.1) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2020 | -5.0 (0.97, 36) | -5 (3.5/2.1) | - | -4.0 (1.00, 60) | -4 (2.5/2.0) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2021 | -5.0 (0.97, 35) | -5 (4.4/2.5) | - | -4.0 (0.98, 62) | -4 (5.8/1.9) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2022 | -5.0 (1.00, 36) | -5 (6.3/2.3) | - | -4.0 (0.94, 62) | -4 (5.7/2.1) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2023 | -5.0 (0.89, 36) | -5 (8.7/2.6) | - | -4.0 (0.65, 60) | -4 (8.3/2.3) | - | CONFIRMED / ok (holes) |
| USDCHF | 2024 | -5.0 (1.00, 37) | -5 (9.8/2.6) | - | -4.0 (1.00, 60) | -4 (13.7/2.3) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2025 | -5.0 (1.00, 36) | -5 (5.2/2.4) | - | -4.0 (1.00, 60) | -4 (5.9/1.8) | - | CONFIRMED / CONFIRMED |
| USDCHF | 2026 | -5.0 (1.00, 19) | -5 (4.7/2.1) | - | -4.0 (1.00, 53) | -4 (6.0/1.8) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2013 | -5.0 (1.00, 36) | -5 (4.5/1.6) | - | -4.0 (1.00, 60) | -4 (6.7/1.6) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2014 | -5.0 (1.00, 36) | -5 (5.8/2.0) | - | -4.0 (0.98, 60) | -4 (7.1/1.5) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2015 | -5.0 (1.00, 34) | -5 (6.1/2.1) | - | -4.0 (1.00, 60) | -4 (6.9/1.8) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2016 | -5.0 (1.00, 35) | -5 (3.0/1.6) | - | -4.0 (1.00, 62) | -4 (4.1/1.6) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2017 | -5.0 (0.97, 36) | -5 (3.8/2.2) | - | -4.0 (1.00, 62) | -4 (4.9/1.6) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2018 | -5.0 (1.00, 37) | -5 (3.5/1.9) | - | -4.0 (1.00, 62) | -4 (4.7/1.6) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2019 | -5.0 (1.00, 36) | -5 (3.6/1.6) | - | -4.0 (0.98, 60) | -4 (4.6/1.9) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2020 | -5.0 (0.97, 36) | -5 (2.3/1.8) | - | -4.0 (1.00, 60) | -4 (2.5/1.8) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2021 | -5.0 (1.00, 35) | -5 (3.3/1.8) | - | -4.0 (1.00, 62) | -4 (5.3/1.9) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2022 | -5.0 (1.00, 36) | -5 (6.6/2.0) | - | -4.0 (1.00, 62) | -4 (6.0/2.0) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2023 | -5.0 (0.83, 36) | -5 (8.0/2.2) | - | -4.0 (0.72, 60) | -4 (7.3/1.9) | - | ok (holes) / ok (holes) |
| AUDUSD | 2024 | -5.0 (1.00, 37) | -5 (7.8/2.1) | - | -4.0 (1.00, 60) | -4 (10.3/2.2) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2025 | -5.0 (1.00, 36) | -5 (3.9/2.0) | - | -4.0 (1.00, 60) | -4 (4.8/1.6) | - | CONFIRMED / CONFIRMED |
| AUDUSD | 2026 | -5.0 (1.00, 19) | -5 (3.2/2.8) | - | -4.0 (1.00, 53) | -4 (5.2/1.8) | - | P1-ONLY / CONFIRMED |
| NZDUSD | 2013 | -5.0 (1.00, 36) | -5 (3.9/2.0) | - | -4.0 (1.00, 60) | -4 (6.7/1.7) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2014 | -5.0 (1.00, 36) | -5 (5.9/1.8) | - | -4.0 (0.98, 60) | -4 (7.5/1.6) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2015 | -5.0 (1.00, 34) | -5 (7.2/2.0) | - | -4.0 (1.00, 60) | -4 (6.8/1.6) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2016 | -5.0 (0.97, 35) | -5 (3.0/1.7) | - | -4.0 (1.00, 62) | -4 (4.6/1.7) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2017 | -5.0 (0.97, 36) | -5 (3.9/2.0) | - | -4.0 (1.00, 62) | -4 (4.7/1.6) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2018 | -5.0 (1.00, 37) | -5 (3.6/1.5) | - | -4.0 (1.00, 62) | -4 (4.6/1.5) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2019 | -5.0 (1.00, 36) | -5 (4.0/1.5) | - | -4.0 (0.98, 60) | -4 (5.6/1.7) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2020 | -5.0 (0.97, 36) | -5 (2.5/2.0) | - | -4.0 (1.00, 60) | -4 (2.5/1.9) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2021 | -5.0 (1.00, 35) | -5 (3.3/1.9) | - | -4.0 (1.00, 62) | -4 (5.2/1.8) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2022 | -5.0 (0.97, 36) | -5 (6.3/2.0) | - | -4.0 (1.00, 62) | -4 (6.0/2.1) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2023 | -5.0 (0.83, 36) | -5 (7.7/2.3) | - | -4.0 (0.70, 60) | -4 (6.1/1.8) | - | ok (holes) / ok (holes) |
| NZDUSD | 2024 | -5.0 (0.97, 37) | -5 (7.8/3.1) | - | -4.0 (1.00, 60) | -4 (11.1/2.2) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2025 | -5.0 (1.00, 36) | -5 (3.5/2.4) | - | -4.0 (1.00, 60) | -4 (5.2/1.8) | - | CONFIRMED / CONFIRMED |
| NZDUSD | 2026 | -5.0 (1.00, 19) | -5 (3.6/2.4) | - | -4.0 (1.00, 53) | -4 (5.4/1.7) | - | CONFIRMED / CONFIRMED |
| EURGBP | 2013 | -5.0 (1.00, 36) | -8 (3.3/2.5) | - | -4.0 (1.00, 60) | -8 (5.9/4.3) | - | P1-ONLY / P1-ONLY |
| EURGBP | 2014 | -5.0 (1.00, 36) | -5 (4.2/2.6) | - | -4.0 (0.98, 60) | -8 (4.3/3.5) | - | CONFIRMED / P1-ONLY |
| EURGBP | 2015 | -5.0 (1.00, 34) | -5 (4.7/4.0) | - | -4.0 (1.00, 60) | -4 (4.6/3.5) | - | CONFIRMED / CONFIRMED |
| EURGBP | 2016 | -5.0 (0.97, 35) | -5 (3.0/2.5) | - | -4.0 (1.00, 62) | -4 (2.7/2.7) | - | CONFIRMED / P1-ONLY |
| EURGBP | 2017 | -5.0 (0.97, 36) | -8 (2.5/2.3) | - | -4.0 (1.00, 62) | -7 (4.3/4.2) | - | P1-ONLY / P1-ONLY |
| EURGBP | 2018 | -5.0 (0.97, 37) | -8 (3.2/2.5) | - | -4.0 (1.00, 62) | -8 (3.2/3.2) | - | P1-ONLY / P1-ONLY |
| EURGBP | 2019 | -5.0 (1.00, 36) | +2 (5.3/3.2) | - | -4.0 (0.98, 60) | -4 (2.9/2.3) | - | P1-ONLY / CONFIRMED |
| EURGBP | 2020 | -5.0 (0.97, 36) | -5 (2.7/2.3) | - | -4.0 (1.00, 60) | -4 (2.6/2.2) | - | CONFIRMED / CONFIRMED |
| EURGBP | 2021 | -5.0 (1.00, 35) | -8 (3.9/3.0) | - | -4.0 (1.00, 62) | -4 (3.4/2.7) | - | P1-ONLY / CONFIRMED |
| EURGBP | 2022 | -5.0 (1.00, 36) | -8 (3.8/2.8) | - | -4.0 (1.00, 62) | -7 (3.8/2.9) | - | P1-ONLY / P1-ONLY |
| EURGBP | 2023 | -5.0 (0.86, 36) | -5 (4.4/4.4) | - | -4.0 (0.72, 60) | -7 (4.4/4.3) | - | P1-ONLY / ok (holes) |
| EURGBP | 2024 | -5.0 (1.00, 37) | -8 (4.0/3.4) | - | -4.0 (1.00, 60) | -4 (4.7/3.8) | - | P1-ONLY / CONFIRMED |
| EURGBP | 2025 | -5.0 (1.00, 36) | -8 (4.4/2.8) | - | -4.0 (1.00, 60) | -4 (3.7/2.6) | - | P1-ONLY / CONFIRMED |
| EURGBP | 2026 | -5.0 (1.00, 19) | -8 (4.3/3.5) | - | -4.0 (1.00, 53) | -7 (2.4/2.3) | - | P1-ONLY / P1-ONLY |
| EURJPY | 2013 | -5.0 (1.00, 36) | -5 (2.7/1.8) | - | -4.0 (1.00, 60) | -4 (4.8/1.8) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2014 | -5.0 (1.00, 36) | -5 (5.4/2.0) | - | -4.0 (0.98, 60) | -4 (3.9/1.9) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2015 | -5.0 (1.00, 34) | -5 (4.6/2.1) | - | -4.0 (1.00, 60) | -4 (4.5/1.9) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2016 | -5.0 (0.97, 35) | -5 (3.0/1.7) | - | -4.0 (1.00, 62) | -4 (2.9/1.6) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2017 | -5.0 (0.97, 36) | -5 (2.6/2.0) | - | -4.0 (1.00, 62) | -4 (3.6/1.9) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2018 | -5.0 (1.00, 37) | -6 (2.5/2.2) | - | -4.0 (1.00, 62) | -4 (2.5/1.8) | - | P1-ONLY / CONFIRMED |
| EURJPY | 2019 | -5.0 (1.00, 36) | -5 (3.2/2.3) | - | -4.0 (0.98, 60) | -4 (3.7/2.3) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2020 | -5.0 (0.97, 36) | -5 (2.5/1.7) | - | -4.0 (1.00, 60) | -4 (2.6/2.1) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2021 | -5.0 (1.00, 35) | -5 (2.9/2.1) | - | -4.0 (1.00, 62) | -4 (3.2/2.1) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2022 | -5.0 (1.00, 36) | -5 (3.5/1.9) | - | -4.0 (1.00, 62) | -4 (3.6/1.7) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2023 | -5.0 (0.89, 36) | -5 (4.9/2.1) | - | -4.0 (0.75, 60) | -4 (4.9/2.0) | - | CONFIRMED / ok (holes) |
| EURJPY | 2024 | -5.0 (0.95, 37) | -5 (3.7/1.8) | - | -4.0 (1.00, 60) | -4 (6.5/1.9) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2025 | -5.0 (1.00, 36) | -5 (3.1/1.9) | - | -4.0 (1.00, 60) | -4 (3.8/2.0) | - | CONFIRMED / CONFIRMED |
| EURJPY | 2026 | -5.0 (1.00, 19) | -5 (2.8/2.1) | - | -4.0 (1.00, 53) | -4 (2.9/2.1) | - | CONFIRMED / CONFIRMED |
| GBPJPY | 2013 | -5.0 (1.00, 36) | -5 (3.0/1.9) | - | -4.0 (1.00, 60) | -4 (4.9/3.3) | - | CONFIRMED / CONFIRMED |
| GBPJPY | 2014 | -5.0 (1.00, 36) | -5 (6.9/1.8) | - | -4.0 (0.98, 60) | -4 (3.9/3.2) | - | CONFIRMED / CONFIRMED |
| GBPJPY | 2015 | -5.0 (1.00, 34) | -5 (3.8/3.1) | - | -4.0 (1.00, 60) | -4 (3.7/3.3) | - | CONFIRMED / P1-ONLY |
| GBPJPY | 2016 | -5.0 (0.97, 35) | -5 (2.9/2.1) | - | -4.0 (1.00, 62) | -4 (2.9/2.6) | - | CONFIRMED / P1-ONLY |
| GBPJPY | 2017 | -5.0 (0.97, 36) | -5 (2.9/1.8) | - | -4.0 (1.00, 62) | -8 (3.6/3.6) | - | CONFIRMED / P1-ONLY |
| GBPJPY | 2018 | -5.0 (1.00, 37) | -8 (2.4/2.0) | - | -4.0 (1.00, 62) | -8 (2.7/2.5) | - | P1-ONLY / P1-ONLY |
| GBPJPY | 2019 | -5.0 (1.00, 36) | -5 (3.2/3.0) | - | -4.0 (0.98, 60) | -4 (3.5/2.1) | - | P1-ONLY / CONFIRMED |
| GBPJPY | 2020 | -5.0 (0.97, 36) | -5 (2.4/2.0) | - | -4.0 (1.00, 60) | -4 (2.5/2.2) | - | CONFIRMED / P1-ONLY |
| GBPJPY | 2021 | -5.0 (1.00, 35) | -8 (3.6/2.1) | - | -4.0 (1.00, 62) | -4 (3.0/2.4) | - | P1-ONLY / CONFIRMED |
| GBPJPY | 2022 | -5.0 (1.00, 36) | -5 (2.7/2.3) | - | -4.0 (1.00, 62) | -4 (3.4/2.5) | - | CONFIRMED / CONFIRMED |
| GBPJPY | 2023 | -5.0 (0.86, 36) | -5 (3.9/3.0) | - | -4.0 (0.67, 60) | -4 (3.8/2.8) | - | CONFIRMED / ok (holes) |
| GBPJPY | 2024 | -5.0 (1.00, 37) | -5 (4.1/2.5) | - | -4.0 (0.98, 60) | -4 (6.9/1.9) | - | CONFIRMED / CONFIRMED |
| GBPJPY | 2025 | -5.0 (0.97, 36) | -5 (3.0/2.5) | - | -4.0 (1.00, 60) | -4 (4.6/2.1) | - | CONFIRMED / CONFIRMED |
| GBPJPY | 2026 | -5.0 (1.00, 19) | -5 (3.5/3.0) | - | -4.0 (0.98, 53) | -4 (3.0/2.0) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2013 | -5.0 (1.00, 36) | -5 (3.6/1.5) | - | -4.0 (1.00, 60) | -4 (4.1/1.4) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2014 | -5.0 (1.00, 36) | -5 (3.6/1.5) | - | -4.0 (0.98, 60) | -4 (3.2/1.6) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2015 | -5.0 (1.00, 34) | -5 (2.5/1.6) | - | -4.0 (1.00, 60) | -4 (3.3/1.4) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2016 | -5.0 (0.97, 35) | -5 (3.4/1.9) | - | -4.0 (1.00, 62) | -4 (2.3/1.5) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2017 | -5.0 (0.97, 36) | -5 (2.8/1.8) | - | -4.0 (1.00, 62) | -4 (3.6/1.7) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2018 | -5.0 (1.00, 37) | -5 (2.2/1.7) | - | -4.0 (1.00, 62) | -4 (2.9/1.6) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2019 | -5.0 (1.00, 36) | -5 (3.1/1.8) | - | -4.0 (0.98, 60) | -4 (3.7/1.8) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2020 | -5.0 (0.97, 36) | -5 (2.0/1.7) | - | -4.0 (1.00, 60) | -4 (3.0/1.8) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2021 | -5.0 (1.00, 35) | -5 (2.5/1.7) | - | -4.0 (1.00, 62) | -4 (2.9/2.0) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2022 | -5.0 (0.97, 36) | -5 (3.5/1.5) | - | -4.0 (0.98, 62) | -4 (3.8/1.6) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2023 | -5.0 (0.89, 36) | -5 (3.1/2.0) | - | -4.0 (0.73, 60) | -4 (4.8/1.6) | - | CONFIRMED / ok (holes) |
| AUDJPY | 2024 | -5.0 (1.00, 37) | -5 (3.0/2.0) | - | -4.0 (1.00, 60) | -4 (6.8/1.9) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2025 | -5.0 (1.00, 36) | -5 (3.3/1.9) | - | -4.0 (1.00, 60) | -4 (3.8/1.8) | - | CONFIRMED / CONFIRMED |
| AUDJPY | 2026 | -5.0 (1.00, 19) | -5 (3.8/2.3) | - | -4.0 (0.98, 53) | -4 (2.8/1.6) | - | CONFIRMED / CONFIRMED |
| AUDNZD | 2013 | -5.0 (1.00, 36) | -5 (2.9/1.8) | - | -4.0 (1.00, 60) | -4 (2.8/1.5) | - | CONFIRMED / CONFIRMED |
| AUDNZD | 2014 | -5.0 (1.00, 36) | -5 (2.6/1.8) | - | -4.0 (0.98, 60) | -4 (2.9/1.6) | - | CONFIRMED / CONFIRMED |
| AUDNZD | 2015 | -5.0 (1.00, 34) | -5 (3.5/1.7) | - | -4.0 (1.00, 60) | -4 (2.8/1.2) | - | CONFIRMED / CONFIRMED |
| AUDNZD | 2016 | -5.0 (0.97, 35) | -5 (2.5/1.7) | - | -4.0 (1.00, 62) | -4 (2.7/1.4) | - | CONFIRMED / CONFIRMED |
| AUDNZD | 2017 | -5.0 (0.97, 36) | +2 (2.2/1.9) | - | -4.0 (1.00, 62) | -4 (2.5/1.5) | - | P1-ONLY / CONFIRMED |
| AUDNZD | 2018 | -5.0 (1.00, 37) | -5 (2.0/1.6) | - | -4.0 (1.00, 62) | -4 (2.6/1.6) | - | CONFIRMED / CONFIRMED |
| AUDNZD | 2019 | -5.0 (1.00, 36) | +2 (2.4/2.2) | - | -4.0 (0.98, 60) | -4 (3.9/1.7) | - | P1-ONLY / CONFIRMED |
| AUDNZD | 2020 | -5.0 (0.97, 36) | +2 (3.0/2.0) | - | -4.0 (1.00, 60) | -4 (2.6/1.4) | - | P1-ONLY / CONFIRMED |
| AUDNZD | 2021 | -5.0 (1.00, 35) | +2 (3.6/2.6) | - | -4.0 (1.00, 62) | -4 (2.9/2.0) | - | P1-ONLY / CONFIRMED |
| AUDNZD | 2022 | -5.0 (0.97, 36) | -5 (3.2/1.5) | - | -4.0 (1.00, 62) | -4 (2.9/1.6) | - | CONFIRMED / CONFIRMED |
| AUDNZD | 2023 | -5.0 (0.83, 36) | -5 (3.1/1.8) | - | -4.0 (0.68, 60) | -4 (3.4/1.6) | - | ok (holes) / ok (holes) |
| AUDNZD | 2024 | -5.0 (1.00, 37) | -5 (2.6/1.5) | - | -4.0 (0.98, 60) | -4 (4.1/1.6) | - | CONFIRMED / CONFIRMED |
| AUDNZD | 2025 | -5.0 (1.00, 36) | -5 (2.3/1.4) | - | -4.0 (1.00, 60) | -4 (2.3/1.6) | - | CONFIRMED / CONFIRMED |
| AUDNZD | 2026 | -5.0 (1.00, 19) | -5 (3.3/1.9) | - | -4.0 (1.00, 53) | -4 (2.7/1.3) | - | CONFIRMED / CONFIRMED |

Verdict counts over 17 symbols x 14 years x 2 seasons: CONFIRMED: 415, P1-ONLY: 38, ok (holes): 23.

## 5. Exact switch dates from the daily 17:00-NY break (XAUUSD, XAGUSD, NAS100, SPX500)

Per symbol-year, the first Mon-Thu day in Mar-Apr and the last Mon-Thu day in Oct-Nov on which the daily break sits at its summer position, compared with the dates of the US rule and the EU rule (first Monday after the switch Sunday / the Thursday before the end Sunday). "US/US" = both ends follow US dates; "EU/EU" = both follow EU dates.

| Year | NAS100 | SPX500 | XAGUSD | XAUUSD |
|---|---|---|---|---|
| 2013 | US/US | US/US | US/US | US/US |
| 2014 | US/US | US/US | US/US | US/US |
| 2015 | US/US | US/US | US/US | US/US |
| 2016 | US/US | US/US | US/US | US/US |
| 2017 | US/US | US/US | US/US | US/US |
| 2018 | US/US | US/US | US/US | US/US |
| 2019 | EU/EU | EU/EU | EU/EU | EU/EU |
| 2020 | EU/EU | EU/EU | EU/EU | EU/EU |
| 2021 | EU/EU | EU/EU | EU/EU | EU/EU |
| 2022 | EU/EU | EU/EU | EU/EU | EU/EU |
| 2023 | EU/EU | EU/EU | other(2023-04-10)/EU | other(2023-04-24)/EU |
| 2024 | EU/other(2024-10-23) | EU/EU | EU/other(2024-10-23) | EU/EU |
| 2025 | EU/EU | EU/EU | EU/EU | EU/EU |
| 2026 | EU/other(2026-10-08) | EU/other(2026-10-08) | EU/other(2026-10-08) | EU/other(2026-10-08) |

Exceptions: 2023 XAGUSD/XAUUSD start "other" = hole-ridden spring 2023 (break not detectable on many days, the following detected day is 2023-04-10/04-24); 2024-10-23 for NAS100/XAGUSD = Thursday 2024-10-24 has no detectable break (last Wed observed; the Oct-27 EU switch is clearly followed by 2024-10-28 winter break); 2026-10-08 = data end (the 2026-10-25 EU switch has not happened yet). All other 40 symbol-years in 2013-2025 follow the stated rule to the day. Nothing in the data supports a different date for any switch.

## 6. Mismatch cells M and O, pooled by era (the decisive weeks)

Expected: era 2013-18: M and O offset -4 (NY rule); era 2019-26: M and O offset -5 (EU rule). P1 = median implied offset, n events; P2/P3 as above (pooled over years, so n is large enough).

| Symbol | Era | Cell | P1 | n | P2 | P3 |
|---|---|---|---|---|---|---|
| SPX500 | 2013-18 | M | -4.00 | 28 | -3 (2.6/2.4) | -4 (3.2/2.1) |
| SPX500 | 2013-18 | O | -4.00 | 12 | +2 (2.5/2.3) | -4 (2.8/2.0) |
| SPX500 | 2019-26 | M | -5.00 | 41 | -5 (3.0/2.8) | -5 (3.5/2.3) |
| SPX500 | 2019-26 | O | -5.00 | 14 | -4 (4.0/3.9) | -5 (3.9/3.5) |
| DAX | 2013-18 | M | -4.00 | 28 | -4 (1.7/1.7) | -4 (3.1/2.0) |
| DAX | 2013-18 | O | -4.00 | 12 | -4 (2.0/1.7) | -4 (2.8/2.3) |
| DAX | 2019-26 | M | -5.03 | 41 | -5 (1.9/1.8) | -5 (2.9/2.8) |
| DAX | 2019-26 | O | -5.00 | 14 | -5 (2.8/2.1) | -5 (4.3/2.8) |
| NAS100 | 2013-18 | M | -4.00 | 28 | -3 (3.8/3.4) | -4 (4.3/2.7) |
| NAS100 | 2013-18 | O | -4.00 | 12 | -3 (4.0/2.9) | -4 (4.5/2.3) |
| NAS100 | 2019-26 | M | -5.00 | 41 | -4 (3.5/3.1) | -5 (4.2/2.5) |
| NAS100 | 2019-26 | O | -5.00 | 14 | -4 (5.4/4.9) | -5 (4.9/3.9) |
| XAGUSD | 2013-18 | M | -4.00 | 29 | -4 (3.9/2.3) | - |
| XAGUSD | 2013-18 | O | -4.00 | 12 | -4 (4.6/3.5) | - |
| XAGUSD | 2019-26 | M | -5.00 | 41 | -5 (2.3/2.2) | - |
| XAGUSD | 2019-26 | O | -5.00 | 14 | -5 (4.3/3.1) | - |
| XAUUSD | 2013-18 | M | -4.00 | 29 | -4 (3.8/2.4) | - |
| XAUUSD | 2013-18 | O | -4.00 | 12 | -4 (5.0/3.4) | - |
| XAUUSD | 2019-26 | M | -5.00 | 41 | -5 (2.4/2.1) | - |
| XAUUSD | 2019-26 | O | -5.00 | 14 | -5 (4.5/2.9) | - |
| USDJPY | 2013-18 | M | -4.00 | 30 | -4 (3.6/2.1) | - |
| USDJPY | 2013-18 | O | -4.00 | 12 | -4 (5.2/2.3) | - |
| USDJPY | 2019-26 | M | -5.00 | 42 | -5 (2.6/1.7) | - |
| USDJPY | 2019-26 | O | -5.00 | 14 | -5 (5.9/2.3) | - |
| GBPUSD | 2013-18 | M | -4.00 | 30 | -7 (3.9/3.2) | - |
| GBPUSD | 2013-18 | O | -4.00 | 12 | -6 (5.4/4.5) | - |
| GBPUSD | 2019-26 | M | -5.00 | 42 | -5 (2.6/2.0) | - |
| GBPUSD | 2019-26 | O | -5.00 | 14 | -5 (5.5/3.8) | - |
| AUDJPY | 2013-18 | M | -4.00 | 30 | -4 (2.1/1.8) | - |
| AUDJPY | 2013-18 | O | -4.00 | 12 | -4 (2.4/1.8) | - |
| AUDJPY | 2019-26 | M | -5.00 | 42 | -5 (1.9/1.4) | - |
| AUDJPY | 2019-26 | O | -5.00 | 14 | -5 (2.4/1.8) | - |
| EURGBP | 2013-18 | M | -4.00 | 30 | -7 (3.9/2.7) | - |
| EURGBP | 2013-18 | O | -4.00 | 12 | -6 (5.2/3.3) | - |
| EURGBP | 2019-26 | M | -5.00 | 42 | -5 (2.3/2.2) | - |
| EURGBP | 2019-26 | O | -5.00 | 14 | -7 (5.1/3.5) | - |
| USDCAD | 2013-18 | M | -4.00 | 30 | -4 (4.4/2.5) | - |
| USDCAD | 2013-18 | O | -4.00 | 12 | -4 (7.3/3.0) | - |
| USDCAD | 2019-26 | M | -5.00 | 42 | -5 (3.0/1.8) | - |
| USDCAD | 2019-26 | O | -4.99 | 14 | -5 (5.2/2.7) | - |
| EURJPY | 2013-18 | M | -4.00 | 30 | -4 (2.7/1.9) | - |
| EURJPY | 2013-18 | O | -4.00 | 12 | -4 (2.6/1.8) | - |
| EURJPY | 2019-26 | M | -5.00 | 42 | -5 (2.0/1.7) | - |
| EURJPY | 2019-26 | O | -5.00 | 14 | -5 (3.2/1.7) | - |
| EURUSD | 2013-18 | M | -4.00 | 30 | -4 (3.9/2.7) | - |
| EURUSD | 2013-18 | O | -4.00 | 12 | -4 (6.0/3.3) | - |
| EURUSD | 2019-26 | M | -5.00 | 42 | -5 (2.7/1.6) | - |
| EURUSD | 2019-26 | O | -5.00 | 14 | -5 (6.9/2.8) | - |
| GBPJPY | 2013-18 | M | -4.00 | 30 | -7 (3.0/2.4) | - |
| GBPJPY | 2013-18 | O | -4.00 | 12 | -6 (4.3/2.8) | - |
| GBPJPY | 2019-26 | M | -5.00 | 42 | -5 (1.9/1.7) | - |
| GBPJPY | 2019-26 | O | -5.00 | 14 | -7 (3.5/3.2) | - |
| AUDUSD | 2013-18 | M | -4.00 | 30 | -4 (3.0/2.2) | - |
| AUDUSD | 2013-18 | O | -4.00 | 12 | -4 (4.8/2.3) | - |
| AUDUSD | 2019-26 | M | -5.00 | 42 | -5 (2.4/1.7) | - |
| AUDUSD | 2019-26 | O | -4.99 | 14 | -5 (4.7/2.7) | - |
| AUDNZD | 2013-18 | M | -4.00 | 30 | -4 (1.8/1.7) | - |
| AUDNZD | 2013-18 | O | -4.00 | 12 | +2 (3.3/2.6) | - |
| AUDNZD | 2019-26 | M | -5.00 | 42 | -5 (1.9/1.8) | - |
| AUDNZD | 2019-26 | O | -5.00 | 14 | -5 (2.5/2.0) | - |
| NZDUSD | 2013-18 | M | -4.00 | 30 | -4 (2.8/2.4) | - |
| NZDUSD | 2013-18 | O | -4.00 | 12 | -4 (4.5/2.5) | - |
| NZDUSD | 2019-26 | M | -5.00 | 42 | -5 (2.6/1.6) | - |
| NZDUSD | 2019-26 | O | -5.00 | 14 | -5 (5.0/2.7) | - |
| USDCHF | 2013-18 | M | -4.00 | 30 | -4 (4.0/2.5) | - |
| USDCHF | 2013-18 | O | -4.00 | 12 | -4 (5.5/3.2) | - |
| USDCHF | 2019-26 | M | -5.00 | 42 | -5 (2.6/1.6) | - |
| USDCHF | 2019-26 | O | -5.00 | 14 | -5 (6.4/2.4) | - |

P1 is -4.0 (2013-18) or -5.0 (2019-26) for every one of the 17 symbols; the spike probes agree for XAU, XAG, DAX (P3, EU anchored) and all liquid FX pairs, and are weak or off by one hour for a few quiet crosses and for the index 08:30 confound (see the printed ratios).

## 7. Validation after conversion

After applying the rule the weekly open of all 12 FX pairs is at 22:00 UTC (NY winter) / 21:00 UTC (NY summer), independent of cell and year; XAU/XAG open at 23:00/22:00 UTC and close at 22:00/21:00 UTC; share of weeks within 5 min of the expected UTC time: FX close 97-98% in every cell, FX open 89-94%, metals 91-99% (the misses are hole weeks, concentrated in 2021-2023). There is no cell dependence, which is the signature of a correct conversion (a wrong rule would show a one-hour error in exactly the M/O cells). DJI converted data (section 9) correlates 0.93 (SPX500) / 0.79 (NAS100) at lag 0 minutes and ~0.00 at lag +-1 minute and +-1 hour, and the 2025-05-20..2026-10 part of the DJI export (which is genuinely UTC) agrees at lag 0 with the converted HistData SPX500/NAS100, an independent check on the 2025-26 HistData rule.

## 8. Ambiguities, anomalies and data problems found while probing

* No symbol-year is ambiguous about the clock rule. Where "AMBIGUOUS" or "ok (holes)" appears in section 4, the cause is missing data (2023, plus some 2020-2022 FX weeks), not a different clock.
* HistData 2023 (Feb-Jul in particular) has many missing whole hours in the FX pairs (e.g. EURUSD 2023-05-12: no bars in hours 06, 09-10, 13-14 label). Weekly open/close and 08:30 probes are less clean there; the daily break/weekly consensus still follow the EU rule.
* EU DST Sundays 2019-2026: the files repeat the label hour 19:00-19:59 on the autumn Sunday (60 bit-identical bars; the label clock itself does not repeat: first copy is a spurious file duplicate of real UTC 00:00-00:59, checked against the DJI export) and lack that hour on the spring Sunday (23:00-23:59 UTC missing). With the date-based boundary used (00:00 label on the switch Sunday) both are handled correctly: the duplicate collapses to one bar per minute and the hole stays a hole. Verified on 2025-10-26 and 2026-03-29 against the DJI export (converted HistData SPX500/NAS100 correlates 0.93-0.97 in the UTC hours before and after, hour-0 copy 0.74).
* Single anomalous weeks (holiday/hole weeks) listed in section 3.
* NAS100 and SPX500: the daily break/close moved from 17:00 NY to 16:15 NY (weekly close 16:15 NY, daily break begins 16:15 NY) from about 2018-12-17; it is a change of the product session, not of the clock (offset between W and S stays exactly one hour). The weekly open is Sunday 18:00 NY throughout.
* DAX (GRXEUR): daily/weekly session structure changed in 2019 (Sunday 18:00 NY opening from 2019, previously Monday 02:00 NY = 08:00 CET); P1 for DAX is therefore measured relative to its own winter value of each year with an EU-anchored expectation; P3 (09:00 CET open) agrees in 26 of 28 W/S cells. Start dates: NAS100 2013-01-02, SPX500 2013-01-02, DAX 2013-01-02 (see CLEAN_REPORT.md).
* EURGBP, AUDNZD, GBPJPY: 08:30 ET spike weak (quiet pairs at NY data time); clock established by P1 and by the pooled analysis.

## 9. DJIUSD.prop MT5 export (US30): the clock is NOT UTC for most of the history

The user stated GMT+0. Measured against the converted SPX500/NAS100 minute returns (day-by-day cross-correlation of 1-minute returns over label offsets -1..+5 h; median correlation 0.80, 2428 days; confirmed by the position of the daily 17:00 NY break and by 08:30/09:30 ET spikes) the label-UTC offset is:

| Label period (Mon-Fri days) | label - UTC | Comment |
|---|---|---|
| 2016-01-04 .. 2016-12-22 | unknown | one DAILY bar per day, stamped 02:00; not M1, clock not verifiable |
| 2016-12-23 .. 2017-05-12 13:45 | +3h winter / +4h summer (EU dates; March 2017 switch is mixed in the monthly test, consistent with 2017-03-26) | HOURLY bars (04:00..23:00 stamps), not M1; verified against SPX500 hourly returns (corr 0.93-0.96 at that offset, ~0 at others) |
| 2017-05-12 13:46 .. 2018-04-04 16:37 | +3h winter (2017-10-29 .. 2018-03-24), +4h summer (2017-05 .. 2017-10-28, 2018-03-25 ..) | first true M1 bars; session 04:00-23:59 label only (no overnight data); day-level corr 0.70-0.77 |
| 2018-04-04 16:39 .. 2021-03-19 | +2h constant (no DST at all, even in summer) | source switch inside the series: label minute 2018-04-04 16:37 belongs to the old source (+4 => 12:37 UTC), 16:39 to the new one (+2 => 14:39 UTC): a 2-hour hole in real time. Session becomes 00:05-22:54 label with a break 22:55-00:05 (shifted 5 minutes vs NY 17:00) |
| 2021-03-20 .. 2025-05-19 21:xx | +3h from 2021-03-21 (note: one week BEFORE the EU date 2021-03-28), then EU rule: +3 summer (last Sun Mar .. last Sun Oct), +2 winter; 2021-10-31, 2022-03-27/10-30, 2023-03-26/10-29, 2024-03-31/10-27, 2025-03-30 | EET/EEST-style server time; the daily break stays at 00:00 label except in the mismatch weeks (23:00) |
| 2025-05-19 22:01 .. 2026-10-15 02:16 | 0h = true UTC | a second source switch inside the series: label minute 2025-05-19 22:00 is still old (+3 => 19:00 UTC), 22:01 is new (UTC): a 3-hour hole in real time. On 2025-05-20..05-22 an extra 1-hour gap 23:58-01:00 UTC is visible, from 2025-05-27 only the normal 20:57-22:01 UTC (21:00 UTC) break remains |

So the claim "GMT+0" holds only for the last ~17 months (from 2025-05-19 22:01 label). Earlier bars were converted bar by bar with the table above (utc = label - k). After conversion: 0 of 2428 days prefer a different hour offset, minute-lag correlation peaks only at 0 (SPX500 0.926, NAS100 0.790), no duplicate or non-monotone timestamps, no Saturday bars. The two source switches create real-time holes of 2h (2018-04-04 12:37..14:39 UTC) and 3h (2025-05-19 19:00..22:01 UTC) that are real gaps in the converted series.

Periods treated as M1: from 2017-05-12 13:46 label (= 09:46 UTC). The earlier daily (2016-01-04..2016-12-22, 252 bars) and hourly (2016-12-23..2017-05-12 13:00, 1939 bars) data are kept separately in bin/US30_coarse_pre_m1_utc.npy (fields t, o, h, l, c, tf [seconds per bar], t_label; t for hourly bars = UTC via the +3/+4 rule, for daily bars t = the calendar date at 00:00 UTC and is NOT a verified clock). They are NOT in US30_m1.bin / US30_utc.npy.
