#!/usr/bin/env python3
"""Fill intraweek holes of a harness M1 file with Dukascopy BID bars (cross-validated, basis-corrected).
usage: fill_gaps.py BIN_IN BIN_OUT SYM DUKAS_DIR DSYM [--min-gap 30] [--dry]
 - BIN files: int64 t (UK local: GMT/BST), f64 o,h,l,c.  Dukascopy bars are UTC and converted to the same clock.
 - Only minutes inside a hole of >= min-gap minutes (both ends on weekdays, outside Fri 20:00-Sun 22:00 UTC and the 21:00-23:00 UTC
   daily break) are inserted; existing bars are never changed.
 - Basis correction: Dukascopy prices are shifted by the median (HistData - Dukascopy) close difference over matching minutes of
   the same UTC day (falls back to the whole period).  The cross-validation statistics are printed."""
import sys,os,numpy as np,pandas as pd
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
import dukas_read as R
from clockrule import uk_summer_mask_utc
DT=np.dtype([('t','<i8'),('o','<f8'),('h','<f8'),('l','<f8'),('c','<f8')])
def main():
    a=[x for x in sys.argv[1:] if not x.startswith('--')]; dry='--dry' in sys.argv
    mg=int(sys.argv[sys.argv.index('--min-gap')+1]) if '--min-gap' in sys.argv else 30
    if '--min-gap' in sys.argv: a=[x for x in a if x!=str(mg)]
    bi,bo,sym,dd,ds=a[:5]
    B=np.fromfile(bi,dtype=DT); K=R.read_dir(os.path.join(dd,ds,'BID'),ds)
    if K is None or not len(K): print(sym,'no dukascopy data'); return
    kt=K['t']+3600*uk_summer_mask_utc(K['t'])          # UTC -> UK local (harness file clock)
    # cross-validation on overlapping minutes
    idx=np.searchsorted(B['t'],kt); idx=np.minimum(idx,len(B)-1); hit=B['t'][idx]==kt
    diff=B['c'][idx[hit]]-K['c'][hit]; day=(K['t'][hit]//86400)
    med_day=pd.Series(diff).groupby(day).median(); med_all=float(np.median(diff)) if len(diff) else 0.0
    rel=np.abs(diff-med_all)/np.median(B['c'][idx[hit]])*1e4 if len(diff) else np.array([0.])
    ret_h=np.diff(B['c'][idx[hit]]); ret_k=np.diff(K['c'][hit]); cr=np.corrcoef(ret_h,ret_k)[0,1] if len(ret_h)>10 else np.nan
    # holes in the harness file (UTC based tests)
    def utc_of(t): return t-3600*uk_summer_mask_utc(t-3600)   # approx inverse (exact except the switch hour)
    bu=utc_of(B['t']); g=np.diff(bu); s=bu[:-1]; e=bu[1:]
    def closed(x):
        dw=((x//86400)+3)%7; hr=(x%86400)/3600.
        return ((dw==4)&(hr>=20))|(dw==5)|((dw==6)&(hr<22))|((hr>=21)&(hr<23))
    holes=[(s[i],e[i]) for i in np.where((g>=mg*60)&(g<3*86400))[0]]
    ku=K['t']; take=np.zeros(len(K),bool)
    for s0,e0 in holes:
        lo,hi=np.searchsorted(ku,s0+60),np.searchsorted(ku,e0)
        take[lo:hi]=True
    take&=~closed(ku)&~hit
    off=np.array([med_day.get(d,med_all) for d in ku[take]//86400])
    N=np.zeros(int(take.sum()),dtype=DT); N['t']=kt[take]
    for c in 'ohlc': N[c]=K[c][take]+off
    print(f'{sym}: dukascopy bars {len(K)}, overlap {int(hit.sum())}, ret corr {cr:.3f}, median basis {med_all:.5g}, |dev| p50/p95 {np.median(rel):.2f}/{np.percentile(rel,95):.2f} bp; holes>={mg}m {len(holes)}, bars inserted {len(N)}')
    if len(N):
        yrs=pd.Series(pd.to_datetime(ku[take],unit='s').year).value_counts().sort_index().to_dict(); print('   inserted by year',yrs)
    if not dry:
        M=np.concatenate([B,N]); M=M[np.argsort(M['t'],kind='stable')]; M.tofile(bo)
if __name__=='__main__': main()
