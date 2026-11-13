"""DJIUSD.prop MT5 export -> US30 files. Clock is NOT UTC for most of the history (see CLOCK_REPORT.md section DJI)."""
import sys, os, json, datetime as dt, numpy as np, pandas as pd
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dji_load import load
from rules import eu_dates, day_ord
from clockrule import uk_summer_mask_utc
from clean import coverage, DT
src, W = sys.argv[1], sys.argv[2]
TS = lambda s: int(pd.Timestamp(s).value // 10**9)
B_A = TS('2018-04-04 16:38:00')      # label time: source switch k=+4 -> +2 (2h hole in real time)
B_Z21 = TS('2021-03-20 00:00:00')    # 2021 only: switched to +3 one week before EU rule
B_B = TS('2025-05-19 22:01:00')      # label time: source switch k=+3 -> 0 (UTC)
def eu_summer_label(t):
    d = t // 86400; y = (d.astype('datetime64[D]').astype('datetime64[Y]').astype(int) + 1970); s = np.zeros(len(t), bool)
    for yy in np.unique(y):
        a, b = eu_dates(yy); m = y == yy; s[m] = (d[m] >= day_ord(a)) & (d[m] < day_ord(b))
        if yy == 2021: s[m] = (d[m] >= B_Z21 // 86400) & (d[m] < day_ord(b))
    return s
def k_of(t):
    """label - UTC in hours"""
    t = np.asarray(t); sm = eu_summer_label(t)
    k = np.where(t < B_A, np.where(sm, 4, 3), np.where(t < B_B, np.where((t < B_Z21), 2, np.where(sm, 3, 2)), 0))
    return k
a = load(src); st = {'sym': 'US30', 'source_rows': int(len(a))}
t = a['t']
# classify coarse vs M1
FIRST_M1 = TS('2017-05-12 13:46:00')
coarse = a[t < FIRST_M1]; m1 = a[t >= FIRST_M1]
st['coarse_rows'] = int(len(coarse))
# ---- coarse file
ct = coarse['t']; tf = np.where(ct < TS('2016-12-23'), 86400, 3600).astype('<i4')
cdt = np.dtype([('t', '<i8'), ('o', '<f8'), ('h', '<f8'), ('l', '<f8'), ('c', '<f8'), ('tf', '<i4'), ('t_label', '<i8')])
co = np.empty(len(coarse), dtype=cdt)
sm = eu_summer_label(ct); kk = np.where(sm, 4, 3)
co['t'] = np.where(tf == 3600, ct - kk * 3600, ct - (ct % 86400))   # H1: label-3/4h (verified vs SPX hourly); D1: calendar date 00:00 UTC (clock unverified)
for k in 'ohlc': co[k] = coarse[k]
co['tf'] = tf; co['t_label'] = ct
np.save(f'{W}/bin/US30_coarse_pre_m1_utc.npy', co)
st['coarse_daily_rows'] = int((tf == 86400).sum()); st['coarse_hourly_rows'] = int((tf == 3600).sum())
st['coarse_range_label'] = [str(pd.Timestamp(int(ct[0]), unit='s')), str(pd.Timestamp(int(ct[-1]), unit='s'))]
# ---- M1 cleaning
a = m1; st['m1_raw_rows'] = int(len(a))
o_ = np.argsort(a['t'], kind='stable'); st['unsorted_rows'] = int((o_ != np.arange(len(o_))).sum()); a = a[o_]
dup = np.r_[False, a['t'][1:] == a['t'][:-1]]; st['dup_label_ts'] = int(dup.sum()); a = a[~dup]
o, h, l, c = a['o'], a['h'], a['l'], a['c']
with np.errstate(invalid='ignore'):
    m_nan = ~np.isfinite(o) | ~np.isfinite(h) | ~np.isfinite(l) | ~np.isfinite(c); m_hl = h < l; m_np = (o <= 0) | (h <= 0) | (l <= 0) | (c <= 0)
    m_out = (o > h) | (o < l) | (c > h) | (c < l)
bad = m_nan | m_hl | m_np | m_out
st.update(bad_nan=int(m_nan.sum()), bad_high_lt_low=int(m_hl.sum()), bad_nonpositive=int(m_np.sum()), bad_oc_outside_hl=int(m_out.sum()), bad_total=int(bad.sum()))
a = a[~bad]
tl = a['t']; k = k_of(tl); utc = tl - k * 3600
st['k_counts'] = {int(v): int((k == v).sum()) for v in np.unique(k)}
o2 = np.argsort(utc, kind='stable'); st['reordered_after_conversion'] = int((o2 != np.arange(len(o2))).sum()); a = a[o2]; utc = utc[o2]
dup2 = np.r_[False, utc[1:] == utc[:-1]]; st['dup_after_conversion'] = int(dup2.sum()); a = a[~dup2]; utc = utc[~dup2]
dd = utc // 86400; dow = (dd + 3) % 7; hr = (utc % 86400) // 3600
flat = (a['o'] == a['h']) & (a['h'] == a['l']) & (a['l'] == a['c'])
wk = (dow == 5) | ((dow == 4) & (hr >= 22)) | ((dow == 6) & (hr < 20))
st['weekend_bars_any'] = int(wk.sum()); st['weekend_flat_dropped'] = int((wk & flat).sum()); st['flat_bars_total'] = int(flat.sum())
a = a[~(wk & flat)]; utc = utc[~(wk & flat)]
out = np.empty(len(a), dtype=DT); out['t'] = utc
for kk_ in 'ohlc': out[kk_] = a[kk_]
st['rows'] = int(len(out)); st['first_utc'] = str(pd.Timestamp(int(utc[0]), unit='s')); st['last_utc'] = str(pd.Timestamp(int(utc[-1]), unit='s'))
r = np.abs(np.diff(np.log(out['c']))) * 1e4; st['moves_gt_300bp_1m'] = int((r > 300).sum()); st['max_move_bp_1m'] = float(r.max())
st['cov'] = coverage(utc)
np.save(f'{W}/bin/US30_utc.npy', out)
uk = out.copy(); uk['t'] = utc + 3600 * uk_summer_mask_utc(utc); uk.tofile(f'{W}/bin/US30_m1.bin')
json.dump(st, open(f'{W}/out/clean_US30.json', 'w'), indent=1)
print(json.dumps({k: v for k, v in st.items() if k != 'cov'}, indent=1)); print(st['cov']['top_gaps'][:4]); print(st['cov']['low_weeks'][:10])
