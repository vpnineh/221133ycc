import sys, os, json, numpy as np, pandas as pd
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from clockrule import *
from multiprocessing import Pool
DT = np.dtype([('t', '<i8'), ('o', '<f8'), ('h', '<f8'), ('l', '<f8'), ('c', '<f8')])
W = sys.argv[1]
SYMS = ['XAUUSD','XAGUSD','NAS100','SPX500','DAX','USDJPY','AUDJPY','EURGBP','GBPUSD','USDCAD','EURUSD','EURJPY','GBPJPY','AUDUSD','AUDNZD','NZDUSD','USDCHF']
def coverage(t):
    """t: UTC epoch seconds sorted. returns dict"""
    ts = pd.to_datetime(t, unit='s')
    yr = pd.Series(1, index=ts).groupby(ts.year).sum().to_dict()
    d = np.diff(t); idx = np.argsort(d)[::-1][:6]
    gaps = [dict(days=round(float(d[i]) / 86400, 3), start=str(ts[i]), end=str(ts[i + 1])) for i in idx]
    # weeks (Mon-start, UTC) with no bars or < 25% of median weekly bars
    wk = pd.Series(1, index=ts).resample('W-SUN').sum()
    med = float(wk[wk > 0].median())
    low = wk[wk < 0.25 * med]
    # exclude first/last partial weeks
    low = low.iloc[(low.index > wk.index[0]) & (low.index < wk.index[-1])] if len(low) else low
    miss = [dict(week_end=str(i.date()), bars=int(v)) for i, v in low.items()]
    # number of gaps longer than 1 day not explained by weekend: count weekday-gaps > 6h
    return dict(per_year={int(k): int(v) for k, v in yr.items()}, top_gaps=gaps, median_weekly_bars=med, low_weeks=miss)
def one(sym):
    st = {'sym': sym}
    a = np.load(f'{W}/raw/{sym}.npy'); st['raw_rows'] = int(len(a))
    nan = ~np.isfinite(a['o']) | ~np.isfinite(a['h']) | ~np.isfinite(a['l']) | ~np.isfinite(a['c'])
    st['nan_rows'] = int(nan.sum())
    a = a[np.argsort(a['t'], kind='stable')]
    t = a['t']; dup = np.r_[False, t[1:] == t[:-1]]
    # conflicting duplicates: dup row differs from kept first row of the same timestamp
    first_idx = np.maximum.accumulate(np.where(~dup, np.arange(len(a)), 0))
    same = (a['o'] == a['o'][first_idx]) & (a['h'] == a['h'][first_idx]) & (a['l'] == a['l'][first_idx]) & (a['c'] == a['c'][first_idx])
    st['dup_label_ts'] = int(dup.sum()); st['dup_label_conflicting'] = int((dup & ~same).sum())
    a = a[~dup]
    o, h, l, c = a['o'], a['h'], a['l'], a['c']
    with np.errstate(invalid='ignore'):
        m_nan = ~np.isfinite(o) | ~np.isfinite(h) | ~np.isfinite(l) | ~np.isfinite(c)
        m_hl = h < l
        m_np = (o <= 0) | (h <= 0) | (l <= 0) | (c <= 0)
        m_out = (o > h) | (o < l) | (c > h) | (c < l)
    bad = m_nan | m_hl | m_np | m_out
    st.update(bad_nan=int(m_nan.sum()), bad_high_lt_low=int(m_hl.sum()), bad_nonpositive=int(m_np.sum()), bad_oc_outside_hl=int(m_out.sum()), bad_total=int(bad.sum()))
    a = a[~bad]
    # clock -> UTC
    tl = a['t']; off = label_offset_hours(tl)
    utc = tl - off * 3600
    st['n_summer_label_rows'] = int((off == -4).sum())
    # after conversion: must be strictly increasing; fix any ordering/duplicates (keep first)
    o2 = np.argsort(utc, kind='stable'); st['reordered_after_conversion'] = int((o2 != np.arange(len(o2))).sum())
    a = a[o2]; utc = utc[o2]
    dup2 = np.r_[False, utc[1:] == utc[:-1]]; st['dup_after_conversion'] = int(dup2.sum())
    a = a[~dup2]; utc = utc[~dup2]
    # weekend filler: flat bars on UTC Saturday, Friday >= 22:00 UTC, Sunday < 20:00 UTC
    dd = utc // 86400; dow = (dd + 3) % 7; hr = (utc % 86400) // 3600
    flat = (a['o'] == a['h']) & (a['h'] == a['l']) & (a['l'] == a['c'])
    wkend = (dow == 5) | ((dow == 4) & (hr >= 22)) | ((dow == 6) & (hr < 20))
    st['weekend_bars_any'] = int(wkend.sum()); st['weekend_flat_dropped'] = int((wkend & flat).sum())
    st['flat_bars_total'] = int(flat.sum())
    keep = ~(wkend & flat)
    a = a[keep]; utc = utc[keep]
    n = len(a)
    out = np.empty(n, dtype=DT); out['t'] = utc
    for k in 'ohlc': out[k] = a[k]
    st['rows'] = int(n); st['first_utc'] = str(pd.Timestamp(int(utc[0]), unit='s')); st['last_utc'] = str(pd.Timestamp(int(utc[-1]), unit='s'))
    # extreme 1-min close-to-close moves (not dropped, reported)
    r = np.abs(np.diff(np.log(out['c']))) * 1e4; st['moves_gt_300bp_1m'] = int((r > 300).sum()); st['max_move_bp_1m'] = float(r.max())
    st['cov'] = coverage(utc)
    os.makedirs(f'{W}/bin', exist_ok=True)
    np.save(f'{W}/bin/{sym}_utc.npy', out)
    uk = out.copy(); uk['t'] = utc + 3600 * uk_summer_mask_utc(utc)
    uk.tofile(f'{W}/bin/{sym}_m1.bin')
    json.dump(st, open(f'{W}/out/clean_{sym}.json', 'w'), indent=1)
    os.remove(f'{W}/raw/{sym}.npy')
    return sym, n
if __name__ == '__main__':
    syms = sys.argv[2].split(',') if len(sys.argv) > 2 else SYMS
    with Pool(3) as p:
        for r in p.imap_unordered(one, syms): print(r, flush=True); os.system('df -h /tmp | tail -1')
