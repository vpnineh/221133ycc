# summ.py OUTDIR NAME... : CAGR, maxDD (intraday eq_min vs prior peak), worst day, Sharpe, trades, skips, per-period returns
import sys,json,pandas as pd,numpy as np
o=sys.argv[1]
print(f"{'run':14s} {'final%':>8s} {'CAGR':>6s} {'mDD%':>6s} {'wDay%':>6s} {'Shp':>5s} {'n':>5s} {'skips':>6s}  P1(16-18) P2(19-22) P3(23-26)")
for n in sys.argv[2:]:
    try: E=pd.read_csv(f'{o}/{n}_eq.csv'); D=pd.read_csv(f'{o}/{n}_deals.csv')
    except Exception as e: print(n,'missing'); continue
    b0=D.profit.iloc[0]; eq=E.eq_close.values; mn=E.eq_min.values
    pk=np.maximum.accumulate(np.r_[b0,eq])[:-1]; dd=((pk-np.minimum(mn,eq))/pk).max()*100
    prev=np.r_[b0,eq[:-1]]; wd=((mn-prev)/prev).min()*100
    r=np.diff(np.r_[b0,eq])/np.r_[b0,eq[:-1]]; sh=r.mean()/r.std()*np.sqrt(252) if r.std()>0 else 0
    yrs=(E.day.iloc[-1]-E.day.iloc[0])/365.25; cagr=((eq[-1]/b0)**(1/yrs)-1)*100
    sk=json.loads(open(f'{o}/{n}.log').read().strip().splitlines()[-1]).get('skips',-1)
    nt=int(((D.type<2)&(D.entry==0)).sum())
    d=pd.to_datetime(E.day,unit='D'); per=[]
    for a,b in [(2016,2018),(2019,2022),(2023,2026)]:
        m=(d.dt.year>=a)&(d.dt.year<=b); x=eq[m.values]; i0=np.argmax(m.values)
        if len(x)==0: per.append(float('nan')); continue   # period not in this run
        s=eq[i0-1] if i0>0 else b0; per.append((x[-1]/s-1)*100)
    print(f"{n:14s} {(eq[-1]/b0-1)*100:8.1f} {cagr:6.2f} {dd:6.2f} {wd:6.2f} {sh:5.2f} {nt:5d} {sk:6d}  "+'  '.join(f'{p:8.1f}' for p in per))
