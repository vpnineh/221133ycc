# isoos.py OUT NAME... : Sharpe and return in IS 2016-2020 / OOS 2021-2026
import sys,pandas as pd,numpy as np
o=sys.argv[1]
for n in sys.argv[2:]:
    E=pd.read_csv(f'{o}/{n}_eq.csv'); d=pd.to_datetime(E.day,unit='D'); eq=E.eq_close.values; r=np.diff(np.r_[100000,eq])/np.r_[100000,eq[:-1]]
    out=[]
    for a,b in [(2016,2020),(2021,2026)]:
        m=((d.dt.year>=a)&(d.dt.year<=b)).values; x=r[m]; out.append((x.mean()/x.std()*np.sqrt(252),(np.prod(1+x)-1)*100))
    print(f'{n:8s} IS Shp {out[0][0]:.2f} ret {out[0][1]:6.1f}% | OOS Shp {out[1][0]:.2f} ret {out[1][1]:6.1f}%')
