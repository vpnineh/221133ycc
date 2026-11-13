"""Fast challenge simulator on per-sleeve daily equity streams (single-sleeve EA runs, additive approximation)."""
import numpy as np, pandas as pd, os
OUT=os.path.dirname(os.path.abspath(__file__))+'/out'
def load(names):
    P={}; M={}
    for n in names:
        e=pd.read_csv(f'{OUT}/{n}_eq.csv'); idx=pd.to_datetime(e.day*86400,unit='s')
        c=pd.Series(e.eq_close.values,index=idx); m=pd.Series(e.eq_min.values,index=idx)
        P[n]=c.diff().fillna(0); M[n]=(m-c.shift(1)).fillna(0).clip(upper=0)
    P=pd.DataFrame(P).fillna(0); M=pd.DataFrame(M).fillna(0)
    P=P[P.index.dayofweek<5]; M=M.loc[P.index]
    return P,M
def port(P,M,w):
    w=pd.Series(w).reindex(P.columns).fillna(0)
    return (P*w).sum(1).values, (M*w).sum(1).values, P.index
def phase(pn,dip,D,s,tg,r,pol):
    """returns (outcome, end index). eq in % of initial (100). pnl arrays in $ per 100k at multiplier 1."""
    eq=100.0; n=len(pn); peak=100.0; days=0
    for t in range(s,n):
        f=r
        d=(eq-90.0)/10.0
        if pol.get('cppi',0): f*=max(pol.get('fmin',0.25),min(pol.get('fmax',3.0),d**pol['cppi']))
        rem=100+tg-eq
        if pol.get('tl') and rem<pol['tl']: f*=pol.get('tlf',0.5)
        if pol.get('ddb') and eq<peak-pol['ddb']: f*=pol.get('ddm',0.5)
        lo=eq+(pn[t-1]*0+dip[t])*f/1000.0       # intraday low vs previous close
        eq2=eq+pn[t]*f/1000.0
        days+=1
        if lo<=90.0+pol.get('buf',0) or (eq-lo)>=5.0: return 'fail',t
        eq=eq2; peak=max(peak,eq)
        if eq>=100+tg and days>=5: return 'pass',t
    return 'open',n-1
def challenge(pn,dip,D,r1,r2,pol={},step=3,since='2011-01-01'):
    s0=int(np.searchsorted(D,pd.Timestamp(since))); R=[]
    for s in range(max(1,s0),len(pn)-2,step):
        o1,t1=phase(pn,dip,D,s,8.0,r1,pol)
        if o1!='pass': R.append((o1,None,None)); continue
        o2,t2=phase(pn,dip,D,t1+1,5.0,r2,pol) if t1+2<len(pn) else ('open',len(pn)-1)
        R.append((o2,(D[t1]-D[s]).days+1,(D[t2]-D[s]).days+1 if o2=='pass' else None))
    r=pd.DataFrame(R,columns=['o','d1','d']); P=r.o=='pass'; F=r.o=='fail'; done=r.o!='open'
    return dict(pass_=P[done].mean()*100,fail=F[done].mean()*100,med=r.d[P].median(),p60=(P&(r.d<=60)).sum()/done.sum()*100,
                p90=(P&(r.d<=90)).sum()/done.sum()*100,p180=(P&(r.d<=180)).sum()/done.sum()*100,med1=r.d1.dropna().median())
def fmt(lab,x): return f"{lab:34s} pass {x['pass_']:5.1f}% fail {x['fail']:5.1f}% med {x['med']:5.0f}d (P1 {x['med1']:4.0f}d) <=60d {x['p60']:5.1f}% <=90d {x['p90']:5.1f}% <=180d {x['p180']:5.1f}%"
