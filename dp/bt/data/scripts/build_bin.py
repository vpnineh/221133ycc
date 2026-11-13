#!/usr/bin/env python3
"""HistData zips -> harness M1 files.  usage: build_bin.py ZIPDIR BINDIR [SYM,SYM,...] [--keep-first]
ZIPDIR/<SYM>/*.zip as written by histdata_get.py (nested zips are fine). Writes BINDIR/<SYM>_m1.bin (int64 t, f64 o,h,l,c,
t = UK local clock GMT/BST, which the harness converts with DSHIFT=1) and BINDIR/<SYM>_utc.npy (same bars, true UTC).
Cleaning: stable sort by label; identical duplicate minutes dropped; CONFLICTING duplicates (2026 monthly-file glitch: a flat
1-tick copy next to the real bar) resolved by fix_dups rule (largest range wins; two real bars are merged) unless --keep-first
(the original 2026-10 build); bad bars (NaN, h<l, <=0, o/c outside h-l) dropped; label (EST no DST, see clockrule.py) -> UTC;
weekend flat filler bars dropped."""
import sys,os,io,zipfile,glob,numpy as np,pandas as pd
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from clockrule import label_offset_hours, uk_summer_mask_utc
DT=np.dtype([('t','<i8'),('o','<f8'),('h','<f8'),('l','<f8'),('c','<f8')])
def csvs(zf):
    for i in zf.infolist():
        n=i.filename.lower()
        if n.endswith('.zip'): yield from csvs(zipfile.ZipFile(io.BytesIO(zf.read(i))))
        elif n.endswith('.csv'): yield zf.read(i)
def load(zdir):
    parts=[]
    for f in sorted(glob.glob(os.path.join(zdir,'*.zip'))):
        for b in csvs(zipfile.ZipFile(f)):
            d=pd.read_csv(io.BytesIO(b),sep=';',header=None,usecols=[0,1,2,3,4],names=['ts','o','h','l','c'],dtype={'ts':str})
            d['t']=pd.to_datetime(d.ts,format='%Y%m%d %H%M%S',errors='coerce'); parts.append(d.dropna(subset=['t']))
    D=pd.concat(parts,ignore_index=True); D['t']=D.t.values.astype('datetime64[s]').astype(np.int64)
    return D[['t','o','h','l','c']]
def dedup(D,keep_first):
    D=D.iloc[np.argsort(D.t.values,kind='stable')].drop_duplicates(); st=dict(conflicting=0)
    dup=D.t.duplicated(keep=False)
    if not dup.any(): return D,st
    one=D[~dup]; g=D[dup]; rows=[]
    for t,x in g.groupby('t',sort=False):
        st['conflicting']+=1
        if keep_first: rows.append(x.iloc[0].values); continue
        r=(x.h-x.l).values; real=x[r>0]
        if len(real)<=1: rows.append(x.iloc[int(np.argmax(r))].values)
        else: rows.append([t,real.o.iloc[0],real.h.max(),real.l.min(),real.c.iloc[-1]])
    R=pd.DataFrame(rows,columns=D.columns); R['t']=R.t.astype(np.int64)
    out=pd.concat([one,R]); return out.iloc[np.argsort(out.t.values,kind='stable')],st
def build(sym,zdir,bdir,keep_first=False):
    D=load(zdir); n0=len(D); D,st=dedup(D,keep_first)
    o,h,l,c=(D[k].values for k in 'ohlc')
    bad=~np.isfinite(o+h+l+c)|(h<l)|(np.minimum.reduce([o,h,l,c])<=0)|(o>h)|(o<l)|(c>h)|(c<l); D=D[~bad]
    utc=D.t.values-label_offset_hours(D.t.values)*3600; k=np.argsort(utc,kind='stable'); D=D.iloc[k]; utc=utc[k]
    keep=np.r_[True,utc[1:]!=utc[:-1]]; D=D[keep]; utc=utc[keep]
    dd=utc//86400; dow=(dd+3)%7; hr=(utc%86400)//3600
    flat=(D.o==D.h).values&(D.h==D.l).values&(D.l==D.c).values
    wk=(dow==5)|((dow==4)&(hr>=22))|((dow==6)&(hr<20)); D=D[~(wk&flat)]; utc=utc[~(wk&flat)]
    A=np.empty(len(D),dtype=DT); A['t']=utc
    for x in 'ohlc': A[x]=D[x].values
    os.makedirs(bdir,exist_ok=True); np.save(os.path.join(bdir,f'{sym}_utc.npy'),A)
    U=A.copy(); U['t']=utc+3600*uk_summer_mask_utc(utc); U.tofile(os.path.join(bdir,f'{sym}_m1.bin'))
    print(f'{sym}: raw {n0} rows, conflicting dup minutes {st["conflicting"]}, bad {int(bad.sum())}, written {len(A)} ({pd.Timestamp(utc[0],unit="s")} .. {pd.Timestamp(utc[-1],unit="s")} UTC)',flush=True)
if __name__=='__main__':
    a=[x for x in sys.argv[1:] if not x.startswith('--')]; kf='--keep-first' in sys.argv
    zd,bd=a[0],a[1]; syms=a[2].split(',') if len(a)>2 else sorted(os.listdir(zd))
    for s in syms: build(s,os.path.join(zd,s),bd,kf)
