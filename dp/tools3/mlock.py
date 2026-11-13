# mlock.py OUT NAME : offline month-to-date lock / brake on daily P&L (linear approx; risk on initial balance)
import sys,numpy as np,pandas as pd
o,n=sys.argv[1],sys.argv[2]; B=100000
E=pd.read_csv(f'{o}/{n}_eq.csv'); d=pd.to_datetime(E.day,unit='D'); E['m']=d.dt.to_period('M')
pnl=np.diff(np.r_[B,E.eq_close.values])/B*100; lowd=(E.eq_min.values-np.r_[B,E.eq_close.values[:-1]])/B*100
def sim(L=None,f=1.0,brk=None,fb=1.0,sc=1.0):
    mt=0; cm=None; out=[]; eq=0; pk=0; dd=0; wd=0
    for i in range(len(pnl)):
        if E.m.iat[i]!=cm: cm=E.m.iat[i]; mt=0; out.append(0)
        s=sc
        if L is not None and mt>=L: s*=f
        if brk is not None and mt<=-brk: s*=fb
        x=pnl[i]*s; mt+=x; out[-1]+=x; eq+=x; pk=max(pk,eq); dd=max(dd,pk-eq); wd=min(wd,lowd[i]*s)
    m=np.array(out); return m.mean(),(m<0).sum(),(m<0.5).sum(),m.min(),dd,wd,m.std()
print(f"{'rule':28s} %/mo  neg <0.5  worstMo  DD   wDay  std")
for lab,kw in [('base',{}),('scale1.3',dict(sc=1.3))]+[(f'lock L{L} f{f}',dict(L=L,f=f)) for L in (2,3,4,5) for f in (0,0.25,0.5)]+[(f'lock L{L} f{f} sc{sc}',dict(L=L,f=f,sc=sc)) for L,f,sc in ((3,0.25,1.3),(4,0.5,1.3),(3,0.5,1.5))]+[(f'brake {b} f{fb}',dict(brk=b,fb=fb)) for b in (2,3) for fb in (0.5,0)]:
    r=sim(**kw); print(f"{lab:28s} {r[0]:5.2f} {r[1]:4d} {r[2]:4d} {r[3]:7.2f} {r[4]:5.2f} {r[5]:6.2f} {r[6]:5.2f}")
