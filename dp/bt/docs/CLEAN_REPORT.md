# CLEAN_REPORT: cleaning and coverage of the converted files (bin/*_m1.bin = London clock, bin/*_utc.npy = true UTC)

Cleaning order per symbol: concatenate all CSVs, stable sort by label time, drop duplicate label timestamps (keep first), drop bad bars (NaN, high<low, any price <= 0, open or close outside [low, high]), convert label to true UTC with the verified rule (CLOCK_REPORT.md), re-check ordering/duplicates after conversion (none arose), drop flat bars (o=h=l=c) that fall in the weekend window (UTC Saturday, Friday >= 22:00, Sunday < 20:00; none existed). Flat bars during the trading week are kept (they are real quiet minutes). Prices unadjusted. US30 = DJIUSD.prop MT5 export, M1 part only (see CLOCK_REPORT section 9 for its clock).

| Symbol | First bar (UTC) | Last bar (UTC) | Rows kept | Raw rows | Dup label ts dropped (conflicting) | Bad bars dropped (h<l / <=0 / o,c outside) | Weekend flat dropped | Flat bars kept | 1-min moves > 300bp |
|---|---|---|---|---|---|---|---|---|---|
| XAUUSD | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 4,824,644 | 4,825,852 | 1208 (787) | 0 (0 / 0 / 0) | 0 | 16,592 | 0 |
| XAGUSD | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 4,690,820 | 4,691,505 | 685 (265) | 0 (0 / 0 / 0) | 0 | 132,421 | 17 |
| NAS100 | 2013-01-02 11:00:00 | 2026-10-09 20:13:00 | 4,601,360 | 4,602,166 | 806 (386) | 0 (0 / 0 / 0) | 0 | 127,159 | 9 |
| SPX500 | 2013-01-02 11:00:00 | 2026-10-09 20:13:00 | 4,105,712 | 4,106,133 | 421 (6) | 0 (0 / 0 / 0) | 0 | 309,728 | 6 |
| DAX | 2013-01-02 07:00:00 | 2026-10-09 19:58:00 | 3,336,556 | 3,336,732 | 176 (6) | 0 (0 / 0 / 0) | 0 | 86,764 | 15 |
| US30 | 2017-05-12 09:46:00 | 2026-10-15 02:16:00 | 3,233,023 | 3,233,023 | 0 (-) | 0 (0 / 0 / 0) | 0 | 15,060 | 2 |
| USDJPY | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,064,711 | 5,065,238 | 527 (107) | 0 (0 / 0 / 0) | 0 | 59,162 | 0 |
| AUDJPY | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,077,266 | 5,077,785 | 519 (99) | 0 (0 / 0 / 0) | 0 | 30,901 | 1 |
| EURGBP | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,046,830 | 5,047,260 | 430 (10) | 0 (0 / 0 / 0) | 0 | 81,536 | 0 |
| GBPUSD | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,067,496 | 5,067,932 | 436 (16) | 0 (0 / 0 / 0) | 0 | 65,315 | 0 |
| USDCAD | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,031,092 | 5,031,533 | 441 (22) | 0 (0 / 0 / 0) | 0 | 93,638 | 0 |
| EURUSD | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,066,517 | 5,066,883 | 366 (6) | 0 (0 / 0 / 0) | 0 | 69,087 | 0 |
| EURJPY | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,074,715 | 5,075,362 | 647 (226) | 0 (0 / 0 / 0) | 0 | 19,083 | 1 |
| GBPJPY | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,069,002 | 5,069,707 | 705 (284) | 0 (0 / 0 / 0) | 0 | 22,672 | 0 |
| AUDUSD | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,050,338 | 5,050,766 | 428 (8) | 0 (0 / 0 / 0) | 0 | 80,311 | 0 |
| AUDNZD | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,068,400 | 5,068,856 | 456 (35) | 0 (0 / 0 / 0) | 0 | 51,663 | 1 |
| NZDUSD | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,046,297 | 5,046,735 | 438 (18) | 0 (0 / 0 / 0) | 0 | 98,268 | 0 |
| USDCHF | 2013-01-01 22:00:00 | 2026-10-09 20:58:00 | 5,045,305 | 5,045,732 | 427 (8) | 0 (0 / 0 / 0) | 0 | 146,907 | 1 |

Bad-bar counts are zero for every symbol: HistData bars are internally consistent. The only losses are duplicate label timestamps. Two kinds: (1) every EU fall-back Sunday 2019-2025 the files contain the label hour 19:00-19:59 twice, 60 bars, bit-identical copies (verified on SPX500 2025-10-26: that hour is real UTC 00:00-00:59, the extra copy is a pure file duplicate; correlation 0.74 with the DJI export at that UTC hour, -0.08 at 23:00 UTC) - dropping the second copy is correct; (2) the 2026 monthly files (2026-06 to 2026-10) contain pairs of consecutive minutes written twice with DIFFERENT OHLC (e.g. XAUUSD 786 bars, EURJPY 226; all in 2026, appear to be an overlap/rewind glitch of the monthly update files). Rule applied as instructed: keep the first occurrence in file order; which of the two copies is the better bar cannot be decided from the data. The counts in brackets are the conflicting ones.

## Bars per UTC year

| Symbol | 2013 | 2014 | 2015 | 2016 | 2017 | 2018 | 2019 | 2020 | 2021 | 2022 | 2023 | 2024 | 2025 | 2026 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| XAUUSD | 355,160 | 351,606 | 351,666 | 353,415 | 352,360 | 353,778 | 352,628 | 354,291 | 353,386 | 354,568 | 308,752 | 355,592 | 353,951 | 273,491 |
| XAGUSD | 342,532 | 335,480 | 333,551 | 338,733 | 322,329 | 345,881 | 334,830 | 351,420 | 350,676 | 351,300 | 304,497 | 353,319 | 353,641 | 272,631 |
| NAS100 | 314,774 | 318,873 | 335,257 | 341,629 | 330,380 | 348,607 | 341,335 | 339,962 | 342,946 | 342,711 | 299,289 | 340,926 | 340,493 | 264,178 |
| SPX500 | 229,617 | 233,233 | 281,125 | 282,913 | 222,026 | 310,381 | 308,298 | 334,202 | 333,528 | 341,611 | 291,380 | 334,338 | 339,076 | 263,984 |
| DAX | 210,927 | 209,954 | 211,996 | 213,764 | 206,648 | 213,273 | 325,977 | 259,840 | 187,516 | 205,728 | 159,626 | 332,439 | 335,844 | 263,024 |
| US30 | 0 | 0 | 0 | 0 | 182,409 | 332,381 | 344,990 | 345,171 | 349,280 | 353,955 | 351,338 | 343,626 | 350,601 | 279,272 |
| USDJPY | 370,808 | 366,741 | 371,891 | 372,417 | 371,298 | 371,800 | 372,525 | 372,140 | 369,333 | 372,536 | 322,896 | 372,023 | 371,643 | 286,660 |
| AUDJPY | 371,662 | 370,824 | 372,641 | 373,161 | 372,157 | 372,850 | 372,885 | 372,755 | 372,112 | 372,449 | 322,787 | 372,377 | 371,817 | 286,789 |
| EURGBP | 366,931 | 360,785 | 370,668 | 371,701 | 370,253 | 369,384 | 372,095 | 372,416 | 370,127 | 372,211 | 322,377 | 370,260 | 371,225 | 286,397 |
| GBPUSD | 370,762 | 368,916 | 372,231 | 372,480 | 371,204 | 372,261 | 372,336 | 372,997 | 370,338 | 372,434 | 322,407 | 372,047 | 371,031 | 286,052 |
| USDCAD | 364,836 | 358,110 | 368,537 | 369,715 | 366,222 | 367,942 | 372,245 | 372,233 | 369,364 | 371,340 | 321,279 | 370,884 | 371,992 | 286,393 |
| EURUSD | 370,611 | 366,477 | 372,210 | 372,679 | 371,635 | 372,607 | 372,470 | 372,275 | 369,442 | 372,805 | 322,578 | 372,379 | 372,024 | 286,325 |
| EURJPY | 371,877 | 370,892 | 372,605 | 373,130 | 372,258 | 373,304 | 372,550 | 372,647 | 371,245 | 372,134 | 323,040 | 371,471 | 371,923 | 285,639 |
| GBPJPY | 371,327 | 370,155 | 371,946 | 371,282 | 370,949 | 372,495 | 372,042 | 372,649 | 371,509 | 372,229 | 322,071 | 372,582 | 371,016 | 286,750 |
| AUDUSD | 370,120 | 365,190 | 371,187 | 371,005 | 368,436 | 369,457 | 371,938 | 371,815 | 369,684 | 371,495 | 322,089 | 370,690 | 371,161 | 286,071 |
| AUDNZD | 370,715 | 369,930 | 372,365 | 372,787 | 371,779 | 372,819 | 372,043 | 372,119 | 370,501 | 371,016 | 322,099 | 372,412 | 370,960 | 286,855 |
| NZDUSD | 369,180 | 367,530 | 370,151 | 370,255 | 367,888 | 369,917 | 371,956 | 371,342 | 368,973 | 370,359 | 320,972 | 370,326 | 371,075 | 286,373 |
| USDCHF | 370,700 | 366,487 | 370,863 | 371,548 | 369,892 | 370,264 | 371,081 | 370,347 | 365,490 | 369,348 | 320,733 | 370,974 | 371,205 | 286,373 |

Reference: a full year of a 23h/5-day market has about 330-350k bars. 2026 is a partial year (data to early October).

## Longest gaps (days) and low-coverage weeks

* **XAUUSD**: longest gaps 3.18 d (2015-12-24 18:45 -> 2015-12-27 23:01); 3.18 d (2020-12-24 18:44 -> 2020-12-27 23:00); 3.08 d (2023-04-06 20:58 -> 2023-04-09 23:00); 3.05 d (2016-12-30 21:48 -> 2017-01-02 23:01). Median weekly bars 6892. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **XAGUSD**: longest gaps 3.18 d (2020-12-24 18:44 -> 2020-12-27 23:00); 3.18 d (2015-12-24 18:45 -> 2015-12-27 23:01); 3.05 d (2015-12-31 21:44 -> 2016-01-03 23:01); 3.05 d (2016-12-30 21:50 -> 2017-01-02 23:01). Median weekly bars 6713. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **NAS100**: longest gaps 9.04 d (2017-02-17 21:59 -> 2017-02-26 23:01); 3.20 d (2015-12-24 18:15 -> 2015-12-27 23:01); 3.20 d (2020-12-24 18:14 -> 2020-12-27 23:00); 3.08 d (2025-04-17 20:13 -> 2025-04-20 22:05). Median weekly bars 6646. Weeks (Sun-ending, UTC) with < 25% of the median bars: 2017-02-26 (49).
* **SPX500**: longest gaps 9.04 d (2017-02-17 21:59 -> 2017-02-26 23:01); 3.20 d (2015-12-24 18:15 -> 2015-12-27 23:01); 3.20 d (2020-12-24 18:14 -> 2020-12-27 23:00); 3.08 d (2022-12-30 21:12 -> 2023-01-02 23:00). Median weekly bars 6142. Weeks (Sun-ending, UTC) with < 25% of the median bars: 2017-02-26 (29).
* **DAX**: longest gaps 9.42 d (2017-02-17 20:59 -> 2017-02-27 07:01); 5.45 d (2018-12-21 21:14 -> 2018-12-27 07:55); 5.00 d (2014-12-24 07:01 -> 2014-12-29 07:00); 4.42 d (2017-12-22 20:59 -> 2017-12-27 07:01). Median weekly bars 4196. Weeks (Sun-ending, UTC) with < 25% of the median bars: 2017-02-26 (0).
* **US30**: longest gaps 3.38 d (2017-12-29 20:59 -> 2018-01-02 06:00); 3.20 d (2020-12-24 18:14 -> 2020-12-27 23:00); 3.17 d (2017-12-22 20:59 -> 2017-12-26 01:01); 3.17 d (2018-03-29 19:59 -> 2018-04-02 00:01). Median weekly bars 6774. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **USDJPY**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:06); 3.00 d (2015-12-31 21:57 -> 2016-01-03 22:00); 3.00 d (2020-12-31 21:58 -> 2021-01-03 22:00); 3.00 d (2017-12-29 21:58 -> 2018-01-01 22:00). Median weekly bars 7172. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **AUDJPY**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:10); 3.00 d (2023-12-29 21:58 -> 2024-01-01 22:04); 3.00 d (2015-12-31 21:57 -> 2016-01-03 22:00); 3.00 d (2017-12-29 21:57 -> 2018-01-01 22:00). Median weekly bars 7185. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **EURGBP**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:08); 3.00 d (2023-12-29 21:58 -> 2024-01-01 22:04); 3.00 d (2017-12-29 21:57 -> 2018-01-01 22:01); 3.00 d (2015-12-31 21:58 -> 2016-01-03 22:00). Median weekly bars 7154. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **GBPUSD**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:06); 3.00 d (2017-12-29 21:57 -> 2018-01-01 22:00); 3.00 d (2020-12-31 21:58 -> 2021-01-03 22:00); 3.00 d (2023-12-29 21:58 -> 2024-01-01 22:00). Median weekly bars 7176. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **USDCAD**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:08); 3.00 d (2023-12-29 21:58 -> 2024-01-01 22:04); 3.00 d (2017-12-29 21:58 -> 2018-01-01 22:00); 3.00 d (2020-12-31 21:58 -> 2021-01-03 22:00). Median weekly bars 7132. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **EURUSD**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:07); 3.00 d (2017-12-29 21:57 -> 2018-01-01 22:00); 3.00 d (2020-12-31 21:58 -> 2021-01-03 22:00); 3.00 d (2015-12-31 21:58 -> 2016-01-03 22:00). Median weekly bars 7177. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **EURJPY**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:07); 3.01 d (2023-12-29 21:58 -> 2024-01-01 22:06); 3.00 d (2017-12-29 21:57 -> 2018-01-01 22:01); 3.00 d (2017-12-22 21:58 -> 2017-12-25 22:01). Median weekly bars 7182. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **GBPJPY**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:07); 3.00 d (2017-12-29 21:55 -> 2018-01-01 22:02); 3.00 d (2023-12-29 21:58 -> 2024-01-01 22:04); 3.00 d (2017-12-22 21:58 -> 2017-12-25 22:02). Median weekly bars 7174. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **AUDUSD**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:04); 3.00 d (2023-12-29 21:58 -> 2024-01-01 22:04); 3.00 d (2017-12-29 21:57 -> 2018-01-01 22:00); 3.00 d (2020-12-31 21:58 -> 2021-01-03 22:00). Median weekly bars 7156. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **AUDNZD**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:05); 3.00 d (2023-12-29 21:58 -> 2024-01-01 22:04); 3.00 d (2017-12-29 21:55 -> 2018-01-01 22:00); 3.00 d (2020-12-31 21:58 -> 2021-01-03 22:00). Median weekly bars 7176. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **NZDUSD**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:05); 3.00 d (2017-12-29 21:56 -> 2018-01-01 22:02); 3.00 d (2023-12-29 21:58 -> 2024-01-01 22:04); 3.00 d (2017-12-22 21:59 -> 2017-12-25 22:02). Median weekly bars 7146. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.
* **USDCHF**: longest gaps 3.13 d (2015-12-24 18:59 -> 2015-12-27 22:07); 3.00 d (2023-12-29 21:58 -> 2024-01-01 22:04); 3.00 d (2020-12-31 21:58 -> 2021-01-03 22:00); 3.00 d (2017-12-29 21:58 -> 2018-01-01 22:00). Median weekly bars 7150. Weeks (Sun-ending, UTC) with < 25% of the median bars: none.

Systematic hole: on every EU spring-forward Sunday 2019-2026 the label hour 19:00-19:59 is missing in the HistData files, i.e. one hour 23:00-23:59 UTC gap right after the Sunday open (verified on SPX500 2026-03-29: the DJI export has bars there, HistData none). It is a data hole, not a clock error.

Normal weekend gaps are about 2 days and Christmas/New-Year gaps about 3.1-3.4 days; anything above ~3.4 days is a data hole and is listed above where it applies.
