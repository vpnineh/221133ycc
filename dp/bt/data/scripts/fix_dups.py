#!/usr/bin/env python3
"""Repair conflicting duplicate minutes in HistData M1 files (seen in the 2026 monthly files: a feed restart writes
t, t+1(flat 1-tick bar), t(flat 1-tick bar), t+1(full bar)). The cleaning step kept the FIRST copy, which keeps a flat
1-tick bar and drops the real bar. Rule here: among copies of one label minute keep the one with the largest range
(the real bar); if several copies have a range, merge them (first open, max high, min low, last close).
usage: fix_dups.py BIN_IN BIN_OUT SYM:histdata.zip [SYM:zip ...] [--dry]
  zips may nest yearly/monthly HistData zips. Writes BIN_OUT/SYM_m1.bin (copy of BIN_IN with repaired bars)."""
import sys,os,io,zipfile,shutil,numpy as np,pandas as pd
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from clockrule import label_to_utc, uk_summer_mask_utc
DT=np.dtype([('t','<i8'),('o','<f8'),('h','<f8'),('l','<f8'),('c','<f8')])
def csvs(zf):
    for i in zf.infolist():
        n=i.filename.lower()
        if n.endswith('.zip'): yield from csvs(zipfile.ZipFile(io.BytesIO(zf.read(i))))
        elif n.endswith('.csv'): yield i.filename,zf.read(i)
def fixes(zpath):
    out=[];st=dict(groups=0,flat_vs_real=0,merged=0)
    for name,b in csvs(zipfile.ZipFile(zpath)):
        df=pd.read_csv(io.BytesIO(b),sep=';',header=None,usecols=[0,1,2,3,4],names=['ts','o','h','l','c'],dtype={'ts':str})
        d=df[df.ts.duplicated(keep=False)]
        for ts,g in d.groupby('ts',sort=False):
            if len(g[['o','h','l','c']].drop_duplicates())<2: continue
            st['groups']+=1; r=(g.h-g.l).values; real=g[r>0]
            if len(real)<=1: st['flat_vs_real']+=1; row=g.iloc[int(np.argmax(r))]; o,h,l,c=row.o,row.h,row.l,row.c
            else: st['merged']+=1; o,h,l,c=real.o.iloc[0],real.h.max(),real.l.min(),real.c.iloc[-1]
            out.append((ts,o,h,l,c))
    F=pd.DataFrame(out,columns=['ts','o','h','l','c'])
    if len(F):
        lab=pd.to_datetime(F.ts,format='%Y%m%d %H%M%S').values.astype('datetime64[s]').astype(np.int64)
        utc=label_to_utc(lab); F['t']=utc+3600*uk_summer_mask_utc(utc)
    return F,st
if __name__=='__main__':
    a=[x for x in sys.argv[1:] if x!='--dry']; dry='--dry' in sys.argv
    bi,bo=a[0],a[1]
    if not dry: os.makedirs(bo,exist_ok=True)
    for spec in a[2:]:
        sym,zp=spec.split(':',1); F,st=fixes(zp)
        src=os.path.join(bi,sym+'_m1.bin'); dst=os.path.join(bo,sym+'_m1.bin')
        if not dry and os.path.realpath(src)!=os.path.realpath(dst): shutil.copyfile(src,dst)
        A=np.memmap(src if dry else dst,dtype=DT,mode='r' if dry else 'r+')
        idx=np.searchsorted(A['t'],F.t.values) if len(F) else np.array([],int)
        ok=(idx<len(A))&(A['t'][np.minimum(idx,len(A)-1)]==F.t.values) if len(F) else np.array([],bool)
        ch=0
        for k in np.where(ok)[0]:
            i=idx[k]; new=(F.o.iloc[k],F.h.iloc[k],F.l.iloc[k],F.c.iloc[k])
            if (A['o'][i],A['h'][i],A['l'][i],A['c'][i])!=new:
                ch+=1
                if not dry: A['o'][i],A['h'][i],A['l'][i],A['c'][i]=new
        if not dry: A.flush()
        print(f'{sym}: conflicting groups {st["groups"]} (flat-vs-real {st["flat_vs_real"]}, merged {st["merged"]}), bars found {int(ok.sum())}, bars changed {ch}')
