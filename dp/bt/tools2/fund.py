# fund.py OUT NAME... : funded metrics on fixed-risk runs (risk on the initial 100k): avg %/month, Sharpe, worst intraday day,
# rolling start windows (each month 2016-01..2025-10, 12 months): P(eq_min <= start - 9%), P(any day <= -4.5%), worst window, P1/P2/P3
import sys,numpy as np,pandas as pd
o=sys.argv[1]
print(f"{'run':18s} {'%/mo':>5s} {'Shp':>5s} {'wDay':>6s} {'mDD':>5s} {'P(-9%)':>7s} {'P(day-4.5)':>10s} {'worstWin':>8s} {'negYrs':>6s}  P1/P2/P3 %/mo")
for n in sys.argv[2:]:
    try: E=pd.read_csv(f'{o}/{n}_eq.csv')
    except: print(n,'missing'); continue
    B0=pd.read_csv(f'{o}/{n}_deals.csv',nrows=1).profit.iloc[0]; K=1e5/B0
    d=pd.to_datetime(E.day,unit='D'); eq=E.eq_close.values*K; mn=E.eq_min.values*K; prev=np.r_[1e5,eq[:-1]]
    months=(d.iloc[-1]-d.iloc[0]).days/30.44; mo=(eq[-1]/1e5-1)*100/months
    r=(eq-prev)/1e5; sh=r.mean()/r.std()*np.sqrt(252); wd=((mn-prev)/1e5).min()*100
    pk=np.maximum.accumulate(np.r_[1e5,eq])[1:]; mdd=((pk-mn)/1e5).max()*100
    br=0;db=0;tot=0;ww=0
    for s in pd.date_range('2016-01-01','2025-10-01',freq='MS'):
        i0=np.searchsorted(d.values,np.datetime64(s)); i1=np.searchsorted(d.values,np.datetime64(s+pd.DateOffset(months=12)))
        if i1<=i0+5: continue
        base=eq[i0-1] if i0>0 else 1e5; w=mn[i0:i1]-base; tot+=1
        br+=w.min()<=-9000; db+=(((mn[i0:i1]-np.r_[base,eq[i0:i1-1]])/1e5).min()<=-0.045); ww=min(ww,w.min()/1e3)
    y=d.dt.year; yr=pd.Series(r).groupby(y.values).sum(); neg=int((yr<0).sum())
    per=[]
    for a,b in [(2016,2018),(2019,2022),(2023,2026)]:
        m=((y>=a)&(y<=b)).values; per.append(r[m].sum()*100/(m.sum()/21))
    print(f"{n:18s} {mo:5.2f} {sh:5.2f} {wd:6.2f} {mdd:5.2f} {br/tot*100:6.1f}% {db/tot*100:9.1f}% {ww:7.1f}k {neg:6d}  "+' '.join(f'{p:.2f}' for p in per))
