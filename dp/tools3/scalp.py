# scalp.py RULE [LH] : SC-series offline screen (see prereg/PREREG_v13.14.txt). RULE in SC1 SC2 SC3; K / variant via env.
import sys, os, numpy as np, pandas as pd
from numba import njit
sys.path.insert(0, '/home/user/221133ycc/dp/bt/data/scripts')
from clockrule import uk_summer_mask_utc
BIN = '/home/user/221133ycc/dp/bt/data/bin/'; BINLH = '/home/user/221133ycc/dp/bt/data/bin_lh/'
DT = np.dtype([('t', '<i8'), ('o', '<f8'), ('h', '<f8'), ('l', '<f8'), ('c', '<f8')])
# spread, commission per lot per side (USD), contract, quote->USD approx (JPY pairs: 1/150)
COST = {'XAUUSD': (0.44, 1.5, 100, 1), 'XAGUSD': (0.037, 1.5, 5000, 1), 'NAS100': (1.70, 0, 10, 1), 'SPX500': (0.65, 0, 10, 1),
        'US30': (3.9, 0, 10, 1), 'USDJPY': (0.0097, 1.5, 100000, 1 / 150), 'AUDJPY': (0.0115, 1.5, 100000, 1 / 150),
        'EURGBP': (0.00010, 1.5, 100000, 1.27), 'GBPUSD': (0.00012, 1.5, 100000, 1)}
GRID = {'USDJPY': 0.5, 'AUDJPY': 0.5, 'EURGBP': 0.005, 'GBPUSD': 0.005, 'XAUUSD': 10.0, 'XAGUSD': 0.5, 'NAS100': 100.0, 'SPX500': 25.0, 'US30': 100.0}

def load(sym, lh=False):
    if lh:
        a = np.fromfile(BINLH + sym + '_m1.bin', dtype=DT)
        t = a['t'].copy(); t = t - 3600 * uk_summer_mask_utc(t - 3600)   # UK local -> UTC
        a['t'] = t; return a
    return np.load(BIN + sym + '_utc.npy') if os.path.exists(BIN + sym + '_utc.npy') else np.fromfile(BIN + sym + '_m1.bin', dtype=DT)

@njit(cache=False)
def run(t, o, h, l, c, sig_i, sig_d, sig_lim, sig_px, sig_tp, sig_sl, sig_tb, spr, slip, comm):
    n = len(sig_i); N = len(t)
    out_i = np.full(n, -1); pnl = np.zeros(n); risk = np.zeros(n); busy = -1
    for s in range(n):
        i = sig_i[s]
        if i <= busy or i >= N - 1: continue
        d = sig_d[s]
        if sig_lim[s] == 1:
            lv = sig_px[s]
            if d > 0: ep = lv            # buy limit at the ask = lv  (filled when bid low <= lv - spr; checked by the caller)
            else: ep = lv                # sell limit at the bid = lv
            cost_e = 0.0
        else:
            if d > 0: ep = o[i] + spr + slip
            else: ep = o[i] - slip
        tp = ep + d * sig_tp[s]; sl = ep - d * sig_sl[s]
        ex = 0.0; j = i; done = False
        jmax = min(N - 1, i + sig_tb[s])
        while j <= jmax:
            if t[j] - t[i] > sig_tb[s] * 60 + 3600: break   # data hole: leave at the last close seen
            if d > 0:
                if l[j] <= sl:
                    ex = min(sl, o[j]) - slip if j > i else sl - slip; done = True; break
                if j > i and h[j] >= tp: ex = tp; done = True; break
            else:
                if h[j] + spr >= sl:
                    ex = max(sl, o[j] + spr) + slip if j > i else sl + slip; done = True; break
                if j > i and l[j] + spr <= tp: ex = tp; done = True; break
            j += 1
        if not done:
            j = min(j, jmax)
            ex = c[j] if d > 0 else c[j] + spr
        pnl[s] = d * (ex - ep) - comm; risk[s] = sig_sl[s]; out_i[s] = i; busy = j
    return out_i, pnl, risk

def signals(rule, a, sym, K):
    t, o, h, l, c = a['t'], a['o'], a['h'], a['l'], a['c']
    N = len(t)
    if rule in ('SC1', 'SC2'):
        r5 = np.full(N, np.nan); r5[5:] = c[5:] - c[:-5]
        ok = np.zeros(N, bool); ok[5:] = (t[5:] - t[:-5]) == 300
        r5[~ok] = np.nan
        sd = pd.Series(r5).rolling(27600, min_periods=10000).std().shift(1).values
        trig = ok & (np.abs(r5) >= K * sd)
        idx = np.nonzero(trig)[0]; idx = idx[idx < N - 2]
        m = np.abs(r5[idx]); dmove = np.sign(r5[idx])
        si = idx + 1
        if rule == 'SC1': d = -dmove; tp = 0.5 * m; sl = 1.0 * m
        else: d = dmove; tp = 1.0 * m; sl = 0.5 * m
        return si, d.astype(np.int64), np.zeros(len(si), np.int64), np.zeros(len(si)), tp, sl, np.full(len(si), 60, np.int64)
    if rule == 'SC4':
        si, d, lim, px, tp, sl, tb = signals('SC3X', a, sym, K)
        return si, -d, np.zeros(len(si), np.int64), px, tp, sl, tb
    if rule in ('SC3', 'SC3X'):
        g = GRID[sym]; spr = COST[sym][0]
        # ATR15: mean 15-min range over the previous 20 days
        b = t // 900
        df = pd.DataFrame({'b': b, 'h': h, 'l': l}).groupby('b').agg(h=('h', 'max'), l=('l', 'min'))
        rng = (df.h - df.l).rolling(1900, min_periods=500).mean().shift(1)
        A = rng.reindex(b).values
        day = (t - 22 * 3600) // 86400
        # running day low / high before bar i
        s = pd.DataFrame({'d': day, 'h': h, 'l': l})
        dl = s.groupby("d").l.cummin().shift(1).to_numpy().copy(); dh = s.groupby("d").h.cummax().shift(1).to_numpy().copy()
        newday = np.r_[True, day[1:] != day[:-1]]; dl[newday] = np.inf; dh[newday] = -np.inf
        hi60 = pd.Series(h).rolling(60).max().shift(1).values; lo60 = pd.Series(l).rolling(60).min().shift(1).values
        pc = np.r_[np.nan, c[:-1]]
        L = np.floor(pc / g) * g; U = np.ceil(pc / g) * g
        if rule == 'SC3X':
            buy = (l <= L - 0.1 * A) & (o > L) & (dl > L) & (hi60 >= L + 0.5 * A) & np.isfinite(A)
            sell = (h >= U + 0.1 * A) & (o < U) & (dh < U) & (lo60 <= U - 0.5 * A) & np.isfinite(A) & ~buy
            ib = np.nonzero(buy)[0]; isl = np.nonzero(sell)[0]
            si = np.r_[ib, isl]; d = np.r_[np.ones(len(ib)), -np.ones(len(isl))].astype(np.int64); px = np.r_[L[ib], U[isl]]
            o_ = np.argsort(si, kind='stable'); si, d, px = si[o_], d[o_], px[o_]
            si = si + 1; m_ = si < N - 1; si, d, px = si[m_], d[m_], px[m_]; Aa = A[si - 1]
            return si, d, np.zeros(len(si), np.int64), px, 0.5 * Aa, 0.5 * Aa, np.full(len(si), 30, np.int64)
        buy = (l <= L - spr) & (o > L) & (dl > L) & (hi60 >= L + 0.5 * A) & np.isfinite(A)
        sell = (h >= U) & (o < U) & (dh < U) & (lo60 <= U - 0.5 * A) & np.isfinite(A) & ~buy
        ib = np.nonzero(buy)[0]; isl = np.nonzero(sell)[0]
        si = np.r_[ib, isl]; d = np.r_[np.ones(len(ib)), -np.ones(len(isl))].astype(np.int64); px = np.r_[L[ib], U[isl]]
        o_ = np.argsort(si, kind='stable'); si, d, px = si[o_], d[o_], px[o_]
        Aa = A[si]
        return si, d, np.ones(len(si), np.int64), px, 0.5 * Aa, 0.5 * Aa, np.full(len(si), 30, np.int64)

def evaluate(rule, sym, K, lh=False, cx=1.0):
    a = load(sym, lh)
    si, d, lim, px, tp, sl, tb = signals(rule, a, sym, K)
    spr, cm, ct, q = COST[sym]
    spr *= cx; slip = 0.5 * spr; comm = 2 * cm * cx / ct / q   # commission in price units (quote ccy)
    oi, pnl, risk = run(a['t'], a['o'], a['h'], a['l'], a['c'], si, d, lim, px, tp, sl, tb, spr, slip, comm)
    m = oi >= 0
    T = pd.DataFrame({'t': a['t'][oi[m]], 'pnl': pnl[m], 'R': pnl[m] / risk[m]})
    return T

def stats(T):
    if len(T) < 5: return dict(n=len(T))
    y = pd.to_datetime(T.t, unit='s').dt.year
    g = T.pnl[T.pnl > 0].sum(); b = -T.pnl[T.pnl < 0].sum(); pf = g / b if b > 0 else np.inf
    dd = T.groupby(T.t // 86400).R.sum(); tt = dd.mean() / dd.std() * np.sqrt(len(dd)) if dd.std() > 0 else 0
    per = [T.R[(y >= a) & (y <= b_)].sum() for a, b_ in ((2016, 2018), (2019, 2022), (2023, 2026))]
    yrs = (T.t.iloc[-1] - T.t.iloc[0]) / 365.25 / 86400
    return dict(n=len(T), perYr=round(len(T) / max(yrs, 1e-9), 1), PF=round(pf, 3), meanR=round(T.R.mean(), 4), t=round(tt, 2),
                P1=round(per[0], 1), P2=round(per[1], 1), P3=round(per[2], 1))

if __name__ == '__main__':
    rule = sys.argv[1]; K = float(os.environ.get('K', '4'))
    syms = os.environ.get('SYMS', 'XAUUSD,XAGUSD,NAS100,SPX500,USDJPY,AUDJPY,EURGBP,GBPUSD,US30').split(',')
    rows = []
    for s in syms:
        T = evaluate(rule, s, K); T = T[T.t >= 1451606400]
        T2 = evaluate(rule, s, K, cx=2.0); T2 = T2[T2.t >= 1451606400]
        r = stats(T); r2 = stats(T2)
        lhR = np.nan
        if s != 'US30':
            TL = evaluate(rule, s, K, lh=True); TL = TL[(TL.t >= 1275350400) & (TL.t < 1451606400)]; lhR = round(TL.R.sum(), 1) if len(TL) else np.nan
        r.update(sym=s, PFx2=r2.get('PF'), LH_R=lhR)
        ok = (r.get('PF', 0) >= 1.15 and (r2.get('PF') or 0) > 1 and r.get('t', 0) >= 2.5 and min(r.get('P1', -1), r.get('P2', -1), r.get('P3', -1)) > 0
              and r.get('perYr', 0) >= 8 and (s == 'US30' or (lhR == lhR and lhR >= 0)))
        r['PASS'] = ok; rows.append(r); print(r, flush=True)
    pd.DataFrame(rows).to_csv(f'/home/user/221133ycc/dp/notes/scalp_{rule}_K{K}.csv', index=False)
