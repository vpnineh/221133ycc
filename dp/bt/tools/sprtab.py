#!/usr/bin/env python3
"""Build a broker spread-history table (SPRT file) from parsed tester journals (logparse.py *_mkt.csv).
usage: sprtab.py OUT.csv IN_mkt.csv [IN2_mkt.csv ...] [--nohour]
Model: spread(sym, t) = level(sym, month) x hour_mult(sym, server hour), fitted by alternating medians on the ask-bid quoted at
the EA's market orders (rollover hour 00:xx excluded; ROLL=1 models it). Months with < 3 samples are skipped (the step function
carries the last level forward); hours with < 8 samples get multiplier 1.  Rows: 'SYM,YYYY-MM-01,level' and 'SYM,Hhh,mult'."""
import sys,numpy as np,pandas as pd
MAP={'NDX100':'NAS100','USTEC':'NAS100','US500':'SPX500'};MINN=3;MINH=8
fs=[f for f in sys.argv[2:] if not f.startswith('--')];noh='--nohour' in sys.argv
A=pd.concat([pd.read_csv(f,parse_dates=['t']) for f in fs]);A['sym']=A.sym.str.rstrip(',').replace(MAP)
A=A.drop_duplicates(['t','side','sym','bid']);A=A[(A.t.dt.hour!=0)&(A.spr>0)].copy();A['m']=A.t.dt.to_period('M');A['h']=A.t.dt.hour
out=['# sym,month_start,level  |  sym,Hhh,hour multiplier   (spread = level x mult)']
for s,a in A.groupby('sym'):
    mult=pd.Series(1.0,index=range(24))
    for it in range(4):
        lv=(a.spr/a.h.map(mult)).groupby(a.m).median()
        r=a.spr/a.m.map(lv); g=r.groupby(a.h); mh=g.median().where(g.size()>=MINH,1.0)
        mult=pd.Series(1.0,index=range(24)); 
        if not noh: mult.update(mh)
        mult/=np.median(a.h.map(mult))
    lv=(a.spr/a.h.map(mult)).groupby(a.m).agg(['median','size']); lv=lv[lv['size']>=MINN]
    out+= [f"{s},{p.start_time.date()},{v:.6g}" for p,v in lv['median'].items()]
    out+= [f"{s},H{h:02d},{mult[h]:.4g}" for h in range(24) if abs(mult[h]-1)>1e-9]
open(fs and sys.argv[1],'w').write('\n'.join(out)+'\n')
print(sys.argv[1],len(out)-1,'rows; hour multipliers:',{l.split(',')[0]+l.split(',')[1]:l.split(',')[2] for l in out[1:] if ',H' in l})
