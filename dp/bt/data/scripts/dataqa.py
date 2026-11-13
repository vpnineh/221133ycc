#!/usr/bin/env python3
"""Data-quality report for harness M1 files (<SYM>_m1.bin: int64 t, f64 o,h,l,c; UK-local stamps).
usage: dataqa.py DATADIR [SYM,...] [--from 2016-01-01]
Reports per symbol: rows, duplicate / non-increasing stamps, invalid bars, intraweek gaps (>=60 min outside the weekend
and the 20:00-23:30 UTC daily break) per year, top gaps, isolated spikes (bar extreme > K x local median range and reverting)."""
import sys,os,numpy as np,pandas as pd
DT=np.dtype([('t','<i8'),('o','<f8'),('h','<f8'),('l','<f8'),('c','<f8')])
def last_sun(y,m):
    d=pd.Timestamp(y,m,1)+pd.offsets.MonthEnd(0); return d-pd.Timedelta(days=(d.weekday()+1)%7)
def uk_to_utc(t):
    ts=t.copy(); yrs=pd.to_datetime(t,unit='s').year.values
    for y in np.unique(yrs):
        a=int(last_sun(y,3).timestamp())+3600; b=int(last_sun(y,10).timestamp())+7200
        m=(yrs==y)&(t>=a)&(t<b); ts[m]-=3600
    return ts
def qa(path,sym,t0):
    a=np.fromfile(path,dtype=DT); r=dict(sym=sym,rows=len(a))
    d=np.diff(a['t']); r['dup']=int((d==0).sum()); r['nonincr']=int((d<0).sum())
    o,h,l,c=a['o'],a['h'],a['l'],a['c']
    r['bad']=int(((h<l)|(o>h)|(o<l)|(c>h)|(c<l)|(l<=0)|~np.isfinite(c)).sum())
    u=uk_to_utc(a['t']); k=u>=t0; u=u[k]; o,h,l,c=o[k],h[k],l[k],c[k]
    if len(u)<10: return r,[]
    g=np.diff(u); s=u[:-1]; e=u[1:]
    def wk(x):
        dw=((x//86400)+3)%7; hr=(x%86400)/3600.
        return (dw==4)&(hr>=20)|(dw==5)|(dw==6)&(hr<23.5)
    def brk(x): hr=(x%86400)/3600.; return (hr>=20)&(hr<23.5)
    m=(g>=3600)&~wk(s+60)&~wk(e-60)&~(brk(s+60)&brk(e-60))&(g<4*86400)
    G=pd.DataFrame(dict(start=pd.to_datetime(s[m]+60,unit='s'),hours=g[m]/3600.))
    r['gaps60']=len(G); r['gap_h']=round(G.hours.sum(),1)
    r['gap_by_year']=G.groupby(G.start.dt.year).hours.sum().round(1).to_dict()
    rng=pd.Series(h-l); med=rng.rolling(240,min_periods=30,center=True).median().values
    pc=np.r_[c[0],c[:-1]]; nc=np.r_[o[1:],o[-1]]
    up=(h-np.maximum(pc,nc)); dn=(np.minimum(pc,nc)-l); ex=np.maximum(up,dn)
    sp=(ex>np.maximum(25*med,8*np.nanmedian(rng)))&(med>0)
    r['spikes']=int(sp.sum())
    r['spike_eg']=[str(pd.to_datetime(x,unit='s')) for x in u[sp][:3]]
    return r,G.sort_values('hours',ascending=False).head(5)
if __name__=='__main__':
    D=sys.argv[1]; syms=None; t0=int(pd.Timestamp('2016-01-01').timestamp())
    args=sys.argv[2:]
    if '--from' in args: i=args.index('--from'); t0=int(pd.Timestamp(args[i+1]).timestamp()); del args[i:i+2]
    if args: syms=args[0].split(',')
    syms=syms or sorted(f[:-7] for f in os.listdir(D) if f.endswith('_m1.bin'))
    for s in syms:
        r,G=qa(os.path.join(D,s+'_m1.bin'),s,t0)
        print(f"{s}: rows {r['rows']} dup {r['dup']} nonincr {r['nonincr']} bad {r['bad']} gaps>=1h {r.get('gaps60')} ({r.get('gap_h')} h) spikes {r.get('spikes')} {r.get('spike_eg')}")
        print('   gap hours by year',r.get('gap_by_year'))
        for _,x in (G.iterrows() if len(G) else []): print(f"   gap {x.start} {x.hours:.1f}h")
