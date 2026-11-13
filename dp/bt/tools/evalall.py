#!/usr/bin/env python3
"""Compare harness replications with the 5 MT5 tester runs.
usage: evalall.py TAG [TAG2 ...] [-v]     (harness outputs in $OUTDIR or <pkg>/out: {T23,T24,T25,MQ22,MQ18}_TAG_deals.csv)
metrics per run: match% (trades paired by sleeve+side within 36h), SdR = sum over sleeves |sumR_tester - sumR_harness|,
mdR = mean |dR| over matched trades, aE = mean |entry price diff| / stop distance (R units), aSw = mean |swap diff| $ per matched trade, corrD = daily net PnL correlation, bal = final balance tester/harness ($k, start 100k), dBal = harness-tester (% of start)."""
import sys,os,pandas as pd,numpy as np,warnings;warnings.filterwarnings('ignore')
P=os.path.dirname(os.path.abspath(__file__))+'/..'
OUT=os.environ.get('OUTDIR',P+'/out')+'/'
TD=P+'/tester/'
NAMES=["GOLD T60","GOLD T240","GOLD MOM","GOLD TOM","SILV T240","SILV MA","DOW RSI2","DOW IBS","NAS RSI2","NAS IBS","AUDJPY BB","SILV H4-55","JPY GOTOBI","NAS 3RD-FRI","JPY FIX-REV","GOLD LOTTERY","NAS LOTTERY","JPY LOTTERY","EURGBP MR","NAS GAP-LONG","GOLD IBS","SPX RSI2"]
END='2026-10-10'
RUNS=[('T23','fn_t23','2024-01-01'),('T24','fn_t24','2024-01-01'),('T25','fn_t25','2025-01-01'),('MQ22','mq22','2022-01-01'),('MQ18','mq18','2018-01-01')]
def harness(path):
    d=pd.read_csv(path);d=d[d.magic>0].copy();d['t']=pd.to_datetime(d.time,unit='s');d['pl']=d.profit+d.comm+d.swap
    e=d[d.entry==0].set_index('pos');x=d[d.entry==1]
    H=pd.DataFrame(dict(t_in=e.t,ep=e.price,vol0=e.vol,sl=e.sl,k=e.magic-110000,sym=e.sym,side=np.where(e.type==0,1,-1)))
    g=d.groupby('pos');H['net']=g.pl.sum();H['swap']=g.swap.sum();H['gross']=g.profit.sum()
    H['t_out']=x.groupby('pos').t.max();H['xp']=x.groupby('pos').price.last();H['nx']=x.groupby('pos').size()
    xs=H[(H.nx==1)&(H.gross.abs()>5)];f=xs.gross/((xs.xp-xs.ep)*xs.side*xs.vol0)
    fy=f.groupby([xs.sym,xs.t_in.dt.year]).median();fa=f.groupby(xs.sym).median()
    H['fac']=[fy.get((s,y),fa.get(s)) for s,y in zip(H.sym,H.t_in.dt.year)]
    H['risk']=(H.ep-H.sl).abs()*H.vol0*H.fac;H['R']=np.where(H.sl>0,H.net/H.risk,np.nan)
    H['name']=H.k.map(lambda k:NAMES[k]);return H[H.t_out.notna()].reset_index()
def match(T,G,hrs=36):
    rows=[];pairs=[]
    for nm in sorted(set(T.name)|set(G.name)):
        t=T[T.name==nm].sort_values('t_in');g=G[G.name==nm].sort_values('t_in');used=set();m=0
        gt=g.t_in.values;gs=g.side.values;gi=g.index.values
        for i,r in t.iterrows():
            ok=(np.abs((gt-np.datetime64(r.t_in)).astype('timedelta64[s]').astype(float))<=hrs*3600)&(gs==r.side)&~np.isin(gi,list(used))
            if ok.any():
                c=np.where(ok)[0];j=gi[c[np.argmin(np.abs((gt[c]-np.datetime64(r.t_in)).astype('timedelta64[s]').astype(float)))]];used.add(j);m+=1;pairs.append((i,j))
        rows.append(dict(name=nm,nt=len(t),nh=len(g),matched=m,tR=t.R.sum(),hR=g.R.sum(),tnet=t.net.sum()/1e3,hnet=g.net.sum()/1e3))
    return pd.DataFrame(rows).set_index('name'),pairs
def one(run,tf,a,tag):
    f=OUT+f'{run}_{tag}_deals.csv'
    if not os.path.exists(f): return None
    T=pd.read_csv(TD+tf+'_pos.csv',parse_dates=['t_in','t_out']);G=harness(f)
    T=T[(T.t_in>=a)&(T.t_in<END)].reset_index(drop=True);G=G[(G.t_in>=a)&(G.t_in<END)].reset_index(drop=True)
    M,pairs=match(T,G)
    dR=np.array([T.R[i]-G.R[j] for i,j in pairs]);dR=dR[np.isfinite(dR)]
    aSw=np.mean([abs(T.swap[i]-G.swap[j]) for i,j in pairs])
    aE=np.nanmean([abs(G.ep[j]-T.ep[i])/abs(T.ep[i]-T.sl[i]) if T.sl[i] and abs(T.ep[i]-T.sl[i])>0 else np.nan for i,j in pairs])
    t=T.groupby(T.t_out.dt.floor('D')).net.sum();g=G.groupby(G.t_out.dt.floor('D')).net.sum();idx=pd.bdate_range(a,END)
    c=t.reindex(idx,fill_value=0).corr(g.reindex(idx,fill_value=0))
    r=dict(n=f'{len(T)}/{len(G)}',match=100*2*M.matched.sum()/(len(T)+len(G)),SdR=(M.tR-M.hR).abs().sum(),mdR=np.abs(dR).mean(),aE=aE,aSw=aSw,corrD=c,
           balT=100+T.net.sum()/1e3,balH=100+G.net.sum()/1e3,dBal=(G.net.sum()-T.net.sum())/1e3)
    return r,M
if __name__=='__main__':
    v='-v' in sys.argv;tags=[x for x in sys.argv[1:] if x!='-v'];rows=[]
    for tag in tags:
        for run,tf,a in RUNS:
            o=one(run,tf,a,tag)
            if o is None: continue
            r,M=o;rows.append(dict(tag=tag,run=run,**r))
            if v: M['dR']=M.tR-M.hR;print(f'--- {tag} {run}');print(M.round(2).to_string())
    D=pd.DataFrame(rows)
    pd.set_option('display.width',200);print(D.round(3).to_string(index=False))
    if len(D):
        A=D.groupby('tag',sort=False).agg(match=('match','mean'),SdR=('SdR','sum'),mdR=('mdR','mean'),aE=('aE','mean'),aSw=('aSw','mean'),corrD=('corrD','mean'),absdBal=('dBal',lambda s:s.abs().sum()))
        print('summary (match mean, SdR sum, mdR mean, corrD mean, sum|dBal| $k)');print(A.round(3).to_string())
