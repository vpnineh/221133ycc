#include "sim.h"
//+------------------------------------------------------------------+
//| DonchianProp Portfolio v13.12 — multi-strategy, regime-aware EA   |
//| built for prop challenges (FundedNext Stellar 2-Step rules).     |
//|                                                                  |
//| 18 active sleeves on XAUUSD, XAGUSD, US30, NAS100, SPX500,       |
//| AUDJPY, USDJPY, EURGBP.  Every rule below was chosen on 2011-2017 and    |
//| 2018-2022 data and checked on untouched 2023-2026 data.          |
//|   GOLD  T60  Donchian-60 H1, both sides, H1 ADX>25 with the H1   |
//|              EMA200 side; half off at 1.5R; time stop 15 days    |
//|   GOLD  T240 Donchian-240 H1, long, H8 above a rising EMA200;    |
//|              stop to break-even at 0.5R; pyramid 2 adds          |
//|   GOLD  MOM  120-day momentum, H12 ADX>25; half off at 1R,       |
//|              2 ATR trail, time stop 15 days                      |
//|   GOLD  TOM  turn-of-month long, exit from the 4th weekday       |
//|   SILV  T240 Donchian-240 H1, long, H6 close>EMA200; stale exit  |
//|   SILV  MA   EMA20/EMA100 H1 cross, H6 ADX>25                    |
//|   SILV  H4-55 Donchian-55 H4, long; lock 2R once 3R is reached   |
//|   DOW RSI2 (H12 EMA200 rising), DOW IBS (flat before weekends),  |
//|   NAS RSI2, NAS IBS  daily dip buys                              |
//|   AUDJPY BB  H4 Bollinger breakout, M15 ADX>25, 2R trail         |
//|   JPY GOTOBI, NAS 3RD-FRI, NAS GAP-LONG  calendar / clock sleeves|
//|   JPY LOTTERY  60h breakout, 10 ATR target                       |
//|   EURGBP MR  50-day z-score reversion; stale exit after 72h      |
//|                                                                  |
//| v13.12 (Nov 2026) - production checks; trading identical to 13.11 |
//|  - Performance alert (InpPerfAlertPct, funded / personal): closed |
//|    result of the last 91 days <= -4% of the initial balance (the  |
//|    worst 5% of 2016-2026) -> Alert + push notification, once per  |
//|    episode. No trading change (risk brakes tested and rejected).  |
//|  - Fault tests (harness): 2% / 10% of trade requests refused,     |
//|    daily 1 h and 4 h connection outages, 12 / 40 terminal         |
//|    restarts at random times: no rule breach, every position had  |
//|    its stop; restarts reproduce the uninterrupted run exactly.    |
//|  - Monte Carlo (20,000 x 12 months, block bootstrap): P(max-loss  |
//|    breach) 1.0% (2x costs 2.3%), P(daily breach) 0%.             |
//|  - FundedNext MT5 tester 2025-02..2026-11 (10k): within 5% of the |
//|    harness (CALR).                                               |
//| v13.11 (Nov 2026) - limiter hunt + adaptive challenge risk:      |
//|  - TURBO challenge: risk x clamp(cushion, MIN, 1) where cushion = |
//|    (equity - max-loss floor) / 10% of the stage start: full risk |
//|    at or above the start, half risk at -5% (InpTurboCushionMin). |
//|    39 starts, restart at -6%: pass per attempt 64% -> 74%,        |
//|    median to funded 101 -> 95 days, within 90 days 44% -> 49%.   |
//|  - FUNDED gold same-direction cap 2.0 -> 2.5 R (InpGoldDirCap-    |
//|    FundedR; phases keep 2.0: 3.0 there slowed the challenge).    |
//|    CALR 100k 2.16 -> 2.23 %/month, max DD 10.65 -> 9.92%,         |
//|    worst 12-month window -9.2k -> -8.4k; plateau 2.5 / 3 / 4.     |
//|  - Rejected: drawdown brake off/5/6% (2.33%/month but 14% DD and |
//|    12-month windows below -10% = max-loss breach), soft stop 3 / |
//|    3.5 and room x2.5 (never binding), open-risk cap 8 / 10 (P2   |
//|    worse), funded 1.1% with cap 3 (2x costs: -9.4k window),      |
//|    20 new markets with the sleeve rules (only Nikkei IBS passed; |
//|    same profile as SPX IBS, which made the portfolio worse).     |
//| v13.10 (Nov 2026) - exit-management matrix (trade management):   |
//|  - 64 + 23 full 10-year runs over trail (ATR / R), break-even,    |
//|    profit lock, 1st/2nd partials, fixed TP, stale exit, time stop |
//|    for every sleeve; gates: sleeve +10%, >=2 of 3 periods, plateau|
//|    of neighbours, portfolio better, still better at 2x costs.    |
//|  - Kept (all else was worse; trailing / TP of the trend sleeves   |
//|    is already the best tested):                                  |
//|    NAS IBS   trail 0.5R behind the best price once +0.5R          |
//|    NAS RSI2  stale exit after 24 H1 bars if MFE < 0.2R            |
//|    AUDJPY BB stale exit after 48 H1 bars if MFE < 0.5R            |
//|    GOLD T240 no break-even at +0.5R (the 1R pyramid add moves it) |
//|  - CALR 2016-2026 funded 1.0%: 100k 2.16%/month (v13.9 2.07),     |
//|    Sharpe 2.22 (2.13); 10k 1.87 (1.81); 5k 1.90 (1.79); 2x costs |
//|    1.59 (1.50). Challenge TURBO, 39 starts, restart at -6%:      |
//|    median to funded 101 days (109), 44% within 90 days (42%).    |
//| v13.9 (Nov 2026) - speed / income release (risk-policy matrix):  |
//|  - Found the limiter: the open-profit lock (InpLockOpenProfit)    |
//|    cut every winner once the open stops exceeded the daily room, |
//|    so more risk gave no more profit. Now OFF; the entry check    |
//|    "today's loss + open stops + new risk" uses 2x the daily room |
//|    (InpDailyRoomMult); the EA's own 4% intraday guard is kept.   |
//|  - FUNDED risk 1.00% (was 0.60%): CALR 2016-2026 100k 2.07%/month |
//|    (simple), Sharpe 2.13, worst day -3.6%, all periods > 0;     |
//|    0.9% = 1.91, 1.1% = 2.21 (plateau); 2x costs 1.50%/month.     |
//|    39 new funded accounts (12 months each): avg 2.00%/month,     |
//|    0 breaches, worst trough -6.6%, 5% of starts lose a year.    |
//|  - TURBO risk mode (default): phase 1 2.0% -> 2.5% after +2%,    |
//|    phase 2 2.0%, soft stop 3.5%, DD brake 6% (challenge only).  |
//|    39 starts: pass per attempt 74%, median pass 84 days; with   |
//|    a restart at -6% (stall alert): median to funded incl.       |
//|    retries 109 days, 42% within 90 days (FAST: 178 / 8%).       |
//|  - Not achievable with this edge: funded in <= 90 days every     |
//|    time; 3.0/2.5 risk: 54% pass, not faster.                     |
//| v13.8 (Nov 2026):                                                |
//|  - InpAccountType: PROP (default, unchanged behaviour: phases,   |
//|    targets, FundedNext daily / max-loss rules) or PERSONAL:      |
//|    risk 0.75% of the day's balance (compounding), own daily stop |
//|    4% (all closed until the next server day), max-drawdown stop  |
//|    20% from the equity peak (all closed, manual Reset in the     |
//|    panel), no targets / phases / min-days helper / prop news     |
//|    rule. v13.9 settings (lock off, room x2), CALR 2016-2026 100k:|
//|    0.75% CAGR 19.1%, max DD 7.3%, worst day 2.4%, Sharpe 2.12;   |
//|    2x costs CAGR 14.9%. 1.0% = CAGR 24.9%, DD 9.6%, Sharpe 2.14.  |
//|  - PROP mode trades exactly as v13.7.1 (harness: same deals).    |
//|  - Account size (FundedNext costs, CALR, funded 0.6%, 2016-26):  |
//|    100k +165%, 25k +158%, 10k +137%, 5k +105% (broker min lot:   |
//|    gold / silver / Dow signals skipped on small accounts).       |
//|    Challenge, 39 quarterly starts, 24-month window: pass 87-97%, |
//|    0 max-loss failures at every size; median days to funded     |
//|    228 (100k) / 330 (25k) / 305 (10k) / 270 (5k).               |
//|  - Rejected in the Nov 2026 research (not added): LightGBM meta  |
//|    sizing of all sleeves (walk-forward rho +0.03, p 0.21);       |
//|    tighter gold stops 0.3-0.6x (Sharpe 1.54-1.71 vs 2.05; the    |
//|    TIGHT profile 0.75x is the optimum); fractional min-lot       |
//|    sizing for 5k/10k (not robust, 5k Sharpe 1.79 -> 1.51);       |
//|    intraday momentum SPX/NAS/US30 (t -5); pre-holiday index long |
//|    (t ~0); FX cross-sectional momentum (PF 1.09, t 0.8).         |
//| v13.7:                                                           |
//|  - FUNDED only: new entries use the TIGHT stop profile while     |
//|    equity >= initial balance +5% (InpFundedTightCushionPct;      |
//|    0 = off). An open trade keeps the profile of its entry.       |
//|    Rejected in research (not added): cushion risk scaling of the |
//|    funded risk (more DD than profit), PHASE 2 risk step-up.      |
//| v13.6:                                                           |
//|  - FED POST exit at 15:58 New York or 2 min before the broker's  |
//|    session end, whichever is first (MetaQuotes: 16:00 NY = the   |
//|    23:00 session close -> 122/131 exits slipped to 00:00).       |
//|    A FED POST entry queued by a closed market is dropped once    |
//|    the 14:06-16:00 NY window of its day is over (journal line).  |
//|  - live / demo: WebRequest whitelist REQUIRED for FED POST. Both |
//|    URLs are tested at start and daily; error 4014 pauses only    |
//|    the FED POST sleeves (Alert once, journal, panel line) and    |
//|    they resume by themselves once the URLs are allowed. Offline  |
//|    (allowed but unreachable): no pause, cache / calendar / table.|
//|  - news filter (live): MT5 calendar -> ForexFactory USD high     |
//|    impact (weekly JSON) -> cache -> FOMC table (tester: off).    |
//|  - PHASE 1 (FAST) starts at 1.00% risk and switches to 1.25%   |
//|    once equity is +2% (InpP1RampStartPct; 0 = flat 1.25%).       |
//|  - journal: entry lines show the ACTUAL risk after all caps and  |
//|    which cap / rounding changed the lot; clock-detection lines   |
//|    only in DEMO mode (one final line otherwise).                 |
//| v13.5:                                                           |
//|  - FED POST FOMC dates self-updating (live / demo): MT5 economic |
//|    calendar -> ForexFactory weekly JSON -> federalreserve.gov ->  |
//|    file cache -> built-in table (Strategy Tester: table only).    |
//|    Emergency meetings move the entry window; restart-safe guard. |
//|    Web sources (optional): allow WebRequest in                  |
//|    Tools > Options > Expert Advisors > Allow WebRequest, add      |
//|    https://www.federalreserve.gov and                            |
//|    https://nfs.faireconomy.media                                 |
//|  - 2027 table fixed (decision = 2nd meeting day; provisional)    |
//|  - defaults: DEMO mode off; challenge FAST risk 1.25% / 1.00%    |
//| v13.3:                                                           |
//|  - market-closed gate: before every trade request (entry, add,   |
//|    close, partial, stop move, profit lock) the symbol's trade    |
//|    session and the age of its last tick are checked. Closed ->   |
//|    nothing is sent or printed; the stop / close / partial / add  |
//|    / entry is queued and sent on the first tick after reopen     |
//|    (entries and adds only while price is within 0.25 ATR of the  |
//|    signal and the stop is still valid). Ends the 23:00 metals-   |
//|    break modify spam (14,375 lines in a 10y MetaQuotes test)     |
//|  - money-per-lot cross-check warning printed at start-up         |
//|  - SILV MA off by default (-12R over 10 years at tester costs)   |
//|  - InpStopProfile: STANDARD (default) or TIGHT (walk-forward     |
//|    tighter stops: more profit, proportionally more DD)           |
//| v13.2 (bug fixes from the MetaQuotes / FundedNext tester logs):  |
//|  - lot size from OrderCalcProfit, cross-checked against the tick |
//|    value and contract x price; the largest loss per lot wins     |
//|    when they disagree by >30% (MetaQuotes-Demo metals reported   |
//|    1/10 of the real tick value: risk was 9.8x the plan)          |
//|  - one server clock for every symbol (FX weekly open first,      |
//|    then a vote); metals / indices no longer get their own clock  |
//|  - tester: saved state of an earlier run is deleted at start;    |
//|    live: the saved preset is restored only for the same input    |
//|  - entry rejected with 10018 (market closed): retried each       |
//|    minute for 15 min while price stays within 0.25 ATR           |
//|  - rejected closes / stop moves: quiet backoff, journal note at  |
//|    most every 5 min per sleeve; stop moves retried each minute   |
//| v13.1: second partial close: 1/3 of the remaining volume        |
//| v13.1:   at +3R on the trend sleeves (0,1,2,4,10,11,17)         |
//| v13.1:   (round rx, Oct 2026; earlier partials kept)            |
//| v13.1: tester reads the server clock from the broker bars;      |
//|  min-lot risk cap; open-profit lock vs the daily guard; second   |
//|  partial at +3R on trend sleeves; market-closed backoff; tester  |
//|  chains phase1 -> phase2 -> funded; panel: no side box, list     |
//|  can be collapsed; skipped entries logged                        |
//| v13.0 (Oct 2026, 10-year re-test on clean HistData 2016-2026):  |
//|  - new sleeve SPX RSI2: S&P 500 daily RSI(2)<10 dip buy, D1      |
//|    EMA200 rising, stop 3 ATR, exit close>SMA5, 7-day time stop   |
//|    (only claim of the v13-v15 research that passed the unseen    |
//|    2016-2018 + 2026 hold-out)                                    |
//|  - closes / partial closes set the filling mode of the position  |
//|    symbol and retry once; entries check the broker stop level    |
//| v12.1: symbol search by name only - the tester no longer        |
//| downloads every server symbol (MetaQuotes-Demo crash), stocks    |
//| (AG, AMPX ...) are never picked for silver / Nasdaq.             |
//| v12.0 (round 20: 1,900+ backtests of every sleeve parameter,     |
//|   trend filters on M15..W1, trade management; real swap model):  |
//|  - new trend-filter engine: any timeframe M15..W1 per sleeve     |
//|  - trade management per sleeve: break-even, profit lock, R trail,|
//|    partial close, stale-trade exit, time stop, weekend flat      |
//|  - risk caps scale with the risk unit (open risk 6x, gold 2x)    |
//|  - FAST risk mode by default: phase 1 1.25%, phase 2 1.00%       |
//|  - fixes: daily bars wrong after a summer/winter clock change    |
//|    when started in the other season; an exit delayed by news    |
//|    could expire and leave the trade open; turn-of-month exit     |
//|    missed when its day had no 02:00 bar; DEMO mode guessed the   |
//|    clock of broker data (now only custom data); a guessed clock  |
//|    survived a restart of the EA on the same chart                |
//|  - panel: open-trade tooltip splits price P/L and swap           |
//|                                                                  |
//| CPU: all logic runs once per closed H1 bar of each symbol from   |
//| a 1-second timer; trend features on other timeframes are cached; |
//| the panel writes objects only when a value changed. Hard stops   |
//| sit on the broker server.                                        |
//+------------------------------------------------------------------+







#define NSLV 26
#define NROLE 8
#define NREG 5          // roles shown in the panel regime row (USDJPY Gotobi is calendar-only)
#define EA_VER "13.12"
// v13.7.1: no trading change. Tester stage summary in the journal (days, worst closed day,
//          peak-to-trough drawdown, daily-guard stops); panel title from EA_VER (was "v13.5");
//          sleeve text for FED POST / DXY-JPY / Z-CONT (all showed "Tokyo-fix reversal");
//          "no daily bars" message instead of "only -1 daily bars". GOLD Z-CONT stays OFF.

enum ENUM_PRESET { PRESET_PHASE1 = 0,   // فاز ۱ (چالش)
                   PRESET_PHASE2 = 1,   // فاز ۲ (تایید)
                   PRESET_FUNDED = 2,   // حساب فاند
                   PRESET_CUSTOM = 3 }; // سفارشی
enum ENUM_ACCTYPE { ACC_PROP = 0,       // پراپ (چالش / فاند) — پیش‌فرض
                    ACC_PERSONAL = 1 }; // شخصی (حساب خودتان، بدون قوانین پراپ)
#define PRESET_PERSONAL 4              // internal preset of a personal account (not selectable in the panel)
enum ENUM_RISKMODE { RM_SAFE = 0,      // امن (فاز ۱ ۱.۰۰٪ / فاز ۲ ۰.۷۵٪ / فاند ۰.۵۰٪)
                     RM_FAST = 1,      // سریع (فاز ۱ ۱.۲۵٪ / فاز ۲ ۱.۰۰٪ / فاند ۱.۰۰٪)
                     RM_TURBO = 2 };   // توربو (پیش‌فرض v13.9: فاز ۱ ۲.۵٪ / فاز ۲ ۲.۰٪ / فاند ۱.۰۰٪ — پاس سریع)
enum ENUM_LANG   { LANG_FA = 0,         // فارسی
                   LANG_EN = 1 };       // English
enum ENUM_DL_BASE { DLB_INITIAL = 0,    // درصدی از بالانس اولیه (FundedNext / FTMO)
                    DLB_DAYSTART = 1 }; // درصدی از مبنای شروع روز
enum ENUM_DL_REF  { DLR_MAX_BAL_EQ = 0, // بیشترِ بالانس و اکوییتی در شروع روز
                    DLR_BALANCE = 1,    // بالانس شروع روز
                    DLR_EQUITY = 2 };   // اکوییتی شروع روز
enum ENUM_ML_TYPE { ML_STATIC = 0,      // ثابت از بالانس اولیه (FundedNext)
                    ML_TRAIL_BALANCE = 1, // شناور از سقف بالانس
                    ML_TRAIL_EQUITY = 2 };// شناور از سقف اکوییتی
enum ENUM_STOPPROF { STOP_STANDARD = 0, // استاندارد (پیش‌فرض)
                     STOP_TIGHT = 1 };  // تنگ: سود بیشتر، افت به همان نسبت بیشتر، نه لبه‌ی بهتر
enum ENUM_NEWS    { NEWS_OFF = 0,       // خاموش
                    NEWS_FUNDED = 1,    // فقط در حساب فاند
                    NEWS_ALWAYS = 2 };  // همیشه

//--------------------------------------------------------------------- inputs

bool InpTesterChain           = true;   // فقط در Strategy Tester: با رسیدن به هدف، مرحله‌ی بعد (فاز ۲، بعد فاند) از همان بالانس شروع می‌شود و تست ادامه پیدا می‌کند
bool InpDemoMode               = false;   // حالت DEMO / تست: در Strategy Tester نمادهای custom (مثل XAUUSD-back) را خودش پیدا می‌کند و ساعت دیتا را از کندل‌ها تشخیص می‌دهد. در حساب زنده اثری ندارد.


ENUM_ACCTYPE InpAccountType    = ACC_PROP; // پراپ: فازها، هدف‌ها و قوانین پراپ (بخش «قوانین پراپ») | شخصی: سیاست‌های بخش «حساب شخصی»

double InpPersRiskPct          = 0.75;   // واحد ریسک هر معامله (٪ بالانس) — CALR ۲۰۱۶ تا ۲۰۲۶: ۰.۷۵ = رشد سالانه ۱۹٪ افت ۷.۳٪؛ ۱.۰ = رشد ۲۵٪ (حدود ۱.۹٪ در ماه) افت ۹.۶٪
bool   InpPersCompound         = true;   // ریسک از بالانس روز محاسبه شود (سود مرکب؛ خاموش = از بالانس اولیه)
double InpPersDailyStopPct     = 4.0;    // توقف روزانه: با این ضرر روز (٪ مبنای شروع روز) همه بسته و تا روز بعد ورودی نیست
double InpPersMaxDDPct         = 20.0;   // توقف اضطراری: افت اکوییتی از سقف (٪) — همه بسته و ربات تا Reset دستی در پنل متوقف

ENUM_PRESET InpPreset          = PRESET_PHASE1; // پریست شروع (انتخاب پنل ذخیره می‌شود)
double InpRiskPhase1           = 1.00;   // حالت امن: واحد ریسک فاز ۱ (٪ بالانس اولیه) — v13.5 شبیه‌سازی ۲۰۱۶ تا ۲۰۲۶ (CALR): پاس ۱۰۰٪، میانه کل ۲۳۰ روز
double InpRiskPhase2           = 0.75;   // حالت امن: واحد ریسک فاز ۲ (٪)
double InpRiskFunded           = 0.50;   // حالت امن: واحد ریسک حساب فاند (٪) — CALR ده‌ساله: افت ۵.۹٪، بدترین روز ۱.۶٪
double InpRiskCustom           = 0.50;   // واحد ریسک سفارشی (٪)
ENUM_RISKMODE InpRiskMode      = RM_TURBO; // حالت ریسک (دکمه‌ی پنل هم دارد) — v13.9 پیش‌فرض توربو: ۳۹ شروع ۲۰۱۶ تا ۲۰۲۵ (CALR): پاس هر تلاش ۷۴٪، میانه‌ی پاس ۸۴ روز؛ با قانون رهاکردن در -۶٪: میانه تا فاند با تلاش دوباره ۱۰۹ روز، ۴۲٪ زیر ۹۰ روز (سریع: ۱۷۸ روز، ۸٪)
double InpFastRiskPhase1       = 1.25;   // حالت سریع (پیش‌فرض): ریسک فاز ۱ (٪) — v13.5 شبیه‌سازی ۲۰۱۶ تا ۲۰۲۶ CALR: پاس ۱۰۰٪، رد ۰٪، میانه ۱۷۴ روز (فاز ۱ ۹۲، فاز ۲ ۶۰)؛ هزینه‌ی ۲ برابر: پاس ۹۹.۸٪، میانه ۲۲۸
double InpFastRiskPhase2       = 1.00;   // حالت سریع (پیش‌فرض): ریسک فاز ۲ (٪)
double InpTurboRiskPhase1      = 2.50;   // v13.9 توربو: ریسک فاز ۱ (٪) بعد از رسیدن به +۲٪
double InpTurboRampStartPct    = 2.00;   // v13.9 توربو: ریسک شروع فاز ۱ تا +InpP1RampAtPct٪
double InpTurboRiskPhase2      = 2.00;   // v13.9 توربو: ریسک فاز ۲ (٪)
double InpTurboSoftDailyStopPct= 3.5;    // v13.9 توربو (فقط فاز ۱ و ۲): توقف ورود جدید بعد از این ضرر روزانه (٪)
double InpTurboDDBrakePct      = 6.0;    // v13.9 توربو (فقط فاز ۱ و ۲): ترمز افت (٪)
double InpTurboCushionMin      = 0.50;   // v13.11 توربو (فقط فاز ۱ و ۲): ریسک × فاصله‌ی اکوییتی تا کف حد ضرر کل (۱ در شروع یا بالاتر، ۰.۵ در -۵٪)، کمترین ضریب = این عدد (۱ = خاموش). ۳۹ شروع: پاس هر تلاش ۶۴ -> ۷۴٪، میانه تا فاند ۱۰۱ -> ۹۵ روز
double InpPerfAlertPct         = 4.0;    // v13.12 هشدار عملکرد (فقط حساب فاند و شخصی): اگر نتیجه‌ی بسته‌شده‌ی ۹۱ روز اخیر به این ٪ ضرر (از بالانس اولیه) برسد هشدار و نوتیفیکیشن (بدترین ۵٪ تاریخچه ۲۰۱۶-۲۰۲۶)؛ معامله تغییر نمی‌کند (۰ = خاموش)
double InpChallengeStallPct    = 6.0;    // v13.9 هشدار گیرکردن چالش: اکوییتی فاز ۱/۲ این ٪ زیر شروع مرحله — پژوهش: رهاکردن و خرید چالش تازه سریع‌تر است (۰ = خاموش)
double InpFastRiskFunded       = 1.00;   // حالت سریع و توربو: ریسک حساب فاند (٪) — v13.9 (قفل سود خاموش، جای روزانه ×۲): CALR ۲۰۱۶ تا ۲۰۲۶ = ۲.۰۷٪ در ماه، شارپ ۲.۱۳، بدترین روز ۳.۶٪؛ ۳۹ حساب فاند تازه: میانگین ۲.۰٪ در ماه، صفر نقض
double InpP1RampStartPct        = 1.00;   // v13.6: ریسک شروع فاز ۱ در حالت سریع (۰ = خاموش)؛ با رسیدن اکوییتی به +InpP1RampAtPct٪ به ریسک فاز ۱ (۱.۲۵٪) می‌رود — شبیه‌سازی CALR: پاس ۱۰۰٪، بدترین کف اکوییتی فاز ۱ از +۰.۰۵٪ به +۱.۳۷٪ بالای کف ۱۰٪، میانه ۱۹۰ روز به جای ۱۷۹
double InpP1RampAtPct           = 2.0;    // v13.6: آستانه‌ی سود فاز ۱ برای رفتن به ریسک کامل (٪ بالانس اولیه)
double InpFundedRampStartPct   = 0.0;    // v13.9 حساب فاند: ریسک تا رسیدن اکوییتی به +InpFundedRampAtPct٪ (۰ = خاموش)؛ زیر این سطح دوباره همین ریسک
double InpFundedRampAtPct      = 3.0;    // v13.9 کوشن حساب فاند برای رفتن به ریسک کامل (٪ بالانس اولیه)
double InpGoldDirCapFundedR    = 2.5;    // v13.11 فقط حساب فاند: سقف ریسک هم‌جهت طلا (۲.۵ به جای ۲.۰: ۲.۱۶ -> ۲.۲۳٪ در ماه، افت کمتر؛ در فازها ۲.۰ می‌ماند چون ۳.۰ چالش را کند کرد) — ۰ = همان InpGoldDirCapR
double InpGoldDirCapR          = 2.0;    // سقف مجموع ریسک هم‌جهت طلا تا حد ضرر، از قیمت فعلی (چند برابر واحد ریسک، ۰ = خاموش)
bool   InpVolTarget            = false;  // هدف‌گذاری نوسان پورتفوی (ضریب ریسک = هدف ÷ نوسان اخیر حساب)
double InpVolTargetPct         = 6.0;    // نوسان سالانه‌ی هدف (٪ بالانس اولیه)
double InpVolGMin              = 0.5;    // حداقل ضریب
double InpVolGMax              = 1.5;    // حداکثر ضریب
bool   InpGoldCapDyn           = false;  // v13.4 (آزمایشی): سقف پویای طلا — ADX روزانه بالا و شیب EMA200 هم‌جهت: سقف بالا؛ ADX پایین: سقف پایین
double InpGoldCapHiR           = 3.0;    // v13.4: سقف طلا در روند قوی (چند برابر واحد ریسک)
double InpGoldCapLoR           = 1.5;    // v13.4: سقف طلا در رنج
double InpGoldCapAdxHi         = 35;     // v13.4: آستانه ADX روند قوی
double InpGoldCapAdxLo         = 20;     // v13.4: آستانه ADX رنج
bool   InpGoldRegime           = false;  // v13.4 (آزمایشی): سوییچ رژیم طلا — وقتی ADX روزانه طلا زیر آستانه است، ریسک اسلیوهای روندی فلزات کم و اسلیوهای برگشتی غیرفلزی زیاد می‌شود
double InpGRAdx                = 20;     // v13.4: آستانه ADX رنج طلا
double InpGRTrendMult          = 0.5;    // v13.4: ضریب ریسک اسلیوهای روندی فلزات در رنج طلا
double InpGRMRMult             = 1.25;   // v13.4: ضریب ریسک اسلیوهای برگشتی غیرفلزی در رنج طلا
double InpBookTakePct          = 0;      // v13.4 (آزمایشی): وقتی سود باز کل ربات از این ٪ بالانس اولیه بیشتر شد (۰ = خاموش)
int    InpBookTakeMode         = 0;      // v13.4: ۰ = بستن بخشی از هر معامله‌ی سودده، ۱ = قفل نیمی از سود باز با جابجایی حد ضرر
double InpBookTakeFrac         = 0.5;    // v13.4: کسر بسته‌شده در حالت ۰

bool   InpUseFedPostUs30  = true;    // US30: فروش بعد از اعلام FOMC (۱۴:۰۶ تا ۱۶:۰۰ نیویورک). تاریخ‌ها خودکار (تقویم MT5 / وب / جدول). منابع وب: Tools>Options>Expert Advisors>Allow WebRequest: https://www.federalreserve.gov و https://nfs.faireconomy.media
bool   InpUseFedPostSpx   = true;    // SPX500: فروش بعد از اعلام FOMC (همان پنجره و همان منابع تاریخ)
bool   InpUseDxyJpy       = false;   // v13.4 (off) USDJPY: بازگشت شاخص دلار DXY (مجیک ۱۱۰۰۲۴)
bool   InpUseGoldZcont    = false;   // v13.4 (off) طلا: ادامه‌ی حرکت بعد از z≥۲ (فقط خرید، بالای EMA200، خروج ۵ روزه)
double InpFedPostStopATR  = 1.0;     // FED POST: حد ضرر (×ATR روزانه)
double InpDxyZ            = 2.5;     // DXY-JPY: آستانه z شاخص دلار
double InpDxyStopATR      = 1.5;     // DXY-JPY: حد ضرر (×ATR روزانه USDJPY)
int    InpDxyHoldH        = 48;      // DXY-JPY: خروج زمانی (ساعت)
double InpGoldZc          = 2.0;     // GOLD Z-CONT: آستانه z
double InpGoldZcStopATR   = 2.0;     // GOLD Z-CONT: حد ضرر (×ATR)
int    InpGoldZcDays      = 5;       // GOLD Z-CONT: خروج زمانی (روز)
string InpEurUsdSymbol    = "";      // نماد EURUSD برای شاخص دلار (خالی=خودکار، -=غیرفعال)
string InpGbpUsdSymbol    = "";      // نماد GBPUSD برای شاخص دلار
string InpUsdCadSymbol    = "";      // نماد USDCAD برای شاخص دلار
string InpUsdChfSymbol    = "";      // نماد USDCHF برای شاخص دلار


double InpInitialBalance       = 0.0;    // بالانس اولیه حساب (۰ = خودکار از اولین واریز)
double InpPhase1TargetPct      = 8.0;    // هدف سود فاز ۱ (٪)
double InpPhase2TargetPct      = 5.0;    // هدف سود فاز ۲ (٪)
double InpFundedTargetPct      = 0.0;    // هدف حساب فاند (٪، ۰ = بدون هدف)
double InpCustomTargetPct      = 0.0;    // هدف پریست سفارشی (٪، ۰ = بدون هدف)
int    InpMinTradingDays       = 5;      // حداقل روزهای معاملاتی هر فاز
double InpDailyLossPct         = 5.0;    // حداکثر ضرر روزانه (٪)
ENUM_DL_BASE InpDailyLossBase  = DLB_INITIAL;    // مبنای محاسبه مقدار ضرر روزانه
ENUM_DL_REF  InpDailyLossRef   = DLR_MAX_BAL_EQ; // نقطه مرجع ضرر روزانه (شروع روز سرور)
double InpMaxLossPct           = 10.0;   // حداکثر ضرر کل (٪ بالانس اولیه)
ENUM_ML_TYPE InpMaxLossType    = ML_STATIC;      // نوع حد ضرر کل
bool   InpTrailLockAtInitial   = true;   // حد شناور: توقف شناوری در بالانس اولیه
double InpSafetyBufferPct      = 1.0;    // حاشیه امن: بستن همه قبل از رسیدن به حد (٪)
double InpSoftDailyStopPct     = 2.5;    // توقف ورود جدید بعد از این ضرر روزانه (٪)
double InpMaxOpenRiskR         = 6.0;    // سقف مجموع ریسک معاملات باز تا حد ضرر (چند برابر واحد ریسک)
bool   InpLockOpenProfit      = false;   // v13.9 خاموش: قفل نصف سود باز وقتی مجموع حد ضررها از حد روزانه بیشتر شود — با ریسک بالاتر مدام فعال می‌شد و سود روندها را می‌برید (ریسک ۱٪: ۱.۲۲٪ در ماه با قفل، ۲.۰۷٪ بدون قفل)
double InpMinLotOverRisk      = 1.5;    // v13.1: حداقل لات بروکر حداکثر چند برابر ریسک خود اسلیو مجاز است (حساب‌های کوچک؛ 0 = خاموش)
double InpDailyRoomMult        = 2.0;    // v13.9 پیش از هر ورود: ضرر امروز + مجموع حد ضررهای باز + ریسک جدید ≤ این ضریب × حد روزانه (۱ = سخت‌گیرانه‌ترین)
double InpMaxRiskPerTradeR     = 2.0;    // سقف ریسک وقتی حداقل لات بروکر از ریسک برنامه‌ریزی‌شده بیشتر است، و سقف هر پله‌ی پیرامید (چند برابر واحد ریسک)
double InpDDBrakePct           = 4.0;    // ترمز افت: وقتی بالانس این مقدار (٪ بالانس اولیه) زیر سقف قبلی است، ریسک کم شود (۰ = خاموش)
double InpDDBrakeMult          = 0.5;    // ترمز افت: ضریب ریسک در زمان افت (۰.۵ = نصف)
bool   InpCloseOnTarget        = true;   // با رسیدن به هدف فاز، همه بسته و ربات متوقف شود
bool   InpMinDaysHelper        = true;   // بعد از هدف: معامله ۰.۰۱ روزانه تا تکمیل حداقل روزها
bool   InpCloseBeforeWeekend   = false;  // بستن همه جمعه ساعت ۲۱ سرور (FundedNext لازم ندارد)
ENUM_NEWS InpNewsFilter        = NEWS_FUNDED; // فیلتر اخبار مهم دلار (قانون ۵ دقیقه فاند)
int    InpNewsWindowMin        = 5;      // فاصله ممنوع قبل و بعد از خبر (دقیقه)


string InpGoldSymbol           = "";     // نماد طلا (مثلاً XAUUSD یا GOLD)
string InpSilverSymbol         = "";     // نماد نقره (مثلاً XAGUSD یا SILVER)
string InpDowSymbol            = "";     // نماد داوجونز (مثلاً US30 یا DJ30)
string InpNasdaqSymbol         = "";     // نماد نزدک (مثلاً NDX100 یا US100)
string InpAudJpySymbol         = "";     // نماد AUDJPY
string InpUsdJpySymbol         = "";     // نماد USDJPY
string InpEurGbpSymbol         = "";     // نماد EURGBP
string InpSpxSymbol            = "";     // نماد S&P500 (مثلاً US500 یا SPX500)


bool InpUseGoldT60   = true;   // طلا: شکست کانال ۶۰ ساعته (خرید و فروش، ADX>25)
bool InpUseGoldT240  = true;   // طلا: شکست کانال ۲۴۰ ساعته (فقط خرید، بالای EMA200)
bool InpUseGoldMOM   = true;   // طلا: مومنتوم ۱۲۰ روزه (ADX>25)
bool InpUseGoldTOM   = true;   // طلا: اثر پایان/شروع ماه (فقط خرید)
bool InpUseSilverT240= true;   // نقره: شکست کانال ۲۴۰ ساعته (فقط خرید)
bool InpUseSilverMA  = false;  // نقره: کراس EMA20/EMA100 هم‌جهت روند روزانه (v13.3: خاموش — در تست ۱۰ ساله با هزینه‌ی واقعی تستر منفی بود، حدود -۱۲R)
bool InpUseSpxRSI2   = true;   // S&P500: خرید اصلاح RSI(2) (v13)
bool InpUseDowRSI2   = true;   // داوجونز: خرید اصلاح RSI(2)
bool InpUseDowIBS    = true;   // داوجونز: خرید اصلاح IBS
bool InpUseNasRSI2   = true;   // نزدک: خرید اصلاح RSI(2)
bool InpUseNasIBS    = true;   // نزدک: خرید اصلاح IBS
bool InpUseAudJpyBB  = true;   // AUDJPY: شکست بولینگر H4 هم‌جهت EMA200
bool InpUseSilverH4  = false;  // نقره: شکست کانال ۵۵ کندل H4 (فقط خرید) — v13.4: خاموش (تکرار SILV T240: ۵۵×H4 ≈ ۲۴۰ ساعت؛ با هزینه‌ی فاندد در ۲۰۱۶-۲۰ تقریباً صفر؛ حذفش شارپ را در هر سه دوره بالا برد)
bool InpUseUsdJpyGotobi = true; // USDJPY: اثر فیکس توکیو در روزهای گوتوبی (خرید ۱۵:۵۵ تا ۰۰:۵۵ UTC)
int  InpGotobiStopPips = 60;   // گوتوبی: حد ضرر (پیپ)
bool InpUseNas3FPS   = true;   // نزدک: اثر جمعه‌ی سوم ماه (فروش ۰۹:۳۰ تا ۱۲:۰۰ نیویورک اگر شب قبل صعودی بود)
double InpTfpsStopPct = 1.0;   // جمعه‌ی سوم: حد ضرر (درصد قیمت)
bool InpUseNasLottery  = false; // نزدک: «بلیت بخت‌آزمایی» (آزمایشی، خاموش: در تست کامل افت را به ۹.۶٪ رساند)
bool InpUseJpyLottery  = true;  // USDJPY: «بلیت بخت‌آزمایی» — شکست ۶۰ ساعته، فقط خرید، ADX>25 و بالای EMA200، حد ضرر ۲ ATR، حد سود ۱۰ ATR، سربه‌سر در ۲R (وزن ۰.۵)
bool InpUseEurGbpMR    = true;  // EURGBP: بازگشت به میانگین روزانه (z-score ۵۰ روزه: ورود در |z|≥۲، خروج با برگشت به میانگین، حد ضرر در z=۴)
bool InpUseNasGapLong  = true;  // نزدک: خرید ۰۹:۳۰ نیویورک بعد از گپ منفی بزرگ (z≤-۲ نسبت به ۶۰ روز)، هدف بسته‌شدن روز قبل، خروج ۱۱:۰۰
bool InpUseGoldIbs     = false; // طلا: خرید روز با IBS پایین (≤۰.۲۰) بالای EMA200 روزانه (پیشنهاد گزارش دیگر؛ در تست کامل تأیید نشد، خاموش)
bool InpUseGoldLottery = false; // طلا: «بلیت بخت‌آزمایی» (آزمایشی، خاموش: هم‌جهت با T60/T240، افت را به ۱۲٪ رساند)
bool InpUseUsdJpyFixRev = false; // USDJPY: برگشت بعد از فیکس توکیو (آزمایشی، خاموش: روی USDJPY ۲۰۱۰ تا ۲۰۱۶ تأیید نشد)
int  InpFixRevStopPips = 40;   // برگشت فیکس: حد ضرر (پیپ)


long   InpMagicBase            = 110000; // مجیک پایه (هر استراتژی = پایه + شماره)
double InpMaxSpreadATR         = 0.15;   // رد ورود اگر اسپرد بیشتر از این ضریب ATR روزانه باشد
int    InpDeviationPoints      = 50;     // حداکثر لغزش قیمت (پوینت)
int    InpServerGMTOffset      = -99;    // اختلاف ساعت سرور با UTC (ساعت؛ -99 = خودکار)
ENUM_STOPPROF InpStopProfile   = STOP_STANDARD; // v13.3 پروفایل حد ضرر. TIGHT (تأییدشده در walk-forward): GOLD T60 و GOLD T240 = 1.875 ATR (به جای 2.5)، GOLD MOM = 2.25 (به جای 3.0)، SILV MA و NAS IBS = 2.25 (به جای 2.5)، EURGBP MR = 1.8 انحراف معیار (به جای 2.0)، توقف زمانی NAS RSI2 = 255 کندل H1 (به جای 170). سود بیشتر، افت به همان نسبت بیشتر، نه لبه‌ی بهتر (more profit, proportionally more DD, not better edge)
double InpFundedTightCushionPct = 5.0; // v13.7: فقط حساب فاند — وقتی اکوییتی حداقل این ٪ بالای بالانس اولیه است، ورودهای جدید با پروفایل حد ضرر TIGHT (۰ = خاموش؛ معامله‌ی باز پروفایل ورودش را نگه می‌دارد)


bool      InpShowPanel         = true;    // نمایش پنل
ENUM_LANG InpLanguage          = LANG_FA; // زبان پنل
int       InpPanelX            = 10;      // فاصله پنل از چپ (پیکسل)
int       InpPanelY            = 20;      // فاصله پنل از بالا (پیکسل)
bool      InpOpenCharts        = false;   // حساب لایو/دمو: باز کردن چارت H1 برای هر نماد (پیش‌فرض خاموش؛ ربات روی هر چارتی کار می‌کند). در بکتست ویژوال همیشه فقط H1 باز می‌شود
bool      InpChartOverlay      = true;    // نمایش روی چارت هر نماد باز: سطح ورود، ورود/حد ضرر/حد سود، فلش معاملات، کادر وضعیت
int       InpChartHistDays     = 60;      // فلش معاملات گذشته روی چارت (روز)

//--------------------------------------------------------------------- sleeve table
// kind: 0 Donchian, 1 MA cross, 2 TS-momentum, 3 turn-of-month, 4 RSI2, 5 IBS
// kind 6: H4 Bollinger(20,2.5) breakout with H4 EMA200 side, exit at the middle band, stop 2 ATR(H4)
// kind 7: H4 Donchian 55 breakout with rising H4 EMA200, exit at the 20-bar opposite channel, stop 3 ATR(H4)
int    SL_ROLE[NSLV]  = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 1, 5, 3, 5, 0, 3, 5, 6, 3, 0, 7, 2, 7, 5, 0};
int    SL_KIND[NSLV]  = {0, 0, 2, 3, 0, 1, 4, 5, 4, 5, 6, 7, 8, 9, 10, 0, 0, 0, 11, 14, 5, 4, 15, 15, 16, 17};   // 8 = Gotobi / Tokyo fix, 9 = third-Friday spike, 10 = post-fix reversal (timer driven), 11 = daily z-score mean reversion (SL_N days, entry |z|>=SL_P, stop SL_SLK sd beyond entry)
int    SL_N[NSLV]     = {60, 240, 120, 0, 240, 0, 0, 0, 0, 0, 20, 55, 0, 0, 0, 60, 60, 60, 50, 60, 0, 0, 0, 0, 0, 0};
int    SL_SIDE[NSLV]  = {0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1};       // 0 both, 1 long only
int    SL_REG[NSLV]   = {8, 6, 1, 2, 3, 1, 5, 0, 0, 0, 1, 0, 0, 0, 0, 4, 4, 4, 0, 0, 2, 5, 0, 0, 0, 2};       // 0 any, 1 ADX>25, 2 close>EMA200, 3 dir-aligned EMA200, 4 ADX>25 and price>EMA200
double SL_P[NSLV]     = {0, 0, 0, 0, 0, 0, 10.0, 0.10, 10.0, 0.30, 2.5, 20, 0, 0, 0, 0, 0, 0, 2.0, 2.0, 0.20, 10.0, 0, 0, 2.5, 2.0}; // RSI2 / IBS threshold, BB sigma, exit channel
double SL_SLK[NSLV]   = {2.5, 2.5, 3.0, 2.0, 2.5, 2.5, 3.0, 2.5, 3.0, 2.5, 2.0, 3.0, 0.6, 1.0, 0.4, 2.0, 2.0, 2.0, 2.0, 1.0, 2.0, 3.0, 1.0, 1.0, 1.5, 2.0};
double SL_TRK[NSLV]   = {2.0, 2.0, 2.0, 0.0, 3.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double SL_TPK[NSLV]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10.0, 10.0, 10.0, 0, 0, 0, 0, 0, 0, 0, 0};   // fixed target, x daily ATR (0 = none)
double SL_BEK[NSLV]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4.0, 4.0, 4.0, 0, 0, 0, 0, 0, 0, 0, 0};    // one-time break-even when the bar high/low reaches entry + x ATR (0 = none)
double SL_W[NSLV]     = {0.75, 1.0, 0.75, 1.5, 0.75, 0.75, 1.0, 1.0, 1.0, 1.0, 0.75, 0.5, 0.75, 1.5, 0.5, 1.0, 0.5, 0.5, 1.5, 1.0, 1.0, 1.0, 1.0, 0.75, 1.0, 0.75};   // v13.4: EURGBP MR 1.0 -> 1.5
int    SL_ADDS[NSLV]  = {0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};       // pyramid adds (step 1R, half size); GOLD T60 add off since v11.7 (the add alone: PF 0.61 in both periods)
int    SL_MAXB[NSLV]  = {360, 0, 360, 0, 0, 0, 170, 51, 170, 85, 0, 0, 0, 0, 0, 0, 0, 0, 3600, 0, 170, 170, 0, 0, 48, 120};   // time stop, H1 bars
int    SL_RTF[NSLV]  = {60, 480, 720, 1440, 360, 360, 720, 1440, 1440, 1440, 15, 1440, 1440, 1440, 1440, 1440, 1440, 1440, 1440, 1440, 1440, 1440, 1440, 1440, 1440, 1440};   // v12: regime timeframe in minutes (1440 = D1 features, 10080 = W1)
double SL_ADXT[NSLV] = {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25};   // v12: ADX threshold
double SL_X[NSLV]    = {0, 0, 0, -4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   // v12: kind 3 exit rank, kind 4 exit SMA, kind 5 exit IBS, kind 1 fast EMA
double SL_BER[NSLV] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   // v12 mgmt: stop to entry once MFE >= BER R
double SL_BEO[NSLV] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   //   ... plus this many R (lock)
double SL_LKA[NSLV] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   // v12 mgmt: once MFE >= LKA R ...
double SL_LKB[NSLV] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   //   ... stop >= entry + LKB R
double SL_RTR[NSLV] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0.5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   // v12 mgmt: trail RTR R behind the best high/low once MFE >= RTR R
double SL_PTR[NSLV] = {1.5, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   // v12 mgmt: partial close at PTR R ...
double SL_PFR[NSLV] = {0.5, 0, 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   //   ... of this fraction
double SL_STB[NSLV] = {0, 0, 0, 0, 72, 0, 0, 0, 24, 0, 48, 0, 0, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 0};   // v12 mgmt: stale exit after STB H1 bars ...
double SL_STR[NSLV] = {0, 0, 0, 0, 0.5, 0, 0, 0, 0.2, 0, 0.5, 0, 0, 0, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0, 0, 0};   //   ... if MFE < STR R
double SL_WKD[NSLV] = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   // v12 mgmt: 1 = flat before the weekend (Friday 15:00 New York), no new entries after it
double SL_PT2[NSLV] = {3, 3, 3, 0, 3, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0};   // v13.1: second partial close at PT2 R ...
double SL_PF2[NSLV] = {0.333, 0.333, 0.333, 0, 0.333, 0, 0, 0, 0, 0, 0.333, 0.333, 0, 0, 0, 0, 0, 0.333, 0, 0, 0, 0, 0, 0, 0, 0};   //   ... of this fraction of the remaining volume
bool   g_pdone2[NSLV];            // v13.1: second partial done
double g_bx[NSLV];                // v12: best high/low since entry
bool   g_pdone[NSLV];             // v12: partial close done
MArr<double> g_d1c;                   // v12: daily closes per role (role*1000 + i)
int    g_d1n[NROLE];
string SL_NAME[NSLV]  = {"GOLD T60", "GOLD T240", "GOLD MOM", "GOLD TOM", "SILV T240", "SILV MA", "DOW RSI2", "DOW IBS", "NAS RSI2", "NAS IBS", "AUDJPY BB", "SILV H4-55", "JPY GOTOBI", "NAS 3RD-FRI", "JPY FIX-REV", "GOLD LOTTERY", "NAS LOTTERY", "JPY LOTTERY", "EURGBP MR", "NAS GAP-LONG", "GOLD IBS", "SPX RSI2", "FED POST US30", "FED POST SPX", "DXY-JPY MR", "GOLD Z-CONT"};

//--------------------------------------------------------------------- state
struct DayFeat { bool ok; datetime day; double atr, adx, ema200, slope, close, rsi2, sma5, ibs, ret120, z50, sd50; };
struct SleeveState { int dir; double ep, r0, best; int adds; datetime t0; double base_lots; string status; };

CTrade   g_trade;
string   g_sym[NROLE];
bool     g_sym_ok[NROLE];
datetime g_last_h1[NROLE];
datetime g_last_h4[NROLE];
DayFeat  g_df[NROLE];
SleeveState g_ss[NSLV];
bool   g_sp_init = false;              // v13.7: STANDARD stop values captured once
double g_slk_std[NSLV];
int    g_maxb_std[NSLV];
bool   g_tight_cur[NSLV];               // v13.7: profile loaded in SL_SLK / SL_MAXB for the sleeve (true = TIGHT)
bool   g_tight_pos[NSLV];               // v13.7: profile of the sleeve's open position (saved with the sleeve state)
bool StopProfSleeve(const int k) { return k == 0 || k == 1 || k == 2 || k == 5 || k == 8 || k == 9 || k == 18; }
bool     g_use[NSLV];
int      g_preset = PRESET_PHASE1;
int      g_rmode = RM_SAFE;          // risk mode: safe / fast (panel button, saved)
bool     g_paused = false;
int      g_halt = 0;             // 0 none, 1 daily (auto-resume next day), 2 max-loss (manual reset)
datetime g_halt_day = 0;
bool     g_target_hit = false;
double   g_init_bal = 0;
datetime g_start_time = 0;
datetime g_day = 0;
double   g_day_ref = 0;          // max(balance, equity) at the start of the server day
double   g_eq_prevday = 0, g_vt_var = -1, g_vt_g = 1.0;   // portfolio volatility targeting state
int      g_trading_days = 0;
datetime g_last_helper_day = 0;
string   g_gv = "";
string   g_msg = "";
int      g_lang = LANG_FA;
MArr<MqlRates> g_r;
int      g_pend_act[NSLV];       // 0 none, 1 enter, 2 exit (delayed by the news filter)
int      g_pend_dir[NSLV];
datetime g_pend_t[NSLV];
MArr<datetime> g_news;
datetime g_news_loaded = 0;
double   g_hwm = 0;
double   g_bal_peak = 0;         // drawdown brake: highest closed balance
double   g_fill_price = 0;
double   g_fill_vol = 0;             // v11.9: volume actually filled by the last OpenTrade (after caps / partial fills)
bool     g_close_retry[NSLV];        // v11.9: a close was rejected; retried from the timer
MArr<double> g_w1;
MArr<double> g_w2;
datetime g_gb_exit = 0;          // Gotobi: scheduled exit (server time) of the open trade
long     g_gb_tried = -1;        // Gotobi: UTC day number of the last entry attempt
double   g_tf_ref = 0;           // 3rd Friday: index price at Thursday 16:00 New York
long     g_tf_refday = -1;       // 3rd Friday: New York day number of that reference
long     g_tf_tried = -1;        // 3rd Friday: New York day number of the last entry attempt
double   g_fr_ref = 0;           // fix reversal: USDJPY bid at 15:55 UTC the weekday before the Gotobi day
long     g_fr_refday = -1;       // fix reversal: UTC day number of the Gotobi day that reference belongs to
long     g_fr_tried = -1;
bool     g_nyalign = true;
bool     g_tz_ok = false;        // v11.9 demo: time zone of the tester data detected from the bars
int      g_tz_s = 3, g_tz_w = 2; // detected UTC offset of the data (hours) while the US is on summer / winter time
datetime g_mclosed_until[NROLE];   // v13.1: market closed for this symbol (rc 10018) - no close/modify requests until then
datetime g_mod_until[NROLE];       // v13.2: stop modification rejected (10018) - next try after this (1 minute)
double   g_st_pend[NSLV];          // v13.2: stop that could not be set (market closed) - retried each minute
datetime g_cl_until[NSLV];         // v13.2: close backoff per sleeve (rejected closes are not retried every second)
int      g_cl_back[NSLV];          // v13.2: current close backoff, seconds
datetime g_cl_msg[NSLV];           // v13.2: last "close rejected" journal note (at most one per 5 min)
int      g_rt_dir[NSLV];           // v13.2: entry rejected with 10018 - signal kept and retried each minute (0 = none)
datetime g_rt_t0[NSLV];            //   first rejection
datetime g_rt_last[NSLV];          //   last attempt
double   g_rt_px[NSLV];            //   price at the signal
double   g_rt_dist[NSLV];          //   stop distance of the signal
double   g_rt_tp[NSLV];            //   target distance of the signal (0 = none)
uint     g_last_rc = 0;            // v13.2: return code of the last failed OpenTrade order
bool     g_cl_wait[NSLV];          // v13.3: close queued because the market is closed (sent on the first tick after reopen)
int      g_pt_pend[NSLV];          // v13.3: partial close queued while closed (bit 1 = first, bit 2 = second)
int      g_ad_pend[NSLV];          // v13.3: pyramid add queued while closed (0 none, else the add number)
double   g_ad_px[NSLV];            //   price at the add signal
double   g_ad_st[NSLV];            //   shared stop of the add
double   g_ad_lots[NSLV];          //   add volume
int      g_lw_wait = 0;            // v13.3: roles (bit mask) skipped by the open-profit lock while closed - re-checked when one reopens
datetime g_mo_t[NROLE];            // v13.3: MarketOpen cache (one evaluation per role per second)
bool     g_mo_v[NROLE];
long     g_q_cnt[8];               // v13.3 counters (journal / tests): 0 stops queued, 1 queued stops sent, 2 closes queued, 3 queued closes sent,
                                   //   4 entries/adds queued, 5 queued entries/adds sent, 6 queued entries/adds dropped, 7 partials queued+sent
string   g_mpp_warned = ",";       // v13.2: symbols whose money-per-lot sources disagreed (warned once)
int      g_tzc_s[NROLE];           // v13.2: clock candidates read per symbol (summer / winter offset, method 1 spike, 2 weekly open, 0 none)
int      g_tzc_w[NROLE];
int      g_tzc_m[NROLE];
bool     g_demo_custom = false;  // at least one custom symbol was chosen
bool     g_tzr_ok[NROLE];        // DEMO: this symbol has its own detected clock (custom data from different sources)
int      g_tzr_s[NROLE];
int      g_tzr_w[NROLE];
bool     g_nyal_r[NROLE];        // DEMO: that clock ends the day at 17:00 New York
int      g_cur_role = -1;        // symbol whose clock ServerUTCOffsetSec / TD use (-1 = the common clock)
int      g_df_n[NROLE];          // daily bars available for the features (panel)       // v11.8: server midnight = 17:00 New York (EET + US DST, all backtests); false -> D1/H4 rebuilt from H1
bool     g_ready = false;        // v11.8: core initialised (live: only once the broker connection and account data are up)
int      g_init_tries = 0;
datetime g_last_c1[NROLE];       // last closed H1 bar processed per symbol (restart catch-up of the daily signals)
double   g_token = 0;            // single-instance guard: token of this copy
bool     g_idle = false;         // another copy of this EA already trades this account
datetime g_last_hb = 0;
datetime g_last_scan = 0;        // last H1 bar processed (any symbol), shown in the panel
string   g_inf_s[NSLV];          // live condition of each sleeve: short text (panel)
string   g_inf_f[NSLV];          // long text (tooltip and chart box)
double   g_lv_up[NSLV];          // entry levels drawn on the charts (0 = none)
double   g_lv_dn[NSLV];
datetime g_lv_t0[NSLV];          // start of the window that defines the level
datetime g_inf_time = 0;
double   g_gl_z = 0;             // NAS GAP-LONG: today's gap z-score (panel)
long     g_gl_zday = -1;
bool     g_inf_near[NSLV];       // close to its entry condition (panel highlight)
string   g_st_prev[NSLV];        // last seen block reason and when it appeared (shown for 24 h)
datetime g_st_t[NSLV];
datetime g_ov_last = 0;          // chart overlay refresh state
datetime g_ov_scan = -1;
int      g_ov_np = -1;

//--------------------------------------------------------------------- small helpers
// ---- hoisted globals
double g_pers_peak = 0;               // personal account: equity high-water mark (max-drawdown stop)
bool g_p1_ramp_done = false;          // v13.6: Phase-1 ramp reached its equity threshold (latched for the stage)
bool g_pa_on = false;
MArr<double> g_tfc_v;           // cache: role*16+slot -> cl, ema, adx, slope, key
bool g_bt_fired = false;
int g_news_src = 0;                    // 1 = MT5 calendar, 2 = ForexFactory (fresh or cached), 3 = FOMC table
string g_lot_note = "";              // v13.6: what changed the lot after the planned risk (journal: actual risk line)
int g_nskip = 0;
long g_gl_tried = -1;
long     g_fl_dn[4][64];             // v13.5: decision days (New York day number) per source, sorted, one per day
int      g_fl_min[4][64];           //        announcement minute of the New York day (840 = 14:00)
int      g_fl_n[4] = {0, 0, 0, 0};
long     g_fl_c0[4] = {0, 0, 0, 0}, g_fl_c1[4] = {-1, -1, -1, -1};   // New York days the source covers (inclusive)
bool     g_fl_ok[4] = {false, false, false, false};
datetime g_fl_t[4] = {0, 0, 0, 0};    // fetch time (server)
datetime g_ffn_t[128];               // v13.5: FF USD high-impact event times (UTC; cached, for later use)
int      g_ffn_n = 0;
datetime g_ffn_tf = 0;                // fetch time of g_ffn_t (server)
ulong    g_fomc_ev = 0;               // calendar event id of the Fed rate decision
bool     g_cal_alive = false;         // the MT5 calendar returned US events
datetime g_fomc_next = 0;             // next calendar read (server)
datetime g_web_next[2] = {0, 0};      // next fetch: 0 = federalreserve.gov, 1 = ForexFactory
bool     g_web_blk[2] = {false, false};   // error 4014 (URL not whitelisted)
bool     g_fp_paused = false;         // v13.6: FED POST paused (live / demo) because a source URL is not in the WebRequest whitelist
bool     g_fomc_cache_loaded = false;
string   g_fomc_said = ",";           // one-time notes already printed
long g_fp_tried[2] = {-1, -1};       // v15: FOMC-day entry guard per sleeve (k=22, k=23); v13.5: also in a terminal variable (restart-safe)
int  g_fp_amin[2] = {840, 840};      // v13.5: announcement minute of the open trade's day (exit at +120 min)
string g_dxy_sym[5];
bool   g_dxy_ok[5];
double g_dxy_w[5] = {-0.576, 0.136, -0.119, 0.091, 0.036};
double g_dxy_c[270];                  // per-day log index level (work array; 260 bars + margin)
datetime g_dxy_t[270];
long g_ns_day = -1;                    // v15: server day already processed by the 02:00 sleeve scan
datetime g_stage_t0 = 0;
double   g_stage_peak = 0, g_stage_mdd = 0, g_stage_wday = 0, g_stage_b0 = 0;
int      g_stage_halts = 0, g_stage_td0 = 0;
MArr<string> g_pk;
MArr<string> g_pv;
uint   g_arm_close = 0, g_arm_reset = 0;
bool   g_panel = false;
bool   g_panel_fpw = false;     // v13.6: the panel was built with the FED POST WebRequest warning
datetime g_last_panel = 0;
int      g_redraw_left = 0;     // forced redraws right after the panel is built (a redraw inside OnInit can be ignored by the terminal)
bool g_pmin = false;       // panel minimised to its header
bool g_smin = false;       // v13.1: strategy list collapsed (default: open)
int  g_pos_off = 0;        // first visible row of the open-positions list
// ---- prototypes
string GvKey(const string k);
double GvGet(const string k, const double def);
void   GvSet(const string k, const double v);
long   DayNum(const datetime t);
int    DowOf(const long dn);
bool   IsWeekday(const long dn);
int    HourOf(const datetime t);
long   SleeveMagic(const int k);
int    SleeveOfMagic(const long m);
bool   IsTester();
bool   IsPersonal();
double CushionMult();
void PerfAlertUpdate();
double BaseRiskPct();
string RmName();
bool TurboChallenge();
double SoftStopPct();
double DDBrakePct();
bool DDBrakeOn();
double RiskUnitPct();
double TargetPct();
string PresetName(const int p);
string NormSym(const string s);
string RoleAliases(const int role);
string RoleDescKeys(const int role);
bool IsIdxRole(const int role);
string RoleBad(const int role);
bool ListHas(const string list, const string n);
bool SymTradable(const string s);
int RoleScoreName(const int role, const string ns, const string aliases);
bool SymIsStock(const string s);
bool SymUsable(const string s, const int role);
string ResolveRole(const int role, const string user);
void ListServerIndices();
bool CalcDayFeatures(const int role);
int RegimeOf(const int role);
long TfKey(const datetime t, const int mn);
bool TfFeat(const int role, const int mn, double &cl, double &ema, double &adx, double &slope);
bool TfFeatCalc(const int role, const int mn, double &cl, double &ema, double &adx, double &slope);
bool RegimeAllows(const int k, const int dir);
int SleevePositions(const int k, double &lots, double &pnl);
datetime SrvNow();
bool SessionOpen(const string sym, const datetime now, datetime &sess_start);
datetime SessionEnd(const string sym, const datetime now);
bool MarketOpen(const int role);
void MarkClosed(const int role);
bool CloseNote(const int k);
void CloseSleeve(const int k, const string why);
void CloseAll(const string why);
void SetSleeveStop(const int k, const double st);
void QueuedActions();
void StopRetries();
void SaveSleeve(const int k);
void LoadSleeve(const int k);
double CcyToAccount(const string ccy);
double LossPerLot(const string sym, const double dist, const int dir = 1);
double MoneyPerPriceUnit(const string sym);
double StopOutFromNow();
double GoldCapBase();
double GoldCapR(const int dir);
double GoldRegimeMult(const int k);
void BookTake();
void LockWinners();
bool LockWaitOpen();
double OpenStopRisk();
bool NewsActive();
void NewsAdd(const datetime t);
void LoadNews();
bool NewsBlocked();
double DailyLoss();
double DayRefNow();
double DailyLimitMoney(const double pct);
double HardDailyPct();
double LossFloor(const double pct);
double GuardFloor();
bool EntryAllowed(string &why);
void LotNote(const string n);
double LotsFor(const string sym, const double risk_money, const double dist, string &why, const int dir = 1);
bool OpenTrade(const int k, const int dir, const double sl_dist, double lots, const string tag, const double tp_dist = 0);
void LogSkip(const int k, const int dir, const string why);
void EntryRetries();
string RiskNote(const int k, const string sym, const double dist, const int dir, const double lots, const double planned);
void TryEnter(const int k, const int dir, const double dist_override = 0, const double tp_override = 0);
int DayShiftSec(const datetime srv);
bool NyAligned();
datetime TD(const datetime srv);
int DailyFromH1(const string sym, const int days);
int GetH4(const string sym, const int count);
int DaysInMonth(const int y, const int m);
int WeekdaysLeftAfter(const datetime t);
int WeekdayRank(const datetime t);
bool WeekendZone();
bool DoPartial(const int k, const int which);
bool MgmtExtra(const int k, const int role, const double hi, const double lo, double &st);
void QueuedAdd(const int k);
void ManageOpen(const int k, const int role, const double c1, const datetime bar_open);
void RefreshNyAlign();
void OnNewH1(const int role, const datetime bar_open);
void OnNewH4Sleeve(const int k, const int role, const datetime bar_open);
int ServerUTCOffsetSec(const datetime srv);
bool DemoActive();
bool SpikeTZ(const int role, int &os, int &ow);
bool WeekOpenTZ(const int role, int &os, int &ow);
bool ChooseClock(int &cs, int &cw, string &how);
bool DetectDataTZ(const bool broker = false);
int NormHour(const int h);
int DemoBonus(const string s);
bool IsGotobiDay(const long dn);
void GotobiTimer();
int NYOffsetSec(const datetime utc);
bool IsThirdFriday(const long dn);
void ThirdFridayTimer();
void GapLongTimer();
void FomcReset();
int  FomcYmd(const long dn);
long FomcDn(const int y, const int mo, const int d);
bool FomcTableDay(const long dn);
void FomcSay(const string key, const string msg);
bool FomcUsDst(const datetime utc);
long FomcLiveOff();
datetime FomcCalToUTC(const datetime t);
long FomcNyDay(const datetime utc);
int  FomcNyMin(const datetime utc);
int  FomcSnap(int mn);
void FomcAdd(const int L, const long dn, const int mn);
void FomcCommit(const int L, const datetime t);
string FomcSrcName(const int L);
void FomcCompareTable(const int L);
bool FomcIsDecisionEvent(const MqlCalendarEvent &e);
bool FomcFindEvent();
bool FomcCalendarRead(const datetime now);
int FomcMonthOf(const string s);
string FomcTrim(string s);
bool FomcParseFed(const string html, const datetime now);
string FomcJsonStr(const string obj, const string key);
datetime FomcIsoToUTC(const string d);
bool FomcParseFF(const string js, const datetime now);
void FomcCacheSave();
bool FomcCacheLoad();
int FomcFetch(const string url, string &body);
void FpPauseUpdate();
void FomcWebStep(const int i, const datetime now);
void FomcRefresh(const bool force = false);
int FomcAnnounceMin(const long ny_dn);
bool IsFomcDay(const long ny_dn);
string FpExitWhy(const int ki, const int role, const datetime srv, const int ny_min);
bool FpQueueExpired(const int k);
void FomcPostTimer();
string AuxResolve(const string user, const string base);
void InitDxySymbols();
bool DxyCalc(double &z);
void DxyJpySleeve(const int k);
void GoldZcontSleeve(const int k);
void NewSleevesDaily();
void FixReversalTimer();
int CountTradingDays();
void DetectInitialBalance();
void StageReset();
void StageSummary(const string what);
void PropChecks();
void MinDayTrade();
string Px(const int role, const double p);
string DDMM(const datetime t);
string SrvTxt(const datetime t);
string TfName(const int mn);
string RegDesc(const int k);
string RegTxt(const int k, const int dir);
datetime NyToSrv(const long ny_dn, const int ny_mins, const datetime srv);
string DistTxt(const double d);
void SleeveInfo(const int k, const bool recalc);
color CBG();
color CCARD();
color CTXT();
color CSUB();
color CGRN();
color CRED();
color CAMB();
color CCYA();
color CBTN();
string Tr(const string key);
void PObj(const string name, const int type);
void PRect(const string name, const int x, const int y, const int w, const int h, const color bg);
void PLabel(const string name, const int x, const int y, const int fs);
void PButton(const string name, const int x, const int y, const int w, const int h);
bool PCache(const string key, const string val);
bool PText(const string name, const string txt, const color c);
bool PBg(const string name, const color c);
bool PWidth(const string name, const int w);
string ShortSt(const string key);
void PLabelA(const string name, const int x, const int y, const int fs, const int anchor);
bool PTip(const string name, const string tip);
string HHMM(const datetime t);
string ClockText();
void PanelBuild();
string Money(const double v);
string MoneyS(const double v);
bool Gauge(const string g, const string label, const string val, const double frac, const color c);
string FpWarnLine(const int i);
void PanelRefresh(const bool force);
void SetPreset(const int p);
bool ShowVisual();
void UpdateInfo();
void OvLabel(const long id, const string n, const int x, const int y, const string txt, const color c, const int fs);
void OvRect(const long id, const string n, const int x, const int y, const int w, const int h, const color bg);
void OvTrend(const long id, const string n, const datetime t1, const double p, const datetime t2, const color c, const int style, const int width);
void OvText(const long id, const string n, const datetime t, const double p, const string txt, const color c);
void OvArrow(const long id, const string n, const datetime t, const double p, const int code, const color c, const string tip);
color SleeveColor(const int k);
void OverlayChart(const long id, const bool events);
void ChartOverlay(const bool force);
void OpenSymbolCharts();
void ChartOverlayClear();
int InitCore();
void SetSleeveStops(const int k, const bool t);
bool TightNow();
void ApplyStopProfile();
void SyncStopProfile();
void WaitBanner();
int OnInit();
bool OwnInstance();
void OnDeinit(const int reason);
void OnTimer();
void OnTick();
void OnTradeTransaction(const MqlTradeTransaction &trans, const MqlTradeRequest &request, const MqlTradeResult &result);
void OnChartEvent(const int id, const long &lparam, const double &dparam, const string &sparam);
string GvKey(const string k) { return g_gv + k; }
double GvGet(const string k, const double def) { double v; if(GlobalVariableGet(GvKey(k), v)) return v; return def; }
void   GvSet(const string k, const double v) { GlobalVariableSet(GvKey(k), v); }
long   DayNum(const datetime t) { return (long)(t / 86400); }
int    DowOf(const long dn) { return (int)((dn + 4) % 7); }            // 0 Sunday .. 6 Saturday
bool   IsWeekday(const long dn) { int d = DowOf(dn); return d >= 1 && d <= 5; }
int    HourOf(const datetime t) { return (int)((t % 86400) / 3600); }
long   SleeveMagic(const int k) { return InpMagicBase + k; }
int    SleeveOfMagic(const long m) { long k = m - InpMagicBase; return (k >= 0 && k < NSLV) ? (int)k : -1; }
bool   IsTester() { return MQLInfoInteger(MQL_TESTER) != 0; }

bool   IsPersonal() { return InpAccountType == ACC_PERSONAL; }


// v13.11: TURBO challenge risk scaled by the cushion over the max-loss floor (start - 10%): 1 at or above the stage start,
// (equity - floor) / (10% of start) below it, never under InpTurboCushionMin (0.5 = half risk at -5%)
double CushionMult()
{
   if(InpTurboCushionMin >= 1.0 || g_init_bal <= 0) return 1.0;
   double c = (AccountInfoDouble(ACCOUNT_EQUITY) - 0.90 * g_init_bal) / (0.10 * g_init_bal);
   return MathMax(MathMax(InpTurboCushionMin, 0.0), MathMin(1.0, c));
}
// v13.12: performance alert (display / notification only)

void PerfAlertUpdate()
{
   if(InpPerfAlertPct <= 0 || g_init_bal <= 0 || !(g_preset == PRESET_FUNDED || g_preset == PRESET_PERSONAL)) return;
   datetime now = TimeCurrent(); double p = 0;
   if(!HistorySelect(now - 91 * 86400, now + 60)) return;
   for(int i = HistoryDealsTotal() - 1; i >= 0; i--)
   {
      ulong tk = HistoryDealGetTicket(i);
      if(tk == 0 || SleeveOfMagic(HistoryDealGetInteger(tk, DEAL_MAGIC)) < 0) continue;
      p += HistoryDealGetDouble(tk, DEAL_PROFIT) + HistoryDealGetDouble(tk, DEAL_COMMISSION) + HistoryDealGetDouble(tk, DEAL_SWAP);
   }
   p = p / g_init_bal * 100.0;
   if(!g_pa_on && p <= -InpPerfAlertPct)
   {
      g_pa_on = true;
      string m = (InpLanguage == LANG_FA)
         ? StringFormat("DonchianProp: نتیجه‌ی ۹۱ روز اخیر %+.2f%% (سطح هشدار -%.1f%%) - بدتر از ۹۵%% دوره‌های بک‌تست ۲۰۱۶ تا ۲۰۲۶. هزینه‌های بروکر و ژورنال را بررسی کنید.", p, InpPerfAlertPct)
         : StringFormat("DonchianProp: closed result of the last 91 days %+.2f%% (alert level -%.1f%%) - worse than 95%% of the 2016-2026 backtest. Check the broker costs and the journal.", p, InpPerfAlertPct);
      Print(m);
      if(!IsTester()) { Alert(m); SendNotification(m); }
   }
   else if(g_pa_on && p >= 0) { g_pa_on = false; PrintFormat("v13.12 performance alert cleared: last 91 days %+.2f%%", p); }
}
double BaseRiskPct()
{
   bool fast = (g_rmode != RM_SAFE), turbo = (g_rmode == RM_TURBO);
   double p1full = turbo ? InpTurboRiskPhase1 : InpFastRiskPhase1, p1start = turbo ? InpTurboRampStartPct : InpP1RampStartPct;
   if(g_preset == PRESET_PHASE1 && fast && p1start > 0 && !g_p1_ramp_done && g_init_bal > 0)
   {
      if(AccountInfoDouble(ACCOUNT_EQUITY) >= g_init_bal * (1.0 + InpP1RampAtPct / 100.0))
      { g_p1_ramp_done = true; GvSet("p1ramp", 1); PrintFormat("v13.6 PHASE1 ramp: equity +%.1f%% reached - risk %.2f%% -> %.2f%%", InpP1RampAtPct, p1start, p1full); }
      else return MathMin(p1start, p1full);
   }
   if(turbo && g_preset == PRESET_PHASE1) return InpTurboRiskPhase1 * CushionMult();
   if(turbo && g_preset == PRESET_PHASE2) return InpTurboRiskPhase2 * CushionMult();
   if(g_preset == PRESET_PERSONAL) return InpPersRiskPct;
   if(g_preset == PRESET_PHASE1) return fast ? InpFastRiskPhase1 : InpRiskPhase1;
   if(g_preset == PRESET_PHASE2) return fast ? InpFastRiskPhase2 : InpRiskPhase2;
   if(g_preset == PRESET_FUNDED)
   {
      double rf = fast ? InpFastRiskFunded : InpRiskFunded;
      if(InpFundedRampStartPct > 0 && g_init_bal > 0 && AccountInfoDouble(ACCOUNT_EQUITY) < g_init_bal * (1.0 + InpFundedRampAtPct / 100.0))
         return MathMin(InpFundedRampStartPct, rf);       // no cushion yet: reduced risk
      return rf;
   }
   return InpRiskCustom;
}
string RmName() { return g_rmode == RM_TURBO ? "TURBO" : (g_rmode == RM_FAST ? "FAST" : "SAFE"); }
bool TurboChallenge() { return g_rmode == RM_TURBO && (g_preset == PRESET_PHASE1 || g_preset == PRESET_PHASE2); }
double SoftStopPct() { return TurboChallenge() ? InpTurboSoftDailyStopPct : InpSoftDailyStopPct; }
double DDBrakePct()  { return TurboChallenge() ? InpTurboDDBrakePct : InpDDBrakePct; }
bool DDBrakeOn()                          // closed balance more than the brake level below its peak (in % of the initial balance)
{
   if(DDBrakePct() <= 0 || g_init_bal <= 0) return false;
   return (g_bal_peak - AccountInfoDouble(ACCOUNT_BALANCE)) >= DDBrakePct() / 100.0 * g_init_bal;
}
// Drawdown brake: tested on the real EA's 3,400 trades 2011-2026 against ~50 other sizing rules (static tilts, streaks,
// regime, equity-curve filters). It was the only rule that cut the challenge failure rate without hurting the in-sample MAR.
double RiskUnitPct() { return BaseRiskPct() * (DDBrakeOn() ? InpDDBrakeMult : 1.0) * (InpVolTarget ? g_vt_g : 1.0); }
double TargetPct()
{
   if(g_preset == PRESET_PERSONAL) return 0;
   if(g_preset == PRESET_PHASE1) return InpPhase1TargetPct;
   if(g_preset == PRESET_PHASE2) return InpPhase2TargetPct;
   if(g_preset == PRESET_FUNDED) return InpFundedTargetPct;
   return InpCustomTargetPct;
}
string PresetName(const int p)
{
   if(p == PRESET_PHASE1) return "PHASE1";
   if(p == PRESET_PHASE2) return "PHASE2";
   if(p == PRESET_FUNDED) return "FUNDED";
   if(p == PRESET_PERSONAL) return "PERSONAL";
   return "CUSTOM";
}

string NormSym(const string s)            // upper-case letters and digits only ("xauusd.r" -> "XAUUSDR")
{
   string u = s; StringToUpper(u);
   string o = "";
   for(int i = 0; i < StringLen(u); i++)
   {
      int c = StringGetCharacter(u, i);
      if((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) o += StringSubstr(u, i, 1);
   }
   return o;
}

string RoleAliases(const int role)         // comma list, most common first (normalised: upper-case letters and digits)
{
   if(role == 0) return "XAUUSD,GOLD,XAUUSDM,GOLDSPOT,GOLDUSD,SPOTGOLD,XAU";
   if(role == 1) return "XAGUSD,SILVER,XAGUSDM,SILVERSPOT,SILVERUSD,SPOTSILVER,XAG";
   if(role == 2) return "US30,DJ30,DJI30,WS30,DOW30,USA30,WALLST30,WALLSTREET30,WALLSTREET,DJIA,US30CASH,DOWJONES,DOWJONES30,USINDU,INDU,DJI,DOW,YM";
   if(role == 3) return "NDX100,NAS100,US100,USTEC,USTECH,NASDAQ100,NQ100,USTECH100,TECH100,NASDAQ,NDX,US100CASH,NAS,NQ";
   if(role == 4) return "AUDJPY";
   if(role == 5) return "USDJPY";
   if(role == 6) return "EURGBP";
   return "US500,SPX500,SP500,USA500,SPX,US500CASH,SPXUSD,USSPX500,SANDP500,SNP500,SP500M";
}
string RoleDescKeys(const int role)        // words looked for in the symbol description when the name says nothing
{
   if(role == 0) return "GOLD";
   if(role == 1) return "SILVER";
   if(role == 2) return "DOWJONES,WALLSTREET,DJIA,DOW30,US30,DJ30,USA30,WS30";
   if(role == 3) return "NASDAQ100,USTECH,US100,NDX100,TECH100,NAS100";
   if(role == 4) return "AUSTRALIANDOLLARVSJAPANESEYEN,AUSTRALIANDOLLARJAPANESEYEN";
   if(role == 5) return "USDOLLARVSJAPANESEYEN,USDOLLARJAPANESEYEN";
   if(role == 7) return "SP500,SANDP500,STANDARDPOORS,US500,SPX500,USA500";
   return "EUROVSGREATBRITAINPOUND,EUROVSBRITISHPOUND,EUROBRITISHPOUND,EUROVSPOUNDSTERLING";
}
bool IsIdxRole(const int role) { return role <= 3 || role == 7; }   // v13: role 7 = S&P 500
string RoleBad(const int role)             // words that mean a different instrument (crosses, other indices)
{
   if(role <= 1) return "EUR,AUD,GBP,JPY,CHF,CAD,SGD,TRY,CNH,EURO,POUND,YEN,FRANC,RUPEE,KRW,MXN,ZAR,HUF,PLN,CZK,THB";
   if(role == 7) return "2000,RUSSELL,NASDAQ,NDX,DOW,DJI,DAX,FTSE,NIKKEI,JP225,JPN225,GER,UK100,HK50,HSI,AUS200,EU50,STOXX,CAC,IBEX,SMI,CHINA,VIX,EQUALWEIGHT";
   if(role <= 3) return "2000,500,SPX,RUSSELL,SP500,DAX,FTSE,NIKKEI,JP225,JPN225,GER,UK100,HK50,HSI,AUS200,EU50,STOXX,CAC,IBEX,SMI,CHINA,VIX";
   return "";
}
bool ListHas(const string list, const string n)   // any comma item contained in n
{
   if(list == "" || n == "") return false;
   MArr<string> p;
   int k = StringSplit(list, ',', p);
   for(int i = 0; i < k; i++) if(p[i] != "" && StringFind(n, p[i]) >= 0) return true;
   return false;
}
bool SymTradable(const string s)
{
   long tm = SymbolInfoInteger(s, SYMBOL_TRADE_MODE);
   return tm != SYMBOL_TRADE_MODE_DISABLED && tm != SYMBOL_TRADE_MODE_CLOSEONLY;
}
// v12.1: the score uses the NAME only.  Reading the description/path of a symbol makes the strategy tester download
// ("synchronize") that symbol; scanning thousands of server symbols that way (MetaQuotes-Demo lists ~2000 stocks) flooded
// the tester, broke its history download ("rates base receive error") and let stocks such as AG or AMPX win on their description.
int RoleScoreName(const int role, const string ns, const string aliases)
{
   MArr<string> al;
   int na = StringSplit(aliases, ',', al);
   bool metal_usd = (role <= 1 && (StringFind(ns, "XAU") == 0 || StringFind(ns, "XAG") == 0) && StringFind(ns, "USD") > 0);
   if(!metal_usd && ListHas(RoleBad(role), ns)) return -1;
   int best = -1;
   for(int j = 0; j < na; j++)
   {
      string a = al[j];
      if(a == "") continue;
      int sc = -1;
      if(ns == a) sc = 1000;
      else if(StringFind(ns, a) == 0) sc = 800 - StringLen(ns);                          // XAUUSD.r, US30Cash
      else if(StringLen(a) >= 4 && StringFind(ns, a) > 0) sc = 600 - StringLen(ns);      // #US30, mXAUUSD
      if(sc < 0) continue;
      sc -= j;                                                                           // prefer earlier aliases
      if(sc > best) best = sc;
   }
   if(best < 0) return -1;
   if(IsIdxRole(role) && ListHas("MINI,MICRO,FUT", ns)) best -= 300;
   return best;
}
bool SymIsStock(const string s)            // read only for a few candidates
{
   string np = NormSym(SymbolInfoString(s, SYMBOL_PATH));
   return ListHas("STOCK,SHARE,EQUIT,ETF,FUND,BOND", np);
}
bool SymUsable(const string s, const int role)
{
   bool cust = (SymbolInfoInteger(s, SYMBOL_CUSTOM) != 0);
   if(cust && !DemoActive()) return false;             // custom symbols cannot be traded on an account: only in the DEMO tester
   if(!cust && SymIsStock(s)) return false;
   string nd = NormSym(SymbolInfoString(s, SYMBOL_DESCRIPTION));
   bool metal_usd = (role <= 1 && StringFind(NormSym(s), "USD") > 0);
   if(!metal_usd && ListHas(RoleBad(role), nd)) return false;
   return SymbolSelect(s, true) && SymTradable(s);
}
string ResolveRole(const int role, const string user)
{
   if(user == "-") return "";
   if(user != "" && SymbolSelect(user, true) && SymTradable(user)) return user;
   if(user != "") PrintFormat("v13: symbol '%s' not found or not tradable - searching automatically", user);
   string aliases = (user != "" ? NormSym(user) + "," + RoleAliases(role) : RoleAliases(role));
   MArr<string> cand; MArr<int> csc; int m = 0;
   int n = SymbolsTotal(false);
   for(int i = 0; i < n; i++)                          // pass 1: names only (no symbol is touched)
   {
      string s = SymbolName(i, false);
      int sc = RoleScoreName(role, NormSym(s), aliases);
      if(sc < 0) continue;
      ArrayResize(cand, m + 1); ArrayResize(csc, m + 1); cand[m] = s; csc[m] = sc; m++;
   }
   if(DemoActive()) for(int i = 0; i < m; i++) csc[i] += DemoBonus(cand[i]);
   for(int t = 0; t < m && t < 12; t++)                // best first; skip symbols this account cannot trade
   {
      int b = -1;
      for(int i = 0; i < m; i++) if(csc[i] >= 0 && (b < 0 || csc[i] > csc[b])) b = i;
      if(b < 0) break;
      string s = cand[b]; csc[b] = -1;
      if(SymUsable(s, role)) return s;
      PrintFormat("v13: %s skipped (stock, custom, other instrument or trading disabled)", s);
   }
   // pass 2 reads descriptions; in the tester only of the Market Watch symbols (already synchronized, see above)
   bool mw = IsTester();
   int n2 = SymbolsTotal(mw);
   string keys = RoleDescKeys(role);
   int best = -1; string bs = "";
   for(int i = 0; i < n2 && role >= 2; i++)            // metals and FX always carry their name; only indices get odd names ("WallSt")
   {
      string s = SymbolName(i, mw);
      string nd = NormSym(SymbolInfoString(s, SYMBOL_DESCRIPTION));
      if(!ListHas(keys, nd) || ListHas(RoleBad(role), nd)) continue;
      int sc = 400 - StringLen(s);
      if(sc > best && SymUsable(s, role)) { best = sc; bs = s; }
   }
   return bs;
}

void ListServerIndices()                   // help the user pick a name when an index is not found
{
   string out = ""; int c = 0;
   int n = SymbolsTotal(false);
   for(int i = 0; i < n && c < 40; i++)
   {
      string s = SymbolName(i, false);
      string ns = NormSym(s);
      if(StringFind(ns, "30") >= 0 || StringFind(ns, "100") >= 0 || StringFind(ns, "NAS") >= 0 || StringFind(ns, "DOW") >= 0 || StringFind(ns, "TEC") >= 0)
      { out += (c > 0 ? ", " : "") + s; c++; }
   }
   PrintFormat("v13: index-like symbols on this server: %s", c > 0 ? out : "(none)");
}

//--------------------------------------------------------------------- indicators on D1 (closed bars)
bool CalcDayFeatures(const int role)
{
   string sym = g_sym[role];
   g_cur_role = role;
   int n = (NyAligned() ? CopyRates(sym, PERIOD_D1, 1, 1000, g_r) : DailyFromH1(sym, 1000));
   g_df_n[role] = MathMax(n, 0);
   int nmin = (DemoActive() ? 130 : 260);      // DEMO: the tester may hold little history before the start date; precision grows as the test runs
   if(n < nmin)
   {
      if(g_df[role].ok || g_df[role].day == 0) { string hn = (n < 0 ? "no" : "only " + IntegerToString(n)); PrintFormat("v13 %s: %s daily bars of history (%d needed) - its sleeves wait. Raise Tools > Options > Charts > Max bars", sym, hn, nmin); }
      g_df[role].ok = false; g_df[role].day = 1; return false;
   }
   ArrayResize(g_w1, n); ArrayResize(g_w2, n);
   // ATR14 (Wilder) + ADX14
   double atr = 0, pdm_s = 0, mdm_s = 0, tr_s = 0, adx = 0, ema = g_r[0].close;
   double up_s = 0, dn_s = 0;
   int    cnt = 0;
   for(int i = 1; i < n; i++)
   {
      double h = g_r[i].high, l = g_r[i].low, pc = g_r[i - 1].close;
      double tr = MathMax(h - l, MathMax(MathAbs(h - pc), MathAbs(l - pc)));
      double up = h - g_r[i - 1].high, dn = g_r[i - 1].low - l;
      double pdm = (up > dn && up > 0) ? up : 0.0, mdm = (dn > up && dn > 0) ? dn : 0.0;
      if(i <= 14) { atr += tr / 14.0; tr_s += tr; pdm_s += pdm; mdm_s += mdm; }
      else
      {
         atr = (atr * 13.0 + tr) / 14.0;
         tr_s = tr_s - tr_s / 14.0 + tr; pdm_s = pdm_s - pdm_s / 14.0 + pdm; mdm_s = mdm_s - mdm_s / 14.0 + mdm;
      }
      if(i >= 14)
      {
         double pdi = (tr_s > 0 ? 100.0 * pdm_s / tr_s : 0), mdi = (tr_s > 0 ? 100.0 * mdm_s / tr_s : 0);
         double dx = (pdi + mdi > 0 ? 100.0 * MathAbs(pdi - mdi) / (pdi + mdi) : 0);
         cnt++;
         if(cnt <= 14) adx += dx / 14.0; else adx = (adx * 13.0 + dx) / 14.0;
      }
      // RSI(2) Wilder on closes
      double d = g_r[i].close - pc;
      double u = MathMax(d, 0.0), v = MathMax(-d, 0.0);
      if(i <= 2) { up_s += u / 2.0; dn_s += v / 2.0; }
      else { up_s = (up_s + u) / 2.0; dn_s = (dn_s + v) / 2.0; }
      ema = ema + (g_r[i].close - ema) * 2.0 / 201.0;
      g_w1[i] = ema;
   }
   DayFeat f;
   f.ok = true; f.day = g_r[n - 1].time;
   f.atr = atr; f.adx = adx; f.ema200 = ema; f.close = g_r[n - 1].close;
   f.slope = (g_w1[n - 1] - g_w1[n - 11]) / MathMax(atr, 1e-12);
   f.rsi2 = (dn_s <= 0 ? 100.0 : 100.0 - 100.0 / (1.0 + up_s / dn_s));
   double s5 = 0; for(int i = n - 5; i < n; i++) s5 += g_r[i].close; f.sma5 = s5 / 5.0;
   double rng = g_r[n - 1].high - g_r[n - 1].low;
   f.ibs = (rng > 0 ? (g_r[n - 1].close - g_r[n - 1].low) / rng : 0.5);
   f.ret120 = g_r[n - 1].close / g_r[n - 121].close - 1.0;
   double m50 = 0, q50 = 0;                    // z-score of the log close over the last 50 closed days (sample sd)
   for(int i = n - 50; i < n; i++) m50 += MathLog(g_r[i].close);
   m50 /= 50.0;
   for(int i = n - 50; i < n; i++) q50 += MathPow(MathLog(g_r[i].close) - m50, 2);
   f.sd50 = MathSqrt(q50 / 49.0);
   f.z50 = (f.sd50 > 0 ? (MathLog(f.close) - m50) / f.sd50 : 0);
   g_df[role] = f;
   if(ArraySize(g_d1c) < NROLE * 1000) ArrayResize(g_d1c, NROLE * 1000);
   int nn = MathMin(n, 1000); g_d1n[role] = nn;
   for(int i = 0; i < nn; i++) g_d1c[role * 1000 + i] = g_r[n - nn + i].close;
   return true;
}

int RegimeOf(const int role)    // 1 up-trend, -1 down-trend, 0 range (panel display)
{
   DayFeat f = g_df[role];
   if(!f.ok) return 0;
   if(f.close > f.ema200 && f.slope > 0.5 && f.adx > 20) return 1;
   if(f.close < f.ema200 && f.slope < -0.5 && f.adx > 20) return -1;
   return 0;
}

long TfKey(const datetime t, const int mn)
{
   if(mn >= 10080) return (DayNum(TD(t)) + 3) / 7;              // weeks (Monday based) on the trading-day clock
   return (long)TD(t) / ((long)mn * 60);
}

bool TfFeat(const int role, const int mn, double &cl, double &ema, double &adx, double &slope)
{
   string sym = g_sym[role];
   int slot = (mn == 15 ? 0 : mn == 30 ? 1 : mn == 60 ? 2 : mn == 120 ? 3 : mn == 180 ? 4 : mn == 240 ? 5 : mn == 360 ? 6 : mn == 480 ? 7 : mn == 720 ? 8 : 9);
   double key = (double)iTime(sym, mn < 60 ? PERIOD_M15 : PERIOD_H1, 0);
   if(ArraySize(g_tfc_v) < NROLE * 16 * 5) { ArrayResize(g_tfc_v, NROLE * 16 * 5); for(int i = 0; i < NROLE * 16 * 5; i++) g_tfc_v[i] = 0; }
   int ci = (role * 16 + slot) * 5;
   if(key > 0 && g_tfc_v[ci + 4] == key) { cl = g_tfc_v[ci]; ema = g_tfc_v[ci + 1]; adx = g_tfc_v[ci + 2]; slope = g_tfc_v[ci + 3]; return cl > 0; }
   bool ok = TfFeatCalc(role, mn, cl, ema, adx, slope);
   if(!ok) cl = 0;
   g_tfc_v[ci] = cl; g_tfc_v[ci + 1] = ema; g_tfc_v[ci + 2] = adx; g_tfc_v[ci + 3] = slope; g_tfc_v[ci + 4] = key;
   return ok;
}
bool TfFeatCalc(const int role, const int mn, double &cl, double &ema, double &adx, double &slope)
{
   string sym = g_sym[role];
   int cnt = 700;
   MArr<MqlRates> hb;
   ENUM_TIMEFRAMES src = (mn < 60 ? (mn == 15 ? PERIOD_M15 : PERIOD_M30) : PERIOD_H1);
   int ratio = (mn < 60 ? 1 : (mn >= 10080 ? 120 : mn / 60));
   int n = CopyRates(sym, src, mn < 60 ? 1 : 0, MathMin(cnt * ratio + 8, 60000), hb);
   if(n < 2) return false;
   MArr<MqlRates> t; ArrayResize(t, cnt + 64);
   long cur = TfKey(hb[n - 1].time, mn < 60 ? 1440000 : mn);
   if(mn < 60) cur = LONG_MAX;
   int m = 0; long key = -1;
   for(int i = 0; i < n; i++)
   {
      long g = (mn < 60 ? (long)i : TfKey(hb[i].time, mn));
      if(g >= cur) break;
      if(g != key)
      {
         if(m >= ArraySize(t)) ArrayResize(t, m + 256);
         t[m].open = hb[i].open; t[m].high = hb[i].high; t[m].low = hb[i].low; t[m].close = hb[i].close; key = g; m++;
      }
      else { t[m - 1].high = MathMax(t[m - 1].high, hb[i].high); t[m - 1].low = MathMin(t[m - 1].low, hb[i].low); t[m - 1].close = hb[i].close; }
   }
   int ep = (mn >= 10080 ? 40 : 200);
   if(m < (mn >= 10080 ? 60 : 260)) return false;
   double atr = 0, tr_s = 0, pdm_s = 0, mdm_s = 0, ax = 0, e = t[0].close, e10 = 0; int c = 0;
   for(int i = 1; i < m; i++)
   {
      double hh = t[i].high, l = t[i].low, pc = t[i - 1].close;
      double tr = MathMax(hh - l, MathMax(MathAbs(hh - pc), MathAbs(l - pc)));
      double up = hh - t[i - 1].high, dn = t[i - 1].low - l;
      double pdm = (up > dn && up > 0) ? up : 0.0, mdm = (dn > up && dn > 0) ? dn : 0.0;
      if(i <= 14) { atr += tr / 14.0; tr_s += tr; pdm_s += pdm; mdm_s += mdm; }
      else { atr = (atr * 13.0 + tr) / 14.0; tr_s = tr_s - tr_s / 14.0 + tr; pdm_s = pdm_s - pdm_s / 14.0 + pdm; mdm_s = mdm_s - mdm_s / 14.0 + mdm; }
      if(i >= 14)
      {
         double pdi = (tr_s > 0 ? 100.0 * pdm_s / tr_s : 0), mdi = (tr_s > 0 ? 100.0 * mdm_s / tr_s : 0);
         double dx = (pdi + mdi > 0 ? 100.0 * MathAbs(pdi - mdi) / (pdi + mdi) : 0);
         c++; if(c <= 14) ax += dx / 14.0; else ax = (ax * 13.0 + dx) / 14.0;
      }
      e = e + (t[i].close - e) * 2.0 / (ep + 1.0);
      if(i == m - 11) e10 = e;
   }
   cl = t[m - 1].close; ema = e; adx = ax; slope = (e - e10) / MathMax(atr, 1e-12);
   return true;
}

bool RegimeAllows(const int k, const int dir)
{
   int role = SL_ROLE[k];
   DayFeat f = g_df[role];
   if(!f.ok) return false;
   int rg = SL_REG[k];
   if(rg == 0) return true;
   double cl = f.close, ema = f.ema200, adx = f.adx, slope = f.slope, thr = SL_ADXT[k];
   bool d1 = (SL_RTF[k] == 1440);
   if(!d1 && !TfFeat(role, SL_RTF[k], cl, ema, adx, slope)) return false;
   double px = d1 ? SymbolInfoDouble(g_sym[role], SYMBOL_BID) : cl;
   if(rg == 1) return adx > thr;
   if(rg == 2) return cl > ema;
   if(rg == 3) return (cl - ema) * dir > 0;
   if(rg == 4) return adx > thr && px > ema;
   if(rg == 5) return slope * dir > 0;
   if(rg == 6) return (cl - ema) * dir > 0 && slope * dir > 0;
   if(rg == 7) return adx < thr;
   if(rg == 8) return (cl - ema) * dir > 0 && adx > thr;
   if(rg == 9) return (cl - ema) * dir > 0 && adx < thr;
   return true;
}

//--------------------------------------------------------------------- positions
int SleevePositions(const int k, double &lots, double &pnl)
{
   lots = 0; pnl = 0; int c = 0;
   for(int i = PositionsTotal() - 1; i >= 0; i--)
   {
      ulong t = PositionGetTicket(i);
      if(t == 0 || PositionGetInteger(POSITION_MAGIC) != SleeveMagic(k)) continue;
      c++; lots += PositionGetDouble(POSITION_VOLUME);
      pnl += PositionGetDouble(POSITION_PROFIT) + PositionGetDouble(POSITION_SWAP);
   }
   return c;
}

// v13.3: market-closed gate.  A symbol is open when (a) the broker's trade session covers the current server time (when the
// broker publishes sessions at all) and (b) its last tick is fresh: younger than 120 s, or arrived since the session (re)opened.
// A 10018 reply despite this (session table wrong) closes the symbol for 60 s.  Nothing is sent or printed while closed.
datetime SrvNow() { return IsTester() ? TimeCurrent() : TimeTradeServer(); }
bool SessionOpen(const string sym, const datetime now, datetime &sess_start)
{
   sess_start = 0;
   MqlDateTime dt; TimeToStruct(now, dt);
   int tod = dt.hour * 3600 + dt.min * 60 + dt.sec;
   bool any = false;
   for(int d = 0; d < 7 && !any; d++)
   {
      datetime f, t;
      if(SymbolInfoSessionTrade(sym, (ENUM_DAY_OF_WEEK)d, 0, f, t)) any = true;
   }
   if(!any) return true;                                   // no session table published: rely on the tick test and 10018
   datetime day0 = now - tod;
   for(uint i = 0; i < 16; i++)
   {
      datetime f, t;
      if(!SymbolInfoSessionTrade(sym, (ENUM_DAY_OF_WEEK)dt.day_of_week, i, f, t)) break;
      int a = (int)((long)f % 86400), b = (int)MathMin((double)(long)t, 2.0 * 86400);   // 'to' = 86400 means 24:00
      if(b <= a) b += 86400;                                // a session written across midnight
      if(tod >= a && tod < b) { sess_start = day0 + a; return true; }
      if(tod + 86400 >= a && tod + 86400 < b) { sess_start = day0 - 86400 + a; return true; }
   }
   return false;
}
// v13.6: end (server time) of the trade session that contains 'now'; 0 = no session table / not in a session / the session runs
// on without a break (ends at 24:00 and the next day's first session starts at 00:00).
datetime SessionEnd(const string sym, const datetime now)
{
   MqlDateTime dt; TimeToStruct(now, dt);
   int tod = dt.hour * 3600 + dt.min * 60 + dt.sec;
   datetime day0 = now - tod;
   for(int back = 0; back <= 1; back++)                   // today's sessions, then yesterday's session written across midnight
   {
      int dw = (dt.day_of_week + 7 - back) % 7;
      for(uint i = 0; i < 16; i++)
      {
         datetime f, t;
         if(!SymbolInfoSessionTrade(sym, (ENUM_DAY_OF_WEEK)dw, i, f, t)) break;
         int a = (int)((long)f % 86400), b = (int)MathMin((double)(long)t, 2.0 * 86400);
         if(b <= a) b += 86400;
         int x = tod + back * 86400;
         if(x < a || x >= b) continue;
         if(b % 86400 == 0)                                 // ends at midnight: a real close only if the next day does not open at 00:00
         {
            datetime f2, t2;
            int nd = (dw + b / 86400) % 7;
            if(SymbolInfoSessionTrade(sym, (ENUM_DAY_OF_WEEK)nd, 0, f2, t2) && (long)f2 % 86400 == 0) return 0;
         }
         return day0 - back * 86400 + b;
      }
   }
   return 0;
}
bool MarketOpen(const int role)
{
   if(role < 0 || role >= NROLE || !g_sym_ok[role]) return false;
   datetime now = SrvNow();
   if(g_mo_t[role] == now && now != 0) return g_mo_v[role];
   bool ok = true;
   string sym = g_sym[role];
   if(now < g_mclosed_until[role]) ok = false;             // the broker answered 10018 recently
   datetime ss = 0;
   if(ok && !SessionOpen(sym, now, ss)) ok = false;
   if(ok)
   {
      MqlTick tk;
      if(!SymbolInfoTick(sym, tk) || tk.time <= 0) ok = false;
      else if(TimeCurrent() - tk.time > 120 && !(ss > 0 && tk.time >= ss)) ok = false;
   }
   g_mo_t[role] = now; g_mo_v[role] = ok;
   return ok;
}
void MarkClosed(const int role) { if(role >= 0 && role < NROLE) { g_mclosed_until[role] = SrvNow() + 60; g_mo_t[role] = 0; } }

bool CloseNote(const int k)               // v13.2: at most one "close rejected / deferred" journal line per sleeve per 5 min
{
   if(g_cl_msg[k] != 0 && TimeCurrent() - g_cl_msg[k] < 300) return false;
   g_cl_msg[k] = TimeCurrent(); return true;
}
void CloseSleeve(const int k, const string why)
{
   datetime now = TimeCurrent();
   bool failed = false, deferred = false;
   int role = SL_ROLE[k];
   double l0, p0;
   if(SleevePositions(k, l0, p0) > 0 && !MarketOpen(role))      // v13.3: market closed - queue the close, send nothing, print nothing
   {
      if(!g_cl_wait[k]) g_q_cnt[2]++;
      g_cl_wait[k] = true; g_close_retry[k] = true; g_ss[k].status = "close deferred";
      return;
   }
   for(int i = PositionsTotal() - 1; i >= 0; i--)
   {
      ulong t = PositionGetTicket(i);
      if(t == 0 || PositionGetInteger(POSITION_MAGIC) != SleeveMagic(k)) continue;
      if(now < g_cl_until[k]) { deferred = true; continue; }   // v13.2: backoff after a rejected close - no request now
      g_trade.SetTypeFillingBySymbol(PositionGetString(POSITION_SYMBOL));
      if(g_trade.PositionClose(t)) continue;
      if(g_trade.ResultRetcode() == 10018) { MarkClosed(role); deferred = true; break; }   // session table wrong: closed for 60 s, quietly
      if(!g_trade.PositionClose(t)) { failed = true; if(CloseNote(k)) PrintFormat("v13 close failed %s ticket %I64u rc %u", SL_NAME[k], t, g_trade.ResultRetcode()); }
   }
   double l, p;
   if(SleevePositions(k, l, p) > 0)        // v11.9: a rejected close keeps the sleeve state; the timer retries
   {
      g_close_retry[k] = true; g_ss[k].status = (deferred && !failed ? "close deferred" : "close retry");
      if(deferred && !failed && !MarketOpen(role)) { if(!g_cl_wait[k]) g_q_cnt[2]++; g_cl_wait[k] = true; }
      if(failed)                           // v13.2: backoff 10 s, 20 s, 40 s ... 5 min instead of a request (and a journal line) every second
      {
         g_cl_back[k] = (g_cl_back[k] <= 0 ? 10 : (int)MathMin(300, 2 * g_cl_back[k]));
         g_cl_until[k] = now + g_cl_back[k];
         if(CloseNote(k)) PrintFormat("v13 %s: close rejected (%s) - retrying in %d s (journal note at most every 5 min)", SL_NAME[k], why, g_cl_back[k]);
      }
      return;
   }
   g_close_retry[k] = false; g_cl_wait[k] = false; g_cl_back[k] = 0; g_cl_until[k] = 0; g_cl_msg[k] = 0;
   g_st_pend[k] = 0; g_pt_pend[k] = 0; g_ad_pend[k] = 0;
   g_ss[k].dir = 0; g_ss[k].adds = 0;
   SaveSleeve(k);
   PrintFormat("v13 %s closed: %s", SL_NAME[k], why);
}

void CloseAll(const string why)
{
   for(int k = 0; k < NSLV; k++)
   {
      double l, p;
      if(SleevePositions(k, l, p) > 0) CloseSleeve(k, why);
   }
}

void SetSleeveStop(const int k, const double st)
{
   int role = SL_ROLE[k];
   string sym = g_sym[role];
   int dg = (int)SymbolInfoInteger(sym, SYMBOL_DIGITS);
   double nst = NormalizeDouble(st, dg);
   if(nst <= 0) return;
   if(!MarketOpen(role))                     // v13.3: closed - keep the tightest wanted stop, send it on the first tick after reopen
   {
      bool better = false;
      for(int i = PositionsTotal() - 1; i >= 0; i--)
      {
         ulong t = PositionGetTicket(i);
         if(t == 0 || PositionGetInteger(POSITION_MAGIC) != SleeveMagic(k)) continue;
         double cur = PositionGetDouble(POSITION_SL);
         bool buy = (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY);
         if(cur <= 0 || (buy ? nst > cur + 1e-9 : nst < cur - 1e-9)) { better = true; if(g_st_pend[k] == 0 || (buy ? nst > g_st_pend[k] : nst < g_st_pend[k])) { if(g_st_pend[k] == 0) g_q_cnt[0]++; g_st_pend[k] = nst; } break; }
      }
      return;
   }
   double stops = SymbolInfoInteger(sym, SYMBOL_TRADE_STOPS_LEVEL) * SymbolInfoDouble(sym, SYMBOL_POINT);
   double bid = SymbolInfoDouble(sym, SYMBOL_BID), ask = SymbolInfoDouble(sym, SYMBOL_ASK);
   bool blocked = false;
   for(int i = PositionsTotal() - 1; i >= 0; i--)
   {
      ulong t = PositionGetTicket(i);
      if(t == 0 || PositionGetInteger(POSITION_MAGIC) != SleeveMagic(k)) continue;
      double cur = PositionGetDouble(POSITION_SL);
      long typ = PositionGetInteger(POSITION_TYPE);
      if(typ == POSITION_TYPE_BUY)
      {
         if(cur > 0 && nst <= cur + 1e-9) continue;
         if(nst >= bid - stops) continue;              // too close: keep the old stop
      }
      else
      {
         if(cur > 0 && nst >= cur - 1e-9) continue;
         if(nst <= ask + stops) continue;
      }
      if(!g_trade.PositionModify(t, nst, PositionGetDouble(POSITION_TP)))
      {
         if(g_trade.ResultRetcode() == 10018) { MarkClosed(role); blocked = true; break; }   // session table wrong: closed for 60 s, quietly
         else PrintFormat("v13 modify failed %s rc %u", SL_NAME[k], g_trade.ResultRetcode());
      }
   }
   g_st_pend[k] = (blocked ? nst : 0);
}

// v13.3: everything queued while a market was closed is sent on the first tick after it reopens (called every timer tick;
// MarketOpen is cached per second, so a closed market costs nothing and sends nothing)
void QueuedActions()
{
   for(int k = 0; k < NSLV; k++)
   {
      bool any = (g_st_pend[k] != 0 || g_cl_wait[k] || g_pt_pend[k] != 0 || g_ad_pend[k] != 0);
      if(!any) continue;
      int role = SL_ROLE[k];
      if(!MarketOpen(role)) continue;
      double l, p;
      if(SleevePositions(k, l, p) == 0) { g_st_pend[k] = 0; g_cl_wait[k] = false; g_pt_pend[k] = 0; g_ad_pend[k] = 0; g_close_retry[k] = false; continue; }
      g_cur_role = role;
      if(g_cl_wait[k])
      {   // cleared first: a close the broker then rejects for another reason goes to the normal backoff (no request per tick)
         g_cl_wait[k] = false; g_cl_until[k] = 0;
         CloseSleeve(k, "queued close (market reopened)");
         if(SleevePositions(k, l, p) == 0) g_q_cnt[3]++;
         g_cur_role = -1; continue;
      }
      if(g_st_pend[k] != 0) { double st = g_st_pend[k]; g_q_cnt[1]++; SetSleeveStop(k, st); }
      if(g_pt_pend[k] != 0)
      {
         int w = g_pt_pend[k]; g_pt_pend[k] = 0;
         if((w & 1) != 0 && DoPartial(k, 1)) { double x = g_ss[k].ep + g_ss[k].dir * 0.5 * g_ss[k].r0; SetSleeveStop(k, x); }
         if((w & 2) != 0) DoPartial(k, 2);
         SaveSleeve(k);
      }
      if(g_ad_pend[k] != 0) QueuedAdd(k);
      g_cur_role = -1;
   }
}
void StopRetries() { QueuedActions(); }      // v13.2 name kept (unit tests)

void SaveSleeve(const int k)
{
   string p = "s" + IntegerToString(k) + "_";
   GvSet(p + "dir", g_ss[k].dir); GvSet(p + "ep", g_ss[k].ep); GvSet(p + "r0", g_ss[k].r0);
   GvSet(p + "best", g_ss[k].best); GvSet(p + "adds", g_ss[k].adds); GvSet(p + "t0", (double)g_ss[k].t0);
   GvSet(p + "lots", g_ss[k].base_lots);
   GvSet(p + "bx", g_bx[k]); GvSet(p + "pd", g_pdone[k] ? 1 : 0); GvSet(p + "pd2", g_pdone2[k] ? 1 : 0);
   GvSet(p + "tp", g_tight_pos[k] ? 1 : 0);   // v13.7
}

void LoadSleeve(const int k)
{
   string p = "s" + IntegerToString(k) + "_";
   double lots, pnl;
   int c = SleevePositions(k, lots, pnl);
   g_ss[k].status = "";
   if(c == 0) { g_ss[k].dir = 0; g_ss[k].adds = 0; g_tight_pos[k] = false; SaveSleeve(k); return; }
   g_ss[k].dir = (int)GvGet(p + "dir", 0);
   g_tight_pos[k] = (GvGet(p + "tp", 0) > 0.5);   // v13.7
   g_ss[k].ep = GvGet(p + "ep", 0); g_ss[k].r0 = GvGet(p + "r0", 0); g_ss[k].best = GvGet(p + "best", 0);
   g_ss[k].adds = (int)GvGet(p + "adds", c - 1); g_ss[k].t0 = (datetime)GvGet(p + "t0", 0);
   g_ss[k].base_lots = GvGet(p + "lots", 0);
   g_bx[k] = GvGet(p + "bx", 0); g_pdone[k] = (GvGet(p + "pd", 0) > 0.5); g_pdone2[k] = (GvGet(p + "pd2", 0) > 0.5);
   if(g_ss[k].dir == 0 || g_ss[k].ep <= 0 || g_ss[k].r0 <= 0)
   {   // no saved state (e.g. another terminal): rebuild from the oldest position
      datetime oldest = 0;
      for(int i = PositionsTotal() - 1; i >= 0; i--)
      {
         ulong t = PositionGetTicket(i);
         if(t == 0 || PositionGetInteger(POSITION_MAGIC) != SleeveMagic(k)) continue;
         datetime ot = (datetime)PositionGetInteger(POSITION_TIME);
         if(oldest == 0 || ot < oldest)
         {
            oldest = ot;
            g_ss[k].dir = (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY ? 1 : -1);
            g_ss[k].ep = PositionGetDouble(POSITION_PRICE_OPEN);
            g_ss[k].base_lots = PositionGetDouble(POSITION_VOLUME);
            double sl = PositionGetDouble(POSITION_SL);
            g_ss[k].r0 = (sl > 0 && (g_ss[k].ep - sl) * g_ss[k].dir > 0) ? MathAbs(g_ss[k].ep - sl) : SL_SLK[k] * g_df[SL_ROLE[k]].atr;
         }
      }
      g_ss[k].t0 = oldest; g_ss[k].best = g_ss[k].ep; g_ss[k].adds = c - 1;
      g_bx[k] = 0; g_pdone[k] = (SL_PTR[k] > 0 && lots < g_ss[k].base_lots - 1e-9); g_pdone2[k] = false;
      SaveSleeve(k);
      PrintFormat("v13 %s: state rebuilt from %d open position(s)", SL_NAME[k], c);
   }
}

//--------------------------------------------------------------------- risk & guards
// v13.2: money per lot, from three sources.  MetaQuotes-Demo reported SYMBOL_TRADE_TICK_VALUE / TICK_SIZE for XAUUSD and XAGUSD at
// 1/10 of the money a deal really books, so v13.1 traded 9.8x its planned risk there.  OrderCalcProfit (the broker's own profit
// formula) is the primary source; the tick value (loss side) and contract size x price move (converted to the account currency)
// check it.  When they disagree by more than 30% the largest loss per lot is used and the journal says so once per symbol.
double CcyToAccount(const string ccy)       // value of 1 unit of ccy in the account currency (0 = no conversion symbol)
{
   string acc = AccountInfoString(ACCOUNT_CURRENCY);
   if(ccy == "" || ccy == acc) return 1.0;
   string a = ccy + acc, b = acc + ccy;
   double p = SymbolInfoDouble(a, SYMBOL_BID);
   if(p > 0) return p;
   p = SymbolInfoDouble(b, SYMBOL_BID);
   if(p > 0) return 1.0 / p;
   for(int r = 0; r < NROLE; r++)           // a traded symbol with a broker suffix (USDJPY.r)
   {
      if(!g_sym_ok[r]) continue;
      string n = NormSym(g_sym[r]);
      double q = SymbolInfoDouble(g_sym[r], SYMBOL_BID);
      if(q <= 0) continue;
      if(StringFind(n, a) == 0) return q;
      if(StringFind(n, b) == 0) return 1.0 / q;
   }
   return 0;
}
double LossPerLot(const string sym, const double dist, const int dir)   // account money lost by 1 lot when price moves dist against it
{
   if(dist <= 0) return 0;
   double bid = SymbolInfoDouble(sym, SYMBOL_BID), ask = SymbolInfoDouble(sym, SYMBOL_ASK);
   double ts = SymbolInfoDouble(sym, SYMBOL_TRADE_TICK_SIZE);
   double tvl = SymbolInfoDouble(sym, SYMBOL_TRADE_TICK_VALUE_LOSS);
   if(tvl <= 0) tvl = SymbolInfoDouble(sym, SYMBOL_TRADE_TICK_VALUE);
   double a = 0, b = (tvl > 0 && ts > 0 ? dist * tvl / ts : 0), c = 0, pr = 0;
   bool buy = (dir >= 0);
   if(buy && ask - dist <= 0) buy = false;                  // a stop below zero cannot be priced: measure the move upwards
   double e = (buy ? ask : bid), s = (buy ? e - dist : e + dist);
   if(e > 0 && OrderCalcProfit(buy ? ORDER_TYPE_BUY : ORDER_TYPE_SELL, sym, 1.0, e, s, pr) && pr < 0) a = -pr;
   long cm = SymbolInfoInteger(sym, SYMBOL_TRADE_CALC_MODE);
   if(cm == SYMBOL_CALC_MODE_FOREX || cm == SYMBOL_CALC_MODE_FOREX_NO_LEVERAGE || cm == SYMBOL_CALC_MODE_CFD || cm == SYMBOL_CALC_MODE_CFDLEVERAGE)
   {   // profit = price move x contract size x lots in these modes (not for futures / CFD-index / exchange modes)
      double cs = SymbolInfoDouble(sym, SYMBOL_TRADE_CONTRACT_SIZE), cv = CcyToAccount(SymbolInfoString(sym, SYMBOL_CURRENCY_PROFIT));
      if(cs > 0 && cv > 0) c = dist * cs * cv;
   }
   double lo = 0, hi = 0;
   if(a > 0) { lo = (lo == 0 ? a : MathMin(lo, a)); hi = MathMax(hi, a); }
   if(b > 0) { lo = (lo == 0 ? b : MathMin(lo, b)); hi = MathMax(hi, b); }
   if(c > 0) { lo = (lo == 0 ? c : MathMin(lo, c)); hi = MathMax(hi, c); }
   if(hi <= 0) return 0;
   if(hi > 1.3 * lo)
   {
      if(StringFind(g_mpp_warned, "," + sym + ",") < 0)
      {
         g_mpp_warned += sym + ",";
         PrintFormat("v13.2 WARNING %s: money per lot for a %.5f move disagrees - OrderCalcProfit %.2f, tick value %.2f, contract x move %.2f %s. The largest (%.2f) is used for sizing and risk",
                     sym, dist, a, b, c, AccountInfoString(ACCOUNT_CURRENCY), hi);
      }
      return hi;
   }
   return (a > 0 ? a : hi);
}
double MoneyPerPriceUnit(const string sym)    // account money per 1.0 price move per 1 lot (v13.2: from LossPerLot)
{
   double px = SymbolInfoDouble(sym, SYMBOL_BID);
   if(px <= 0) px = SymbolInfoDouble(sym, SYMBOL_ASK);
   if(px > 0) { double ref = 0.01 * px; return LossPerLot(sym, ref) / ref; }
   double tv = SymbolInfoDouble(sym, SYMBOL_TRADE_TICK_VALUE), ts = SymbolInfoDouble(sym, SYMBOL_TRADE_TICK_SIZE);
   if(tv <= 0 || ts <= 0) return 0;
   return tv / ts;
}

// v13.1: money lost from the CURRENT prices if every stop is hit (open profit included - the daily reference counts it)
double StopOutFromNow()
{
   double r = 0;
   for(int i = PositionsTotal() - 1; i >= 0; i--)
   {
      ulong t = PositionGetTicket(i);
      if(t == 0 || SleeveOfMagic(PositionGetInteger(POSITION_MAGIC)) < 0) continue;
      string sym = PositionGetString(POSITION_SYMBOL);
      double sl = PositionGetDouble(POSITION_SL), v = PositionGetDouble(POSITION_VOLUME), cur = PositionGetDouble(POSITION_PRICE_CURRENT);
      int dir = (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY ? 1 : -1);
      if(sl <= 0) { r += cur * v * 0.05 * MoneyPerPriceUnit(sym); continue; }
      double d = (cur - sl) * dir;
      if(d > 0) r += d * v * MoneyPerPriceUnit(sym);
   }
   return r;
}
// v13.1: when all stops together could break the daily guard, lock half of the open profit of every winner
void LockWinners();
// v13.4: dynamic gold cap and gold-regime risk switch (completed D1 bar of gold: ADX14, EMA200 slope over 10 bars in ATR)
double GoldCapBase() { return (g_preset == PRESET_FUNDED && InpGoldDirCapFundedR > 0) ? InpGoldDirCapFundedR : InpGoldDirCapR; }
double GoldCapR(const int dir)
{
   if(!InpGoldCapDyn || !g_df[0].ok) return GoldCapBase();
   if(g_df[0].adx > InpGoldCapAdxHi && g_df[0].slope * dir > 0.5) return InpGoldCapHiR;
   if(g_df[0].adx < InpGoldCapAdxLo) return InpGoldCapLoR;
   return GoldCapBase();
}
double GoldRegimeMult(const int k)
{
   if(!InpGoldRegime || !g_df[0].ok || g_df[0].adx >= InpGRAdx) return 1.0;
   int r = SL_ROLE[k], kd = SL_KIND[k];
   bool metal = (r == 0 || r == 1);
   if(metal && (kd == 0 || kd == 1 || kd == 2 || kd == 6 || kd == 7)) return InpGRTrendMult;
   if(!metal && (kd == 4 || kd == 5 || kd == 11)) return InpGRMRMult;
   return 1.0;
}
// v13.4: book-level profit capture.  Open P/L of all sleeve positions above InpBookTakePct % of the initial balance:
// mode 0 closes InpBookTakeFrac of every winning position, mode 1 moves every winner's stop to lock half its open profit.
// Fires once per episode; re-armed when the open P/L falls below half the trigger.

void BookTake()
{
   if(InpBookTakePct <= 0 || g_init_bal <= 0) return;
   double op = 0;
   for(int i = PositionsTotal() - 1; i >= 0; i--)
   {
      ulong t = PositionGetTicket(i);
      if(t == 0 || SleeveOfMagic(PositionGetInteger(POSITION_MAGIC)) < 0) continue;
      op += PositionGetDouble(POSITION_PROFIT) + PositionGetDouble(POSITION_SWAP);
   }
   double trig = InpBookTakePct / 100.0 * g_init_bal;
   if(g_bt_fired) { if(op < 0.5 * trig) g_bt_fired = false; return; }
   if(op <= trig) return;
   int n = 0;
   if(InpBookTakeMode == 1) { LockWinners(); n = -1; }
   else
   {
      for(int i = PositionsTotal() - 1; i >= 0; i--)
      {
         ulong t = PositionGetTicket(i);
         int k = (t == 0 ? -1 : SleeveOfMagic(PositionGetInteger(POSITION_MAGIC)));
         if(k < 0 || PositionGetDouble(POSITION_PROFIT) <= 0) continue;
         if(!MarketOpen(SL_ROLE[k])) continue;
         string sym = PositionGetString(POSITION_SYMBOL);
         double v = PositionGetDouble(POSITION_VOLUME), vst = SymbolInfoDouble(sym, SYMBOL_VOLUME_STEP);
         double cv = MathFloor(v * InpBookTakeFrac / vst + 1e-9) * vst;
         if(cv < SymbolInfoDouble(sym, SYMBOL_VOLUME_MIN) || cv >= v) continue;
         g_trade.SetTypeFillingBySymbol(sym);
         if(g_trade.PositionClosePartial(t, cv)) n++;
      }
   }
   g_bt_fired = true;
   PrintFormat("v13 book take: open P/L %.2f > %.2f -> %s", op, trig, n < 0 ? "winners locked" : IntegerToString(n) + " partial close(s)");
}
void LockWinners()
{
   int n = 0;
   g_lw_wait = 0;
   for(int i = PositionsTotal() - 1; i >= 0; i--)
   {
      ulong t = PositionGetTicket(i);
      int k = (t == 0 ? -1 : SleeveOfMagic(PositionGetInteger(POSITION_MAGIC)));
      if(k < 0) continue;
      if(!MarketOpen(SL_ROLE[k])) { g_lw_wait |= (1 << SL_ROLE[k]); continue; }    // v13.3: closed - checked again on the first tick after the reopen
      string sym = PositionGetString(POSITION_SYMBOL);
      double op = PositionGetDouble(POSITION_PRICE_OPEN), sl = PositionGetDouble(POSITION_SL), cur = PositionGetDouble(POSITION_PRICE_CURRENT);
      int dir = (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY ? 1 : -1);
      if((cur - op) * dir <= 0) continue;
      int dg = (int)SymbolInfoInteger(sym, SYMBOL_DIGITS);
      double nst = NormalizeDouble(op + dir * 0.5 * (cur - op), dg);
      double minstop = (SymbolInfoInteger(sym, SYMBOL_TRADE_STOPS_LEVEL) + 1) * SymbolInfoDouble(sym, SYMBOL_POINT);
      if(sl > 0 && (nst - sl) * dir <= 0) continue;
      if((cur - nst) * dir <= minstop) continue;
      if(g_trade.PositionModify(t, nst, PositionGetDouble(POSITION_TP))) n++;
      else if(g_trade.ResultRetcode() == 10018) { MarkClosed(SL_ROLE[k]); g_lw_wait |= (1 << SL_ROLE[k]); }   // v13.3: session table wrong - quietly, again after the reopen
   }
   if(n > 0) PrintFormat("v13 open-profit lock: %d stop(s) moved to half of the open profit (stops could have broken the daily guard)", n);
}
bool LockWaitOpen()        // v13.3: a symbol skipped by LockWinners while closed is open again
{
   for(int r = 0; r < NROLE; r++) if((g_lw_wait & (1 << r)) != 0 && MarketOpen(r)) return true;
   return false;
}
double OpenStopRisk()      // money lost if every stop is hit from the entry prices (only stops on the loss side)
{
   double r = 0;
   for(int i = PositionsTotal() - 1; i >= 0; i--)
   {
      ulong t = PositionGetTicket(i);
      if(t == 0 || SleeveOfMagic(PositionGetInteger(POSITION_MAGIC)) < 0) continue;
      string sym = PositionGetString(POSITION_SYMBOL);
      double op = PositionGetDouble(POSITION_PRICE_OPEN), sl = PositionGetDouble(POSITION_SL), v = PositionGetDouble(POSITION_VOLUME);
      int dir = (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY ? 1 : -1);
      if(sl <= 0) { r += op * v * 0.05 * MoneyPerPriceUnit(sym); continue; }
      double loss = (op - sl) * dir;
      if(loss > 0) r += loss * v * MoneyPerPriceUnit(sym);
   }
   return r;
}

//--------------------------------------------------------------------- news filter (live only; the tester has no calendar)
bool NewsActive() { return InpNewsFilter == NEWS_ALWAYS || (InpNewsFilter == NEWS_FUNDED && g_preset == PRESET_FUNDED); }
// v13.6: live sources in the same order as the FOMC dates: 1) MT5 calendar (USD, high importance) while it works; 2) ForexFactory
// weekly JSON, USD high impact (fetched by the FOMC chain, also from its cache; at most 8 days old); 3) the FOMC table (decision
// times).  The Strategy Tester has no calendar: filter off there, as before.

void NewsAdd(const datetime t) { int m = ArraySize(g_news); ArrayResize(g_news, m + 1); g_news[m] = t; }
void LoadNews()
{
   datetime now = TimeTradeServer();
   if(g_news_loaded != 0 && now - g_news_loaded < 1800) return;
   g_news_loaded = now;
   ArrayResize(g_news, 0);
   MArr<MqlCalendarValue> vals;
   ResetLastError();
   int n = CalendarValueHistory(vals, now - 3600, now + 4 * 3600, NULL, "USD");
   for(int i = 0; i < n; i++)
   {
      MqlCalendarEvent ev;
      if(!CalendarEventById(vals[i].event_id, ev) || ev.importance != CALENDAR_IMPORTANCE_HIGH) continue;
      NewsAdd(vals[i].time);
   }
   if(n > 0 || (n == 0 && g_cal_alive)) { g_news_src = 1; return; }
   long off = FomcLiveOff();                                 // UTC -> server
   if(g_ffn_tf > 0 && now - g_ffn_tf < 8 * 86400)
   {
      for(int i = 0; i < g_ffn_n; i++) NewsAdd((datetime)(g_ffn_t[i] + off));
      g_news_src = 2; return;
   }
   g_news_src = 3;
   datetime utc = (datetime)(now - off);
   for(long dn = FomcNyDay(utc) - 1; dn <= FomcNyDay(utc) + 1; dn++)
   {
      int am = FomcAnnounceMin(dn); if(am < 0) continue;
      datetime ny = (datetime)(dn * 86400 + am * 60);         // New York wall clock -> UTC (offset of that moment)
      datetime u = (datetime)(ny - NYOffsetSec((datetime)(ny + 5 * 3600)));
      NewsAdd((datetime)(u + off));
   }
}
bool NewsBlocked()
{
   if(!NewsActive() || IsTester()) return false;
   LoadNews();
   datetime now = TimeTradeServer();
   for(int i = 0; i < ArraySize(g_news); i++)
   {
      long gap = (long)(now > g_news[i] ? now - g_news[i] : g_news[i] - now);
      if(gap <= (long)InpNewsWindowMin * 60) return true;
   }
   return false;
}

double DailyLoss() { return MathMax(0.0, g_day_ref - AccountInfoDouble(ACCOUNT_EQUITY)); }
double DayRefNow()   // reference level captured at the server-day start
{
   double bal = AccountInfoDouble(ACCOUNT_BALANCE), eq = AccountInfoDouble(ACCOUNT_EQUITY);
   if(InpDailyLossRef == DLR_BALANCE) return bal;
   if(InpDailyLossRef == DLR_EQUITY) return eq;
   return MathMax(bal, eq);
}
double DailyLimitMoney(const double pct) { return pct / 100.0 * (InpDailyLossBase == DLB_INITIAL ? g_init_bal : g_day_ref); }
// account guards: prop = FundedNext-style daily / max loss with the safety buffer; personal = own daily stop and an
// equity-peak drawdown stop (no static floor: a personal account compounds)
double HardDailyPct() { return IsPersonal() ? InpPersDailyStopPct : InpDailyLossPct - InpSafetyBufferPct; }
double GuardFloor();
double LossFloor(const double pct)        // equity level that must never be reached
{
   double base = (InpMaxLossType == ML_STATIC ? g_init_bal : g_hwm);
   double fl = base - pct / 100.0 * g_init_bal;
   if(InpMaxLossType != ML_STATIC && InpTrailLockAtInitial) fl = MathMin(fl, g_init_bal);
   return fl;
}

double GuardFloor()
{
   if(IsPersonal()) return (InpPersMaxDDPct > 0 && g_pers_peak > 0 ? g_pers_peak * (1.0 - InpPersMaxDDPct / 100.0) : 0);
   return LossFloor(InpMaxLossPct - InpSafetyBufferPct);
}
bool EntryAllowed(string &why)
{
   why = "";
   if(g_paused) { why = "paused"; return false; }
   if(g_halt != 0) { why = "halt"; return false; }
   if(g_target_hit) { why = "target"; return false; }
   if(DailyLoss() >= DailyLimitMoney(SoftStopPct())) { why = "soft stop"; return false; }
   if(NewsBlocked()) { why = "news"; return false; }
   if(!TerminalInfoInteger(TERMINAL_TRADE_ALLOWED) || !MQLInfoInteger(MQL_TRADE_ALLOWED)) { why = "algo off"; return false; }
   return true;
}


void LotNote(const string n) { if(StringFind(g_lot_note, n) < 0) g_lot_note += (g_lot_note == "" ? "" : ", ") + n; }
double LotsFor(const string sym, const double risk_money, const double dist, string &why, const int dir)
{
   why = "";
   double lpl = LossPerLot(sym, dist, dir);       // v13.2: loss of 1 lot from entry to stop, cross-checked (see LossPerLot)
   double vmin = SymbolInfoDouble(sym, SYMBOL_VOLUME_MIN), vmax = SymbolInfoDouble(sym, SYMBOL_VOLUME_MAX), vst = SymbolInfoDouble(sym, SYMBOL_VOLUME_STEP);
   if(lpl <= 0 || dist <= 0 || vst <= 0) { why = "symbol info"; return 0; }
   double lots = MathFloor(risk_money / lpl / vst + 1e-9) * vst;
   if(lots < vmin)
   {
      double minrisk = vmin * lpl;
      if(minrisk > InpMaxRiskPerTradeR * BaseRiskPct() / 100.0 * g_init_bal) { why = "min-lot risk"; return 0; }
      if(InpMinLotOverRisk > 0 && minrisk > InpMinLotOverRisk * risk_money) { why = "min-lot risk"; return 0; }   // v13.1: small accounts - never more than x times the sleeve's own risk
      lots = vmin; LotNote("raised to the broker minimum lot");
   }
   return lots;           // may exceed the broker maximum: OpenTrade splits it into several orders
}

bool OpenTrade(const int k, const int dir, const double sl_dist, double lots, const string tag, const double tp_dist)
{
   string sym = g_sym[SL_ROLE[k]];
   if(!MarketOpen(SL_ROLE[k])) { g_last_rc = 10018; g_ss[k].status = "market closed"; return false; }   // v13.3: gate - nothing is sent while closed
   if(InpGoldDirCapR > 0 && SL_ROLE[k] == 0)      // same-direction gold risk cap (stop risk measured from the current mark)
   {
      double g = 0, mpp = MoneyPerPriceUnit(sym);
      for(int i = PositionsTotal() - 1; i >= 0; i--)
      {
         ulong t = PositionGetTicket(i);
         if(t == 0 || PositionGetString(POSITION_SYMBOL) != sym || SleeveOfMagic(PositionGetInteger(POSITION_MAGIC)) < 0) continue;
         int pd = (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY ? 1 : -1);
         if(pd != dir) continue;
         double sl = PositionGetDouble(POSITION_SL), px = (pd > 0 ? SymbolInfoDouble(sym, SYMBOL_BID) : SymbolInfoDouble(sym, SYMBOL_ASK));
         if(sl > 0) g += MathMax(0.0, (px - sl) * pd) * PositionGetDouble(POSITION_VOLUME) * mpp;
      }
      double room = GoldCapR(dir) * BaseRiskPct() / 100.0 * g_init_bal - g, want = lots * sl_dist * mpp;
      if(room <= 0) { g_ss[k].status = "gold cap"; return false; }
      if(want > room)
      {
         double vst = SymbolInfoDouble(sym, SYMBOL_VOLUME_STEP);
         lots = MathFloor(lots * room / want / vst + 1e-9) * vst;
         if(lots < SymbolInfoDouble(sym, SYMBOL_VOLUME_MIN)) { g_ss[k].status = "gold cap"; return false; }
         LotNote(StringFormat("gold same-direction cap %.1fR", GoldCapR(dir)));
      }
   }
   double add_risk = lots * sl_dist * MoneyPerPriceUnit(sym);
   if(OpenStopRisk() + add_risk > InpMaxOpenRiskR * BaseRiskPct() / 100.0 * g_init_bal) { g_ss[k].status = "risk cap"; return false; }
   if(DailyLoss() + OpenStopRisk() + add_risk > InpDailyRoomMult * DailyLimitMoney(HardDailyPct())) { g_ss[k].status = "daily room"; return false; }
   if(AccountInfoDouble(ACCOUNT_EQUITY) - OpenStopRisk() - add_risk < GuardFloor()) { g_ss[k].status = "loss room"; return false; }
   double margin = 0, price = (dir > 0 ? SymbolInfoDouble(sym, SYMBOL_ASK) : SymbolInfoDouble(sym, SYMBOL_BID));
   if(OrderCalcMargin(dir > 0 ? ORDER_TYPE_BUY : ORDER_TYPE_SELL, sym, lots, price, margin) && margin > 0.7 * AccountInfoDouble(ACCOUNT_MARGIN_FREE))
   { g_ss[k].status = "margin"; return false; }
   int dg = (int)SymbolInfoInteger(sym, SYMBOL_DIGITS);
   double minstop = (SymbolInfoInteger(sym, SYMBOL_TRADE_STOPS_LEVEL) + 1) * SymbolInfoDouble(sym, SYMBOL_POINT);
   if(sl_dist <= minstop || (tp_dist > 0 && tp_dist <= minstop)) { g_ss[k].status = "stops level"; return false; }
   double sl = NormalizeDouble(price - dir * sl_dist, dg);
   double tp = (tp_dist > 0 ? NormalizeDouble(price + dir * tp_dist, dg) : 0);
   g_trade.SetExpertMagicNumber(SleeveMagic(k));
   g_trade.SetTypeFillingBySymbol(sym);
   string cm = "v13 " + SL_NAME[k] + tag;
   double vmax = SymbolInfoDouble(sym, SYMBOL_VOLUME_MAX), vst = SymbolInfoDouble(sym, SYMBOL_VOLUME_STEP);
   double left = lots, wsum = 0, vsum = 0;
   int guard = 0;
   while(left > vst * 0.5 && guard++ < 50)   // large accounts: split above the broker's max volume
   {
      double v = MathMin(left, vmax);
      v = NormalizeDouble(MathFloor(v / vst + 1e-9) * vst, 8);
      if(v <= 0) break;
      bool ok = (dir > 0 ? g_trade.Buy(v, sym, price, sl, tp, cm) : g_trade.Sell(v, sym, price, sl, tp, cm));
      if(!ok)
      {
         g_last_rc = g_trade.ResultRetcode();
         bool quiet = (g_last_rc == 10018);                             // v13.3: market closed (session table wrong): queued quietly
         if(quiet) MarkClosed(SL_ROLE[k]);
         if(!quiet) PrintFormat("v13 %s order failed rc %u (%s)", SL_NAME[k], g_trade.ResultRetcode(), g_trade.ResultRetcodeDescription());
         if(vsum == 0) { g_ss[k].status = (g_last_rc == 10018 ? "market closed" : "order failed"); return false; }
         break;
      }
      double fp = g_trade.ResultPrice();
      wsum += (fp > 0 ? fp : price) * v; vsum += v; left -= v;
   }
   g_fill_price = (vsum > 0 ? wsum / vsum : price);
   g_fill_vol = vsum;
   if(vsum > 0 && vsum < lots - vst * 0.5) LotNote(StringFormat("partly filled %.2f of %.2f lots", vsum, lots));
   return vsum > 0;
}


void LogSkip(const int k, const int dir, const string why)     // v13.1: every skipped entry signal goes to the journal
{
   g_nskip++;
   PrintFormat("v13 %s %s signal skipped: %s", SL_NAME[k], dir > 0 ? "BUY" : "SELL", why);
}
// v13.3: entries blocked by a closed market (gate) or rejected with 10018 are kept and sent on the first tick after the reopen,
// while the price is still within 0.25 daily ATR of the price at the signal and the signal's stop is still on the right side
// of the price; the stop distance and target of the signal are kept.  Otherwise the signal is dropped once (LogSkip).
void EntryRetries()
{
   for(int k = 0; k < NSLV; k++)
   {
      int dir = g_rt_dir[k];
      if(dir == 0) continue;
      double l, p;
      if(SleevePositions(k, l, p) > 0) { g_rt_dir[k] = 0; continue; }       // entered meanwhile (a new signal)
      if(SL_KIND[k] == 15 && FpQueueExpired(k))                           // v13.6: FED POST never trades outside its own window
      { g_rt_dir[k] = 0; g_q_cnt[6]++; LogSkip(k, dir, "market closed, FED POST window (14:06-16:00 New York) of that day is over - queued entry dropped"); continue; }
      int role = SL_ROLE[k];
      if(!MarketOpen(role)) continue;                                       // still closed: nothing sent, nothing printed
      string sym = g_sym[role];
      double px = (dir > 0 ? SymbolInfoDouble(sym, SYMBOL_ASK) : SymbolInfoDouble(sym, SYMBOL_BID)), atr = g_df[role].atr;
      double stl = g_rt_px[k] - dir * g_rt_dist[k];
      double minstop = (SymbolInfoInteger(sym, SYMBOL_TRADE_STOPS_LEVEL) + 1) * SymbolInfoDouble(sym, SYMBOL_POINT);
      if(px <= 0 || atr <= 0 || MathAbs(px - g_rt_px[k]) > 0.25 * atr)
      { g_rt_dir[k] = 0; g_q_cnt[6]++; LogSkip(k, dir, StringFormat("market closed, then price moved %.2f ATR from the signal", atr > 0 ? MathAbs(px - g_rt_px[k]) / atr : 0)); continue; }
      if((px - stl) * dir <= minstop)
      { g_rt_dir[k] = 0; g_q_cnt[6]++; LogSkip(k, dir, "market closed, then the signal's stop was passed"); continue; }
      int ns = g_nskip;
      g_cur_role = role;
      g_last_rc = 0;
      TryEnter(k, dir, g_rt_dist[k], g_rt_tp[k]);
      g_cur_role = -1;
      if(SleevePositions(k, l, p) > 0) { g_rt_dir[k] = 0; g_q_cnt[5]++; PrintFormat("v13 %s: queued entry sent at the market reopen, %d s after the signal", SL_NAME[k], (int)(TimeCurrent() - g_rt_t0[k])); continue; }
      if(g_last_rc == 10018) continue;                                        // closed again (broker): kept
      g_rt_dir[k] = 0; g_q_cnt[6]++;
      if(g_nskip == ns) LogSkip(k, dir, g_ss[k].status);                     // blocked before the order (guard, spread, regime ...)
   }
}
// v13.6: journal text of an entry - the ACTUAL money at risk to the stop after every cap and lot rounding (v13.5 printed the
// planned risk), the plan, and what changed it: drawdown brake / volatility target / gold regime (risk unit), gold same-direction
// cap, broker minimum lot, lot-step rounding, partial fill.
string RiskNote(const int k, const string sym, const double dist, const int dir, const double lots, const double planned)
{
   double act = lots * LossPerLot(sym, dist, dir);
   string why = "";
   if(DDBrakeOn()) why += StringFormat("drawdown brake x%.2f, ", InpDDBrakeMult);
   if(InpVolTarget && MathAbs(g_vt_g - 1.0) > 1e-9) why += StringFormat("volatility target x%.2f, ", g_vt_g);
   double gm = GoldRegimeMult(k); if(MathAbs(gm - 1.0) > 1e-9) why += StringFormat("gold regime x%.2f, ", gm);
   why += g_lot_note;
   if(g_lot_note == "" && planned > 0 && act < planned * 0.98) why += "lot-step rounding";
   if(StringLen(why) >= 2 && StringSubstr(why, StringLen(why) - 2) == ", ") why = StringSubstr(why, 0, StringLen(why) - 2);
   double unit = BaseRiskPct() / 100.0 * SL_W[k] * g_init_bal;
   return StringFormat("risk %.2f = %.3f%% actual; planned %.2f%s", act, g_init_bal > 0 ? act / g_init_bal * 100.0 : 0, unit,
                       why == "" ? "" : " - changed by: " + why);
}
void TryEnter(const int k, const int dir, const double dist_override, const double tp_override)
{
   int role = SL_ROLE[k];
   string sym = g_sym[role], why;
   if(!EntryAllowed(why)) { g_ss[k].status = why; return; }
   if(SL_SIDE[k] == 1 && dir < 0) return;
   if(!RegimeAllows(k, dir)) { g_ss[k].status = "regime"; return; }
   if(SL_WKD[k] > 0 && WeekendZone()) { g_ss[k].status = "weekend"; return; }
   DayFeat f = g_df[role];
   double spread = SymbolInfoDouble(sym, SYMBOL_ASK) - SymbolInfoDouble(sym, SYMBOL_BID);
   if(spread > InpMaxSpreadATR * f.atr) { g_ss[k].status = "spread"; return; }
   long tm = SymbolInfoInteger(sym, SYMBOL_TRADE_MODE);
   if(tm == SYMBOL_TRADE_MODE_DISABLED || tm == SYMBOL_TRADE_MODE_CLOSEONLY) { g_ss[k].status = "market closed"; return; }
   double dist = (dist_override > 0 ? dist_override : (SL_KIND[k] == 11 ? SL_SLK[k] * f.sd50 * f.close : SL_SLK[k] * f.atr));
   double risk_money = RiskUnitPct() / 100.0 * SL_W[k] * g_init_bal * GoldRegimeMult(k);
   g_lot_note = "";
   double lots = LotsFor(sym, risk_money, dist, why, dir);
   if(lots <= 0) { g_ss[k].status = why; LogSkip(k, dir, StringFormat("%s (needs %.2f lot min, risk %.0f planned)", why, SymbolInfoDouble(sym, SYMBOL_VOLUME_MIN), risk_money)); return; }
   double tpd = (tp_override > 0 ? tp_override : SL_TPK[k] * f.atr);
   g_last_rc = 0;
   if(!OpenTrade(k, dir, dist, lots, "", tpd))
   {
      // v13.3: market closed (gate, or 10018 from the broker): keep the signal for the reopen - nothing printed
      if(g_last_rc == 10018 && g_rt_dir[k] != 0) return;                               // a queued entry still closed: EntryRetries decides
      if(g_last_rc == 10018 && !((SL_KIND[k] >= 8 && SL_KIND[k] <= 10) || SL_KIND[k] == 14))   // not for the clock sleeves (own windows)
      {
         g_rt_dir[k] = dir; g_rt_t0[k] = TimeCurrent(); g_rt_last[k] = TimeCurrent();
         g_rt_px[k] = (dir > 0 ? SymbolInfoDouble(sym, SYMBOL_ASK) : SymbolInfoDouble(sym, SYMBOL_BID));
         g_rt_dist[k] = dist; g_rt_tp[k] = tpd;
         g_ss[k].status = "market closed - queued"; g_q_cnt[4]++;
         return;
      }
      LogSkip(k, dir, g_ss[k].status); return;
   }
   g_rt_dir[k] = 0;
   g_ss[k].dir = dir; g_ss[k].ep = g_fill_price; g_ss[k].r0 = dist; g_ss[k].best = g_ss[k].ep;
   g_bx[k] = 0; g_pdone[k] = false; g_pdone2[k] = false;
   g_ss[k].adds = 0; g_ss[k].t0 = TimeCurrent(); g_ss[k].base_lots = (g_fill_vol > 0 ? g_fill_vol : lots); g_ss[k].status = "";
   g_tight_pos[k] = g_tight_cur[k];
   if(g_ss[k].ep <= 0) g_ss[k].ep = (dir > 0 ? SymbolInfoDouble(sym, SYMBOL_ASK) : SymbolInfoDouble(sym, SYMBOL_BID));
   SaveSleeve(k);
   PrintFormat("v13 %s %s %.2f lots @%.5f SL dist %.5f (%s)%s", SL_NAME[k], dir > 0 ? "BUY" : "SELL", g_ss[k].base_lots, g_ss[k].ep, dist, RiskNote(k, sym, dist, dir, g_ss[k].base_lots, risk_money),
               (StopProfSleeve(k) && g_tight_pos[k] && InpStopProfile != STOP_TIGHT) ? " [v13.7 TIGHT stops: funded cushion]" : "");
}


//--------------------------------------------------------------------- trading-day clock (v11.8)
// Every rule was tested on bars whose day ends at 17:00 New York (server midnight on EET servers with US daylight saving,
// FundedNext's usual setup).  On other servers (e.g. UTC+0) the broker's D1 and H4 bars cut the day elsewhere and add a Sunday
// bar, which changes ATR, RSI2, IBS, ADX, EMA200 and the H4 signals.  There the EA rebuilds D1 and H4 from H1 on the 17:00 NY
// clock.  On NY-aligned servers DayShiftSec is 0 and the broker's own bars are used exactly as before.
int DayShiftSec(const datetime srv)
{
   int off = ServerUTCOffsetSec(srv);
   return -off + NYOffsetSec((datetime)(srv - off)) + 7 * 3600;
}
bool NyAligned() { return (g_cur_role >= 0 && g_cur_role < NROLE && g_tzr_ok[g_cur_role]) ? g_nyal_r[g_cur_role] : g_nyalign; }
datetime TD(const datetime srv) { return NyAligned() ? srv : (datetime)(srv + DayShiftSec(srv)); }

int DailyFromH1(const string sym, const int days)        // closed trading days, oldest first, into g_r
{
   MArr<MqlRates> h;
   int n = CopyRates(sym, PERIOD_H1, 0, days * 24 + 48, h);
   ArrayResize(g_r, 0);
   if(n < 2) return 0;
   long cur = DayNum(TD(h[n - 1].time));                  // the day of the forming bar is not closed
   MArr<MqlRates> t; ArrayResize(t, days + 400);
   int m = 0; long key = -1;
   for(int i = 0; i < n; i++)
   {
      long d = DayNum(TD(h[i].time));
      if(d >= cur) break;
      if(DowOf(d) == 0 || DowOf(d) == 6) continue;        // stray weekend quotes
      if(d != key)
      {
         if(m >= ArraySize(t)) ArrayResize(t, m + 256);
         t[m].time = (datetime)(d * 86400); t[m].open = h[i].open; t[m].high = h[i].high; t[m].low = h[i].low; t[m].close = h[i].close;
         key = d; m++;
      }
      else { t[m - 1].high = MathMax(t[m - 1].high, h[i].high); t[m - 1].low = MathMin(t[m - 1].low, h[i].low); t[m - 1].close = h[i].close; }
   }
   int s0 = MathMax(0, m - days);
   ArrayResize(g_r, m - s0);
   for(int i = s0; i < m; i++) g_r[i - s0] = t[i];
   return m - s0;
}

int GetH4(const string sym, const int count)              // closed H4 bars, oldest first, into g_r
{
   if(NyAligned()) return CopyRates(sym, PERIOD_H4, 1, count, g_r);
   MArr<MqlRates> h;
   int n = CopyRates(sym, PERIOD_H1, 0, count * 4 + 8, h);
   ArrayResize(g_r, 0);
   if(n < 2) return 0;
   long cur = (long)TD(h[n - 1].time) / 14400;
   MArr<MqlRates> t; ArrayResize(t, count + 64);
   int m = 0; long key = -1;
   for(int i = 0; i < n; i++)
   {
      long g = (long)TD(h[i].time) / 14400;
      if(g >= cur) break;
      if(g != key)
      {
         if(m >= ArraySize(t)) ArrayResize(t, m + 256);
         t[m].time = (datetime)(g * 14400); t[m].open = h[i].open; t[m].high = h[i].high; t[m].low = h[i].low; t[m].close = h[i].close;
         key = g; m++;
      }
      else { t[m - 1].high = MathMax(t[m - 1].high, h[i].high); t[m - 1].low = MathMin(t[m - 1].low, h[i].low); t[m - 1].close = h[i].close; }
   }
   int s0 = MathMax(0, m - count);
   ArrayResize(g_r, m - s0);
   for(int i = s0; i < m; i++) g_r[i - s0] = t[i];
   return m - s0;
}

//--------------------------------------------------------------------- calendar helpers (turn-of-month, weekday based)
int DaysInMonth(const int y, const int m)
{
   if(m == 2) return ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) ? 29 : 28;
   if(m == 4 || m == 6 || m == 9 || m == 11) return 30;
   return 31;
}
int WeekdaysLeftAfter(const datetime t)   // weekdays strictly after t's day until the month end
{
   MqlDateTime d; TimeToStruct(t, d);
   long dn = DayNum(t); int c = 0;
   for(int x = d.day + 1; x <= DaysInMonth(d.year, d.mon); x++) if(IsWeekday(dn + (x - d.day))) c++;
   return c;
}
int WeekdayRank(const datetime t)         // 1 = first weekday of the month
{
   MqlDateTime d; TimeToStruct(t, d);
   long dn = DayNum(t); int c = 0;
   for(int x = 1; x <= d.day; x++) if(IsWeekday(dn - (d.day - x))) c++;
   return c;
}

//--------------------------------------------------------------------- per-bar logic
bool WeekendZone()        // Friday from 15:00 New York (trading-day clock hour 22) to the close
{
   int cr = g_cur_role; g_cur_role = -1;
   datetime td = TD(TimeCurrent());
   g_cur_role = cr;
   return DowOf(DayNum(td)) == 5 && HourOf(td) >= 22;
}
// v13.3: partial close of every position of sleeve k (which = 1 first partial, 2 second); returns true when a position was too
// small to split (first partial: the caller locks +0.5R instead).  A 10018 reply re-queues it quietly.
bool DoPartial(const int k, const int which)
{
   int role = SL_ROLE[k];
   string sym = g_sym[role];
   bool cant = false;
   double fr = (which == 1 ? SL_PFR[k] : SL_PF2[k]);
   for(int i = PositionsTotal() - 1; i >= 0; i--)
   {
      ulong t = PositionGetTicket(i);
      if(t == 0 || PositionGetInteger(POSITION_MAGIC) != SleeveMagic(k)) continue;
      double v = PositionGetDouble(POSITION_VOLUME), vst = SymbolInfoDouble(sym, SYMBOL_VOLUME_STEP);
      double cv = MathFloor(v * fr / vst + 1e-9) * vst;
      if(cv >= SymbolInfoDouble(sym, SYMBOL_VOLUME_MIN) && cv < v)
      {
         g_trade.SetTypeFillingBySymbol(sym);
         if(!g_trade.PositionClosePartial(t, cv) && g_trade.ResultRetcode() == 10018) { MarkClosed(role); g_pt_pend[k] |= which; break; }
      }
      else if(which == 1) cant = true;
   }
   return cant;
}
// v12: extra trade management on a closed bar; returns true when the sleeve was closed
bool MgmtExtra(const int k, const int role, const double hi, const double lo, double &st)
{
   int dir = g_ss[k].dir; double r0 = g_ss[k].r0, ep = g_ss[k].ep;
   if(dir == 0 || r0 <= 0) return false;
   if(SL_WKD[k] > 0 && WeekendZone()) { CloseSleeve(k, "weekend flat"); return true; }
   double ext = (dir > 0 ? hi : lo);
   if(g_bx[k] == 0 || dir * (ext - g_bx[k]) > 0) g_bx[k] = ext;
   double mfe = dir * (g_bx[k] - ep) / r0;
   string sym = g_sym[role];
   double spr = SymbolInfoDouble(sym, SYMBOL_ASK) - SymbolInfoDouble(sym, SYMBOL_BID);
   if(SL_STB[k] > 0 && mfe < SL_STR[k] && iBarShift(sym, PERIOD_H1, g_ss[k].t0, false) >= (int)SL_STB[k]) { CloseSleeve(k, "stale"); return true; }
   double ns = 0;
   if(SL_BER[k] > 0 && mfe >= SL_BER[k]) ns = ep + dir * (spr + SL_BEO[k] * r0);
   if(SL_LKA[k] > 0 && mfe >= SL_LKA[k]) { double x = ep + dir * SL_LKB[k] * r0; if(ns == 0 || dir * (x - ns) > 0) ns = x; }
   if(SL_RTR[k] > 0 && mfe >= SL_RTR[k]) { double x = g_bx[k] - dir * SL_RTR[k] * r0; if(ns == 0 || dir * (x - ns) > 0) ns = x; }
   if(ns != 0 && (st == 0 || dir * (ns - st) > 0)) st = ns;
   if(SL_PTR[k] > 0 && !g_pdone[k] && mfe >= SL_PTR[k])
   {
      g_pdone[k] = true;
      if(!MarketOpen(role)) { g_pt_pend[k] |= 1; g_q_cnt[7]++; }      // v13.3: closed - partial queued for the reopen
      else if(DoPartial(k, 1)) { double x = ep + dir * 0.5 * r0; if(st == 0 || dir * (x - st) > 0) st = x; }   // v13.1: position too small to split - lock +0.5R instead
   }
   if(SL_PT2[k] > 0 && !g_pdone2[k] && mfe >= SL_PT2[k])     // v13.1: second partial
   {
      g_pdone2[k] = true;
      if(!MarketOpen(role)) { g_pt_pend[k] |= 2; g_q_cnt[7]++; }
      else DoPartial(k, 2);
   }
   return false;
}

// v13.3: a pyramid add queued while the market was closed, sent at the reopen while price is within 0.25 daily ATR of the add
// signal and the shared stop is still on the right side of the price; otherwise it is dropped once (LogSkip)
void QueuedAdd(const int k)
{
   int n = g_ad_pend[k], dir = g_ss[k].dir, role = SL_ROLE[k];
   string sym = g_sym[role], why;
   if(dir == 0) { g_ad_pend[k] = 0; return; }
   double px = (dir > 0 ? SymbolInfoDouble(sym, SYMBOL_ASK) : SymbolInfoDouble(sym, SYMBOL_BID)), atr = g_df[role].atr, st = g_ad_st[k];
   double minstop = (SymbolInfoInteger(sym, SYMBOL_TRADE_STOPS_LEVEL) + 1) * SymbolInfoDouble(sym, SYMBOL_POINT);
   if(px <= 0 || atr <= 0 || MathAbs(px - g_ad_px[k]) > 0.25 * atr)
   { g_ad_pend[k] = 0; g_q_cnt[6]++; LogSkip(k, dir, StringFormat("pyramid add %d: market was closed, then price moved %.2f ATR", n, atr > 0 ? MathAbs(px - g_ad_px[k]) / atr : 0)); return; }
   if((px - st) * dir <= minstop) { g_ad_pend[k] = 0; g_q_cnt[6]++; LogSkip(k, dir, StringFormat("pyramid add %d: market was closed, stop no longer valid", n)); return; }
   double add_dist = MathAbs(px - st), addl = g_ad_lots[k];
   g_last_rc = 0;
   if(EntryAllowed(why) && addl * add_dist * MoneyPerPriceUnit(sym) <= InpMaxRiskPerTradeR * BaseRiskPct() / 100.0 * g_init_bal
      && OpenTrade(k, dir, add_dist, addl, " add" + IntegerToString(n)))
   {
      g_ad_pend[k] = 0; g_q_cnt[5]++;
      SetSleeveStop(k, st);
      PrintFormat("v13 %s: pyramid add %d sent at the market reopen", SL_NAME[k], n);
      return;
   }
   if(g_last_rc == 10018) return;            // still closed (broker): kept
   g_ad_pend[k] = 0; g_q_cnt[6]++;
   LogSkip(k, dir, StringFormat("pyramid add %d: %s", n, why != "" ? why : g_ss[k].status));
}

void ManageOpen(const int k, const int role, const double c1, const datetime bar_open)
{
   double lots, pnl;
   if(SleevePositions(k, lots, pnl) == 0)
   {
      if(g_ss[k].dir != 0) { g_ss[k].dir = 0; g_ss[k].adds = 0; SaveSleeve(k); }
      return;
   }
   int dir = g_ss[k].dir;
   DayFeat f = g_df[role];
   // time stop (bars since entry, the entry bar counts as 1)
   if(SL_MAXB[k] > 0)
   {
      int sh = iBarShift(g_sym[role], PERIOD_H1, g_ss[k].t0, false);
      if(sh >= SL_MAXB[k]) { CloseSleeve(k, "time stop"); return; }
   }
   // trailing / break-even / pyramid on the closed bar
   if(dir * (c1 - g_ss[k].best) > 0) g_ss[k].best = c1;
   double st = 0;
   if(SL_TRK[k] > 0) st = g_ss[k].best - dir * SL_TRK[k] * f.atr;
   if(MgmtExtra(k, role, iHigh(g_sym[role], PERIOD_H1, 1), iLow(g_sym[role], PERIOD_H1, 1), st)) return;
   if(SL_BEK[k] > 0 && g_ss[k].r0 > 0)     // fixed-target sleeve: stop to entry once price reached entry + BEK*ATR (= BEK/SLK R)
   {
      double ext = (dir > 0 ? iHigh(g_sym[role], PERIOD_H1, 1) : iLow(g_sym[role], PERIOD_H1, 1));
      if(dir * (ext - g_ss[k].ep) >= SL_BEK[k] / SL_SLK[k] * g_ss[k].r0) st = g_ss[k].ep;
   }
   if(SL_ADDS[k] > 0 && g_ss[k].adds < SL_ADDS[k] && dir * (c1 - g_ss[k].ep) >= (g_ss[k].adds + 1) * g_ss[k].r0)
   {
      string sym = g_sym[role], why;
      double vst = SymbolInfoDouble(sym, SYMBOL_VOLUME_STEP), vmin = SymbolInfoDouble(sym, SYMBOL_VOLUME_MIN);
      double addl = MathFloor(g_ss[k].base_lots * 0.5 / vst + 1e-9) * vst;
      if(addl < vmin) addl = vmin;
      double be = g_ss[k].ep + dir * (SymbolInfoDouble(sym, SYMBOL_ASK) - SymbolInfoDouble(sym, SYMBOL_BID));
      if(st == 0 || dir * (be - st) > 0) st = be;
      SetSleeveStop(k, st);
      double px = (dir > 0 ? SymbolInfoDouble(sym, SYMBOL_ASK) : SymbolInfoDouble(sym, SYMBOL_BID));
      double add_dist = MathAbs(px - st);
      g_last_rc = 0;
      if(!MarketOpen(role)) g_last_rc = 10018;      // v13.3: closed - the add is queued below, nothing is sent
      else if(EntryAllowed(why) && add_dist > 0 && addl * add_dist * MoneyPerPriceUnit(sym) <= InpMaxRiskPerTradeR * BaseRiskPct() / 100.0 * g_init_bal
         && OpenTrade(k, dir, add_dist, addl, " add" + IntegerToString(g_ss[k].adds + 1)))
      {
         SetSleeveStop(k, st);        // the add gets the shared stop
      }
      if(g_last_rc == 10018)          // v13.3: market closed (gate or broker): keep the add for the reopen
      {
         g_ad_pend[k] = g_ss[k].adds + 1; g_ad_px[k] = px; g_ad_st[k] = st; g_ad_lots[k] = addl; g_q_cnt[4]++;
      }
      g_ss[k].adds++;                 // counted even when skipped: never chase the same level twice
   }
   if(st != 0) SetSleeveStop(k, st);
   SaveSleeve(k);
}

void RefreshNyAlign()        // v12: the server clock can change its offset during the year (DST); re-checked every hour
{
   int cr = g_cur_role;
   datetime now = (IsTester() ? TimeCurrent() : TimeTradeServer());
   g_cur_role = -1; g_nyalign = (DayShiftSec(now) == 0);
   for(int r = 0; r < NROLE; r++)
   {
      if(!g_tzr_ok[r]) { g_nyal_r[r] = g_nyalign; continue; }
      g_cur_role = r; g_nyal_r[r] = (DayShiftSec(now) == 0);
   }
   g_cur_role = cr;
}

void OnNewH1(const int role, const datetime bar_open)
{
   static datetime s_na = 0;
   if(TimeCurrent() - s_na >= 1800) { s_na = TimeCurrent(); RefreshNyAlign(); }
   g_cur_role = role;
   string sym = g_sym[role];
   // closed bars: shift 1 (just closed) and older
   datetime t1 = iTime(sym, PERIOD_H1, 1), t2 = iTime(sym, PERIOD_H1, 2);
   if(t1 == 0 || t2 == 0) return;
   // first closed bar of a new server day.  Uninterrupted this equals DayNum(t1) != DayNum(t2) (the previous processed bar is t2);
   // after a terminal restart across the day start the daily signals are still taken, if no more than 4 hours late (v11.8)
   // days and hours on the trading-day clock (= server clock on NY-aligned servers)
   datetime prev = g_last_c1[role], d1 = TD(t1), d2 = TD(t2);
   bool new_day = (prev > 0 ? DayNum(d1) != DayNum(TD(prev)) : DayNum(d1) != DayNum(d2));
   bool first_of_day = new_day && (DayNum(d1) != DayNum(d2) || HourOf(d1) <= 3);
   if(new_day && !first_of_day) PrintFormat("v13 %s: daily signals of today skipped (EA was not running at the day start)", sym);
   if(new_day && first_of_day && DayNum(d1) == DayNum(d2)) PrintFormat("v13 %s: daily signals caught up after a restart", sym);
   g_last_c1[role] = t1; GvSet("lc" + IntegerToString(role), (double)t1); g_last_scan = TimeCurrent();
   if(new_day || !g_df[role].ok || DayNum(g_df[role].day) < DayNum(d1) - 5) CalcDayFeatures(role);
   if(!g_df[role].ok) return;
   int need = 3;
   for(int k = 0; k < NSLV; k++) if(SL_ROLE[k] == role && g_use[k])
   {
      if(SL_KIND[k] == 0) need = MathMax(need, SL_N[k] + 3);
      if(SL_KIND[k] == 1) need = MathMax(need, 1500);
   }
   int n = CopyRates(sym, PERIOD_H1, 1, need, g_r);
   if(n < 3) return;
   double c1 = g_r[n - 1].close, c2 = g_r[n - 2].close;
   datetime tb = TD(bar_open);
   int hr = HourOf(tb);
   bool hour_ok = (hr != 23 && hr != 0);
   DayFeat f = g_df[role];
   datetime h4 = (NyAligned() ? iTime(sym, PERIOD_H4, 0) : (datetime)((long)tb / 14400 * 14400));
   bool new_h4 = (h4 != 0 && h4 != g_last_h4[role]);
   if(new_h4) g_last_h4[role] = h4;
   for(int k = 0; k < NSLV; k++)
   {
      if(SL_ROLE[k] != role) continue;
      if((SL_KIND[k] >= 8 && SL_KIND[k] <= 10) || SL_KIND[k] == 14 || SL_KIND[k] >= 15) continue;    // v13.4: + v15 timer kinds; Gotobi / 3rd Friday / fix reversal / gap: handled by their timers
      if(SL_KIND[k] == 6 || SL_KIND[k] == 7)   // H4 sleeves
      {
         if(new_h4) OnNewH4Sleeve(k, role, h4);
         continue;
      }
      double lots, pnl;
      int open = SleevePositions(k, lots, pnl);
      if(open > 0 || g_ss[k].dir != 0) ManageOpen(k, role, c1, bar_open);
      open = SleevePositions(k, lots, pnl);
      if(!g_use[k]) { if(open == 0) g_ss[k].status = "off"; continue; }
      int kind = SL_KIND[k];
      int sig = 0, xsig = 0;
      if(kind == 0 && n >= SL_N[k] + 3)
      {
         int N = SL_N[k];
         double hh1 = -1e300, ll1 = 1e300, hh2 = -1e300, ll2 = 1e300;
         for(int i = n - 1 - N; i <= n - 2; i++) { hh1 = MathMax(hh1, g_r[i].high); ll1 = MathMin(ll1, g_r[i].low); }
         for(int i = n - 2 - N; i <= n - 3; i++) { hh2 = MathMax(hh2, g_r[i].high); ll2 = MathMin(ll2, g_r[i].low); }
         if(c1 > hh1 && c2 <= hh2) sig = 1;
         else if(c1 < ll1 && c2 >= ll2) sig = -1;
         if(!hour_ok) sig = 0;
      }
      else if(kind == 1 && n >= 300)
      {
         double ef = g_r[0].close, es = g_r[0].close, ef2 = 0, es2 = 0;
         for(int i = 1; i < n; i++)
         {
            ef2 = ef; es2 = es;
            ef = ef + (g_r[i].close - ef) * 2.0 / ((SL_X[k] > 0 ? SL_X[k] : 20) + 1.0); es = es + (g_r[i].close - es) * 2.0 / ((SL_N[k] > 0 ? SL_N[k] : 100) + 1.0);
         }
         bool up1 = ef > es, up2 = ef2 > es2;
         if(up1 && !up2) sig = 1; else if(!up1 && up2) sig = -1;
         if(!hour_ok) sig = 0;
      }
      else if(kind == 2 && first_of_day)
      {
         double rr = f.ret120;
         if(SL_N[k] > 0 && SL_N[k] != 120 && g_d1n[role] > SL_N[k] + 1) { int b = role * 1000 + g_d1n[role] - 1; rr = g_d1c[b] / g_d1c[b - SL_N[k]] - 1.0; }
         if(rr > 0) sig = 1; else if(rr < 0) sig = -1;
         if(rr < 0) xsig = 1; else if(rr > 0) xsig = -1;
      }
      else if(kind == 3 && hr == 2 && IsWeekday(DayNum(tb)))
      {
         if(WeekdaysLeftAfter(tb) == (SL_N[k] > 0 ? SL_N[k] : 1)) sig = 1;
         int xr = (SL_X[k] > 0 ? (int)SL_X[k] : 4);
         if(SL_X[k] < 0 ? (WeekdayRank(tb) >= -(int)SL_X[k] && WeekdayRank(tb) < 12) : WeekdayRank(tb) == xr) xsig = 2;
      }
      else if(kind == 4 && first_of_day)
      {
         if(f.rsi2 < SL_P[k]) sig = 1;
         double sm = f.sma5;
         if(SL_X[k] > 0 && (int)SL_X[k] != 5 && g_d1n[role] > (int)SL_X[k]) { int L = (int)SL_X[k], b = role * 1000 + g_d1n[role]; sm = 0; for(int i = b - L; i < b; i++) sm += g_d1c[i]; sm /= L; }
         if(f.close > sm) xsig = 1;
      }
      else if(kind == 5 && first_of_day)
      {
         if(f.ibs < SL_P[k]) sig = 1;
         if(f.ibs > (SL_X[k] > 0 ? SL_X[k] : 0.80)) xsig = 1;
      }
      else if(kind == 11 && hr == 2)                   // two hours after the server day starts (rollover spreads gone)
      {
         double zz = f.z50, sdd = f.sd50;
         if(SL_N[k] > 0 && SL_N[k] != 50 && g_d1n[role] > SL_N[k]) { int L = SL_N[k], b = role * 1000 + g_d1n[role]; double mm = 0, qq = 0; for(int i = b - L; i < b; i++) mm += MathLog(g_d1c[i]); mm /= L; for(int i = b - L; i < b; i++) qq += MathPow(MathLog(g_d1c[i]) - mm, 2); sdd = MathSqrt(qq / (L - 1)); zz = (sdd > 0 ? (MathLog(f.close) - mm) / sdd : 0); }
         if(zz >= SL_P[k]) sig = -1; else if(zz <= -SL_P[k]) sig = 1;
         xsig = (zz >= 0 ? 1 : -1);                 // long exits once back at/above the mean, short at/below
         if(sig != 0 && SL_N[k] != 50) { if(open == 0 && !(SL_SIDE[k] == 1 && sig < 0) && !NewsBlocked()) TryEnter(k, sig, SL_SLK[k] * sdd * f.close); sig = 0; }
      }
      bool exited = false;
      bool news = NewsBlocked();
      if(open > 0)
      {
         if(xsig == 2 || xsig == g_ss[k].dir)
         {
            if(news) { g_pend_act[k] = 2; g_pend_t[k] = bar_open; g_ss[k].status = "news"; continue; }
            CloseSleeve(k, "exit signal"); open = 0; exited = true;
         }
      }
      if(open == 0 && !exited && sig != 0 && news && !(SL_SIDE[k] == 1 && sig < 0))
      { g_pend_act[k] = 1; g_pend_dir[k] = sig; g_pend_t[k] = bar_open; g_ss[k].status = "news"; continue; }
      if(open == 0 && !exited && sig != 0 && !(SL_SIDE[k] == 1 && sig < 0)) TryEnter(k, sig);
      else if(open == 0 && g_ss[k].status == "") g_ss[k].status = "wait";
   }
}

void OnNewH4Sleeve(const int k, const int role, const datetime bar_open)
{
   g_cur_role = role;
   string sym = g_sym[role];
   double lots, pnl;
   int open = SleevePositions(k, lots, pnl);
   if(open == 0 && g_ss[k].dir != 0) { g_ss[k].dir = 0; g_ss[k].adds = 0; SaveSleeve(k); }
   if(!g_use[k]) { if(open == 0) g_ss[k].status = "off"; return; }
   int n = GetH4(sym, 800);                                  // closed H4 bars, oldest first (in g_r)
   if(n < 260) { g_ss[k].status = "wait"; return; }
   // Wilder ATR14 and EMA200 over the window, plus the values one bar earlier
   double atr = 0, ema = g_r[0].close, ema_10 = 0;
   for(int i = 1; i < n; i++)
   {
      double tr = MathMax(g_r[i].high - g_r[i].low, MathMax(MathAbs(g_r[i].high - g_r[i-1].close), MathAbs(g_r[i].low - g_r[i-1].close)));
      atr = (i <= 14 ? atr + tr / 14.0 : (atr * 13.0 + tr) / 14.0);
      ema = ema + (g_r[i].close - ema) * 2.0 / 201.0;
      if(i == n - 11) ema_10 = ema;
   }
   double c1 = g_r[n-1].close, c2 = g_r[n-2].close;
   int sig = 0, xsig = 0;
   if(SL_KIND[k] == 6)
   {
      int N = SL_N[k]; double z = SL_P[k];
      double m1 = 0, q1 = 0, m2 = 0, q2 = 0;
      for(int i = n - N; i < n; i++) { m1 += g_r[i].close; q1 += g_r[i].close * g_r[i].close; }
      for(int i = n - N - 1; i < n - 1; i++) { m2 += g_r[i].close; q2 += g_r[i].close * g_r[i].close; }
      m1 /= N; m2 /= N;
      double sd1 = MathSqrt(MathMax(q1 / N - m1 * m1, 0)), sd2 = MathSqrt(MathMax(q2 / N - m2 * m2, 0));
      if(c1 > m1 + z * sd1 && c2 <= m2 + z * sd2 && c1 > ema) sig = 1;
      else if(c1 < m1 - z * sd1 && c2 >= m2 - z * sd2 && c1 < ema) sig = -1;
      if(c1 < m1) xsig = 1; else if(c1 > m1) xsig = -1;
   }
   else
   {
      int N = SL_N[k], X = (int)SL_P[k];
      double hh = -1e300, ll = 1e300, xl = 1e300, xh = -1e300;
      for(int i = n - 1 - N; i <= n - 2; i++) { hh = MathMax(hh, g_r[i].high); ll = MathMin(ll, g_r[i].low); }
      for(int i = n - 1 - X; i <= n - 2; i++) { xl = MathMin(xl, g_r[i].low); xh = MathMax(xh, g_r[i].high); }
      if(c1 > ema && ema - ema_10 > 0 && c1 > hh) sig = 1;
      else if(c1 < ema && ema - ema_10 < 0 && c1 < ll) sig = -1;
      if(c1 < xl) xsig = 1; else if(c1 > xh) xsig = -1;
   }
   if(open > 0 && SL_MAXB[k] > 0 && iBarShift(sym, PERIOD_H1, g_ss[k].t0, false) >= SL_MAXB[k]) { CloseSleeve(k, "time stop"); return; }
   if(open > 0)
   {
      double st = 0;
      if(SL_TRK[k] > 0)
      {
         if(g_ss[k].dir * (c1 - g_ss[k].best) > 0) g_ss[k].best = c1;
         st = g_ss[k].best - g_ss[k].dir * SL_TRK[k] * atr;
      }
      if(MgmtExtra(k, role, g_r[n-1].high, g_r[n-1].low, st)) return;
      if(st != 0) SetSleeveStop(k, st);
      SaveSleeve(k);
   }
   int hr = HourOf(bar_open);
   if(hr == 23 || hr == 0) sig = 0;                          // rollover hours: no new entries
   bool exited = false;
   if(open > 0 && (xsig == g_ss[k].dir))
   {
      if(NewsBlocked()) { g_pend_act[k] = 2; g_pend_t[k] = TimeCurrent(); g_ss[k].status = "news"; return; }
      CloseSleeve(k, "exit signal (H4)"); open = 0; exited = true;
   }
   if(open == 0 && !exited && sig != 0 && !(SL_SIDE[k] == 1 && sig < 0))
   {
      if(NewsBlocked()) { g_pend_act[k] = 1; g_pend_dir[k] = sig; g_pend_t[k] = TimeCurrent(); g_ss[k].status = "news"; return; }
      TryEnter(k, sig, SL_SLK[k] * atr);
   }
   else if(open == 0 && g_ss[k].status == "") g_ss[k].status = "wait";
}


//--------------------------------------------------------------------- Gotobi / Tokyo-fix sleeve (USDJPY, calendar + clock, no indicators)
// Ito & Yamada (JIE 2017): importer USD demand into the 09:55 JST Tokyo fix on Gotobi days (5,10,15,20,25,30; weekend -> Friday).
// Rule frozen from an independent 2022-25 replication; verified here on EURJPY 2005-2018 and USDJPY 2017-2025 (never used to fit it).
// Long 15:55 UTC on the previous weekday, exit 00:55 UTC (the fix), stop InpGotobiStopPips. Trades that would be held over a weekend are skipped.
int ServerUTCOffsetSec(const datetime srv)
{
   if(InpServerGMTOffset != -99) return InpServerGMTOffset * 3600;
   if(!IsTester())
   {
      long d = (long)(TimeTradeServer() - TimeGMT());
      return (int)(MathRound(d / 1800.0) * 1800);
   }
   // tester: EET with US daylight saving (FundedNext and most prop brokers): UTC+3 between the US DST dates, else UTC+2,
   // unless DEMO mode read another time zone from the data (v11.9)
   static datetime s_y0 = 0, s_y1 = 0, s_a = 0, s_b = 0;      // v12: cached per calendar year
   if(srv < s_y0 || srv >= s_y1)
   {
      MqlDateTime m; TimeToStruct(srv, m);
      s_y0 = StringToTime(StringFormat("%04d.01.01", m.year)); s_y1 = StringToTime(StringFormat("%04d.01.01", m.year + 1));
      datetime mar1 = StringToTime(StringFormat("%04d.03.01", m.year)), nov1 = StringToTime(StringFormat("%04d.11.01", m.year));
      long dm = DayNum(mar1), dn = DayNum(nov1);
      long s2 = dm + ((7 - DowOf(dm)) % 7) + 7, s1 = dn + ((7 - DowOf(dn)) % 7);   // 2nd Sunday of March, 1st Sunday of November
      s_a = (datetime)(s2 * 86400 + 9 * 3600); s_b = (datetime)(s1 * 86400 + 9 * 3600);   // 2am New York, in server time
   }
   bool us_summer = (srv >= s_a && srv < s_b);
   if(g_cur_role >= 0 && g_cur_role < NROLE && g_tzr_ok[g_cur_role]) return (us_summer ? g_tzr_s[g_cur_role] : g_tzr_w[g_cur_role]) * 3600;
   if(g_tz_ok) return (us_summer ? g_tz_s : g_tz_w) * 3600;
   return us_summer ? 3 * 3600 : 2 * 3600;
}

bool DemoActive() { return InpDemoMode && IsTester(); }

// DEMO: UTC offset of the tester data, read from the US cash open: the 15-minute bar that starts at 09:30 New York has by far the
// largest average range of the day on the NASDAQ / DOW index CFDs (gold as a last resort: its peak is the 08:30 NY data bar).
// Measured separately for US summer and winter time over the ~2 years before the test start; offsets are rounded to whole hours.
// spike method for one symbol: indices peak at 09:30 NY, metals at 08:30 NY (15-minute bars, Tue-Thu, last ~2 years)
bool SpikeTZ(const int role, int &os, int &ow)
{
   MArr<MqlRates> r;
   int n = CopyRates(g_sym[role], PERIOD_M15, 0, 96 * 520, r);
   if(n < 96 * 100) return false;
   double rng[2][96]; int cnt[2][96];
   for(int z = 0; z < 2; z++) for(int k = 0; k < 96; k++) { rng[z][k] = 0; cnt[z][k] = 0; }
   for(int i = 0; i < n; i++)
   {
      int dw = DowOf(DayNum(r[i].time));
      if(dw < 2 || dw > 4) continue;
      int z = (NYOffsetSec(r[i].time) == -4 * 3600 ? 0 : 1);
      int k = (int)((r[i].time % 86400) / 900);
      rng[z][k] += r[i].high - r[i].low; cnt[z][k]++;
   }
   int off[2] = {99, 99};
   int ref = (role <= 1 ? 8 * 60 + 30 : 9 * 60 + 30);
   for(int z = 0; z < 2; z++)
   {
      double av[96]; int m = 0; double srt[96];
      for(int k = 0; k < 96; k++) { av[k] = (cnt[z][k] >= 20 ? rng[z][k] / cnt[z][k] : 0); if(av[k] > 0) srt[m++] = av[k]; }
      if(m < 40) continue;
      int hi = 0; for(int k = 1; k < 96; k++) if(av[k] > av[hi]) hi = k;
      for(int i = 1; i < m; i++) { double v = srt[i]; int j = i - 1; while(j >= 0 && srt[j] > v) { srt[j + 1] = srt[j]; j--; } srt[j + 1] = v; }
      if(av[hi] < 1.6 * srt[m / 2]) continue;
      off[z] = NormHour((int)MathRound((hi * 15 - (ref + (z == 0 ? 4 : 5) * 60)) / 60.0));
   }
   if(off[0] == 99 && off[1] == 99) return false;
   if(off[0] == 99) off[0] = off[1];
   if(off[1] == 99) off[1] = off[0];
   if(MathAbs(off[0] - off[1]) > 2) return false;
   os = off[0]; ow = off[1]; return true;
}
// weekly-open method (FX): the first bar after the weekend opens at 17:00 New York (Sunday) in practically every FX feed
bool WeekOpenTZ(const int role, int &os, int &ow)
{
   MArr<MqlRates> r;
   int n = CopyRates(g_sym[role], PERIOD_H1, 0, 24 * 520, r);
   if(n < 24 * 100) return false;
   int cs[25], cw[25]; for(int i = 0; i < 25; i++) { cs[i] = 0; cw[i] = 0; }
   int ns = 0, nw = 0;
   for(int i = 1; i < n; i++)
   {
      if(r[i].time - r[i - 1].time < 36 * 3600) continue;               // not after a weekend
      bool sum = (NYOffsetSec(r[i].time) == -4 * 3600);
      int o = NormHour(HourOf(r[i].time) - (sum ? 21 : 22)) + 12;       // 17:00 NY = 21:00 / 22:00 UTC
      if(o < 0 || o > 24) continue;
      if(sum) { cs[o]++; ns++; } else { cw[o]++; nw++; }
   }
   if(ns + nw < 20) return false;
   int bs = 0, bw = 0;
   for(int i = 1; i < 25; i++) { if(cs[i] > cs[bs]) bs = i; if(cw[i] > cw[bw]) bw = i; }
   if(ns < 8) bs = bw; if(nw < 8) bw = bs;
   if((ns >= 8 && cs[bs] < 0.6 * ns) || (nw >= 8 && cw[bw] < 0.6 * nw)) return false;   // no dominant open hour
   os = bs - 12; ow = bw - 12; return MathAbs(os - ow) <= 2;
}
// v13.2: ONE server clock for every symbol.  v13.1 gave each symbol its own clock when the readings differed, and the readings of
// metals and indices are not reliable: the spike method assumes the metals' busiest 15 minutes are the 08:30 New York data bar, but
// on MetaQuotes-Demo gold and silver peak at the 09:30 cash open (and 10:00 data), which reads one hour too far east (UTC+4/+3
// against UTC+3/+2 for every FX pair); index CFDs with a short history or a daily session break can read a 2-hour summer/winter
// gap (SPX500 UTC+4/+2 on FundedNext).  A broker has one clock, so a split only moved the metal / index sleeves by an hour.
// Choice: 1) the FX weekly open (17:00 New York Sunday open, the most reliable mark), majority of the FX pairs; 2) otherwise a
// majority vote of all symbols whose summer and winter offsets differ by at most 1 hour; 3) otherwise a vote of all readings.
// Ties go to the preferred symbol (USDJPY, AUDJPY, EURGBP, NASDAQ, DOW, S&P, gold, silver).
bool ChooseClock(int &cs, int &cw, string &how)
{
   int pref[NROLE] = {5, 4, 6, 3, 2, 7, 0, 1};
   for(int pass = 0; pass < 3; pass++)
   {
      int best = -1, bv = 0;
      for(int q = 0; q < NROLE; q++)
      {
         int r = pref[q];
         if(g_tzc_m[r] == 0) continue;
         if(pass == 0 && !(g_tzc_m[r] == 2 && r >= 4 && r <= 6)) continue;
         if(pass == 1 && MathAbs(g_tzc_s[r] - g_tzc_w[r]) > 1) continue;
         int v = 0;
         for(int x = 0; x < NROLE; x++)
         {
            if(g_tzc_m[x] == 0 || g_tzc_s[x] != g_tzc_s[r] || g_tzc_w[x] != g_tzc_w[r]) continue;
            if(pass == 0 && !(g_tzc_m[x] == 2 && x >= 4 && x <= 6)) continue;
            if(pass == 1 && MathAbs(g_tzc_s[x] - g_tzc_w[x]) > 1) continue;
            v++;
         }
         if(v > bv) { bv = v; best = r; }
      }
      if(best >= 0)
      {
         cs = g_tzc_s[best]; cw = g_tzc_w[best];
         how = (pass == 0 ? "FX weekly open" : pass == 1 ? "vote of the symbols" : "vote of all readings") + StringFormat(", %d symbol(s) agree", bv);
         return true;
      }
   }
   return false;
}
bool DetectDataTZ(const bool broker)
{
   int nok = 0;
   for(int r = 0; r < NROLE; r++)
   {
      g_tzc_m[r] = 0; g_tzc_s[r] = 0; g_tzc_w[r] = 0; g_tzr_ok[r] = false;
      if(!g_sym_ok[r] || (!broker && SymbolInfoInteger(g_sym[r], SYMBOL_CUSTOM) == 0)) continue;   // v12: broker symbols keep the broker clock (v13.1: unless broker = true)
      int a = 0, b = 0, m = 0;
      if(r <= 3 || r == 7) { if(SpikeTZ(r, a, b)) m = 1; }  // metals and indices: US-open spike
      if(m == 0 && WeekOpenTZ(r, a, b)) m = 2;
      if((r <= 3 || r == 7) && m == 0) continue;
      if(m > 0) { g_tzc_s[r] = a; g_tzc_w[r] = b; g_tzc_m[r] = m; nok++; if(InpDemoMode) PrintFormat("v13 DEMO: %s clock UTC%+d (US summer) / UTC%+d (US winter)", g_sym[r], a, b); }
      else if(InpDemoMode) PrintFormat("v13 DEMO: %s clock not readable", g_sym[r]);   // v13.6: helper lines only in DEMO mode
   }
   if(nok == 0) return false;
   string how = "";
   if(!ChooseClock(g_tz_s, g_tz_w, how)) return false;
   if(InpDemoMode) PrintFormat("v13.2: one server clock for all symbols: UTC%+d (US summer) / UTC%+d (US winter) - %s", g_tz_s, g_tz_w, how);
   for(int r = 0; r < NROLE; r++)
      if(InpDemoMode && g_tzc_m[r] != 0 && (g_tzc_s[r] != g_tz_s || g_tzc_w[r] != g_tz_w))
         PrintFormat("v13.2: %s bars read UTC%+d / UTC%+d - ignored (session gaps and news bars mislead the reading of metals and indices; set InpServerGMTOffset to force a clock)", g_sym[r], g_tzc_s[r], g_tzc_w[r]);
   return true;
}
int NormHour(const int h) { int x = ((h % 24) + 24) % 24; return x > 12 ? x - 24 : x; }

// DEMO: custom symbols (Custom Symbols with your own history) beat the broker's symbols; among several (e.g. -back / -back2)
// the one with the longest history wins.
int DemoBonus(const string s)
{
   if(!DemoActive() || SymbolInfoInteger(s, SYMBOL_CUSTOM) == 0) return 0;
   datetime fd = (datetime)SeriesInfoInteger(s, PERIOD_H1, SERIES_TERMINAL_FIRSTDATE);
   double yrs = (fd > 0 ? (double)(TimeCurrent() - fd) / (365.25 * 86400.0) : 0);
   return 5000 + (int)MathMin(1000.0, MathMax(0.0, yrs) * 40.0);
}
bool IsGotobiDay(const long dn)          // dn = UTC day number of a weekday
{
   MqlDateTime d; TimeToStruct((datetime)(dn * 86400), d);
   if(d.day % 5 == 0 && d.day <= 30) return true;
   if(DowOf(dn) == 5)                     // Friday also carries a Gotobi date that falls on the weekend
      for(int x = 1; x <= 2; x++)
      {
         MqlDateTime w; TimeToStruct((datetime)((dn + x) * 86400), w);
         if(w.mon == d.mon && w.day % 5 == 0 && w.day <= 30) return true;
      }
   return false;
}
void GotobiTimer()
{
   g_cur_role = SL_ROLE[12];
   const int k = 12;
   int role = SL_ROLE[k];
   if(!g_sym_ok[role]) return;
   double lots, pnl;
   int open = SleevePositions(k, lots, pnl);
   datetime srv = TimeCurrent();
   if(open > 0)
   {
      if(g_gb_exit == 0) g_gb_exit = (datetime)GvGet("gb_exit", 0);
      if(g_gb_exit == 0 || srv >= g_gb_exit || srv - g_ss[k].t0 > 14 * 3600)
      { CloseSleeve(k, "Tokyo fix time exit"); g_gb_exit = 0; GvSet("gb_exit", 0); }
      return;
   }
   if(g_ss[k].dir != 0) { g_ss[k].dir = 0; SaveSleeve(k); }
   if(!g_use[k]) { g_ss[k].status = "off"; return; }
   datetime utc = (datetime)(srv - ServerUTCOffsetSec(srv));
   long dn = DayNum(utc); int mins = (int)((utc % 86400) / 60);
   if(mins < 15 * 60 + 55 || mins >= 16 * 60 + 25 || dn == g_gb_tried) { if(g_ss[k].status == "") g_ss[k].status = "wait"; return; }
   int dw = DowOf(dn);
   if(dw < 1 || dw > 4) return;           // Fri entry would hold over the weekend: skipped (EURJPY 2005-20: weekend-held trades lost)
   if(!IsGotobiDay(dn + 1)) return;
   g_gb_tried = dn;
   if(!g_df[role].ok) CalcDayFeatures(role);
   string sym = g_sym[role];
   double pt = SymbolInfoDouble(sym, SYMBOL_POINT); int dg = (int)SymbolInfoInteger(sym, SYMBOL_DIGITS);
   double pip = (dg == 3 || dg == 5) ? 10 * pt : pt;
   TryEnter(k, 1, InpGotobiStopPips * pip);
   if(SleevePositions(k, lots, pnl) > 0)
   {
      g_gb_exit = (datetime)((dn + 1) * 86400 + 55 * 60 + ServerUTCOffsetSec(srv));   // 00:55 UTC next day, in server time
      GvSet("gb_exit", (double)g_gb_exit);
   }
}

//--------------------------------------------------------------------- third-Friday price spike (NAS100, calendar + clock)
// Baltussen, Terstegge & Whelan, "The Third Friday Price Spike" (SSRN 4562800): index futures rise from the Thursday cash close
// into the monthly option expiry open (09:30 New York, third Friday) and give it back by noon.  Only the reversal leg is traded:
// short at 09:30 NY when the price is above the Thursday 16:00 NY reference, exit at 12:00 NY, hard stop InpTfpsStopPct.
// Verified here: NAS100 2005-2020 n=117 PF 2.74 t=4.0, fresh 2020.06-2025 n=53 PF 2.86 t=2.8 (the long overnight leg faded after 2021).
int NYOffsetSec(const datetime utc)       // New York: UTC-4 between 2nd Sunday of March and 1st Sunday of November (07:00/06:00 UTC), else UTC-5
{
   static datetime s_y0 = 0, s_y1 = 0, s_a = 0, s_b = 0;      // v12: DST dates cached per calendar year (TD() runs for every bar)
   if(utc < s_y0 || utc >= s_y1)
   {
      MqlDateTime m; TimeToStruct(utc, m);
      s_y0 = StringToTime(StringFormat("%04d.01.01", m.year)); s_y1 = StringToTime(StringFormat("%04d.01.01", m.year + 1));
      long dm = DayNum(StringToTime(StringFormat("%04d.03.01", m.year))), dn = DayNum(StringToTime(StringFormat("%04d.11.01", m.year)));
      long s2 = dm + ((7 - DowOf(dm)) % 7) + 7, s1 = dn + ((7 - DowOf(dn)) % 7);
      s_a = (datetime)(s2 * 86400 + 7 * 3600); s_b = (datetime)(s1 * 86400 + 6 * 3600);
   }
   return (utc >= s_a && utc < s_b) ? -4 * 3600 : -5 * 3600;
}
bool IsThirdFriday(const long dn)         // dn = New York day number
{
   MqlDateTime d; TimeToStruct((datetime)(dn * 86400), d);
   return DowOf(dn) == 5 && d.day >= 15 && d.day <= 21;
}
void ThirdFridayTimer()
{
   g_cur_role = SL_ROLE[13];
   const int k = 13;
   int role = SL_ROLE[k];
   if(!g_sym_ok[role]) return;
   double lots, pnl;
   int open = SleevePositions(k, lots, pnl);
   datetime srv = TimeCurrent();
   datetime utc = (datetime)(srv - ServerUTCOffsetSec(srv));
   datetime ny = (datetime)(utc + NYOffsetSec(utc));
   long dn = DayNum(ny); int mins = (int)((ny % 86400) / 60);
   if(open > 0)
   {
      if(mins >= (SL_X[k] > 0 ? (int)(SL_X[k] * 60) : 12 * 60) || srv - g_ss[k].t0 > 7 * 3600) CloseSleeve(k, "3rd Friday noon exit");
      return;
   }
   if(g_ss[k].dir != 0) { g_ss[k].dir = 0; SaveSleeve(k); }
   if(!g_use[k]) { g_ss[k].status = "off"; return; }
   if(g_ss[k].status == "") g_ss[k].status = "wait";
   string sym = g_sym[role];
   if(IsThirdFriday(dn + 1) && mins >= 15 * 60 + 59 && mins < 16 * 60 + 15 && g_tf_refday != dn)
   {
      double bid = SymbolInfoDouble(sym, SYMBOL_BID);
      if(bid > 0) { g_tf_ref = bid; g_tf_refday = dn; GvSet("tf_ref", bid); GvSet("tf_refday", (double)dn); }
      return;
   }
   if(!IsThirdFriday(dn) || mins < 9 * 60 + 30 || mins >= 9 * 60 + 45 || dn == g_tf_tried) return;
   g_tf_tried = dn;
   if(g_tf_refday != dn - 1) { g_tf_ref = GvGet("tf_ref", 0); g_tf_refday = (long)GvGet("tf_refday", -1); }
   if(g_tf_refday != dn - 1 || g_tf_ref <= 0) return;      // no Thursday reference (holiday or EA not running): skip
   double bid = SymbolInfoDouble(sym, SYMBOL_BID);
   if(bid <= g_tf_ref) return;                             // reversal only follows an up overnight move
   if(!g_df[role].ok) CalcDayFeatures(role);
   TryEnter(k, -1, bid * InpTfpsStopPct / 100.0);
}

//--------------------------------------------------------------------- NAS100 long after a large negative opening gap (v11.6)
// gap = ln(open 09:30 NY / close 16:00 NY of the previous session), from M30 bars; z vs the previous 60 gaps.  z <= -SL_P: buy at 09:30 NY,
// stop SL_SLK x daily ATR, target = previous 16:00 close, exit 11:00 NY.  Grant, Wolf & Yu (JBF 2005) opening reversals; verified here only on the
// long side: NAS100 2010-17 n=74 PF 1.62, 2018-26 n=83 PF 1.84, 2x cost PF 1.31/1.73, top-5 removed PF 1.45; long-every-day control PF 0.71/0.97.

void GapLongTimer()
{
   g_cur_role = SL_ROLE[19];
   const int k = 19;
   int role = SL_ROLE[k];
   if(!g_sym_ok[role]) return;
   double lots, pnl;
   int open = SleevePositions(k, lots, pnl);
   datetime srv = TimeCurrent();
   datetime utc = (datetime)(srv - ServerUTCOffsetSec(srv));
   datetime ny = (datetime)(utc + NYOffsetSec(utc));
   long dn = DayNum(ny); int mins = (int)((ny % 86400) / 60);
   if(open > 0)
   {
      if(mins >= (SL_X[k] > 0 ? (int)(SL_X[k] * 60) : 11 * 60) || srv - g_ss[k].t0 > 7 * 3600) CloseSleeve(k, "gap 11:00 exit");
      return;
   }
   if(g_ss[k].dir != 0) { g_ss[k].dir = 0; SaveSleeve(k); }
   if(!g_use[k]) { g_ss[k].status = "off"; return; }
   if(g_ss[k].status == "") g_ss[k].status = "wait";
   if(DowOf(dn) == 0 || DowOf(dn) == 6 || mins < 9 * 60 + 30 || mins >= 9 * 60 + 45 || dn == g_gl_tried) return;
   g_gl_tried = dn;
   string sym = g_sym[role];
   MArr<MqlRates> r;
   int n = CopyRates(sym, PERIOD_M30, 0, 70 * 48, r);
   if(n < 60 * 20) return;
   MArr<double> pc, op; MArr<long> dd; int m = 0;
   ArrayResize(pc, 120); ArrayResize(op, 120); ArrayResize(dd, 120);
   for(int i = 0; i < n; i++)
   {
      datetime bu = (datetime)(r[i].time - ServerUTCOffsetSec(r[i].time));
      datetime bn = (datetime)(bu + NYOffsetSec(bu));
      long d = DayNum(bn); int bm = (int)((bn % 86400) / 60);
      if(bm != 15 * 60 + 30 && bm != 9 * 60 + 30) continue;
      if(m == 0 || dd[m - 1] != d) { if(m == 120) break; dd[m] = d; pc[m] = 0; op[m] = 0; m++; }
      if(bm == 15 * 60 + 30) pc[m - 1] = r[i].close; else op[m - 1] = r[i].open;
   }
   if(m < 2 || dd[m - 1] != dn || op[m - 1] <= 0) return;
   MArr<double> g; ArrayResize(g, 120); int ng = 0; double gt = 0, pref = 0;
   for(int i = 1; i < m; i++)
   {
      int j = i - 1;
      if(op[i] <= 0 || pc[j] <= 0 || dd[i] - dd[j] > 4) continue;
      double x = MathLog(op[i] / pc[j]);
      if(i == m - 1) { gt = x; pref = pc[j]; } else g[ng++] = x;
   }
   if(pref <= 0 || ng < SL_N[k]) return;
   double mu = 0, sd = 0;
   for(int i = ng - SL_N[k]; i < ng; i++) mu += g[i];
   mu /= SL_N[k];
   for(int i = ng - SL_N[k]; i < ng; i++) sd += (g[i] - mu) * (g[i] - mu);
   sd = MathSqrt(sd / (SL_N[k] - 1));
   if(sd > 0) { g_gl_z = (gt - mu) / sd; g_gl_zday = dn; }          // display only
   if(sd <= 0 || (gt - mu) / sd > -SL_P[k]) return;
   if(!g_df[role].ok) CalcDayFeatures(role);
   double ask = SymbolInfoDouble(sym, SYMBOL_ASK);
   double tpd = pref - ask;
   TryEnter(k, 1, SL_SLK[k] * g_df[role].atr, tpd > 0 ? tpd : 0);
}

//--------------------------------------------------------------------- v15: FOMC calendar + post-announcement reversal (kinds 15/16/17/18)
// FOMC decision days (second day of the scheduled meetings, statement 14:00 New York).
// Pre-FOMC drift / post-FOMC reversal research: the 22h pre-announcement drift on US indices is NOT
// tradeable after costs, but the post-decision reversal (short 14:06-16:00 NY) on US30/SPX500 was
// validated 2016-2026 (US30 +9.1R t=1.75, SPX500 +8.9R t=1.62, PF ~1.8-2.0; the same short window on
// non-FOMC days is NEGATIVE, so the edge is FOMC-specific; entry 14:05/14:10/14:15/14:25 all work).
//
// v13.5: self-updating decision dates (live / demo only; the Strategy Tester always uses the table -> backtests unchanged).
// Source chain, first source that covers a day wins:
//   1) MQL5 Economic Calendar: event "Fed Interest Rate Decision" (US; found once by event code, else by name + HIGH importance,
//      minutes / projections / press conference excluded; the id is logged).  Covers [today-400d, last calendar decision].
//   2) ForexFactory weekly JSON (nfs.faireconomy.media/ff_calendar_thisweek.json): USD, High impact, "Federal Funds Rate" /
//      "FOMC Statement".  Covers this week (only when it lists a decision): catches moved / emergency meetings.
//   3) federalreserve.gov FOMC calendar page: meetings of this year and next; decision day = last day of the meeting, 14:00 New York;
//      notation votes / unscheduled entries without a time are skipped.
//   4) the built-in table below (also the Strategy Tester source).
// A source whose parse is empty or implausible (6-10 meetings a year, weekdays, a sane week span) is ignored and logged once.
// Calendar read on the 1-second timer at most hourly (daily after a good read); web pages at most daily (6 h after a network
// error, 60 s while not whitelisted), timeout 3 s, never in OnTick.  Web results are cached in Common\Files\DonchianProp_fomc_cache.txt
// with their fetch time and reused when a fetch fails.  Differences between the active source and the table are printed once.
// Calendar times are trade-server time: converted server -> UTC -> New York with the server offset AT THE EVENT TIME.  A time within
// 75 min of 14:00 NY is a scheduled statement (snapped to 14:00); any other time (emergency meeting) moves the window
// (announcement +6..+30 min entry, +120 min exit, market must be open).
// Web sources need the URLs in Tools > Options > Expert Advisors > Allow WebRequest (error 4014 -> one-time instruction; the
// MT5 calendar / cache / table keep working; trading is never blocked by a missing whitelist).
// Table: 2016-2026 verified against the federalreserve.gov FOMC calendars; 2027 = PROVISIONAL (Fed tentative schedule, checked against
// federalreserve.gov on 2026-10-18) - live sources override it; update it every year for the Strategy Tester.
#define FOMC_N 95
int FOMC_DATES[] =
{
   20160127,20160316,20160427,20160615,20160727,20160921,20161102,20161214,
   20170201,20170315,20170503,20170614,20170726,20170920,20171101,20171213,
   20180131,20180321,20180502,20180613,20180801,20180926,20181108,20181219,
   20190130,20190320,20190501,20190619,20190731,20190918,20191030,20191211,
   20200129,20200429,20200610,20200729,20200916,20201105,20201216,
   20210127,20210317,20210428,20210616,20210728,20210922,20211103,20211215,
   20220126,20220316,20220504,20220615,20220727,20220921,20221102,20221214,
   20230201,20230322,20230503,20230614,20230726,20230920,20231101,20231213,
   20240131,20240320,20240501,20240612,20240731,20240918,20241107,20241218,
   20250129,20250319,20250507,20250618,20250730,20250917,20251029,20251210,
   20260128,20260318,20260429,20260617,20260729,20260916,20261028,20261209,   // v13.4: 2026-12-09 added (missing in v15.0)
   20270127,20270317,20270428,20270609,20270728,20270915,20271027,20271208    // 2027 PROVISIONAL. v13.5: fixed - v13.4 had the FIRST meeting day for Apr-Dec
};
#define FOMC_AMAX 64
#define FL_CAL 0
#define FL_FF  1
#define FL_FED 2
#define FL_TMP 3
#define FOMC_URL_FED "https://www.federalreserve.gov/monetarypolicy/fomccalendars.htm"
#define FOMC_URL_FF  "https://nfs.faireconomy.media/ff_calendar_thisweek.json"
#define FOMC_CACHE   "DonchianProp_fomc_cache.txt"
#define FFN_MAX 128















#define  FP_WL_MSG "FED POST paused: add https://www.federalreserve.gov and https://nfs.faireconomy.media in Tools > Options > Expert Advisors > Allow WebRequest"




void FomcReset()
{
   for(int L = 0; L < 4; L++) { g_fl_n[L] = 0; g_fl_c0[L] = 0; g_fl_c1[L] = -1; g_fl_ok[L] = false; g_fl_t[L] = 0; }
   g_ffn_n = 0; g_ffn_tf = 0; g_fomc_ev = 0; g_cal_alive = false; g_fomc_next = 0;
   g_web_next[0] = 0; g_web_next[1] = 0; g_web_blk[0] = false; g_web_blk[1] = false; g_fp_paused = false;
   g_fomc_cache_loaded = false; g_fomc_said = ",";
}
int  FomcYmd(const long dn) { MqlDateTime m; TimeToStruct((datetime)(dn * 86400), m); return m.year * 10000 + m.mon * 100 + m.day; }
long FomcDn(const int y, const int mo, const int d) { return DayNum(StringToTime(StringFormat("%04d.%02d.%02d", y, mo, d))); }
bool FomcTableDay(const long dn)
{
   int ymd = FomcYmd(dn);
   for(int i = 0; i < FOMC_N; i++) if(FOMC_DATES[i] == ymd) return true;
   return false;
}
void FomcSay(const string key, const string msg)          // prints a note once per EA run
{
   if(StringFind(g_fomc_said, "," + key + ",") >= 0) return;
   g_fomc_said += key + ",";
   Print(msg);
}
bool FomcUsDst(const datetime utc) { return NYOffsetSec(utc) == -4 * 3600; }
long FomcLiveOff() { return (long)(MathRound((double)(TimeTradeServer() - TimeGMT()) / 1800.0) * 1800); }
datetime FomcCalToUTC(const datetime t)                    // calendar (server) time -> UTC, with the server offset at time t
{
   if(InpServerGMTOffset != -99 || IsTester()) return (datetime)(t - ServerUTCOffsetSec(t));
   datetime gmt = TimeGMT();
   datetime u = (datetime)(t - FomcLiveOff());
   if(FomcUsDst(u) != FomcUsDst(gmt)) u -= (FomcUsDst(u) ? 3600 : -3600);   // EET + US DST servers (+2/+3) shift with New York
   return u;
}
long FomcNyDay(const datetime utc) { return DayNum((datetime)(utc + NYOffsetSec(utc))); }
int  FomcNyMin(const datetime utc) { datetime ny = (datetime)(utc + NYOffsetSec(utc)); return (int)((ny % 86400) / 60); }
int  FomcSnap(int mn) { if(mn == 0) mn = 14 * 60; if(MathAbs(mn - 14 * 60) <= 75) mn = 14 * 60; return mn; }
void FomcAdd(const int L, const long dn, const int mn)
{
   for(int i = 0; i < g_fl_n[L]; i++) if(g_fl_dn[L][i] == dn) { if(mn < g_fl_min[L][i]) g_fl_min[L][i] = mn; return; }   // one decision per day
   if(g_fl_n[L] >= FOMC_AMAX) return;
   int j = g_fl_n[L]++;
   while(j > 0 && g_fl_dn[L][j - 1] > dn) { g_fl_dn[L][j] = g_fl_dn[L][j - 1]; g_fl_min[L][j] = g_fl_min[L][j - 1]; j--; }
   g_fl_dn[L][j] = dn; g_fl_min[L][j] = mn;
}
void FomcCommit(const int L, const datetime t)             // scratch layer -> source L
{
   g_fl_n[L] = g_fl_n[FL_TMP]; g_fl_c0[L] = g_fl_c0[FL_TMP]; g_fl_c1[L] = g_fl_c1[FL_TMP]; g_fl_ok[L] = true; g_fl_t[L] = t;
   for(int i = 0; i < g_fl_n[L]; i++) { g_fl_dn[L][i] = g_fl_dn[FL_TMP][i]; g_fl_min[L][i] = g_fl_min[FL_TMP][i]; }
}
string FomcSrcName(const int L) { return L == FL_CAL ? "MT5 calendar" : (L == FL_FF ? "ForexFactory" : (L == FL_FED ? "federalreserve.gov" : "table")); }
void FomcCompareTable(const int L)                         // differences vs the table inside the source's window, printed once
{
   for(int i = 0; i < g_fl_n[L]; i++)
   {
      if(g_fl_dn[L][i] < g_fl_c0[L] || g_fl_dn[L][i] > g_fl_c1[L]) continue;
      if(!FomcTableDay(g_fl_dn[L][i]) || g_fl_min[L][i] != 14 * 60)
         FomcSay("a" + IntegerToString(FomcYmd(g_fl_dn[L][i])), StringFormat("v13.5 FOMC: %s decision %d %02d:%02d New York is not a scheduled table date - %s is used (unscheduled / new meeting)",
                 FomcSrcName(L), FomcYmd(g_fl_dn[L][i]), g_fl_min[L][i] / 60, g_fl_min[L][i] % 60, FomcSrcName(L)));
   }
   for(int i = 0; i < FOMC_N; i++)
   {
      long dn = FomcDn(FOMC_DATES[i] / 10000, (FOMC_DATES[i] / 100) % 100, FOMC_DATES[i] % 100);
      if(dn < g_fl_c0[L] || dn > g_fl_c1[L]) continue;
      bool hit = false;
      for(int j = 0; j < g_fl_n[L]; j++) if(g_fl_dn[L][j] == dn) { hit = true; break; }
      if(!hit) FomcSay("t" + IntegerToString(FOMC_DATES[i]), StringFormat("v13.5 FOMC: table date %d is not in %s - %s is trusted (no FED POST trade that day)", FOMC_DATES[i], FomcSrcName(L), FomcSrcName(L)));
   }
}
//---- 1) MT5 calendar
bool FomcIsDecisionEvent(const MqlCalendarEvent &e)
{
   string code = e.event_code, nm = e.name;
   StringToUpper(code); StringToUpper(nm);
   if(code == "FED-INTEREST-RATE-DECISION") return true;
   if(e.importance != CALENDAR_IMPORTANCE_HIGH) return false;
   if(StringFind(nm, "MINUTES") >= 0 || StringFind(nm, "PROJECTION") >= 0 || StringFind(nm, "PRESS") >= 0) return false;
   return StringFind(nm, "INTEREST RATE DECISION") >= 0 && (StringFind(nm, "FED") >= 0 || StringFind(nm, "FOMC") >= 0);
}
bool FomcFindEvent()
{
   MArr<MqlCalendarEvent> evs;
   ResetLastError();
   int n = CalendarEventByCountry("US", evs);
   g_cal_alive = (n > 0);
   if(g_fomc_ev != 0) return true;
   int best = -1;
   for(int i = 0; i < n; i++)
   {
      if(!FomcIsDecisionEvent(evs[i])) continue;
      string code = evs[i].event_code; StringToUpper(code);
      if(best < 0 || code == "FED-INTEREST-RATE-DECISION") best = i;
   }
   if(best >= 0)
   {
      g_fomc_ev = evs[best].id;
      PrintFormat("v13.5 FOMC: MT5 calendar event '%s' (id %s, code %s) - decision dates come from the MQL5 calendar", evs[best].name, IntegerToString((long)g_fomc_ev), evs[best].event_code);
      return true;
   }
   FomcSay(n > 0 ? "noev" : "nocal", StringFormat("v13.5 FOMC: %s (%d US events, error %d) - next source", n > 0 ? "no Fed rate decision event in the MQL5 calendar" : "the MQL5 calendar has no data", n, GetLastError()));
   return false;
}
bool FomcCalendarRead(const datetime now)
{
   if(!FomcFindEvent()) return false;
   MArr<MqlCalendarValue> vals;
   ResetLastError();
   int n = CalendarValueHistoryByEvent(g_fomc_ev, vals, now - 400 * 86400, now + 400 * 86400);
   if(n <= 0)
   {
      FomcSay(n < 0 ? "cfail" : "cnodata", StringFormat("v13.5 FOMC: the MQL5 calendar returned %s for the Fed decision (error %d) - %s", n < 0 ? "an error" : "no data", GetLastError(),
              g_fl_ok[FL_CAL] ? "keeping the dates read earlier" : "next source"));
      return g_fl_ok[FL_CAL];
   }
   g_fl_n[FL_TMP] = 0;
   for(int i = 0; i < n; i++) { datetime u = FomcCalToUTC(vals[i].time); FomcAdd(FL_TMP, FomcNyDay(u), FomcSnap(FomcNyMin(u))); }
   g_fl_c0[FL_TMP] = FomcNyDay(FomcCalToUTC(now)) - 400; g_fl_c1[FL_TMP] = g_fl_dn[FL_TMP][g_fl_n[FL_TMP] - 1];
   FomcCommit(FL_CAL, now);
   FomcCompareTable(FL_CAL);
   FomcSay("cok" + IntegerToString(g_fl_n[FL_CAL]) + "_" + IntegerToString(FomcYmd(g_fl_c1[FL_CAL])), StringFormat("v13.5 FOMC: %d decision date(s) from the MT5 calendar, last %d", g_fl_n[FL_CAL], FomcYmd(g_fl_c1[FL_CAL])));
   return true;
}
//---- 2) / 3) web pages
int FomcMonthOf(const string s)                            // "September" -> 9 (first three letters)
{
   string u = StringSubstr(s, 0, 3); StringToUpper(u);
   string M = "JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";
   int p = StringFind(M, u);
   return (StringLen(u) == 3 && p >= 0 && p % 3 == 0) ? p / 3 + 1 : 0;
}
string FomcTrim(string s) { StringTrimLeft(s); StringTrimRight(s); return s; }
bool FomcParseFed(const string html, const datetime now)   // federalreserve.gov fomccalendars.htm -> FL_FED
{
   MqlDateTime tn; TimeToStruct(now, tn);
   g_fl_n[FL_TMP] = 0; int yrs_ok = 0, ylast = 0;
   for(int y = tn.year; y <= tn.year + 1; y++)
   {
      int p = StringFind(html, IntegerToString(y) + " FOMC Meetings<");
      if(p < 0) continue;
      int e = StringFind(html, " FOMC Meetings<", p + 20); if(e < 0) e = StringLen(html);
      int cnt = 0, bad = 0, q = p;
      long keep_dn[16]; int nk = 0;
      while(true)
      {
         int a = StringFind(html, "fomc-meeting__month", q); if(a < 0 || a >= e) break;
         int s1 = StringFind(html, "<strong>", a), s2 = StringFind(html, "</strong>", a);
         int b = StringFind(html, "fomc-meeting__date", a);
         if(s1 < 0 || s2 < s1 || b < 0 || b >= e) { bad++; break; }
         int b1 = StringFind(html, ">", b), b2 = StringFind(html, "<", b1);
         q = b2 > 0 ? b2 : b + 20;
         string mon = FomcTrim(StringSubstr(html, s1 + 8, s2 - s1 - 8)), dt = FomcTrim(StringSubstr(html, b1 + 1, b2 - b1 - 1));
         StringReplace(dt, "*", "");
         if(StringFind(dt, "(") >= 0) continue;              // notation vote / unscheduled without a statement time
         int dash = StringFind(dt, "-");
         if(dash < 0) { bad++; continue; }
         int d1 = (int)StringToInteger(StringSubstr(dt, 0, dash)), d2 = (int)StringToInteger(StringSubstr(dt, dash + 1));
         int sl = StringFind(mon, "/");
         int m1 = FomcMonthOf(mon), m2 = sl >= 0 ? FomcMonthOf(StringSubstr(mon, sl + 1)) : m1;
         int mo = (d2 < d1) ? (sl >= 0 ? m2 : 0) : m1;
         if(mo < 1 || d2 < 1 || d2 > 31) { bad++; continue; }
         long dn = FomcDn(y, mo, d2);
         int dw = DowOf(dn);
         if(dw < 1 || dw > 5 || FomcYmd(dn) != y * 10000 + mo * 100 + d2) { bad++; continue; }
         if(nk < 16) keep_dn[nk++] = dn;
         cnt++;
      }
      if(cnt < 6 || cnt > 10 || bad > 0)
      {
         FomcSay("fedbad" + IntegerToString(y), StringFormat("v13.5 FOMC: federalreserve.gov %d: %d meeting(s), %d unreadable - implausible, year ignored", y, cnt, bad));
         if(y == tn.year) return false;
         continue;
      }
      for(int i = 0; i < nk; i++) FomcAdd(FL_TMP, keep_dn[i], 14 * 60);
      yrs_ok++; ylast = y;
   }
   if(yrs_ok == 0) { FomcSay("fednone", "v13.5 FOMC: federalreserve.gov page gave no plausible meeting dates - source ignored"); return false; }
   g_fl_c0[FL_TMP] = FomcDn(tn.year, 1, 1); g_fl_c1[FL_TMP] = FomcDn(ylast, 12, 31);
   FomcCommit(FL_FED, now);
   FomcCompareTable(FL_FED);
   return true;
}
string FomcJsonStr(const string obj, const string key)     // "key":"value" (flat object, no nested quotes needed)
{
   int p = StringFind(obj, "\"" + key + "\":\"");
   if(p < 0) return "";
   p += StringLen(key) + 4;
   int e = StringFind(obj, "\"", p);
   return e < 0 ? "" : StringSubstr(obj, p, e - p);
}
datetime FomcIsoToUTC(const string d)                      // 2026-10-28T14:00:00-04:00 -> UTC, 0 if malformed
{
   if(StringLen(d) < 25 || StringSubstr(d, 4, 1) != "-" || StringSubstr(d, 10, 1) != "T") return 0;
   int y = (int)StringToInteger(StringSubstr(d, 0, 4)), mo = (int)StringToInteger(StringSubstr(d, 5, 2)), dd = (int)StringToInteger(StringSubstr(d, 8, 2));
   int hh = (int)StringToInteger(StringSubstr(d, 11, 2)), mi = (int)StringToInteger(StringSubstr(d, 14, 2));
   string sg = StringSubstr(d, 19, 1);
   int oh = (int)StringToInteger(StringSubstr(d, 20, 2)), om = (int)StringToInteger(StringSubstr(d, 23, 2));
   if(y < 2000 || y > 2100 || mo < 1 || mo > 12 || dd < 1 || dd > 31 || hh > 23 || mi > 59 || (sg != "-" && sg != "+") || oh > 14) return 0;
   datetime t = StringToTime(StringFormat("%04d.%02d.%02d %02d:%02d", y, mo, dd, hh, mi));
   if(t <= 0) return 0;
   long off = (oh * 3600 + om * 60) * (sg == "-" ? -1 : 1);
   return (datetime)(t - off);
}
bool FomcParseFF(const string js, const datetime now)      // ff_calendar_thisweek.json -> FL_FF + USD high-impact list
{
   g_fl_n[FL_TMP] = 0;
   datetime nt[FFN_MAX]; int nn = 0, good = 0, bad = 0;
   long d0 = 0, d1 = 0;
   int p = 0;
   while(true)
   {
      int a = StringFind(js, "{", p); if(a < 0) break;
      int b = StringFind(js, "}", a); if(b < 0) break;
      p = b + 1;
      string o = StringSubstr(js, a, b - a + 1);
      string title = FomcJsonStr(o, "title"), cty = FomcJsonStr(o, "country"), imp = FomcJsonStr(o, "impact");
      datetime u = FomcIsoToUTC(FomcJsonStr(o, "date"));
      if(title == "" || u == 0) { bad++; continue; }
      good++;
      long dn = FomcNyDay(u);
      if(good == 1 || dn < d0) d0 = dn;
      if(good == 1 || dn > d1) d1 = dn;
      if(cty != "USD" || imp != "High") continue;
      if(nn < FFN_MAX) nt[nn++] = u;
      string tu = title; StringToUpper(tu);
      if(StringFind(tu, "FEDERAL FUNDS RATE") >= 0 || StringFind(tu, "FOMC STATEMENT") >= 0) FomcAdd(FL_TMP, dn, FomcSnap(FomcNyMin(u)));
   }
   if(good < 10 || bad * 4 > good || d1 - d0 > 8)
   {
      FomcSay("ffbad", StringFormat("v13.5 FOMC: ForexFactory JSON implausible (%d events, %d unreadable, span %d days) - source ignored", good, bad, (int)(d1 - d0)));
      return false;
   }
   g_ffn_n = nn; for(int i = 0; i < nn; i++) g_ffn_t[i] = nt[i]; g_ffn_tf = now;
   g_fl_c0[FL_TMP] = d0; g_fl_c1[FL_TMP] = d1;
   FomcCommit(FL_FF, now);
   if(g_fl_n[FL_FF] > 0) FomcCompareTable(FL_FF);
   return true;
}
void FomcCacheSave()
{
   int h = FileOpen(FOMC_CACHE, FILE_WRITE | FILE_TXT | FILE_ANSI | FILE_COMMON);
   if(h == INVALID_HANDLE) { FomcSay("cachew", StringFormat("v13.5 FOMC: cannot write the cache file (error %d)", GetLastError())); return; }
   FileWriteString(h, "DPFOMC1\n");
   for(int L = FL_FF; L <= FL_FED; L++)
   {
      if(!g_fl_ok[L]) continue;
      FileWriteString(h, StringFormat("L %d %s %s %s %d\n", L, IntegerToString(g_fl_c0[L]), IntegerToString(g_fl_c1[L]), IntegerToString((long)g_fl_t[L]), g_fl_n[L]));
      for(int i = 0; i < g_fl_n[L]; i++) FileWriteString(h, StringFormat("D %d %s %d\n", L, IntegerToString(g_fl_dn[L][i]), g_fl_min[L][i]));
   }
   if(g_ffn_tf > 0) { FileWriteString(h, StringFormat("T %s\n", IntegerToString((long)g_ffn_tf))); for(int i = 0; i < g_ffn_n; i++) FileWriteString(h, StringFormat("N %s\n", IntegerToString((long)g_ffn_t[i]))); }
   FileClose(h);
}
bool FomcCacheLoad()
{
   int h = FileOpen(FOMC_CACHE, FILE_READ | FILE_TXT | FILE_ANSI | FILE_COMMON);
   if(h == INVALID_HANDLE) return false;
   bool okf = (FomcTrim(FileReadString(h)) == "DPFOMC1");
   int n[4] = {0, 0, 0, 0};
   while(okf && !FileIsEnding(h))
   {
      string ln = FomcTrim(FileReadString(h));
      MArr<string> parts;
      int k = StringSplit(ln, ' ', parts);
      if(k >= 6 && parts[0] == "L")
      {
         int L = (int)StringToInteger(parts[1]); if(L != FL_FF && L != FL_FED) continue;
         g_fl_c0[L] = StringToInteger(parts[2]); g_fl_c1[L] = StringToInteger(parts[3]); g_fl_t[L] = (datetime)StringToInteger(parts[4]);
         n[L] = (int)StringToInteger(parts[5]); g_fl_n[L] = 0; g_fl_ok[L] = true;
      }
      else if(k >= 4 && parts[0] == "D") { int L = (int)StringToInteger(parts[1]); if((L == FL_FF || L == FL_FED) && g_fl_ok[L]) FomcAdd(L, StringToInteger(parts[2]), (int)StringToInteger(parts[3])); }
      else if(k >= 2 && parts[0] == "T") { g_ffn_tf = (datetime)StringToInteger(parts[1]); g_ffn_n = 0; }
      else if(k >= 2 && parts[0] == "N" && g_ffn_n < FFN_MAX) g_ffn_t[g_ffn_n++] = (datetime)StringToInteger(parts[1]);
   }
   FileClose(h);
   for(int L = FL_FF; L <= FL_FED; L++) if(g_fl_ok[L] && g_fl_n[L] != n[L]) { g_fl_ok[L] = false; g_fl_n[L] = 0; okf = false; }
   if(!okf) { FomcSay("cacheb", "v13.5 FOMC: cache file unreadable - ignored"); return false; }
   FomcSay("cacher", StringFormat("v13.5 FOMC: cache loaded (federalreserve.gov %d dates, ForexFactory %d, news %d)", g_fl_n[FL_FED], g_fl_n[FL_FF], g_ffn_n));
   return true;
}
int FomcFetch(const string url, string &body)              // 200 = ok, -4014 = not whitelisted, other = network / HTTP error
{
   MArr<char> req, res; string rh;
   ResetLastError();
   int code = WebRequest("GET", url, "User-Agent: Mozilla/5.0 (DonchianProp EA)\r\n", 3000, req, res, rh);
   if(code == -1)
   {
      int e = GetLastError();
      if(e == 4014) return -4014;
      FomcSay("neterr" + url, StringFormat("v13.5 FOMC: %s not reachable (error %d) - cache / calendar / table used", url, e));
      return -1;
   }
   if(code != 200) { FomcSay("http" + url, StringFormat("v13.5 FOMC: %s answered HTTP %d - cache / calendar / table used", url, code)); return code; }
   body = CharArrayToString(res, 0, WHOLE_ARRAY, CP_UTF8);
   return 200;
}
// v13.6: the WebRequest whitelist is REQUIRED for FED POST in live / demo trading (the tester never needs it).  Error 4014 on
// either URL pauses the two FED POST sleeves only (Alert once per pause, journal line, panel line); the URLs are tested again
// every minute while paused and the sleeves resume without a restart.  A network error means the URL IS whitelisted: no pause
// (cache, MT5 calendar and the table give the dates).
void FpPauseUpdate()
{
   bool p = (g_web_blk[0] || g_web_blk[1]) && !IsTester();
   if(p == g_fp_paused) return;
   g_fp_paused = p;
   if(p) { Alert(FP_WL_MSG); Print("v13.6 ", FP_WL_MSG, " (only the FED POST sleeves are paused; they resume by themselves)"); }
   else Print("v13.6 FED POST resumed: WebRequest allowed for https://www.federalreserve.gov and https://nfs.faireconomy.media");
}
void FomcWebStep(const int i, const datetime now)
{
   if(now < g_web_next[i]) return;
   string body = "";
   int code = FomcFetch(i == 0 ? FOMC_URL_FED : FOMC_URL_FF, body);
   if(code == -4014)                                        // v13.6: URL not whitelisted -> FED POST paused, re-tested every minute
   {
      g_web_blk[i] = true; g_web_next[i] = now + 60;
      FpPauseUpdate();
      return;
   }
   g_web_blk[i] = false;                                    // any other result (also offline): the URL is whitelisted
   FpPauseUpdate();
   bool ok = (code == 200) && (i == 0 ? FomcParseFed(body, now) : FomcParseFF(body, now));
   if(ok) { g_web_next[i] = now + 86400; FomcCacheSave(); }
   else g_web_next[i] = now + 6 * 3600;                   // network / bad page: cache, calendar and table stay in use
}
void FomcRefresh(const bool force)                 // live / demo only, timer driven
{
   if(IsTester()) return;
   datetime now = TimeTradeServer();
   if(!g_fomc_cache_loaded && FomcCacheLoad())             // a cache younger than a day is not fetched again (except the forced start-up check)
   {
      if(g_fl_ok[FL_FED] && g_fl_t[FL_FED] + 86400 > g_web_next[0]) g_web_next[0] = g_fl_t[FL_FED] + 86400;
      if(g_fl_ok[FL_FF] && g_fl_t[FL_FF] + 86400 > g_web_next[1]) g_web_next[1] = g_fl_t[FL_FF] + 86400;
   }
   g_fomc_cache_loaded = true;
   if(force || now >= g_fomc_next) g_fomc_next = now + (FomcCalendarRead(now) ? 86400 : 3600);
   if(force) { g_web_next[0] = 0; g_web_next[1] = 0; }
   FomcWebStep(0, now);
   FomcWebStep(1, now);
}
int FomcAnnounceMin(const long ny_dn)                      // announcement minute (New York) on a decision day, -1 = no decision
{
   int order[3] = {FL_CAL, FL_FF, FL_FED};
   if(!IsTester())
      for(int o = 0; o < 3; o++)
      {
         int L = order[o];
         if(!g_fl_ok[L] || ny_dn < g_fl_c0[L] || ny_dn > g_fl_c1[L]) continue;
         if(L == FL_FF && g_fl_n[L] == 0) continue;        // a week without a listed decision proves nothing
         for(int i = 0; i < g_fl_n[L]; i++) if(g_fl_dn[L][i] == ny_dn) return g_fl_min[L][i];
         return -1;
      }
   return FomcTableDay(ny_dn) ? 14 * 60 : -1;
}
bool IsFomcDay(const long ny_dn) { return FomcAnnounceMin(ny_dn) >= 0; }
// v13.6: exit at 15:58 New York (announcement + 118 min) or 2 minutes before the broker's session end, whichever comes first.
// v13.5 closed at 16:00 NY = 23:00 server on EET brokers = the index session close on MetaQuotes-Demo: the close was refused and
// sent at the 00:00 reopen in 122 of 131 tester trades.
string FpExitWhy(const int ki, const int role, const datetime srv, const int ny_min)
{
   if(ny_min >= g_fp_amin[ki] + 118) return StringFormat("FOMC exit %02d:%02d NY", (g_fp_amin[ki] + 118) / 60, (g_fp_amin[ki] + 118) % 60);
   datetime se = SessionEnd(g_sym[role], srv);
   if(se > 0 && srv >= se - 120) return "FOMC exit (2 min before the session end)";
   return "";
}
// v13.6: a FED POST entry queued by a closed market is dropped when the window of its own New York day is over (exit time reached
// or another day) - it is never sent at a later reopen.
bool FpQueueExpired(const int k)
{
   int ki = k - 22; if(ki < 0 || ki > 1) return false;
   datetime srv = TimeCurrent();
   datetime utc = (datetime)(srv - ServerUTCOffsetSec(srv)), ny = (datetime)(utc + NYOffsetSec(utc));
   datetime u0 = (datetime)(g_rt_t0[k] - ServerUTCOffsetSec(g_rt_t0[k])), n0 = (datetime)(u0 + NYOffsetSec(u0));
   if(DayNum(ny) != DayNum(n0)) return true;
   return FpExitWhy(ki, SL_ROLE[k], srv, (int)((ny % 86400) / 60)) != "";
}


void FomcPostTimer()                // v15: short US30 / SPX500 after the FOMC statement (14:00 NY; v13.5: live sources)
{
   FomcRefresh();
   for(int ki = 0; ki < 2; ki++)
   {
      int k = 22 + ki;
      if(SL_KIND[k] != 15) continue;
      g_cur_role = SL_ROLE[k];
      int role = SL_ROLE[k];
      if(!g_sym_ok[role]) continue;
      double lots, pnl;
      int open = SleevePositions(k, lots, pnl);
      datetime srv = TimeCurrent();
      datetime utc = (datetime)(srv - ServerUTCOffsetSec(srv));
      datetime ny = (datetime)(utc + NYOffsetSec(utc));
      long dn = DayNum(ny); int mins = (int)((ny % 86400) / 60);
      if(open > 0)
      {
         string xw = FpExitWhy(ki, role, srv, mins);
         if(xw != "") CloseSleeve(k, xw);
         else if(srv - g_ss[k].t0 > 7 * 3600) CloseSleeve(k, "FOMC exit (7 h after entry)");
         continue;
      }
      if(g_ss[k].dir != 0) { g_ss[k].dir = 0; SaveSleeve(k); }
      if(!g_use[k]) { g_ss[k].status = "off"; continue; }
      if(g_fp_paused) { g_ss[k].status = "paused: WebRequest"; continue; }   // v13.6: live / demo without the WebRequest whitelist
      if(g_ss[k].status == "" || g_ss[k].status == "paused: WebRequest") g_ss[k].status = "wait";
      int am = FomcAnnounceMin(dn);
      if(am < 0 || mins < am + 6 || mins >= am + 30) continue;
      if(g_fp_tried[ki] < 0) g_fp_tried[ki] = (long)GvGet("fp_tried" + IntegerToString(ki), -1);   // restart inside the window
      if(dn == g_fp_tried[ki]) continue;
      g_fp_tried[ki] = dn; GvSet("fp_tried" + IntegerToString(ki), (double)dn);
      if(am != 14 * 60 && !MarketOpen(role)) { PrintFormat("v13.5 FOMC: unscheduled decision %d %02d:%02d NY - %s closed, no trade", FomcYmd(dn), am / 60, am % 60, g_sym[role]); continue; }
      if(!g_df[role].ok) CalcDayFeatures(role);
      if(!g_df[role].ok) continue;
      g_fp_amin[ki] = am;
      TryEnter(k, -1, SL_SLK[k] * g_df[role].atr);
   }
}

// ---- v15: synthetic DXY (5-pair proxy, 95.8% of the index weight; USDSEK omitted) ----
// DXY = 50.143 x EURUSD^-0.576 x USDJPY^0.136 x GBPUSD^-0.119 x USDCAD^0.091 x USDCHF^0.036.
// Research: fading |z|>=2.5 extremes of this index on USDJPY (48h hold, z vs EMA(50) of the log index,
// sd over the prior 50 closed days) earned +10.6R t=2.44 single-position (+14.2R t=2.86 with overlap),
// positive in all three sub-periods; fading USDJPY's OWN z-score fails (-0.6R) -> the DXY information is
// real, not a USDJPY restatement. (Momentum/breakout forms of DXY-JPY lose money; SMA-z is dead: +5.3R t=0.92.)





MArr<MqlRates> g_dxr;
string AuxResolve(const string user, const string base)
{
   if(user == "-") return "";
   if(user != "" && SymbolSelect(user, true)) return user;
   if(user != "") PrintFormat("v15: symbol '%s' not found - searching automatically", user);
   int n = SymbolsTotal(false);
   string best = "";
   for(int i = 0; i < n; i++)
   {
      string s = SymbolName(i, false);
      if(StringFind(NormSym(s), base) == 0 && StringLen(s) < StringLen(base) + 5) { best = s; break; }
   }
   if(best == "" && SymbolSelect(base, true)) best = base;
   return best;
}
void InitDxySymbols()
{
   string base[5]; base[0] = "EURUSD"; base[1] = "USDJPY"; base[2] = "GBPUSD"; base[3] = "USDCAD"; base[4] = "USDCHF";
   string user[5]; user[0] = InpEurUsdSymbol; user[1] = ""; user[2] = InpGbpUsdSymbol; user[3] = InpUsdCadSymbol; user[4] = InpUsdChfSymbol;
   for(int i = 0; i < 5; i++)
   {
      g_dxy_sym[i] = (i == 1 ? g_sym[5] : AuxResolve(user[i], base[i]));   // USDJPY = the trading symbol itself
      g_dxy_ok[i] = (g_dxy_sym[i] != "");
      if(!g_dxy_ok[i] && i != 1 && InpUseDxyJpy) PrintFormat("v15: no %s found on this server - the DXY-JPY sleeve stays off", base[i]);
   }
}
bool DxyCalc(double &z)                 // z of the latest CLOSED DXY day vs the prior 50 closed days
{
   int nn = 0;
   for(int p = 0; p < 5; p++)
   {
      if(!g_dxy_ok[p]) return false;
      int n = CopyRates(g_dxy_sym[p], PERIOD_D1, 1, 260, g_dxr);
      if(n < 100) return false;
      if(n > 270) n = 270;
      if(p == 0)
      {
         nn = n;
         for(int i = 0; i < n; i++) { g_dxy_c[i] = 0; g_dxy_t[i] = g_dxr[i].time; }   // n<=270 (g_dxy_c size)
      }
      else
      {
         if(n != nn) return false;
         for(int i = 0; i < n; i++) if(g_dxy_t[i] != g_dxr[i].time) return false;   // holiday misalignment: no signal
      }
      for(int i = 0; i < n; i++) g_dxy_c[i] += g_dxy_w[p] * MathLog(g_dxr[i].close);
   }
   int n = nn;
   // v15.1: EMA(50) of the log index through the newest closed day; sd over the PRIOR 50 days.
   // (The SMA-50 z scored only +5.3R t=0.92 2016-26; the EMA z is the validated signal: +10.6R t=2.44
   //  single-position, +14.2R t=2.86 with overlap, positive in all three sub-periods.)
   double e = g_dxy_c[0];
   for(int i = 1; i < n; i++) e += (g_dxy_c[i] - e) * 2.0 / 51.0;
   double q = 0;
   for(int i = n - 51; i <= n - 2; i++) q += MathPow(g_dxy_c[i] - e, 2);
   double sd = MathSqrt(q / 49.0);
   if(sd <= 0) return false;
   z = (g_dxy_c[n - 1] - e) / sd;
   return true;
}

void DxyJpySleeve(const int k)         // k = 24 (magic 110024): fade the DXY z-extreme on USDJPY
{
   int role = SL_ROLE[k];
   if(!g_sym_ok[role] || !g_use[k]) { g_ss[k].status = "off"; return; }
   double z = 0;
   if(!DxyCalc(z)) { g_ss[k].status = "dxy wait"; return; }
   if(MathAbs(z) < SL_P[k]) { g_ss[k].status = StringFormat("z=%.2f", z); return; }
   int dir = (z > 0 ? -1 : 1);
   if(!g_df[role].ok) CalcDayFeatures(role);
   if(!g_df[role].ok) return;
   g_ss[k].status = StringFormat("z=%.2f", z);
   TryEnter(k, dir, SL_SLK[k] * g_df[role].atr);
}
void GoldZcontSleeve(const int k)      // k = 25: continuation after z50 >= SL_P above the EMA200
{
   int role = SL_ROLE[k];
   if(!g_sym_ok[role]) return;
   if(!g_use[k]) { g_ss[k].status = "off"; return; }
   if(!g_df[role].ok) CalcDayFeatures(role);
   if(!g_df[role].ok) { g_ss[k].status = "wait"; return; }
   DayFeat f = g_df[role];
   if(f.z50 < SL_P[k] || f.close <= f.ema200) { g_ss[k].status = StringFormat("z=%.2f", f.z50); return; }
   TryEnter(k, 1, SL_SLK[k] * f.atr);
}
void NewSleevesDaily()                 // v15: entries at 02:00 server; time exits any hour
{
   datetime srv = TimeCurrent();
   for(int k = 24; k <= 25; k++)
   {
      if(SL_KIND[k] < 16 || SL_KIND[k] > 17) continue;
      int role = SL_ROLE[k];
      if(!g_sym_ok[role]) continue;
      double lots, pnl;
      int open = SleevePositions(k, lots, pnl);
      if(open > 0)
      {
         if(srv - g_ss[k].t0 >= (long)SL_MAXB[k] * 3600) CloseSleeve(k, "time stop");
         continue;
      }
      if(g_ss[k].dir != 0) { g_ss[k].dir = 0; SaveSleeve(k); }
   }
   long sd = DayNum(srv);
   int h = (int)((srv % 86400) / 3600);
   if(h < 2 || h >= 3 || sd == g_ns_day) return;
   g_ns_day = sd;
   for(int k = 24; k <= 25; k++)
   {
      if(SL_KIND[k] < 16 || SL_KIND[k] > 17) continue;
      double lots, pnl;
      if(SleevePositions(k, lots, pnl) > 0) continue;      // one position at a time per sleeve
      g_cur_role = SL_ROLE[k];
      if(SL_KIND[k] == 16) DxyJpySleeve(k);
      else if(SL_KIND[k] == 17) GoldZcontSleeve(k);
   }
}

//--------------------------------------------------------------------- post-fix reversal (USDJPY, Gotobi days)
// Bessho, Sugimoto & Suzuki (arXiv 2301.13204), leg B: when USDJPY has risen into the 09:55 JST Tokyo fix, it falls back afterwards.
// Short at 00:55 UTC on a Gotobi day (Monday Gotobi days skipped) if the bid is above the bid at 15:55 UTC of the previous weekday;
// exit 03:00 UTC (12:00 JST), stop InpFixRevStopPips.  Verified: EURJPY 2005-2018 n=503 PF 1.37 t=2.5 (never used to fit),
// USDJPY 2019-2025 n=253 PF 1.44; portfolio Sharpe and MAR improved in 2017-20 and 2020-26.
void FixReversalTimer()
{
   g_cur_role = SL_ROLE[14];
   const int k = 14;
   int role = SL_ROLE[k];
   if(!g_sym_ok[role]) return;
   double lots, pnl;
   int open = SleevePositions(k, lots, pnl);
   datetime srv = TimeCurrent();
   datetime utc = (datetime)(srv - ServerUTCOffsetSec(srv));
   long dn = DayNum(utc); int mins = (int)((utc % 86400) / 60);
   if(open > 0)
   {
      if(mins >= 3 * 60 || srv - g_ss[k].t0 > 3 * 3600) CloseSleeve(k, "post-fix time exit");
      return;
   }
   if(g_ss[k].dir != 0) { g_ss[k].dir = 0; SaveSleeve(k); }
   if(!g_use[k]) { g_ss[k].status = "off"; return; }
   if(g_ss[k].status == "") g_ss[k].status = "wait";
   string sym = g_sym[role];
   int dw = DowOf(dn);
   if(mins >= 15 * 60 + 55 && mins < 16 * 60 + 25 && dw >= 1 && dw <= 4 && IsGotobiDay(dn + 1) && g_fr_refday != dn + 1)
   {
      double bid = SymbolInfoDouble(sym, SYMBOL_BID);
      if(bid > 0) { g_fr_ref = bid; g_fr_refday = dn + 1; GvSet("fr_ref", bid); GvSet("fr_refday", (double)(dn + 1)); }
      return;
   }
   if(mins < 55 || mins >= 70 || dn == g_fr_tried || dw < 2 || dw > 5) return;
   g_fr_tried = dn;
   if(g_fr_refday != dn) { g_fr_ref = GvGet("fr_ref", 0); g_fr_refday = (long)GvGet("fr_refday", -1); }
   if(g_fr_refday != dn || g_fr_ref <= 0) return;
   double bid = SymbolInfoDouble(sym, SYMBOL_BID);
   if(bid <= g_fr_ref) return;
   if(!g_df[role].ok) CalcDayFeatures(role);
   double pt = SymbolInfoDouble(sym, SYMBOL_POINT); int dg = (int)SymbolInfoInteger(sym, SYMBOL_DIGITS);
   double pip = (dg == 3 || dg == 5) ? 10 * pt : pt;
   TryEnter(k, -1, InpFixRevStopPips * pip);
}

//--------------------------------------------------------------------- prop accounting
int CountTradingDays()
{
   if(!HistorySelect(g_start_time, TimeCurrent() + 86400)) return g_trading_days;
   long last = -1; int c = 0;
   int n = HistoryDealsTotal();
   for(int i = 0; i < n; i++)
   {
      ulong d = HistoryDealGetTicket(i);
      if(d == 0) continue;
      long typ = HistoryDealGetInteger(d, DEAL_TYPE);
      if(typ != DEAL_TYPE_BUY && typ != DEAL_TYPE_SELL) continue;
      if(HistoryDealGetInteger(d, DEAL_ENTRY) != DEAL_ENTRY_IN) continue;
      long dn = DayNum((datetime)HistoryDealGetInteger(d, DEAL_TIME));
      if(dn != last) { c++; last = dn; }
   }
   return c;
}

void DetectInitialBalance()
{
   g_init_bal = InpInitialBalance;
   g_start_time = (datetime)GvGet("start", 0);
   if(g_init_bal <= 0) g_init_bal = GvGet("init", 0);
   if(g_init_bal <= 0 || g_start_time == 0)
   {
      HistorySelect(0, TimeCurrent() + 86400);
      int n = HistoryDealsTotal();
      for(int i = 0; i < n; i++)
      {
         ulong d = HistoryDealGetTicket(i);
         if(d == 0 || HistoryDealGetInteger(d, DEAL_TYPE) != DEAL_TYPE_BALANCE) continue;
         if(g_init_bal <= 0) g_init_bal = HistoryDealGetDouble(d, DEAL_PROFIT);
         if(g_start_time == 0) g_start_time = (datetime)HistoryDealGetInteger(d, DEAL_TIME);
         break;
      }
   }
   if(g_init_bal <= 0) g_init_bal = AccountInfoDouble(ACCOUNT_BALANCE);
   if(g_start_time == 0) g_start_time = TimeCurrent();
   GvSet("init", g_init_bal); GvSet("start", (double)g_start_time);
}

//--------------------------------------------------------------------- v13.7.1: tester stage summary
// Tester only. At each stage end (target, max-loss guard, end of the test) the journal gets one line:
// calendar + trading days, final result, worst closed day, peak-to-trough drawdown and daily-guard stops.



void StageReset()
{
   g_stage_t0 = TimeCurrent();
   g_stage_peak = AccountInfoDouble(ACCOUNT_EQUITY); g_stage_b0 = AccountInfoDouble(ACCOUNT_BALANCE);
   g_stage_mdd = 0; g_stage_wday = 0; g_stage_halts = 0;
   g_stage_td0 = g_trading_days;
}
void StageSummary(const string what)
{
   if(!IsTester() || g_stage_b0 <= 0 || g_stage_t0 == 0) return;
   double eq = AccountInfoDouble(ACCOUNT_EQUITY);
   PrintFormat("v13.12 STAGE SUMMARY %s: %s | %d calendar days, %d trading days | result %+.2f%% | worst closed day %+.2f%% | max drawdown %.2f%% | daily-guard stops %d",
               PresetName(g_preset), what, (int)((TimeCurrent() - g_stage_t0) / 86400), g_trading_days - g_stage_td0,
               100.0 * (eq / g_stage_b0 - 1.0), g_stage_wday, 100.0 * g_stage_mdd / g_stage_b0, g_stage_halts);
}

void PropChecks()
{
   datetime now = TimeCurrent();
   double eq = AccountInfoDouble(ACCOUNT_EQUITY), bal = AccountInfoDouble(ACCOUNT_BALANCE);
   datetime day = (datetime)(DayNum(now) * 86400);
   if(day != g_day)
   {
      if(g_day != 0 && g_stage_t0 > 0 && g_init_bal > 0)   // v13.7.1: result of the day that just closed
         g_stage_wday = MathMin(g_stage_wday, (eq - g_day_ref) / (IsPersonal() ? g_day_ref : g_init_bal) * 100.0);
      g_day = day; g_day_ref = DayRefNow();
      GvSet("day", (double)g_day); GvSet("dayref", g_day_ref);
      if(InpVolTarget && g_init_bal > 0)       // EWMA (0.94) of daily equity changes, in % of the initial balance
      {
         double tv = InpVolTargetPct / 100.0;
         if(g_vt_var < 0) g_vt_var = tv * tv / 252.0;
         if(g_eq_prevday > 0) { double r = (eq - g_eq_prevday) / g_init_bal; g_vt_var = 0.94 * g_vt_var + 0.06 * r * r; }
         g_eq_prevday = eq;
         double vol = MathSqrt(g_vt_var * 252.0);
         g_vt_g = MathMax(InpVolGMin, MathMin(InpVolGMax, vol > 0 ? tv / vol : InpVolGMax));
         GvSet("vt_var", g_vt_var); GvSet("vt_eq", g_eq_prevday);
      }
      if(g_halt == 1) { g_halt = 0; GvSet("halt", 0); Print("v13 daily halt cleared (new server day)"); }
      PerfAlertUpdate();
      if(IsPersonal() && InpPersCompound && bal > 0) { g_init_bal = bal; GvSet("init", g_init_bal); }   // personal: risk follows the balance
      g_trading_days = CountTradingDays();
   }
   if(g_stage_t0 > 0)                                     // v13.7.1: stage peak-to-trough drawdown
   {
      if(eq > g_stage_peak) g_stage_peak = eq;
      g_stage_mdd = MathMax(g_stage_mdd, g_stage_peak - eq);
   }
   if(IsPersonal() && eq > g_pers_peak) { g_pers_peak = eq; static datetime s_pp = 0; if(now - s_pp >= 60) { s_pp = now; GvSet("ppeak", g_pers_peak); } }
   if(InpChallengeStallPct > 0 && (g_preset == PRESET_PHASE1 || g_preset == PRESET_PHASE2) && g_init_bal > 0
      && eq <= g_init_bal * (1.0 - InpChallengeStallPct / 100.0) && GvGet("stall_" + PresetName(g_preset), 0) < 0.5)
   {
      GvSet("stall_" + PresetName(g_preset), 1);
      string sm = StringFormat("v13.9 CHALLENGE STALL: %s equity %.2f is %.1f%% below the stage start. Research (39 starts 2016-2025): restarting a new challenge here reaches funded faster than waiting.", PresetName(g_preset), eq, 100.0 * (1.0 - eq / g_init_bal));
      Print(sm); if(!IsTester()) Alert(sm);
   }
   // overall loss (static or trailing high-water mark)
   if(bal > g_bal_peak) { g_bal_peak = bal; GvSet("bal_peak", g_bal_peak); }
   double hw = (InpMaxLossType == ML_TRAIL_EQUITY ? eq : bal);
   if(InpMaxLossType != ML_STATIC && hw > g_hwm) { g_hwm = hw; GvSet("hwm", g_hwm); }
   double floor_eq = GuardFloor();
   if(g_halt != 2 && (IsPersonal() ? eq : MathMin(eq, bal)) <= floor_eq)   // v11.9: FundedNext counts balance OR equity below the floor; v13.8 personal: equity vs its peak
   {
      StageSummary("FAILED: max-loss guard");
      CloseAll(IsPersonal() ? "personal max-drawdown stop" : "max-loss guard"); g_halt = 2; GvSet("halt", 2);
      if(IsPersonal()) { GvSet("ppeak", g_pers_peak); g_msg = "MAX DRAWDOWN STOP"; PrintFormat("v13.8 HALT (personal): equity %.2f is %.1f%% below its peak %.2f (stop %.1f%%) - all closed, new entries stopped until Reset in the panel", eq, 100.0 * (1.0 - eq / g_pers_peak), g_pers_peak, InpPersMaxDDPct); }
      else { g_msg = "MAX LOSS GUARD"; Print("v13 HALT: equity reached the max-loss guard"); }
   }
   // daily loss
   if(g_halt == 0 && DailyLoss() >= DailyLimitMoney(HardDailyPct()))
   {
      CloseAll("daily-loss guard"); g_halt = 1; g_halt_day = g_day; GvSet("halt", 1);
      g_stage_halts++;
      g_msg = "DAILY LOSS GUARD"; Print("v13 HALT (today): daily-loss guard");
   }
   // v13.1: open-profit protection (checked once a minute)
   static datetime s_lw = 0;
   if(InpLockOpenProfit && g_halt == 0 && (now - s_lw >= 60 || (g_lw_wait != 0 && LockWaitOpen())))   // v13.3: or a skipped symbol reopened
   {
      s_lw = now; g_lw_wait = 0;
      if(DailyLoss() + StopOutFromNow() > DailyLimitMoney(HardDailyPct())) LockWinners();
   }
   if(InpBookTakePct > 0 && g_halt == 0) { static datetime s_bt = 0; if(now - s_bt >= 60) { s_bt = now; BookTake(); } }
   // target
   double tgt = TargetPct();
   if(tgt > 0 && !g_target_hit && eq >= g_init_bal * (1.0 + tgt / 100.0) && IsTester() && InpTesterChain)
   {
      // v13.1: tester only - a passed stage starts the next one from the current balance, like a new account (phase 1 -> phase 2 -> funded)
      StageSummary("TARGET REACHED");
      CloseAll("target reached");
      int nxt = (g_preset == PRESET_PHASE1 ? PRESET_PHASE2 : PRESET_FUNDED);
      PrintFormat("v13 TESTER: %s target reached %s, equity %.2f (%+.2f%% on the stage) -> %s starts from this balance",
                  PresetName(g_preset), TimeToString(now, TIME_DATE), eq, 100.0 * (eq / g_init_bal - 1.0), PresetName(nxt));
      g_init_bal = AccountInfoDouble(ACCOUNT_BALANCE); GvSet("init", g_init_bal);
      SetPreset(nxt); g_target_hit = false; GvSet("stall_" + PresetName(nxt), 0);
      StageReset();
   }
   else if(tgt > 0 && !g_target_hit && eq >= g_init_bal * (1.0 + tgt / 100.0))
   {
      if(InpCloseOnTarget) CloseAll("target reached");
      g_target_hit = true; GvSet("target_" + PresetName(g_preset), 1);
      Print("v13 TARGET reached for ", PresetName(g_preset));
      StageSummary("TARGET REACHED");
   }
   // weekend flat (optional)
   if(InpCloseBeforeWeekend && DowOf(DayNum(now)) == 5 && HourOf(now) >= 21) CloseAll("weekend");
   // minimum trading days helper (after the target, one micro trade per day)
   if(g_target_hit && InpMinDaysHelper && g_trading_days < InpMinTradingDays && HourOf(now) >= 10 && HourOf(now) < 20
      && IsWeekday(DayNum(now)) && g_last_helper_day != g_day)
      MinDayTrade();
}

void MinDayTrade()
{
   string sym = (g_sym_ok[2] ? g_sym[2] : (g_sym_ok[3] ? g_sym[3] : (g_sym_ok[0] ? g_sym[0] : "")));
   if(sym == "") return;
   long tm = SymbolInfoInteger(sym, SYMBOL_TRADE_MODE);
   if(tm != SYMBOL_TRADE_MODE_FULL) return;
   int hr = (sym == g_sym[2] ? 2 : (sym == g_sym[3] ? 3 : 0));
   if(!MarketOpen(hr)) return;                 // v13.3: closed - tried again next minute (nothing sent)
   g_trade.SetExpertMagicNumber(InpMagicBase + 99);
   g_trade.SetTypeFillingBySymbol(sym);
   double v = SymbolInfoDouble(sym, SYMBOL_VOLUME_MIN);
   if(g_trade.Buy(v, sym, SymbolInfoDouble(sym, SYMBOL_ASK), 0, 0, "v13 min-day"))
   {
      ulong pos_ticket = 0;
      for(int i = PositionsTotal() - 1; i >= 0; i--)
      {
         ulong t = PositionGetTicket(i);
         if(t != 0 && PositionGetInteger(POSITION_MAGIC) == InpMagicBase + 99) { pos_ticket = t; break; }
      }
      if(pos_ticket != 0) { g_trade.SetTypeFillingBySymbol(sym); g_trade.PositionClose(pos_ticket); }
      g_last_helper_day = g_day; GvSet("helper", (double)g_day);
      g_trading_days = CountTradingDays();
      PrintFormat("v13 minimum-days helper trade on %s (days %d/%d)", sym, g_trading_days, InpMinTradingDays);
   }
}


//--------------------------------------------------------------------- live sleeve conditions (display only, v11.8)
// Nothing here trades or changes a trading decision: it re-reads the same bars and features to tell the user how far each
// sleeve is from its entry condition.  Levels are rebuilt on every processed H1 bar, distances to the price every 10 s.
string Px(const int role, const double p) { return DoubleToString(p, (int)SymbolInfoInteger(g_sym[role], SYMBOL_DIGITS)); }
string DDMM(const datetime t) { MqlDateTime d; TimeToStruct(t, d); return StringFormat("%02d.%02d", d.day, d.mon); }
string SrvTxt(const datetime t) { return DDMM(t) + " " + HHMM(t); }
string TfName(const int mn) { return mn == 15 ? "M15" : mn == 30 ? "M30" : mn == 60 ? "H1" : mn == 120 ? "H2" : mn == 180 ? "H3" : mn == 240 ? "H4" : mn == 360 ? "H6" : mn == 480 ? "H8" : mn == 720 ? "H12" : mn == 10080 ? "W1" : "D1"; }
string RegDesc(const int k)                 // the sleeve's trend filter in words (v12)
{
   int rg = SL_REG[k]; string t = TfName(SL_RTF[k]), a = StringFormat("ADX>%.0f", SL_ADXT[k]);
   if(rg == 1) return t + " " + a;
   if(rg == 2 || rg == 3) return t + " close on the trade side of EMA200";
   if(rg == 4) return t + " " + a + " and above EMA200";
   if(rg == 5) return t + " EMA200 sloping with the trade";
   if(rg == 6) return t + " above/below a rising/falling EMA200";
   if(rg == 7) return t + StringFormat(" ADX<%.0f", SL_ADXT[k]);
   if(rg == 8) return t + " " + a + " with the EMA200 side";
   if(rg == 9) return t + StringFormat(" ADX<%.0f", SL_ADXT[k]) + " with the EMA200 side";
   return "no trend filter";
}
string RegTxt(const int k, const int dir)   // why the regime filter blocks this side ("" = allowed)
{
   if(RegimeAllows(k, dir)) return "";
   DayFeat f = g_df[SL_ROLE[k]];
   if(!f.ok) return "no D1 data";
   double cl = f.close, ema = f.ema200, adx = f.adx, slope = f.slope;
   if(SL_RTF[k] != 1440 && !TfFeat(SL_ROLE[k], SL_RTF[k], cl, ema, adx, slope)) return TfName(SL_RTF[k]) + " not ready";
   string t = TfName(SL_RTF[k]) + " ";
   int rg = SL_REG[k];
   if((rg == 1 || rg == 4 || rg == 8) && adx <= SL_ADXT[k]) return t + StringFormat("ADX %.0f<%.0f", adx, SL_ADXT[k]);
   if((rg == 7 || rg == 9) && adx >= SL_ADXT[k]) return t + StringFormat("ADX %.0f>%.0f", adx, SL_ADXT[k]);
   if(rg == 5 || (rg == 6 && (cl - ema) * dir > 0)) return t + (dir > 0 ? "EMA200 falling" : "EMA200 rising");
   return t + (dir > 0 ? "<EMA200" : ">EMA200");
}
datetime NyToSrv(const long ny_dn, const int ny_mins, const datetime srv)    // server time of a New York wall-clock time (today's offsets)
{
   int off = ServerUTCOffsetSec(srv); datetime utc = (datetime)(srv - off);
   return (datetime)(ny_dn * 86400 + ny_mins * 60 - NYOffsetSec(utc) + off);
}
string DistTxt(const double d) { return (d <= 0 ? "at level" : StringFormat("%.1f ATR", d)); }

void SleeveInfo(const int k, const bool recalc)
{
   g_cur_role = SL_ROLE[k];
   int role = SL_ROLE[k];
   g_inf_near[k] = false;
   if(!g_sym_ok[role]) { g_inf_s[k] = ""; g_inf_f[k] = "no symbol on this server"; g_lv_up[k] = 0; g_lv_dn[k] = 0; return; }
   if(!g_use[k]) { g_inf_s[k] = ""; g_inf_f[k] = "off (input)"; g_lv_up[k] = 0; g_lv_dn[k] = 0; return; }
   string sym = g_sym[role];
   DayFeat f = g_df[role];
   if(!f.ok)
   {
      g_inf_s[k] = StringFormat("D1 %d/%d", g_df_n[role], DemoActive() ? 130 : 260);
      g_inf_f[k] = StringFormat("waiting for daily history: %d daily bars available, %d needed", g_df_n[role], DemoActive() ? 130 : 260);
      g_lv_up[k] = 0; g_lv_dn[k] = 0; return;
   }
   double bid = SymbolInfoDouble(sym, SYMBOL_BID), atr = MathMax(f.atr, 1e-12);
   datetime srv = TimeCurrent();
   int kind = SL_KIND[k];
   string a = "", b = "";
   if(kind == 0 || kind == 6 || kind == 7)                 // channel / band breakouts: distance to the level
   {
      if(recalc || g_lv_t0[k] == 0)
      {
         MArr<MqlRates> r;
         g_lv_up[k] = 0; g_lv_dn[k] = 0; g_lv_t0[k] = 0;
         if(kind == 0)
         {
            int n = CopyRates(sym, PERIOD_H1, 1, SL_N[k], r);
            if(n == SL_N[k])
            {
               double hh = -1e300, ll = 1e300;
               for(int i = 0; i < n; i++) { hh = MathMax(hh, r[i].high); ll = MathMin(ll, r[i].low); }
               g_lv_up[k] = hh; g_lv_dn[k] = (SL_SIDE[k] == 0 ? ll : 0); g_lv_t0[k] = r[0].time;
            }
         }
         else
         {
            int n = GetH4(sym, 800);
            if(n >= 260)
            {
               double ema = g_r[0].close, ema_10 = 0;
               for(int i = 1; i < n; i++) { ema = ema + (g_r[i].close - ema) * 2.0 / 201.0; if(i == n - 11) ema_10 = ema; }
               double c1 = g_r[n - 1].close;
               if(kind == 6)
               {
                  int N = SL_N[k]; double m = 0, q = 0;
                  for(int i = n - N; i < n; i++) { m += g_r[i].close; q += g_r[i].close * g_r[i].close; }
                  m /= N; double sd = MathSqrt(MathMax(q / N - m * m, 0));
                  if(c1 > ema) g_lv_up[k] = m + SL_P[k] * sd;            // buys only above the H4 EMA200, sells only below
                  else g_lv_dn[k] = m - SL_P[k] * sd;
                  g_lv_t0[k] = g_r[n - N].time;
               }
               else
               {
                  int N = SL_N[k]; double hh = -1e300;
                  for(int i = n - N; i < n; i++) hh = MathMax(hh, g_r[i].high);
                  if(c1 > ema && ema - ema_10 > 0) g_lv_up[k] = hh;
                  g_lv_t0[k] = g_r[n - N].time;
               }
            }
         }
      }
      string tf = (kind == 0 ? "H1" : "H4");
      string ru = (g_lv_up[k] > 0 ? RegTxt(k, 1) : ""), rd = (g_lv_dn[k] > 0 && SL_SIDE[k] == 0 ? RegTxt(k, -1) : "");
      bool up = (g_lv_up[k] > 0 && ru == ""), dn = (g_lv_dn[k] > 0 && SL_SIDE[k] == 0 && rd == "");
      double du = (g_lv_up[k] - bid) / atr, dd = (bid - g_lv_dn[k]) / atr;
      if(up && (!dn || du <= dd)) { a = "▲ " + DistTxt(du); g_inf_near[k] = du < 0.5; }
      else if(dn) { a = "▼ " + DistTxt(dd); g_inf_near[k] = dd < 0.5; }
      else if(ru != "") a = ru;
      else if(rd != "") a = rd;
      else a = (kind == 6 ? "EMA side" : (kind == 7 ? "EMA200 H4" : "-"));
      if(g_lv_up[k] > 0) b += "buy " + tf + " close > " + Px(role, g_lv_up[k]) + " (" + DistTxt(du) + ")";
      if(g_lv_dn[k] > 0 && SL_SIDE[k] == 0 && b != "") b += " | ";
      if(g_lv_dn[k] > 0 && SL_SIDE[k] == 0) b += "sell < " + Px(role, g_lv_dn[k]) + " (" + DistTxt(dd) + ")";
      string blk = (ru != "" ? ru : rd);
      if(blk != "") b += " | blocked: " + blk;
      if(b == "") b = (kind == 7 ? "needs " + tf + " close > rising H4 EMA200 first" : (kind == 6 ? "band not ready" : "channel not ready"));
      if(SL_TPK[k] > 0) b += StringFormat(" | TP %.0f ATR", SL_TPK[k]);
   }
   else if(kind == 1)                                      // H1 EMA20/EMA100 cross (g_lv_up/dn hold the two EMAs; not drawn)
   {
      if(recalc || g_lv_t0[k] == 0)
      {
         MArr<MqlRates> r;
         int n = CopyRates(sym, PERIOD_H1, 1, 1500, r);
         if(n >= 300)
         {
            double ef = r[0].close, es = r[0].close;
            for(int i = 1; i < n; i++) { ef = ef + (r[i].close - ef) * 2.0 / 21.0; es = es + (r[i].close - es) * 2.0 / 101.0; }
            g_lv_up[k] = ef; g_lv_dn[k] = es; g_lv_t0[k] = r[n - 1].time;
         }
      }
      if(g_lv_t0[k] > 0)
      {
         int nd = (g_lv_up[k] > g_lv_dn[k] ? -1 : 1);
         double gap = MathAbs(g_lv_up[k] - g_lv_dn[k]) / atr;
         string rg = RegTxt(k, nd);
         a = (rg != "" ? rg : (nd > 0 ? "▲ " : "▼ ") + StringFormat("%.1f ATR", gap));
         g_inf_near[k] = (rg == "" && gap < 0.2);
         b = StringFormat("H1 EMA20/100 gap %.2f ATR, next = cross %s", gap, nd > 0 ? "up" : "down") + " if " + RegDesc(k) + (rg != "" ? " | blocked: " + rg : "");
      }
      else { a = "-"; b = "EMA not ready"; }
   }
   else if(kind == 2)
   {
      string rg = RegTxt(k, f.ret120 >= 0 ? 1 : -1);
      a = (rg != "" ? rg : StringFormat("%s %+.0f%%", f.ret120 >= 0 ? "▲" : "▼", 100.0 * f.ret120));
      b = StringFormat("01:00 daily: 120d return %+.1f%% -> %s if %s; time stop %d h, half off at %.1fR", 100.0 * f.ret120, f.ret120 >= 0 ? "long" : "short", RegDesc(k), SL_MAXB[k], SL_PTR[k]);
   }
   else if(kind == 3)
   {
      long d0 = DayNum(srv); string nx = "-";
      for(int x = 0; x < 45; x++)
      {
         long dn = d0 + x;
         if(!IsWeekday(dn) || WeekdaysLeftAfter((datetime)(dn * 86400)) != 1 || (x == 0 && HourOf(srv) >= 3)) continue;
         nx = DDMM((datetime)(dn * 86400)); break;
      }
      string rg = RegTxt(k, 1);
      a = nx + (rg != "" ? " " + rg : "");
      b = "buy " + nx + " 02:00 (2nd-last weekday) if D1 close > EMA200" + (rg != "" ? " (now below)" : " (now above)") + "; exit from the 4th weekday";
   }
   else if(kind == 4 || kind == 5)
   {
      bool rsi = (kind == 4);
      double v = (rsi ? f.rsi2 : f.ibs);
      a = (rsi ? StringFormat("RSI2 %.0f", v) : StringFormat("IBS %.2f", v));
      g_inf_near[k] = (rsi ? v < SL_P[k] + 10 : v < SL_P[k] + 0.1);
      b = (rsi ? StringFormat("01:00 daily: buy if RSI2 < %.0f (now %.1f); exit close > SMA5", SL_P[k], v)
               : StringFormat("01:00 daily: buy if IBS < %.2f (now %.2f); exit IBS > 0.80", SL_P[k], v));
      if(SL_REG[k] > 0) { string rg = RegTxt(k, 1); b += " | filter " + RegDesc(k) + (rg != "" ? " (blocked: " + rg + ")" : ""); }
      if(SL_WKD[k] > 0) b += " | flat Friday 15:00 NY";
   }
   else if(kind == 8)
   {
      int off = ServerUTCOffsetSec(srv); datetime utc = (datetime)(srv - off);
      long d0 = DayNum(utc); int mins = (int)((utc % 86400) / 60); datetime nx = 0;
      for(int x = 0; x < 20 && nx == 0; x++)
      {
         long dn = d0 + x; int dw = DowOf(dn);
         if(dw < 1 || dw > 4 || !IsGotobiDay(dn + 1) || (x == 0 && mins >= 16 * 60 + 25)) continue;
         nx = (datetime)(dn * 86400 + 15 * 3600 + 55 * 60 + off);
      }
      a = (nx > 0 ? SrvTxt(nx) : "-");
      b = "buy " + a + " server (15:55 UTC, eve of a Gotobi fix), exit 00:55 UTC";
   }
   else if(kind == 9 || kind == 14)
   {
      int off = ServerUTCOffsetSec(srv); datetime utc = (datetime)(srv - off), ny = (datetime)(utc + NYOffsetSec(utc));
      long d0 = DayNum(ny); int mins = (int)((ny % 86400) / 60); long nd = -1;
      for(int x = 0; x < 40 && nd < 0; x++)
      {
         long dn = d0 + x;
         if(kind == 9 ? !IsThirdFriday(dn) : !IsWeekday(dn)) continue;
         if(x == 0 && mins >= 9 * 60 + 45) continue;
         nd = dn;
      }
      datetime ts = (nd >= 0 ? NyToSrv(nd, 9 * 60 + 30, srv) : 0);
      if(kind == 9)
      {
         a = (ts > 0 ? DDMM(ts) : "-");
         b = "3rd Friday " + (ts > 0 ? SrvTxt(ts) : "-") + " server (09:30 NY): sell if above Thu 16:00 NY, exit 12:00 NY";
      }
      else
      {
         bool today = (g_gl_zday == d0);
         a = (today ? StringFormat("gap z %+.1f", g_gl_z) : (ts > 0 ? HHMM(ts) : "-"));
         g_inf_near[k] = (today && g_gl_z <= -1.5);
         b = "weekdays 09:30 NY (" + (ts > 0 ? HHMM(ts) : "-") + " server): buy if opening gap z <= -2" + (today ? StringFormat(" (today %+.2f)", g_gl_z) : "");
      }
   }
   else if(kind == 11)
   {
      a = StringFormat("z %+.1f", f.z50);
      g_inf_near[k] = MathAbs(f.z50) >= SL_P[k] - 0.3;
      b = StringFormat("02:00 daily: 50-day z = %+.2f; sell z >= +%.1f, buy z <= -%.1f, exit at the mean", f.z50, SL_P[k], SL_P[k]);
   }
   else if(kind == 15)
   {
      a = "-";
      b = "FOMC day 14:06-16:00 New York: short after the statement (dates: MT5 calendar / web / table)";
   }
   else if(kind == 16)
   {
      a = "-";
      b = "02:00 daily: fade a |z| >= " + DoubleToString(SL_P[k], 1) + " DXY extreme on USDJPY, 48 h hold (needs EURUSD / GBPUSD / USDCAD / USDCHF)";
   }
   else if(kind == 17)
   {
      a = StringFormat("z %+.1f", f.z50);
      g_inf_near[k] = (f.z50 >= SL_P[k] - 0.3 && f.close > f.ema200);
      b = StringFormat("02:00 daily: buy if the 50-day z >= +%.1f above the EMA200, 5-day exit (now z %+.2f)", SL_P[k], f.z50);
   }
   else { a = "-"; b = "Tokyo-fix reversal (calendar)"; }
   g_inf_s[k] = a; g_inf_f[k] = b;
}

//--------------------------------------------------------------------- panel (create once, write on change)
const string PNL = "DP11_";
#define PW 360









color CBG()   { return MQL_RGB(17,21,30); }
color CCARD() { return MQL_RGB(27,33,45); }
color CTXT()  { return MQL_RGB(226,232,240); }
color CSUB()  { return MQL_RGB(139,151,170); }
color CGRN()  { return MQL_RGB(52,199,120); }
color CRED()  { return MQL_RGB(239,83,80); }
color CAMB()  { return MQL_RGB(245,175,25); }
color CCYA()  { return MQL_RGB(56,189,248); }
color CBTN()  { return MQL_RGB(44,54,72); }

string Tr(const string key)
{
   bool fa = (g_lang == LANG_FA);
   if(key == "title")   return "DonchianProp v" + (string)EA_VER;
   if(key == "active")  return fa ? "فعال" : "ACTIVE";
   if(key == "paused")  return fa ? "ورود متوقف" : "PAUSED";
   if(key == "dhalt")   return fa ? "توقف تا فردا (حد روزانه)" : "DAY HALT (daily guard)";
   if(key == "mhalt")   return IsPersonal() ? (fa ? "توقف کامل (افت از سقف) - Reset دستی" : "HALTED (max-drawdown stop) - manual Reset") : (fa ? "توقف کامل (حد ضرر کل)" : "HALTED (max-loss guard)");
   if(key == "target")  return fa ? "هدف رسید" : "TARGET REACHED";
   if(key == "soft")    return fa ? "توقف نرم امروز" : "SOFT STOP today";
   if(key == "newsw")   return fa ? "پنجره خبر مهم" : "NEWS WINDOW";
   if(key == "news")    return fa ? "منتظر عبور خبر" : "news window";
   if(key == "loss room") return fa ? "جای ضرر کل کم" : "no max-loss room";
   if(key == "bal")     return fa ? "بالانس" : "Balance";
   if(key == "eq")      return fa ? "اکوییتی" : "Equity";
   if(key == "day")     return fa ? "ضرر امروز" : "Daily loss";
   if(key == "dd")      return fa ? "افت از سرمایه اولیه" : "Overall loss";
   if(key == "prog")    return fa ? "پیشرفت هدف" : "Target";
   if(key == "orisk")   return fa ? "ریسک باز" : "Open risk";
   if(key == "days")    return fa ? "روز معاملاتی" : "Trading days";
   if(key == "risk")    return fa ? "ریسک پایه" : "Risk unit";
   if(key == "pause")   return fa ? "توقف ورود" : "PAUSE";
   if(key == "resume")  return fa ? "ادامه" : "RESUME";
   if(key == "close")   return fa ? "بستن همه" : "CLOSE ALL";
   if(key == "confirm") return fa ? "تایید؟" : "CONFIRM?";
   if(key == "reset")   return fa ? "رفع توقف" : "RESET";
   if(key == "p1")      return fa ? "فاز ۱" : "PHASE 1";
   if(key == "p2")      return fa ? "فاز ۲" : "PHASE 2";
   if(key == "fu")      return fa ? "فاند" : "FUNDED";
   if(key == "cu")      return fa ? "سفارشی" : "CUSTOM";
   if(key == "rm_s")    return fa ? "امن" : "SAFE";
   if(key == "rm_f")    return fa ? "سریع" : "FAST";
   if(key == "rm_t")    return fa ? "توربو" : "TURBO";
   if(key == "up")      return fa ? "صعودی" : "UP";
   if(key == "down")    return fa ? "نزولی" : "DOWN";
   if(key == "range")   return fa ? "رنج" : "RANGE";
   if(key == "nosym")   return fa ? "نماد ندارد" : "no symbol";
   if(key == "wait")    return fa ? "منتظر سیگنال" : "waiting";
   if(key == "off")     return fa ? "خاموش" : "off";
   if(key == "regime")  return fa ? "رژیم مناسب نیست" : "regime filter";
   if(key == "min-lot risk") return fa ? "حداقل لات پرریسک" : "min-lot too risky";
   if(key == "risk cap")     return fa ? "سقف ریسک باز" : "open-risk cap";
   if(key == "daily room")   return fa ? "جای ضرر روزانه کم" : "no daily room";
   if(key == "spread")       return fa ? "اسپرد بالا" : "spread high";
   if(key == "soft stop")    return fa ? "توقف نرم" : "soft stop";
   if(key == "halt")         return fa ? "محافظ فعال" : "guard";
   if(key == "target_s")     return fa ? "هدف رسید" : "target";
   if(key == "margin")       return fa ? "مارجین" : "margin";
   if(key == "market closed")return fa ? "بازار بسته" : "market closed";
   if(key == "algo off")     return fa ? "الگو خاموش" : "algo off";
   if(key == "order failed") return fa ? "خطای سفارش" : "order failed";
   if(key == "symbol info")  return fa ? "اطلاعات نماد" : "symbol info";
   if(key == "close retry")  return fa ? "بستن رد شد، دوباره تلاش می‌کند" : "close rejected, retrying";
   if(key == "market closed - queued") return fa ? "بازار بسته؛ ورود در صف بازگشایی" : "market closed, entry queued for the reopen";
   if(key == "paused: WebRequest") return fa ? "متوقف تا مجاز شدن WebRequest" : "paused until WebRequest is allowed";
   if(key == "close deferred") return fa ? "بازار بسته؛ بستن در صف بازگشایی" : "market closed, close queued for the reopen";
   if(key == "openpos")      return fa ? "معاملات باز" : "Open positions";
   if(key == "nopos")        return fa ? "معامله‌ی بازی نیست" : "No open positions";
   if(key == "buy")          return fa ? "خرید" : "BUY";
   if(key == "sell")         return fa ? "فروش" : "SELL";
   if(key == "minimize")     return fa ? "کوچک کردن پنل" : "Minimise panel";
   if(key == "expand")       return fa ? "باز کردن پنل" : "Expand panel";
   if(key == "idle")         return fa ? "غیرفعال: نسخه‌ی دیگری روی این حساب اجراست" : "IDLE: another copy runs";
   if(key == "algo")         return fa ? "معامله‌ی الگو (AutoTrading) خاموش است" : "ALGO TRADING OFF";
   if(key == "scan")         return fa ? "آخرین بررسی ساعتی" : "Last hourly scan";
   if(key == "nday")         return fa ? "بررسی روزانه بعدی" : "Next daily check";
   if(key == "clock")        return fa ? "ساعت سرور، اختلاف تشخیص‌داده‌شده با UTC و ساعت نیویورک. ساعت سرور باید با ساعت Market Watch یکی باشد و ساعت نیویورک با ساعت واقعی نیویورک."
                                       : "Server time, detected offset to UTC and New York time. Server time must match Market Watch; NY time must match the real New York clock.";
   return key;
}

void PObj(const string name, const int type)
{
   if(ObjectFind(0, name) >= 0) return;
   ResetLastError();
   if(!ObjectCreate(0, name, (ENUM_OBJECT)type, 0, 0, 0)) { int e = GetLastError(); if(e != 4022 && !IsStopped()) PrintFormat("v13 panel: cannot create %s (error %d)", name, e); }
   ObjectSetInteger(0, name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
   ObjectSetInteger(0, name, OBJPROP_SELECTABLE, 0);
   ObjectSetInteger(0, name, OBJPROP_HIDDEN, 1);
   ObjectSetInteger(0, name, OBJPROP_BACK, 0);
}
void PRect(const string name, const int x, const int y, const int w, const int h, const color bg)
{
   PObj(name, OBJ_RECTANGLE_LABEL);
   ObjectSetInteger(0, name, OBJPROP_XDISTANCE, x); ObjectSetInteger(0, name, OBJPROP_YDISTANCE, y);
   ObjectSetInteger(0, name, OBJPROP_XSIZE, w); ObjectSetInteger(0, name, OBJPROP_YSIZE, h);
   ObjectSetInteger(0, name, OBJPROP_BGCOLOR, bg); ObjectSetInteger(0, name, OBJPROP_BORDER_TYPE, BORDER_FLAT);
   ObjectSetInteger(0, name, OBJPROP_COLOR, bg);
}
void PLabel(const string name, const int x, const int y, const int fs)
{
   PObj(name, OBJ_LABEL);
   ObjectSetInteger(0, name, OBJPROP_XDISTANCE, x); ObjectSetInteger(0, name, OBJPROP_YDISTANCE, y);
   ObjectSetInteger(0, name, OBJPROP_FONTSIZE, fs); ObjectSetString(0, name, OBJPROP_FONT, "Tahoma");
   ObjectSetString(0, name, OBJPROP_TEXT, " ");
}
void PButton(const string name, const int x, const int y, const int w, const int h)
{
   PObj(name, OBJ_BUTTON);
   ObjectSetInteger(0, name, OBJPROP_XDISTANCE, x); ObjectSetInteger(0, name, OBJPROP_YDISTANCE, y);
   ObjectSetInteger(0, name, OBJPROP_XSIZE, w); ObjectSetInteger(0, name, OBJPROP_YSIZE, h);
   ObjectSetInteger(0, name, OBJPROP_FONTSIZE, 8); ObjectSetString(0, name, OBJPROP_FONT, "Tahoma");
   ObjectSetInteger(0, name, OBJPROP_BORDER_COLOR, MQL_RGB(70,82,104));
}
// change-detected setters: one cache entry per (object, property)
bool PCache(const string key, const string val)
{
   int n = ArraySize(g_pk);
   for(int i = 0; i < n; i++) if(g_pk[i] == key) { if(g_pv[i] == val) return false; g_pv[i] = val; return true; }
   ArrayResize(g_pk, n + 1); ArrayResize(g_pv, n + 1); g_pk[n] = key; g_pv[n] = val; return true;
}
bool PText(const string name, const string txt, const color c)
{
   bool ch = false;
   if(PCache(name + "|t", txt)) { ObjectSetString(0, name, OBJPROP_TEXT, txt); ch = true; }
   if(PCache(name + "|c", IntegerToString((long)c))) { ObjectSetInteger(0, name, OBJPROP_COLOR, c); ch = true; }
   return ch;
}
bool PBg(const string name, const color c)
{
   if(!PCache(name + "|b", IntegerToString((long)c))) return false;
   ObjectSetInteger(0, name, OBJPROP_BGCOLOR, c); return true;
}
bool PWidth(const string name, const int w)
{
   if(!PCache(name + "|w", IntegerToString(w))) return false;
   ObjectSetInteger(0, name, OBJPROP_XSIZE, MathMax(w, 1)); return true;
}

#define CHIPW 64
#define GRIDH 16
#define NPOSROW 3




string ShortSt(const string key)     // compact sleeve status for the two-column grid (full text in the tooltip)
{
   bool fa = (g_lang == LANG_FA);
   if(key == "wait")          return fa ? "منتظر" : "waiting";
   if(key == "off")           return fa ? "خاموش" : "off";
   if(key == "nosym")         return fa ? "بی‌نماد" : "no symbol";
   if(key == "regime")        return fa ? "رژیم" : "regime";
   if(key == "min-lot risk")  return fa ? "حداقل لات" : "min lot";
   if(key == "risk cap")      return fa ? "سقف ریسک" : "risk cap";
   if(key == "daily room")    return fa ? "حد روزانه" : "daily cap";
   if(key == "loss room")     return fa ? "حد کل" : "max loss";
   if(key == "spread")        return fa ? "اسپرد" : "spread";
   if(key == "news")          return fa ? "خبر" : "news";
   if(key == "gold cap")      return fa ? "سقف طلا" : "gold cap";
   if(key == "margin")        return fa ? "مارجین" : "margin";
   if(key == "market closed") return fa ? "بازار بسته" : "closed";
   if(key == "order failed")  return fa ? "خطا" : "error";
   if(key == "target_s")      return fa ? "هدف" : "target";
   if(key == "soft stop")     return fa ? "توقف نرم" : "soft stop";
   if(key == "halt")          return fa ? "محافظ" : "guard";
   if(key == "algo off")      return fa ? "الگو خاموش" : "algo off";
   if(key == "symbol info")   return fa ? "نماد" : "sym info";
   if(key == "close retry")   return fa ? "بستن دوباره" : "close retry";
   if(key == "market closed - queued") return fa ? "در صف" : "queued";
   if(key == "paused: WebRequest") return fa ? "متوقف" : "paused";
   if(key == "close deferred") return fa ? "بستن در صف" : "close queued";
   return key;
}

void PLabelA(const string name, const int x, const int y, const int fs, const int anchor)
{
   PLabel(name, x, y, fs);
   ObjectSetInteger(0, name, OBJPROP_ANCHOR, anchor);
}
bool PTip(const string name, const string tip)
{
   if(!PCache(name + "|tip", tip)) return false;
   ObjectSetString(0, name, OBJPROP_TOOLTIP, tip); return false;    // a tooltip needs no redraw
}

string HHMM(const datetime t) { long d = (long)t % 86400; return StringFormat("%02d:%02d", (int)(d / 3600), (int)((d % 3600) / 60)); }
string ClockText()                 // server clock, the UTC offset the EA uses, and New York time derived from it
{
   g_cur_role = (g_sym_ok[0] ? 0 : -1);
   datetime srv = IsTester() ? TimeCurrent() : TimeTradeServer();
   int off = ServerUTCOffsetSec(srv), ao = MathAbs(off);
   datetime utc = (datetime)(srv - off), ny = (datetime)(utc + NYOffsetSec(utc));
   string o = "UTC";
   o += (off >= 0 ? "+" : "-");
   o += IntegerToString(ao / 3600);
   if(ao % 3600 != 0) o += ":30";
   if(InpServerGMTOffset != -99) o += "*";              // * = offset set by hand
   else if(g_tz_ok) o += "~";                           // ~ = read from the tester data (DEMO)
   return "Server " + HHMM(srv) + "  " + o + "  |  NY " + HHMM(ny);
}

void PanelBuild()
{
   if(!InpShowPanel || (IsTester() && !MQLInfoInteger(MQL_VISUAL_MODE))) return;
   ObjectsDeleteAll(0, PNL);
   int x = InpPanelX, y = InpPanelY;
   int rows = (NSLV + 1) / 2;
   int y_chip = y + 206, y_sh = y_chip + 38, y_grid = y_sh + 18, y_scan = y_grid + (g_smin ? 0 : rows * GRIDH) + 6, y_pos = y_scan + 24, y_btn = y_pos + 20 + NPOSROW * GRIDH + 8;
   g_panel_fpw = g_fp_paused;              // v13.6: FED POST WebRequest warning (3 lines under the buttons) while paused
   int y_fpw = y_btn + 22 + 6, h = g_pmin ? (g_fp_paused ? 46 + 40 : 46) : y_btn + 22 + 8 - y + (g_fp_paused ? 42 : 0);
   if(g_pmin) y_fpw = y + 44;
   PRect(PNL + "bg", x, y, PW, h, CBG());
   PLabel(PNL + "title", x + 10, y + 6, 9);
   PButton(PNL + "b_min", x + PW - 28, y + 5, 20, 17);
   PLabelA(PNL + "clock", x + PW - 36, y + 8, 8, ANCHOR_RIGHT_UPPER);
   PLabel(PNL + "status", x + 10, y + 26, 9);
   PLabel(PNL + "preset", x + 270, y + 26, 9);
   if(g_pmin) PLabel(PNL + "eqm", x + 150, y + 28, 8);
   if(!g_pmin)
   {
      PLabel(PNL + "bal_l", x + 10, y + 44, 8);  PLabel(PNL + "bal_v", x + 70, y + 44, 8);
      PLabel(PNL + "eq_l", x + 185, y + 44, 8);  PLabel(PNL + "eq_v", x + 245, y + 44, 8);
      PLabel(PNL + "risk_l", x + 10, y + 60, 8); PLabel(PNL + "risk_v", x + 70, y + 60, 8);
      PLabel(PNL + "days_l", x + 185, y + 60, 8); PLabel(PNL + "days_v", x + 265, y + 60, 8);
      PButton(PNL + "b_rm", x + PW - 58, y + 57, 50, 18);
      int bx = x + 10, bw = 82;
      PButton(PNL + "b_p1", bx, y + 78, bw, 20); PButton(PNL + "b_p2", bx + 86, y + 78, bw, 20);
      PButton(PNL + "b_fu", bx + 172, y + 78, bw, 20); PButton(PNL + "b_cu", bx + 258, y + 78, bw - 2, 20);
      int gy = y + 106;
      string gn[4] = {"g_day", "g_dd", "g_tg", "g_or"};
      for(int i = 0; i < 4; i++)
      {
         PLabel(PNL + gn[i] + "_l", x + 10, gy + i * 24, 8);
         PLabel(PNL + gn[i] + "_v", x + 200, gy + i * 24, 8);
         PRect(PNL + gn[i] + "_bg", x + 10, gy + i * 24 + 15, PW - 20, 5, CCARD());
         PRect(PNL + gn[i] + "_fg", x + 10, gy + i * 24 + 15, 1, 5, CGRN());
      }
      // market regime chips: name on top, arrow + word below, both centred (no mixed-direction text on one line)
      for(int r = 0; r < NREG; r++)
      {
         string rs = IntegerToString(r);
         int cx = x + 10 + r * (CHIPW + 5);
         PRect(PNL + "rc" + rs, cx, y_chip, CHIPW, 32, CCARD());
         PRect(PNL + "ra" + rs, cx, y_chip, 3, 32, CSUB());
         PLabelA(PNL + "rn" + rs, cx + CHIPW / 2 + 1, y_chip + 3, 7, ANCHOR_UPPER);
         PLabelA(PNL + "rv" + rs, cx + CHIPW / 2 + 1, y_chip + 15, 8, ANCHOR_UPPER);
      }
      // strategies: header with an open/close button (v13.1), then two columns, status right-aligned in its column
      PLabel(PNL + "sh", x + 10, y_sh + 2, 8);
      PButton(PNL + "b_smin", x + PW - 28, y_sh, 20, 16);
      if(!g_smin) PRect(PNL + "gsep", x + 180, y_grid, 1, rows * GRIDH - 2, CCARD());
      for(int k = 0; k < NSLV && !g_smin; k++)
      {
         string ks = IntegerToString(k);
         int cx = x + 10 + (k / rows) * 175, cy = y_grid + (k % rows) * GRIDH;
         PLabel(PNL + "sd" + ks, cx, cy + 1, 6);
         PLabel(PNL + "sl" + ks + "_n", cx + 11, cy, 8);
         PLabelA(PNL + "sl" + ks + "_s", cx + 163, cy, 8, ANCHOR_RIGHT_UPPER);
      }
      // heartbeat: the last hourly scan proves the EA is processing bars
      PRect(PNL + "ssep", x + 10, y_scan - 3, PW - 20, 1, CCARD());
      PLabel(PNL + "scan_l", x + 10, y_scan + 1, 7); PLabel(PNL + "scan_v", x + 118, y_scan + 1, 7);
      PLabel(PNL + "nday_l", x + 190, y_scan + 1, 7); PLabelA(PNL + "nday_v", x + PW - 12, y_scan + 1, 7, ANCHOR_RIGHT_UPPER);
      // open positions: three rows, scroll buttons when there are more
      PRect(PNL + "psep", x + 10, y_pos - 5, PW - 20, 1, CCARD());
      PLabel(PNL + "ph", x + 10, y_pos, 8);
      PLabelA(PNL + "pc", x + PW - 58, y_pos, 8, ANCHOR_RIGHT_UPPER);
      PButton(PNL + "b_up", x + PW - 52, y_pos - 1, 20, 16); PButton(PNL + "b_dn", x + PW - 30, y_pos - 1, 20, 16);
      for(int i = 0; i < NPOSROW; i++)
      {
         string is = IntegerToString(i);
         int ry = y_pos + 20 + i * GRIDH;
         PRect(PNL + "pr" + is, x + 10, ry - 1, PW - 20, GRIDH - 1, CBG());
         PLabel(PNL + "p" + is + "_s", x + 16, ry, 8);
         PLabel(PNL + "p" + is + "_d", x + 112, ry, 8);
         PLabel(PNL + "p" + is + "_l", x + 152, ry, 8);
         PLabel(PNL + "p" + is + "_k", x + 196, ry + 1, 7);
         PLabelA(PNL + "p" + is + "_p", x + PW - 16, ry, 8, ANCHOR_RIGHT_UPPER);
      }
      PButton(PNL + "b_pause", bx, y_btn, 100, 22); PButton(PNL + "b_close", bx + 106, y_btn, 100, 22);
      PButton(PNL + "b_reset", bx + 212, y_btn, 70, 22); PButton(PNL + "b_lang", bx + 288, y_btn, 52, 22);
   }
   if(g_fp_paused) for(int i = 0; i < 3; i++) PLabel(PNL + "fpw" + IntegerToString(i), x + 10, y_fpw + i * 13, 7);
   g_panel = true;
   ArrayResize(g_pk, 0); ArrayResize(g_pv, 0);
   PanelRefresh(true);
   ChartRedraw(0);
   g_redraw_left = 3;
}

string Money(const double v) { return (v < 0 ? "-$" : "$") + DoubleToString(MathAbs(v), 2); }
string MoneyS(const double v) { return (v > 0 ? "+$" : (v < 0 ? "-$" : "$")) + DoubleToString(MathAbs(v), 2); }

bool Gauge(const string g, const string label, const string val, const double frac, const color c)
{
   bool ch = false;
   ch |= PText(PNL + g + "_l", label, CSUB());
   ch |= PText(PNL + g + "_v", val, CTXT());
   ch |= PWidth(PNL + g + "_fg", (int)MathRound(MathMax(0.0, MathMin(1.0, frac)) * (PW - 20)));
   ch |= PBg(PNL + g + "_fg", c);
   return ch;
}

string FpWarnLine(const int i)          // FP_WL_MSG in three lines that fit the panel width
{
   if(i == 0) return "FED POST paused: add https://www.federalreserve.gov";
   if(i == 1) return "and https://nfs.faireconomy.media in Tools > Options >";
   return "Expert Advisors > Allow WebRequest";
}
void PanelRefresh(const bool force)
{
   if(!g_panel) return;
   if(g_panel_fpw != g_fp_paused) { PanelBuild(); return; }   // v13.6: warning shown / removed
   bool ch = PText(PNL + "clock", ClockText(), CSUB());      // changes once a minute, also when no ticks arrive
   PTip(PNL + "clock", Tr("clock"));
   datetime now = TimeCurrent();
   if(!force && now == g_last_panel) { if(ch) ChartRedraw(0); return; }
   g_last_panel = now;
   double bal = AccountInfoDouble(ACCOUNT_BALANCE), eq = AccountInfoDouble(ACCOUNT_EQUITY);
   ch |= PText(PNL + "title", Tr("title"), CCYA());
   ch |= PText(PNL + "b_min", g_pmin ? "+" : "–", CTXT());
   ch |= PBg(PNL + "b_min", CBTN());
   PTip(PNL + "b_min", g_pmin ? Tr("expand") : Tr("minimize"));
   string st = ""; color sc = CGRN();
   if(g_idle) { st = Tr("idle"); sc = CRED(); }
   else if(!TerminalInfoInteger(TERMINAL_TRADE_ALLOWED) || !MQLInfoInteger(MQL_TRADE_ALLOWED)) { st = Tr("algo"); sc = CRED(); }
   else if(g_halt == 2) { st = Tr("mhalt"); sc = CRED(); }
   else if(g_halt == 1) { st = Tr("dhalt"); sc = CRED(); }
   else if(g_target_hit) { st = Tr("target"); sc = CCYA(); }
   else if(g_paused) { st = Tr("paused"); sc = CAMB(); }
   else if(DailyLoss() >= DailyLimitMoney(SoftStopPct())) { st = Tr("soft"); sc = CAMB(); }
   else if(NewsBlocked()) { st = Tr("newsw"); sc = CAMB(); }
   else { st = Tr("active"); sc = CGRN(); }
   // Persian words and numbers live in separate labels: correct order whatever the terminal's text direction
   ch |= PText(PNL + "status", "● " + st, sc);
   if(g_fp_paused) for(int i = 0; i < 3; i++) ch |= PText(PNL + "fpw" + IntegerToString(i), FpWarnLine(i), CAMB());
   ch |= PText(PNL + "preset", "[" + (g_preset == PRESET_PERSONAL ? (g_lang == LANG_FA ? "شخصی" : "PERSONAL") : PresetName(g_preset)) + "]", sc);
   if(g_pmin) { ch |= PText(PNL + "eqm", Money(eq), eq >= bal ? CGRN() : CTXT()); if(ch) ChartRedraw(0); return; }
   ch |= PText(PNL + "bal_l", Tr("bal"), CSUB()); ch |= PText(PNL + "bal_v", Money(bal), CTXT());
   ch |= PText(PNL + "eq_l", Tr("eq"), CSUB());   ch |= PText(PNL + "eq_v", Money(eq), eq >= bal ? CGRN() : CTXT());
   ch |= PText(PNL + "risk_l", Tr("risk"), CSUB()); ch |= PText(PNL + "risk_v", StringFormat(DDBrakeOn() ? "%.2f%% (x%.2f)" : "%.2f%%", RiskUnitPct(), InpDDBrakeMult), DDBrakeOn() ? CAMB() : CTXT());
   ch |= PText(PNL + "days_l", Tr("days"), CSUB());
   ch |= PText(PNL + "days_v", StringFormat("%d/%d", g_trading_days, InpMinTradingDays), g_trading_days >= InpMinTradingDays ? CGRN() : CAMB());
   ch |= PText(PNL + "b_rm", g_rmode == RM_TURBO ? Tr("rm_t") : (g_rmode == RM_FAST ? Tr("rm_f") : Tr("rm_s")), g_rmode != RM_SAFE ? MQL_RGB(10,14,20) : CTXT());
   ch |= PBg(PNL + "b_rm", g_rmode == RM_TURBO ? CRED() : (g_rmode == RM_FAST ? CAMB() : CBTN()));
   string bn[4] = {"b_p1", "b_p2", "b_fu", "b_cu"}, bk[4] = {"p1", "p2", "fu", "cu"};
   for(int i = 0; i < 4; i++)
   {
      ch |= PText(PNL + bn[i], Tr(bk[i]), (g_preset == i ? MQL_RGB(10,14,20) : CTXT()));
      ch |= PBg(PNL + bn[i], (g_preset == i ? CCYA() : CBTN()));
   }
   double dl = DailyLoss(), dlim = MathMax(DailyLimitMoney(InpDailyLossPct), 1e-9);
   ch |= Gauge("g_day", Tr("day"), Money(-dl) + " / " + Money(-dlim), dl / dlim, dl / dlim > 0.6 ? CRED() : (dl / dlim > 0.3 ? CAMB() : CGRN()));
   double lf = LossFloor(InpMaxLossPct), olim = InpMaxLossPct / 100.0 * g_init_bal;
   double ol = MathMax(0.0, olim - (eq - lf));
   ch |= Gauge("g_dd", Tr("dd"), Money(-ol) + " / " + Money(-olim), ol / olim, ol / olim > 0.6 ? CRED() : (ol / olim > 0.3 ? CAMB() : CGRN()));
   double tg = TargetPct(), gain = eq - g_init_bal;
   ch |= Gauge("g_tg", Tr("prog"), tg > 0 ? Money(gain) + " / " + Money(tg / 100.0 * g_init_bal) : "-", tg > 0 ? gain / (tg / 100.0 * g_init_bal) : 0, CCYA());
   double orr = OpenStopRisk(), olim2 = InpMaxOpenRiskR * BaseRiskPct() / 100.0 * g_init_bal;
   ch |= Gauge("g_or", Tr("orisk"), Money(orr) + " / " + Money(olim2), orr / olim2, orr / olim2 > 0.8 ? CAMB() : CGRN());
   // regime chips
   string rn[5] = {"GOLD", "SILVER", "DOW", "NASDAQ", "AUDJPY"};
   for(int r = 0; r < NREG; r++)
   {
      string rs = IntegerToString(r);
      int rgm = RegimeOf(r);
      string v; color c;
      if(!g_sym_ok[r]) { v = "—"; c = CSUB(); }
      else if(rgm > 0) { v = "▲ " + Tr("up"); c = CGRN(); }
      else if(rgm < 0) { v = "▼ " + Tr("down"); c = CRED(); }
      else { v = "▬ " + Tr("range"); c = CAMB(); }
      ch |= PText(PNL + "rn" + rs, rn[r], CSUB());
      ch |= PText(PNL + "rv" + rs, v, c);
      ch |= PBg(PNL + "ra" + rs, c);
   }
   // strategy grid
   {
      int na = 0, no = 0; double l0, p0;
      for(int k = 0; k < NSLV; k++) { if(g_use[k]) na++; if(SleevePositions(k, l0, p0) > 0) no++; }
      ch |= PText(PNL + "sh", (g_lang == LANG_FA ? "استراتژی‌ها" : "Strategies") + StringFormat("  %d / %d", no, na), CSUB());
      ch |= PText(PNL + "b_smin", g_smin ? "+" : "–", CTXT());
      ch |= PBg(PNL + "b_smin", CBTN());
      PTip(PNL + "b_smin", g_smin ? (g_lang == LANG_FA ? "نمایش لیست" : "Show list") : (g_lang == LANG_FA ? "بستن لیست" : "Hide list"));
   }
   for(int k = 0; k < NSLV && !g_smin; k++)
   {
      string ks = IntegerToString(k);
      double lots, pnl;
      int c = SleevePositions(k, lots, pnl);
      string s, tip; color col = CSUB(), dot = CSUB(), ncol = CTXT();
      if(!g_sym_ok[SL_ROLE[k]]) { s = ShortSt("nosym"); tip = Tr("nosym"); dot = MQL_RGB(60,68,84); ncol = CSUB(); }
      else if(c > 0)
      {
         double r = (g_ss[k].r0 > 0 && g_ss[k].base_lots > 0) ? pnl / (g_ss[k].r0 * g_ss[k].base_lots * MoneyPerPriceUnit(g_sym[SL_ROLE[k]])) : 0;
         s = MoneyS(pnl); col = pnl >= 0 ? CGRN() : CRED(); dot = col;
         tip = (g_ss[k].dir > 0 ? "BUY " : "SELL ") + DoubleToString(lots, 2) + StringFormat("  %+.1fR", r) + (c > 1 ? StringFormat("  (%d)", c) : "");
      }
      else if(!g_use[k]) { s = ShortSt("off"); tip = Tr("off"); col = MQL_RGB(96,106,124); dot = MQL_RGB(60,68,84); ncol = CSUB(); }
      else
      {
         // a block reason is shown for 24 h after the blocked signal, otherwise the live condition (v11.8)
         string key = (g_ss[k].status == "" ? "wait" : (g_ss[k].status == "target" ? "target_s" : g_ss[k].status));
         bool recent = (key != "wait" && TimeCurrent() - g_st_t[k] < 86400);
         if(!recent && g_inf_s[k] != "")
         {
            s = g_inf_s[k]; tip = g_inf_f[k]; dot = CCYA();
            col = g_inf_near[k] ? CCYA() : CSUB();
         }
         else
         {
            s = ShortSt(key); tip = Tr(key) + (g_inf_f[k] != "" ? "  |  " + g_inf_f[k] : "");
            if(key == "wait") { dot = CCYA(); }
            else { col = CAMB(); dot = CAMB(); }
         }
      }
      ch |= PText(PNL + "sd" + ks, "●", dot);
      ch |= PText(PNL + "sl" + ks + "_n", SL_NAME[k], ncol);
      ch |= PText(PNL + "sl" + ks + "_s", s, col);
      PTip(PNL + "sl" + ks + "_s", tip);
   }
   // heartbeat row
   ch |= PText(PNL + "scan_l", Tr("scan"), CSUB());
   ch |= PText(PNL + "scan_v", g_last_scan > 0 ? HHMM(g_last_scan) : "--:--", g_last_scan > 0 && TimeCurrent() - g_last_scan < 2 * 3600 ? CGRN() : CAMB());
   ch |= PText(PNL + "nday_l", Tr("nday"), CSUB());
   ch |= PText(PNL + "nday_v", "01:00", CTXT());
   PTip(PNL + "scan_v", g_lang == LANG_FA ? "ساعت سرورِ آخرین کندل H1 که ربات بررسی کرد (هر ساعت تازه می‌شود)" : "Server time of the last H1 bar the EA processed (updates every hour)");
   PTip(PNL + "nday_v", g_lang == LANG_FA ? "سیگنال‌های روزانه (RSI2، IBS، مومنتوم) ساعت ۰۱:۰۰ سرور بررسی می‌شوند" : "Daily signals (RSI2, IBS, momentum) are checked at 01:00 server time");
   // open positions of this EA
   MArr<string> ps; MArr<int> pdir, pk; MArr<double> pl, pp, psw;
   int np = 0;
   for(int i = 0; i < PositionsTotal(); i++)
   {
      ulong t = PositionGetTicket(i);
      if(t == 0) continue;
      int k = SleeveOfMagic(PositionGetInteger(POSITION_MAGIC));
      if(k < 0) continue;
      ArrayResize(ps, np + 1); ArrayResize(pdir, np + 1); ArrayResize(pk, np + 1); ArrayResize(pl, np + 1); ArrayResize(pp, np + 1); ArrayResize(psw, np + 1);
      ps[np] = PositionGetString(POSITION_SYMBOL); pk[np] = k;
      pdir[np] = (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY ? 1 : -1);
      pl[np] = PositionGetDouble(POSITION_VOLUME);
      pp[np] = PositionGetDouble(POSITION_PROFIT) + PositionGetDouble(POSITION_SWAP);
      psw[np] = PositionGetDouble(POSITION_SWAP);
      np++;
   }
   g_pos_off = MathMax(0, MathMin(g_pos_off, np - NPOSROW));
   ch |= PText(PNL + "ph", Tr("openpos"), CSUB());
   ch |= PText(PNL + "pc", np > NPOSROW ? StringFormat("%d-%d / %d", g_pos_off + 1, g_pos_off + NPOSROW, np) : (np > 0 ? IntegerToString(np) : " "), CSUB());
   ch |= PText(PNL + "b_up", "▲", g_pos_off > 0 ? CTXT() : MQL_RGB(70,80,96));
   ch |= PText(PNL + "b_dn", "▼", g_pos_off + NPOSROW < np ? CTXT() : MQL_RGB(70,80,96));
   ch |= PBg(PNL + "b_up", CBTN()); ch |= PBg(PNL + "b_dn", CBTN());
   for(int i = 0; i < NPOSROW; i++)
   {
      string is = IntegerToString(i);
      int j = g_pos_off + i;
      if(j < np)
      {
         ch |= PBg(PNL + "pr" + is, CCARD());
         ch |= PText(PNL + "p" + is + "_s", ps[j], CTXT());
         ch |= PText(PNL + "p" + is + "_d", pdir[j] > 0 ? Tr("buy") : Tr("sell"), pdir[j] > 0 ? CGRN() : CRED());
         ch |= PText(PNL + "p" + is + "_l", DoubleToString(pl[j], 2), CTXT());
         ch |= PText(PNL + "p" + is + "_k", SL_NAME[pk[j]], CSUB());
         ch |= PText(PNL + "p" + is + "_p", MoneyS(pp[j]), pp[j] >= 0 ? CGRN() : CRED());
         PTip(PNL + "p" + is + "_p", (g_lang == LANG_FA ? "سود خالص (با سواپ). قیمت: " : "Net P/L incl. swap. Price: ") + MoneyS(pp[j] - psw[j]) + (g_lang == LANG_FA ? "  سواپ: " : "  swap: ") + MoneyS(psw[j]));
      }
      else
      {
         ch |= PBg(PNL + "pr" + is, CBG());
         ch |= PText(PNL + "p" + is + "_s", (i == 0 && np == 0) ? Tr("nopos") : " ", CSUB());
         ch |= PText(PNL + "p" + is + "_d", " ", CSUB()); ch |= PText(PNL + "p" + is + "_l", " ", CSUB());
         ch |= PText(PNL + "p" + is + "_k", " ", CSUB()); ch |= PText(PNL + "p" + is + "_p", " ", CSUB());
      }
   }
   uint ms = GetTickCount();
   if(g_arm_close != 0 && ms - g_arm_close > 5000) g_arm_close = 0;
   if(g_arm_reset != 0 && ms - g_arm_reset > 5000) g_arm_reset = 0;
   ch |= PText(PNL + "b_pause", g_paused ? Tr("resume") : Tr("pause"), CTXT());
   ch |= PBg(PNL + "b_pause", g_paused ? MQL_RGB(120,90,20) : CBTN());
   ch |= PText(PNL + "b_close", g_arm_close != 0 ? Tr("confirm") : Tr("close"), CTXT());
   ch |= PBg(PNL + "b_close", g_arm_close != 0 ? CRED() : MQL_RGB(90,40,44));
   ch |= PText(PNL + "b_reset", g_arm_reset != 0 ? Tr("confirm") : Tr("reset"), g_halt != 0 ? CTXT() : CSUB());
   ch |= PBg(PNL + "b_reset", g_arm_reset != 0 ? CRED() : CBTN());
   ch |= PText(PNL + "b_lang", g_lang == LANG_FA ? "EN" : "FA", CTXT());
   ch |= PBg(PNL + "b_lang", CBTN());
   if(ch) ChartRedraw(0);
}

void SetPreset(const int p)
{
   if(p == g_preset) return;
   if(IsPersonal() && p != PRESET_PERSONAL) { Print("v13.8: personal account - the prop presets (phase 1 / phase 2 / funded / custom) are not used"); return; }
   if(p != g_preset) { g_p1_ramp_done = false; GvSet("p1ramp", 0); }
   g_preset = p; GvSet("preset", p);
   double v;
   g_target_hit = (TargetPct() > 0 && GlobalVariableGet(GvKey("target_" + PresetName(p)), v) && v > 0.5);
   PrintFormat("v13 preset -> %s (risk unit %.2f%%, target %.1f%%)", PresetName(p), RiskUnitPct(), TargetPct());
}


//--------------------------------------------------------------------- live info refresh + chart overlay (display only, v11.8)
bool ShowVisual() { return !IsTester() || MQLInfoInteger(MQL_VISUAL_MODE) != 0; }

void UpdateInfo()
{
   if(!g_ready || !ShowVisual()) return;
   static datetime s_scan = -1;
   datetime now = TimeCurrent();
   bool bar = (g_last_scan != s_scan);
   if(!bar && g_inf_time != 0 && now - g_inf_time < 10) return;
   s_scan = g_last_scan; g_inf_time = now;
   for(int k = 0; k < NSLV; k++)
   {
      if(g_ss[k].status != g_st_prev[k]) { g_st_prev[k] = g_ss[k].status; g_st_t[k] = now; }
      SleeveInfo(k, bar);
   }
}

const string OVP = "DP11C_";
void OvLabel(const long id, const string n, const int x, const int y, const string txt, const color c, const int fs)
{
   if(ObjectFind(id, n) < 0)
   {
      ObjectCreate(id, n, OBJ_LABEL, 0, 0, 0);
      ObjectSetInteger(id, n, OBJPROP_CORNER, CORNER_LEFT_UPPER);
      ObjectSetInteger(id, n, OBJPROP_SELECTABLE, 0); ObjectSetInteger(id, n, OBJPROP_HIDDEN, 1);
      ObjectSetString(id, n, OBJPROP_FONT, "Consolas");
   }
   ObjectSetInteger(id, n, OBJPROP_XDISTANCE, x); ObjectSetInteger(id, n, OBJPROP_YDISTANCE, y);
   ObjectSetInteger(id, n, OBJPROP_FONTSIZE, fs);
   ObjectSetString(id, n, OBJPROP_TEXT, txt == "" ? " " : txt);
   ObjectSetInteger(id, n, OBJPROP_COLOR, c);
}
void OvRect(const long id, const string n, const int x, const int y, const int w, const int h, const color bg)
{
   if(ObjectFind(id, n) < 0)
   {
      ObjectCreate(id, n, OBJ_RECTANGLE_LABEL, 0, 0, 0);
      ObjectSetInteger(id, n, OBJPROP_CORNER, CORNER_LEFT_UPPER);
      ObjectSetInteger(id, n, OBJPROP_SELECTABLE, 0); ObjectSetInteger(id, n, OBJPROP_HIDDEN, 1);
      ObjectSetInteger(id, n, OBJPROP_BORDER_TYPE, BORDER_FLAT);
   }
   ObjectSetInteger(id, n, OBJPROP_XDISTANCE, x); ObjectSetInteger(id, n, OBJPROP_YDISTANCE, y);
   ObjectSetInteger(id, n, OBJPROP_XSIZE, w); ObjectSetInteger(id, n, OBJPROP_YSIZE, h);
   ObjectSetInteger(id, n, OBJPROP_BGCOLOR, bg); ObjectSetInteger(id, n, OBJPROP_COLOR, MQL_RGB(60,70,90));
}
void OvTrend(const long id, const string n, const datetime t1, const double p, const datetime t2, const color c, const int style, const int width)
{
   if(ObjectFind(id, n) < 0)
   {
      ObjectCreate(id, n, OBJ_TREND, 0, t1, p, t2, p);
      ObjectSetInteger(id, n, OBJPROP_RAY_RIGHT, 0);
      ObjectSetInteger(id, n, OBJPROP_SELECTABLE, 0); ObjectSetInteger(id, n, OBJPROP_HIDDEN, 1);
      ObjectSetInteger(id, n, OBJPROP_BACK, 1);
   }
   ObjectMove(id, n, 0, t1, p); ObjectMove(id, n, 1, t2, p);
   ObjectSetInteger(id, n, OBJPROP_COLOR, c); ObjectSetInteger(id, n, OBJPROP_STYLE, style); ObjectSetInteger(id, n, OBJPROP_WIDTH, width);
}
void OvText(const long id, const string n, const datetime t, const double p, const string txt, const color c)
{
   if(ObjectFind(id, n) < 0)
   {
      ObjectCreate(id, n, OBJ_TEXT, 0, t, p);
      ObjectSetInteger(id, n, OBJPROP_SELECTABLE, 0); ObjectSetInteger(id, n, OBJPROP_HIDDEN, 1);
      ObjectSetString(id, n, OBJPROP_FONT, "Tahoma"); ObjectSetInteger(id, n, OBJPROP_FONTSIZE, 7);
      ObjectSetInteger(id, n, OBJPROP_ANCHOR, ANCHOR_LEFT_LOWER);
   }
   ObjectMove(id, n, 0, t, p);
   ObjectSetString(id, n, OBJPROP_TEXT, txt); ObjectSetInteger(id, n, OBJPROP_COLOR, c);
}
void OvArrow(const long id, const string n, const datetime t, const double p, const int code, const color c, const string tip)
{
   if(ObjectFind(id, n) >= 0) return;
   ObjectCreate(id, n, OBJ_ARROW, 0, t, p);
   ObjectSetInteger(id, n, OBJPROP_ARROWCODE, code); ObjectSetInteger(id, n, OBJPROP_COLOR, c);
   ObjectSetInteger(id, n, OBJPROP_WIDTH, 2);
   ObjectSetInteger(id, n, OBJPROP_SELECTABLE, 0); ObjectSetInteger(id, n, OBJPROP_HIDDEN, 1);
   ObjectSetString(id, n, OBJPROP_TOOLTIP, tip);
}
color SleeveColor(const int k)
{
   color pal[6] = {MQL_RGB(56,189,248), MQL_RGB(167,139,250), MQL_RGB(251,146,60), MQL_RGB(52,211,153), MQL_RGB(244,114,182), MQL_RGB(250,204,21)};
   return pal[k % 6];
}

void OverlayChart(const long id, const bool events)
{
   string sym = ChartSymbol(id);
   int role = -1;
   for(int r = 0; r < NROLE; r++) if(g_sym_ok[r] && g_sym[r] == sym) { role = r; break; }
   if(role < 0) { if(ObjectsDeleteAll(id, OVP) > 0) ChartRedraw(id); return; }
   g_cur_role = role;
   datetime now = TimeCurrent();
   int per = PeriodSeconds(ChartPeriod(id));
   if(per <= 0) per = 3600;
   datetime tr = (datetime)(now + 6 * per);
   DayFeat f = g_df[role];
   // 1) entry levels of the sleeves waiting for a breakout on this symbol
   for(int k = 0; k < NSLV; k++)
   {
      if(SL_ROLE[k] != role) continue;
      string ks = IntegerToString(k);
      double lots, pnl;
      bool show = g_use[k] && (SL_KIND[k] == 0 || SL_KIND[k] == 6 || SL_KIND[k] == 7) && SleevePositions(k, lots, pnl) == 0;
      for(int sd = 0; sd < 2; sd++)
      {
         double lv = (sd == 0 ? g_lv_up[k] : g_lv_dn[k]);
         string n = OVP + "lv" + ks + (sd == 0 ? "u" : "d");
         int dir = (sd == 0 ? 1 : -1);
         if(!show || lv <= 0 || g_lv_t0[k] <= 0 || (dir < 0 && SL_SIDE[k] == 1)) { ObjectDelete(id, n); ObjectDelete(id, n + "t"); continue; }
         bool ok = RegimeAllows(k, dir);
         color c = ok ? SleeveColor(k) : MQL_RGB(90,98,112);
         OvTrend(id, n, g_lv_t0[k], lv, tr, c, STYLE_DOT, 1);
         OvText(id, n + "t", tr, lv, " " + SL_NAME[k] + (dir > 0 ? " buy > " : " sell < ") + Px(role, lv) + (ok ? "" : "  (" + RegTxt(k, dir) + ")"), c);
      }
   }
   // 2) D1 EMA200 (the regime filter of most sleeves)
   if(f.ok && f.ema200 > 0)
   {
      OvTrend(id, OVP + "ema", (datetime)(now - 40 * per), f.ema200, tr, MQL_RGB(120,130,150), STYLE_DASHDOT, 1);
      OvText(id, OVP + "emat", (datetime)(now - 40 * per), f.ema200, " D1 EMA200 " + Px(role, f.ema200), MQL_RGB(120,130,150));
   }
   // 3) open positions: entry / stop / target
   if(events) ObjectsDeleteAll(id, OVP + "p");
   for(int i = PositionsTotal() - 1; i >= 0; i--)
   {
      ulong t = PositionGetTicket(i);
      if(t == 0 || PositionGetString(POSITION_SYMBOL) != sym) continue;
      int k = SleeveOfMagic(PositionGetInteger(POSITION_MAGIC));
      if(k < 0) continue;
      string n = OVP + "p" + IntegerToString((long)t);
      datetime t0 = (datetime)PositionGetInteger(POSITION_TIME);
      double op = PositionGetDouble(POSITION_PRICE_OPEN), sl = PositionGetDouble(POSITION_SL), tp = PositionGetDouble(POSITION_TP);
      double pr = PositionGetDouble(POSITION_PROFIT) + PositionGetDouble(POSITION_SWAP);
      bool buy = (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY);
      OvTrend(id, n + "e", t0, op, tr, MQL_RGB(200,205,215), STYLE_SOLID, 1);
      OvText(id, n + "et", tr, op, StringFormat(" %s %s %.2f  %s", SL_NAME[k], buy ? "BUY" : "SELL", PositionGetDouble(POSITION_VOLUME), MoneyS(pr)), pr >= 0 ? CGRN() : CRED());
      if(sl > 0) { OvTrend(id, n + "s", t0, sl, tr, CRED(), STYLE_DASH, 1); OvText(id, n + "st", tr, sl, " SL " + Px(role, sl), CRED()); }
      if(tp > 0) { OvTrend(id, n + "t", t0, tp, tr, CGRN(), STYLE_DASH, 1); OvText(id, n + "tt", tr, tp, " TP " + Px(role, tp), CGRN()); }
   }
   // 4) past entries and exits of this EA (drawn once per deal)
   if(events && InpChartHistDays > 0 && HistorySelect(now - (datetime)InpChartHistDays * 86400, now + 60))
   {
      int nd = HistoryDealsTotal();
      for(int i = 0; i < nd; i++)
      {
         ulong d = HistoryDealGetTicket(i);
         if(d == 0 || HistoryDealGetString(d, DEAL_SYMBOL) != sym) continue;
         int k = SleeveOfMagic(HistoryDealGetInteger(d, DEAL_MAGIC));
         if(k < 0) continue;
         long typ = HistoryDealGetInteger(d, DEAL_TYPE), en = HistoryDealGetInteger(d, DEAL_ENTRY);
         datetime dt = (datetime)HistoryDealGetInteger(d, DEAL_TIME);
         double px = HistoryDealGetDouble(d, DEAL_PRICE);
         string n = OVP + "h" + IntegerToString((long)d);
         if(en == DEAL_ENTRY_IN)
            OvArrow(id, n, dt, px, typ == DEAL_TYPE_BUY ? 233 : 234, typ == DEAL_TYPE_BUY ? MQL_RGB(56,189,248) : MQL_RGB(251,146,60), SL_NAME[k] + (typ == DEAL_TYPE_BUY ? " BUY " : " SELL ") + Px(role, px));
         else
         {
            double net = HistoryDealGetDouble(d, DEAL_PROFIT) + HistoryDealGetDouble(d, DEAL_SWAP) + HistoryDealGetDouble(d, DEAL_COMMISSION);
            OvArrow(id, n, dt, px, 251, net >= 0 ? CGRN() : CRED(), SL_NAME[k] + " exit " + MoneyS(net));
         }
      }
   }
   // 5) v13.1: the separate status box on the right of the chart was removed (everything is in the panel)
   ObjectDelete(id, OVP + "bbg");
   for(int i = 0; i < 16; i++) ObjectDelete(id, OVP + "b" + IntegerToString(i));
   ChartRedraw(id);
}

void ChartOverlay(const bool force)
{
   if(!g_ready || g_idle || !InpChartOverlay || !ShowVisual()) return;
   int np = PositionsTotal();
   bool ev = force || np != g_ov_np || g_last_scan != g_ov_scan;
   datetime lt = TimeLocal();
   if(!ev && (IsTester() || lt - g_ov_last < 5)) return;
   g_ov_last = lt; g_ov_np = np; g_ov_scan = g_last_scan;
   long id = ChartFirst();
   int guard = 0;
   while(id >= 0 && guard++ < 200) { OverlayChart(id, ev); id = ChartNext(id); }
}

// On start: open an H1 chart for every traded symbol that has none, so the user sees every market the EA works on.
// The trading itself never depends on these charts (it reads bars by symbol and timeframe); closing one is harmless.
void OpenSymbolCharts()
{
   // v13.6.1: tester → only in visual mode, H1 only; live/demo → only if InpOpenCharts (default off)
   if(IsTester()) { if(!MQLInfoInteger(MQL_VISUAL_MODE)) return; }
   else if(!InpOpenCharts || !InpChartOverlay) return;
   int opened = 0;
   for(int r = 0; r < NROLE; r++)
   {
      if(!g_sym_ok[r]) continue;
      bool used = false;
      for(int k = 0; k < NSLV; k++) if(SL_ROLE[k] == r && g_use[k]) used = true;
      if(!used) continue;
      bool have = false;
      long id = ChartFirst();
      int guard = 0;
      while(id >= 0 && guard++ < 200) { if(ChartSymbol(id) == g_sym[r]) { have = true; break; } id = ChartNext(id); }
      if(have) continue;
      if(ChartOpen(g_sym[r], PERIOD_H1) > 0) opened++;
      else PrintFormat("v13: could not open a chart for %s", g_sym[r]);
   }
   if(opened > 0)
   {
      PrintFormat("v13: opened %d chart(s) for the traded symbols (H1)", opened);
      ChartSetInteger(ChartID(), CHART_BRING_TO_TOP, 0, 1);      // focus back on the EA's own chart
   }
}

void ChartOverlayClear()
{
   long id = ChartFirst();
   int guard = 0;
   while(id >= 0 && guard++ < 200) { if(ObjectsDeleteAll(id, OVP) > 0) ChartRedraw(id); id = ChartNext(id); }
}

//--------------------------------------------------------------------- MQL5 events
// v11.8: 1 ready, 0 not yet (terminal still connecting after a restart), -1 fatal.  Before v11.8 the hedging check ran at once
// in OnInit; right after a terminal restart the account is not loaded yet (margin mode reads as netting, login 0), so the EA
// failed and MT5 removed it from the chart.
int InitCore()
{
   bool live = !IsTester();
   if(live && (TerminalInfoInteger(TERMINAL_CONNECTED) == 0 || AccountInfoInteger(ACCOUNT_LOGIN) <= 0 || SymbolsTotal(false) == 0)) return 0;
   if(AccountInfoInteger(ACCOUNT_MARGIN_MODE) != ACCOUNT_MARGIN_MODE_RETAIL_HEDGING)
   {
      if(live && g_init_tries < 30) return 0;          // account data may still be loading: retry for 30 s
      Print("v13: a HEDGING account is required (several sleeves trade the same symbol)."); return -1;
   }
   g_gv = "DP11_" + IntegerToString(AccountInfoInteger(ACCOUNT_LOGIN)) + "_" + IntegerToString(InpMagicBase) + "_";
   if(!live)          // v13.2: terminal global variables can survive from an earlier tester run (preset, initial balance, halt, targets ...): a test starts from its inputs
   {
      int nd = GlobalVariablesDeleteAll(g_gv);
      if(nd > 0) PrintFormat("v13.2 tester: %d saved state value(s) of an earlier run deleted - this run starts from its inputs (preset %s)", nd, PresetName(InpPreset));
   }
   string want[NROLE];
   want[0] = InpGoldSymbol; want[1] = InpSilverSymbol; want[2] = InpDowSymbol; want[3] = InpNasdaqSymbol; want[4] = InpAudJpySymbol; want[5] = InpUsdJpySymbol; want[6] = InpEurGbpSymbol; want[7] = InpSpxSymbol;
   int ok = 0;
   for(int r = 0; r < NROLE; r++)
   {
      g_tzr_ok[r] = false; g_nyal_r[r] = true; g_df_n[r] = 0;
      g_sym[r] = ResolveRole(r, want[r]); g_sym_ok[r] = (g_sym[r] != ""); g_last_h1[r] = 0; g_last_h4[r] = 0; g_df[r].ok = false;
      if(want[r] != "-" && !g_sym_ok[r]) PrintFormat("v13: no symbol found for role %d (%s) - its sleeves are disabled", r, want[r] == "" ? RoleAliases(r) : want[r]);
      if(g_sym_ok[r]) ok++;
   }
   if(DemoActive())
   {
      int nc = 0;
      for(int r = 0; r < NROLE; r++) if(g_sym_ok[r] && SymbolInfoInteger(g_sym[r], SYMBOL_CUSTOM) != 0) nc++;
      g_demo_custom = (nc > 0);
      for(int r = 0; r < NROLE && g_demo_custom; r++)
      {
         if(!g_sym_ok[r]) continue;
         bool cust = (SymbolInfoInteger(g_sym[r], SYMBOL_CUSTOM) != 0);
         if(!cust && want[r] == "")       // no custom history for this market: its short broker history (and possibly other clock) is left out
         {
            PrintFormat("v13 DEMO: no custom symbol for role %d - %s left out (set its symbol input to use it anyway)", r, g_sym[r]);
            g_sym[r] = ""; g_sym_ok[r] = false; ok--;
            continue;
         }
         double tv = SymbolInfoDouble(g_sym[r], SYMBOL_TRADE_TICK_VALUE), ts = SymbolInfoDouble(g_sym[r], SYMBOL_TRADE_TICK_SIZE);
         PrintFormat("v13 DEMO: %s%s  contract %.2f  tick %.5f = %.5f %s  digits %d  spread %d pts", g_sym[r], cust ? " (custom)" : "",
                     SymbolInfoDouble(g_sym[r], SYMBOL_TRADE_CONTRACT_SIZE), ts, tv, AccountInfoString(ACCOUNT_CURRENCY),
                     (int)SymbolInfoInteger(g_sym[r], SYMBOL_DIGITS), (int)SymbolInfoInteger(g_sym[r], SYMBOL_SPREAD));
         if(tv <= 0 || ts <= 0)
         {
            PrintFormat("v13 DEMO: %s has no tick value (no data or wrong symbol settings) - left out", g_sym[r]);
            g_sym[r] = ""; g_sym_ok[r] = false; ok--;
            continue;
         }
         if(r == 4 || r == 5)          // JPY-quoted pair: a tick must be worth roughly tick x contract / USDJPY, not tick x contract
         {
            double raw = ts * SymbolInfoDouble(g_sym[r], SYMBOL_TRADE_CONTRACT_SIZE);
            if(raw > 0 && MathAbs(tv - raw) < 0.01 * raw)
               PrintFormat("v13 DEMO WARNING: %s profit currency looks like USD instead of JPY - lot sizes and money results of this symbol are not realistic (create it with Copy from the broker symbol)", g_sym[r]);
         }
      }
      g_tz_ok = false;                                    // v12: never keep a clock read on an earlier start (same chart, new inputs or timeframe)
      for(int r = 0; r < NROLE; r++) g_tzr_ok[r] = false;
      if(InpServerGMTOffset == -99 && g_demo_custom)      // v12: only custom (imported) data gets its clock read from the bars; broker data keeps the broker clock
      {
         g_tz_ok = false;
         if(DetectDataTZ())
         {
            if(g_tz_s == 3 && g_tz_w == 2) g_tz_ok = false;          // EET + US DST: the built-in tester clock already
            else g_tz_ok = true;
         }
         else Print("v13 DEMO: data time zone not readable from the bars - EET (UTC+2/+3) assumed; set InpServerGMTOffset if your data uses another clock");
      }
   }
   // v13.1: the strategy tester has no server clock - read it from the broker bars (weekly open / US cash open) instead of assuming EET.
   // A UTC+0 server (e.g. ZoraCapital) otherwise shifts every clock sleeve by 2-3 hours and builds daily bars on the wrong day cut.
   if(IsTester() && InpServerGMTOffset == -99 && !g_tz_ok && !g_demo_custom)
   {
      for(int r = 0; r < NROLE; r++) g_tzr_ok[r] = false;
      if(DetectDataTZ(true))
      {
         g_tz_ok = !(g_tz_s == 3 && g_tz_w == 2);
         PrintFormat("v13 tester clock read from the broker bars: UTC%+d (US summer) / UTC%+d (US winter)%s", g_tz_s, g_tz_w, g_tz_ok ? "" : " = EET, as assumed");
      }
      else Print("v13 tester clock not readable from the bars - EET (UTC+2/+3) assumed; set InpServerGMTOffset if the server uses another clock");
   }
   if((want[2] != "-" && !g_sym_ok[2]) || (want[3] != "-" && !g_sym_ok[3]) || (want[7] != "-" && !g_sym_ok[7])) ListServerIndices();
   if(ok == 0) { if(live && g_init_tries < 60) return 0; Print("v13: no tradable symbol found"); return -1; }
   bool flags[NSLV];
   flags[0] = InpUseGoldT60; flags[1] = InpUseGoldT240; flags[2] = InpUseGoldMOM; flags[3] = InpUseGoldTOM;
   flags[4] = InpUseSilverT240; flags[5] = InpUseSilverMA; flags[6] = InpUseDowRSI2; flags[7] = InpUseDowIBS;
   flags[8] = InpUseNasRSI2; flags[9] = InpUseNasIBS; flags[10] = InpUseAudJpyBB; flags[11] = InpUseSilverH4; flags[12] = InpUseUsdJpyGotobi; flags[13] = InpUseNas3FPS; flags[14] = InpUseUsdJpyFixRev; flags[15] = InpUseGoldLottery; flags[16] = InpUseNasLottery; flags[17] = InpUseJpyLottery; flags[18] = InpUseEurGbpMR; flags[19] = InpUseNasGapLong; flags[20] = InpUseGoldIbs; flags[21] = InpUseSpxRSI2;
   flags[22] = InpUseFedPostUs30; flags[23] = InpUseFedPostSpx; flags[24] = InpUseDxyJpy; flags[25] = InpUseGoldZcont;   // v13.4 (v15.0 review)
   for(int k = 0; k < NSLV; k++) { g_use[k] = flags[k] && g_sym_ok[SL_ROLE[k]]; g_ss[k].dir = 0; g_ss[k].status = ""; g_close_retry[k] = false;
                                   g_cl_wait[k] = false; g_pt_pend[k] = 0; g_ad_pend[k] = 0; g_st_pend[k] = 0; g_rt_dir[k] = 0; }
   for(int r = 0; r < NROLE; r++) { g_mo_t[r] = 0; g_mclosed_until[r] = 0; }
   SL_SLK[22] = InpFedPostStopATR; SL_SLK[23] = InpFedPostStopATR;      // v13.4 (v15.0 review): table values mirror the inputs
   SL_P[24] = InpDxyZ; SL_SLK[24] = InpDxyStopATR; SL_MAXB[24] = InpDxyHoldH;
   SL_P[25] = InpGoldZc; SL_SLK[25] = InpGoldZcStopATR; SL_MAXB[25] = InpGoldZcDays * 24;
   InitDxySymbols();
   ApplyStopProfile();
   g_trade.SetDeviationInPoints(InpDeviationPoints);
   g_lang = (int)GvGet("lang", InpLanguage);
   double srm = GvGet("rmode", -1);
   g_rmode = (srm >= 0 ? (int)srm : (int)InpRiskMode);
   if(srm >= 0 && (int)srm != (int)InpRiskMode && GvGet("inp_rmode", -1) != (double)InpRiskMode) g_rmode = (int)InpRiskMode;   // a changed input wins
   GvSet("inp_rmode", (double)InpRiskMode); GvSet("rmode", g_rmode);
   // v13.2: the preset chosen in the panel is restored only when it was saved under the same InpPreset input and account login;
   // otherwise the input wins and the saved target flags are cleared (a new challenge stage)
   double saved = GvGet("preset", -1), sinp = GvGet("inp_preset", -1);
   double login = (double)AccountInfoInteger(ACCOUNT_LOGIN), slog = GvGet("login", login);
   bool restore = (saved >= 0 && sinp == (double)InpPreset && slog == login);
   g_preset = (restore ? (int)saved : (int)InpPreset);
   if(IsPersonal()) { g_preset = PRESET_PERSONAL; restore = (saved == PRESET_PERSONAL); }
   else if(g_preset == PRESET_PERSONAL) { g_preset = (int)InpPreset; restore = false; }
   if(saved >= 0 && !restore && (sinp != (double)InpPreset || slog != login))
   {
      for(int p = PRESET_PHASE1; p <= PRESET_CUSTOM; p++) GlobalVariableDel(GvKey("target_" + PresetName(p)));
      PrintFormat("v13.2: saved preset state was made with another preset input / account - reset, preset %s from the input", PresetName(g_preset));
   }
   GvSet("inp_preset", (double)InpPreset); GvSet("preset", g_preset); GvSet("login", login);
   g_p1_ramp_done = (restore && GvGet("p1ramp", 0) > 0.5);
   g_paused = (GvGet("paused", 0) > 0.5);
   g_halt = (int)GvGet("halt", 0);
   g_day = (datetime)GvGet("day", 0); g_day_ref = GvGet("dayref", 0);
   g_vt_var = GvGet("vt_var", -1); g_eq_prevday = GvGet("vt_eq", 0);
   if(g_vt_var > 0) { double v0 = MathSqrt(g_vt_var * 252.0); g_vt_g = MathMax(InpVolGMin, MathMin(InpVolGMax, v0 > 0 ? InpVolTargetPct / 100.0 / v0 : InpVolGMax)); }
   if(g_halt == 1 && DayNum(g_day) != DayNum(TimeCurrent())) g_halt = 0;
   double v;
   g_target_hit = (TargetPct() > 0 && GlobalVariableGet(GvKey("target_" + PresetName(g_preset)), v) && v > 0.5);
   g_last_helper_day = (datetime)GvGet("helper", 0);
   DetectInitialBalance();
   if(IsPersonal())
   {
      if(InpPersCompound && InpInitialBalance <= 0) { g_init_bal = AccountInfoDouble(ACCOUNT_BALANCE); GvSet("init", g_init_bal); }
      g_pers_peak = MathMax(GvGet("ppeak", 0), AccountInfoDouble(ACCOUNT_EQUITY)); GvSet("ppeak", g_pers_peak);
      PrintFormat("v13.8 PERSONAL account: risk %.2f%% of the %s balance, daily stop %.1f%%, max-drawdown stop %.1f%% from the equity peak %.2f; no targets, phases or prop rules",
                  InpPersRiskPct, InpPersCompound ? "current" : "initial", InpPersDailyStopPct, InpPersMaxDDPct, g_pers_peak);
   }
   StageReset();                                          // v13.7.1
   g_nyalign = true;
   int dsh = DayShiftSec(IsTester() ? TimeCurrent() : TimeTradeServer());
   g_nyalign = (dsh == 0);
   for(int r = 0; r < NROLE; r++)
   {
      g_nyal_r[r] = g_nyalign;
      if(!g_tzr_ok[r]) continue;
      g_cur_role = r; g_nyal_r[r] = (DayShiftSec(IsTester() ? TimeCurrent() : TimeTradeServer()) == 0); g_cur_role = -1;
   }
   if(g_nyalign) Print("v13: server day = 17:00 New York (as in the backtests): broker D1/H4 bars used");
   else PrintFormat("v13: server day ends %+d h away from 17:00 New York: D1/H4 bars rebuilt from H1 on the 17:00 NY clock", -dsh / 3600);
   for(int r = 0; r < NROLE; r++) if(g_sym_ok[r]) CalcDayFeatures(r);
   for(int r = 0; r < NROLE; r++)       // v13.3: money-per-lot cross-check at start-up (the warning used to wait for the first trade)
   {
      if(!g_sym_ok[r]) continue;
      double px = SymbolInfoDouble(g_sym[r], SYMBOL_BID);
      double d = (g_df[r].ok && g_df[r].atr > 0 ? 2.5 * g_df[r].atr : 0.01 * px);
      if(d > 0) LossPerLot(g_sym[r], d);
   }
   for(int k = 0; k < NSLV; k++) if(g_sym_ok[SL_ROLE[k]]) LoadSleeve(k);
   g_trading_days = CountTradingDays();
   EventSetTimer(1);
   PrintFormat("v%s ready: preset %s, risk mode %s, initial %.2f, symbols gold=%s silver=%s dow=%s nasdaq=%s audjpy=%s usdjpy=%s eurgbp=%s spx=%s", EA_VER, PresetName(g_preset), RmName(), g_init_bal, g_sym[0], g_sym[1], g_sym[2], g_sym[3], g_sym[4], g_sym[5], g_sym[6], g_sym[7]);
   for(int k = 0; k < NSLV; k++) { g_pend_act[k] = 0; g_pend_dir[k] = 0; g_pend_t[k] = 0; }
   FomcReset();
   if(!IsTester()) FomcRefresh(true);   // v13.5: FOMC dates (calendar / web / cache)
   g_hwm = GvGet("hwm", g_init_bal);
   g_bal_peak = GvGet("bal_peak", g_init_bal);
   g_pmin = (GvGet("pmin", 0) > 0.5);
   g_smin = (GvGet("smin", 0) > 0.5);
   for(int r = 0; r < NROLE; r++) g_last_c1[r] = (datetime)GvGet("lc" + IntegerToString(r), 0);
   g_ready = true;
   PanelBuild();
   OpenSymbolCharts();
   UpdateInfo();
   ChartOverlay(true);
   return 1;
}

// v13.3: stop profile.  TIGHT = the stop multipliers accepted by the walk-forward stop study (fn3, Oct 2026): GOLD T60 / T240
// 2.5 -> 1.875 ATR, GOLD MOM 3.0 -> 2.25, SILV MA 2.5 -> 2.25, NAS IBS 2.5 -> 2.25, EURGBP MR 2.0 -> 1.8 sd, NAS RSI2 time stop
// 170 -> 255 H1 bars.  At a fixed % risk a tighter stop is more leverage: more profit, proportionally more DD, not a better edge.
void SetSleeveStops(const int k, const bool t)
{
   if(k == 0 || k == 1) SL_SLK[k] = t ? 1.875 : g_slk_std[k];        // GOLD T60 / GOLD T240
   else if(k == 2 || k == 5 || k == 9) SL_SLK[k] = t ? 2.25 : g_slk_std[k];   // GOLD MOM / SILV MA / NAS IBS
   else if(k == 18) SL_SLK[k] = t ? 1.8 : g_slk_std[k];               // EURGBP MR (sd of the 50-day z-score)
   else if(k == 8) SL_MAXB[k] = t ? 255 : g_maxb_std[k];              // NAS RSI2 time stop (H1 bars)
}
// v13.7: TIGHT for new entries = input TIGHT, or FUNDED with equity >= initial balance + InpFundedTightCushionPct %.
bool TightNow()
{
   if(InpStopProfile == STOP_TIGHT) return true;
   if(InpFundedTightCushionPct <= 0 || g_preset != PRESET_FUNDED || g_init_bal <= 0) return false;
   return AccountInfoDouble(ACCOUNT_EQUITY) >= g_init_bal * (1.0 + InpFundedTightCushionPct / 100.0);
}
void ApplyStopProfile()
{
   if(!g_sp_init) { for(int k = 0; k < NSLV; k++) { g_slk_std[k] = SL_SLK[k]; g_maxb_std[k] = SL_MAXB[k]; } g_sp_init = true; }
   bool t = (InpStopProfile == STOP_TIGHT);
   for(int k = 0; k < NSLV; k++) if(StopProfSleeve(k)) { g_tight_cur[k] = t; SetSleeveStops(k, t); }
   if(t) Print("v13.3 stop profile TIGHT: GOLD T60/T240 1.875 ATR, GOLD MOM 2.25, SILV MA 2.25, NAS IBS 2.25, EURGBP MR 1.8 sd, NAS RSI2 time stop 255 h (more profit, proportionally more DD, not a better edge)");
   else if(InpFundedTightCushionPct > 0) PrintFormat("v13.7 stop profile: FUNDED - TIGHT for new entries while equity >= initial +%.1f%%, else STANDARD", InpFundedTightCushionPct);
}
// v13.7: every timer call. A flat sleeve follows TightNow(); a sleeve with an open trade keeps the profile of its entry.
void SyncStopProfile()
{
   if(InpStopProfile == STOP_TIGHT || InpFundedTightCushionPct <= 0 || !g_sp_init) return;
   bool now = TightNow();
   for(int k = 0; k < NSLV; k++)
   {
      if(!StopProfSleeve(k)) continue;
      bool t = (g_ss[k].dir != 0 ? g_tight_pos[k] : now);
      if(t != g_tight_cur[k]) { g_tight_cur[k] = t; SetSleeveStops(k, t); }
   }
}

void WaitBanner()                          // shown until InitCore succeeds (live only)
{
   if(!InpShowPanel) return;
   PRect(PNL + "wbg", InpPanelX, InpPanelY, PW, 30, CBG());
   PLabel(PNL + "wtx", InpPanelX + 10, InpPanelY + 8, 9);
   ObjectSetString(0, PNL + "wtx", OBJPROP_TEXT, Tr("title") + "  -  connecting to the broker ...");
   ObjectSetInteger(0, PNL + "wtx", OBJPROP_COLOR, CAMB());
   ChartRedraw(0);
}

int OnInit()
{
   g_ready = false; g_init_tries = 0; g_idle = false;
   MathSrand((int)(GetTickCount() % 100000));
   g_token = 1 + MathRand() * 32768.0 + MathRand();
   EventSetTimer(1);
   int r = InitCore();
   if(r < 0) return INIT_FAILED;
   if(r == 0) { Print("v13: waiting for the broker connection / account data before starting"); WaitBanner(); }
   return INIT_SUCCEEDED;
}

// One copy per account: a copy that finds another live copy (heartbeat < 10 s) stays idle and says so in its panel.
bool OwnInstance()
{
   if(IsTester()) return true;
   datetime now = TimeLocal();
   if(now == g_last_hb && !g_idle) return true;
   g_last_hb = now;
   double tok = GvGet("owner", 0), hb = GvGet("owner_hb", 0);
   bool other = (tok != 0 && tok != g_token && now - (datetime)hb < 10);
   if(other)
   {
      if(!g_idle) { g_idle = true; Print("v13: another copy of this EA already runs on this account - this copy stays idle (attach the EA to ONE chart only)"); PanelBuild(); }
      return false;
   }
   if(g_idle) { g_idle = false; Print("v13: no other copy found - this copy now trades"); PanelBuild(); }
   GvSet("owner", g_token); GvSet("owner_hb", (double)now);
   return true;
}


void OnDeinit(const int reason)
{
   EventKillTimer();
   if(g_ready && g_halt != 2) StageSummary("END OF TEST");   // v13.7.1 (tester only)
   if(g_ready && !g_idle && !IsTester() && GvGet("owner", 0) == g_token) { GlobalVariableDel(GvKey("owner")); GlobalVariableDel(GvKey("owner_hb")); }
   ObjectsDeleteAll(0, PNL);
   ChartOverlayClear();
   ChartRedraw(0);
}

void OnTimer()
{
   g_cur_role = -1;
   if(!g_ready)
   {
      g_init_tries++;
      int r = InitCore();
      if(r < 0) { Alert("DonchianProp: a HEDGING account is required / no tradable symbol - EA removed"); ExpertRemove(); return; }
      if(r == 0) { if(g_init_tries % 10 == 0) WaitBanner(); return; }
      PrintFormat("v13: started after %d s of waiting for the terminal", g_init_tries);
   }
   if(!OwnInstance()) { PanelRefresh(false); return; }
   SyncStopProfile();   // v13.7
   PropChecks();
   for(int k = 0; k < NSLV; k++)          // actions delayed by the news window
   {
      if(g_pend_act[k] == 0 || NewsBlocked()) continue;
      double l, p;
      if(g_pend_act[k] == 2 && SleevePositions(k, l, p) > 0) CloseSleeve(k, "exit signal (after news)");     // v12: a delayed exit never expires
      else if(g_pend_act[k] == 1 && TimeCurrent() - g_pend_t[k] <= 30 * 60 && SleevePositions(k, l, p) == 0) TryEnter(k, g_pend_dir[k]);
      g_pend_act[k] = 0;
   }
   static datetime s_retry = 0;
   if(TimeCurrent() - s_retry >= 10)
   {
      s_retry = TimeCurrent();
      for(int k = 0; k < NSLV; k++) if(g_close_retry[k] && !g_cl_wait[k]) CloseSleeve(k, "retry");   // rejected closes (backoff); market-closed ones: QueuedActions
   }
   QueuedActions();                // v13.3: stops / closes / partials / adds queued while a market was closed - first tick after the reopen
   EntryRetries();                 // v13.3: entries queued while a market was closed
   GotobiTimer();
   ThirdFridayTimer();
   GapLongTimer();
   FomcPostTimer();                 // v13.4 (v15.0 review): FOMC-day post-announcement short (k=22, 23)
   NewSleevesDaily();               // v13.4 (v15.0 review): DXY-JPY / GOLD Z-CONT (k=24, 25)
   FixReversalTimer();
   for(int r = 0; r < NROLE; r++)
   {
      if(!g_sym_ok[r]) continue;
      datetime t = iTime(g_sym[r], PERIOD_H1, 0);
      if(t == 0 || t == g_last_h1[r]) continue;
      if(g_last_h1[r] == 0) { g_last_h1[r] = t; continue; }      // first sight after start: wait for a fresh bar
      g_last_h1[r] = t;
      OnNewH1(r, t);
   }
   if(g_panel && ObjectFind(0, PNL + "bg") < 0) { Print("v13 panel: objects were removed from the chart - rebuilding"); PanelBuild(); }
   if(g_panel && g_redraw_left > 0) { g_redraw_left--; PanelRefresh(true); ChartRedraw(0); }
   UpdateInfo();                   // live sleeve conditions: recomputed on a new H1 bar, prices every 10 s (display only)
   PanelRefresh(false);            // cheap: unchanged texts are not rewritten
   ChartOverlay(false);
}

void OnTick() { }

void OnTradeTransaction(const MqlTradeTransaction &trans, const MqlTradeRequest &request, const MqlTradeResult &result)
{
   if(!g_ready) return;
   if(trans.type != TRADE_TRANSACTION_DEAL_ADD) return;
   if(!HistoryDealSelect(trans.deal)) return;
   if(HistoryDealGetInteger(trans.deal, DEAL_TYPE) == DEAL_TYPE_BALANCE)
   {   // v11.9: a payout withdrawal (or deposit) is not a trading loss/gain: shift the drawdown-brake peak, trailing mark and day reference
      double amt = HistoryDealGetDouble(trans.deal, DEAL_PROFIT);
      if((datetime)HistoryDealGetInteger(trans.deal, DEAL_TIME) > g_start_time && amt != 0)
      {
         g_bal_peak += amt; g_hwm += amt; g_day_ref += amt;
         GvSet("bal_peak", g_bal_peak); GvSet("hwm", g_hwm); GvSet("dayref", g_day_ref);
         PrintFormat("v13: balance operation %+.2f - drawdown brake, trailing limit and day reference adjusted", amt);
      }
      return;
   }
   if(HistoryDealGetInteger(trans.deal, DEAL_ENTRY) == DEAL_ENTRY_IN)
   {
      long dn = DayNum((datetime)HistoryDealGetInteger(trans.deal, DEAL_TIME));
      if(dn != (long)GvGet("lastdealday", -1)) { GvSet("lastdealday", (double)dn); g_trading_days = CountTradingDays(); }
   }
}

void OnChartEvent(const int id, const long &lparam, const double &dparam, const string &sparam)
{
   if(!g_ready || g_idle || id != CHARTEVENT_OBJECT_CLICK || StringFind(sparam, PNL) != 0) return;
   ObjectSetInteger(0, sparam, OBJPROP_STATE, 0);
   uint ms = GetTickCount();
   if(ms == 0) ms = 1;
   if(sparam == PNL + "b_min") { g_pmin = !g_pmin; GvSet("pmin", g_pmin ? 1 : 0); PanelBuild(); return; }
   if(sparam == PNL + "b_smin") { g_smin = !g_smin; GvSet("smin", g_smin ? 1 : 0); PanelBuild(); return; }
   if(sparam == PNL + "b_up") g_pos_off--;
   else if(sparam == PNL + "b_dn") g_pos_off++;
   else if(sparam == PNL + "b_p1") SetPreset(PRESET_PHASE1);
   else if(sparam == PNL + "b_p2") SetPreset(PRESET_PHASE2);
   else if(sparam == PNL + "b_fu") SetPreset(PRESET_FUNDED);
   else if(sparam == PNL + "b_cu") SetPreset(PRESET_CUSTOM);
   else if(sparam == PNL + "b_rm") { g_rmode = (g_rmode == RM_SAFE ? RM_FAST : (g_rmode == RM_FAST ? RM_TURBO : RM_SAFE)); GvSet("rmode", g_rmode); PrintFormat("v13 risk mode -> %s (risk unit %.2f%%)", RmName(), RiskUnitPct()); }
   else if(sparam == PNL + "b_pause") { g_paused = !g_paused; GvSet("paused", g_paused ? 1 : 0); }
   else if(sparam == PNL + "b_lang") { g_lang = (g_lang == LANG_FA ? LANG_EN : LANG_FA); GvSet("lang", g_lang); }
   else if(sparam == PNL + "b_close")
   {
      if(g_arm_close != 0 && ms - g_arm_close <= 5000) { CloseAll("manual close-all"); g_paused = true; GvSet("paused", 1); g_arm_close = 0; }
      else g_arm_close = ms;
   }
   else if(sparam == PNL + "b_reset")
   {
      if(g_halt == 0 && !g_target_hit) { g_arm_reset = 0; }
      else if(g_arm_reset != 0 && ms - g_arm_reset <= 5000)
      {
         double eq = AccountInfoDouble(ACCOUNT_EQUITY);
         if(g_halt == 2 && !IsPersonal() && eq <= GuardFloor())
            Print("v13: reset refused - equity is still at the max-loss guard");
         else { if(IsPersonal() && g_halt == 2) { g_pers_peak = eq; GvSet("ppeak", eq); Print("v13.8 personal: max-drawdown stop reset by the user - the current equity is the new peak"); }
                g_halt = 0; GvSet("halt", 0); g_target_hit = false; GvSet("target_" + PresetName(g_preset), 0); Print("v13: guards reset by user"); }
         g_arm_reset = 0;
      }
      else g_arm_reset = ms;
   }
   PanelRefresh(true);
}
