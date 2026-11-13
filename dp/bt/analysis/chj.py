# chj.py TAG... : challenge objective J = E[days to funded incl. retries] + FEE_DAYS x E[attempts]  (FEE_DAYS = fee / funded income per day)
import sys,glob,numpy as np,pandas as pd
import os; CHD=os.environ.get('CHDIR','out'); FD=14.8
def outcome(E,b0,A,D):
    prev=np.r_[b0,E.eq_close.values[:-1]]; mn=E.eq_min.values; mx=E.eq_max.values; cl=E.eq_close.values; day=E.day.values-E.day.values[0]+1
    base=b0; ph=1; tgt=1.08
    for i in range(len(E)):
        if mn[i]<=base*0.90 or (prev[i]-mn[i])>=0.05*base: return (0,day[i])
        if mx[i]>=base*tgt:
            if ph==1: ph=2; base=E.bal.values[i]; tgt=1.05; continue
            return (1,day[i])
        if A and cl[i]<=b0*(1-A/100): return (0,day[i])
        if D and day[i]>=D: return (0,day[i])
    return (0,day[-1])
print(f"{'tag':8s} {'A%':>3s} {'D':>4s} {'pass%':>5s} {'d/att':>5s} {'E[T]':>5s} {'att':>4s} {'J':>5s}")
for tag in sys.argv[1:]:
    fs=sorted(glob.glob(f'{CHD}/{tag}_2*_eq.csv')); best=None
    Es=[pd.read_csv(f) for f in fs]; B0=[pd.read_csv(f.replace('_eq.csv','_deals.csv'),nrows=1).profit.iloc[0] for f in fs]
    for A in (0,3,4,5,6,7):
        for D in (0,90,120,150,180):
            R=np.array([outcome(E[E.day<=E.day.iloc[0]+365],b,A,D) for E,b in zip(Es,B0)]); p=R[:,0].mean()
            if p==0: continue
            ET=R[:,1].mean()/p; att=1/p; J=ET+FD*att
            if (A,D) in [(6,0),(0,0)] or best is None or J<best[0]: 
                if (A,D) in [(6,0),(0,0)]: print(f'{tag:8s} {A:3d} {D:4d} {100*p:5.0f} {R[:,1].mean():5.0f} {ET:5.0f} {att:4.2f} {J:5.0f}')
            if best is None or J<best[0]: best=(J,A,D,p,R[:,1].mean(),ET,att)
    J,A,D,p,da,ET,att=best; print(f'{tag:8s} {A:3d} {D:4d} {100*p:5.0f} {da:5.0f} {ET:5.0f} {att:4.2f} {J:5.0f}  <- best rule')
