#!/usr/bin/env python3
"""Side-by-side summary of harness runs: trades, total %, PF per period, max DD (% of 100k).  usage: vcmp.py RUN [RUN ...]  ($OUTDIR or <pkg>/out)"""
import pandas as pd,numpy as np,sys,os
O=os.environ.get('OUTDIR',os.path.dirname(os.path.abspath(__file__))+'/../out')+'/'
P=[('2016','2018'),('2019','2022'),('2023','2026')]
def pf(x): return x[x>0].sum()/max(1e-9,-x[x<0].sum())
def st(n):
  d=pd.read_csv(f'{O}{n}_deals.csv');d=d[(d.entry==1)&(d.magic>0)].copy();d['pl']=(d.profit+d.comm+d.swap)/1000;d['y']=pd.to_datetime(d.time,unit='s').dt.year
  out=f'{n:12s} n{len(d):5d} tot{d.pl.sum():+7.1f}% PF{pf(d.pl):4.2f} swap{(d.swap.sum()/1000):+6.1f}%'
  for a,b in P:
    x=d[(d.y>=int(a))&(d.y<=int(b))].pl;out+=f' | {a[2:]}-{b[2:]} {x.sum():+6.1f} PF{pf(x):4.2f}'
  e=pd.read_csv(f'{O}{n}_eq.csv');dd=((e.eq_close.cummax()-e.eq_min).clip(lower=0)/1000).max();return out+f' | maxDD {dd:4.1f}%'
for k in sys.argv[1:]: print(st(k))
