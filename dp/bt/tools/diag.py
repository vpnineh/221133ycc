#!/usr/bin/env python3
"""Per-sleeve decomposition of matched-trade R differences into entry / exit price / swap+comm parts.  usage: diag.py RUN TAG  (RUN in T23,T24,T25,MQ22,MQ18)"""
import sys,os,numpy as np,pandas as pd
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)));import evalall as E
run,tag=sys.argv[1],sys.argv[2];tf,a=[(x[1],x[2]) for x in E.RUNS if x[0]==run][0]
T=pd.read_csv(E.TD+tf+'_pos.csv',parse_dates=['t_in','t_out']);G=E.harness(E.OUT+f'{run}_{tag}_deals.csv')
T=T[(T.t_in>=a)&(T.t_in<E.END)].reset_index(drop=True);G=G[(G.t_in>=a)&(G.t_in<E.END)].reset_index(drop=True)
if 'xp' not in T or T.xp.isna().all(): T['xp']=T.ep+T.gross/(T.side*T.vol0*T.fac)
M,pairs=E.match(T,G);rows=[]
for i,j in pairs:
    t=T.loc[i];g=G.loc[j];d=abs(t.ep-t.sl)
    if not d or not np.isfinite(t.R) or not np.isfinite(g.R): continue
    rows.append(dict(name=t['name'],dtin=(g.t_in-t.t_in).total_seconds()/60,dtout=(g.t_out-t.t_out).total_seconds()/3600,
      dE=-(g.ep-t.ep)*t.side/d,dX=(g.xp-t.xp)*t.side/d,dR=g.R-t.R,dSw=(g.swap-t.swap)/t.risk))
D=pd.DataFrame(rows);pd.set_option('display.width',200)
f=lambda s:s.abs().mean()
print(D.groupby('name').agg(n=('dR','size'),mdR=('dR',f),bias=('dR','mean'),dE=('dE','mean'),adE=('dE',f),dX=('dX','mean'),adX=('dX',f),dSw=('dSw','mean'),sameIn=('dtin',lambda s:(s.abs()<1).mean()),sameOut=('dtout',lambda s:(s.abs()<0.02).mean())).round(3).to_string())
print('ALL mdR',round(f(D.dR),3),'bias',round(D.dR.mean(),3),'|dE|',round(f(D.dE),3),'|dX|',round(f(D.dX),3),'dSw',round(D.dSw.mean(),4),'sameIn',round((D.dtin.abs()<1).mean(),3),'sameOut',round((D.dtout.abs()<0.02).mean(),3))
