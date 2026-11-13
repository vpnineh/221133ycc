# pay.py DIR NAME... : payout-realistic funded metrics. total = eq + withdrawn. Floor = 90k (static). Daily limit 5k (payout-adjusted).
import sys,numpy as np,pandas as pd
o=sys.argv[1]
print(f"{'run':14s} {'%/mo':>5s} {'Shp':>5s} {'minEq':>7s} {'<91k d':>6s} {'wDay%':>6s} {'d<-4%':>5s} {'acctDD':>6s} {'payouts':>7s} {'halt':>4s}  P1/P2/P3 %/mo   yearly%")
for n in sys.argv[2:]:
    try: E=pd.read_csv(f'{o}/{n}_eq.csv')
    except Exception: print(n,'missing'); continue
    if 'wd' not in E: E['wd']=0.0
    d=pd.to_datetime(E.day,unit='D'); tot=(E.eq_close+E.wd).values; months=(d.iloc[-1]-d.iloc[0]).days/30.44
    r=np.diff(np.r_[1e5,tot])/1e5; sh=r.mean()/r.std()*np.sqrt(252)
    w=np.diff(np.r_[0,E.wd.values]); prev=np.r_[1e5,E.eq_close.values[:-1]]-w; dl=(E.eq_min.values-prev)/1e5*100
    pk=np.maximum.accumulate(np.r_[1e5,tot])[1:]; add=(pk-(E.eq_min.values+E.wd.values))/1e3
    y=d.dt.year.values; per=[r[(y>=a)&(y<=b)].sum()*100/(((y>=a)&(y<=b)).sum()/21) for a,b in [(2016,2018),(2019,2022),(2023,2026)]]
    yr=pd.Series(r).groupby(y).sum()*100
    import json,re
    lg=open(f'{o}/{n}.log').read(); halt=re.search(r'"halt":(\d+)',lg); npay=re.search(r'"payouts":(\d+)',lg)
    print(f"{n:14s} {(tot[-1]/1e5-1)*100/months:5.2f} {sh:5.2f} {E.eq_min.min():7.0f} {(E.eq_min<91000).sum():6d} {dl.min():6.2f} {(dl<=-4).sum():5d} {add.max():6.2f} {npay.group(1) if npay else '-':>7s} {halt.group(1) if halt else '?':>4s}  "+' '.join(f'{p:.2f}' for p in per)+'   '+' '.join(f'{v:.0f}' for v in yr))
