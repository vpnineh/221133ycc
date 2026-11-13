# ld.py DIR TAG... : detail of fresh funded cells: min equity, daily-loss tail (payout-adjusted, vs the day reference), payouts
import sys,glob,numpy as np,pandas as pd
d=sys.argv[1]
print(f"{'cell':7s} {'n':>3s} {'minEq p0/p10/p50':>20s} {'worst day%':>10s} {'days<=-3%':>9s} {'<=-4%':>6s} {'<=-4.5%':>7s} {'mean %/mo':>9s} {'med':>5s} {'worst acct':>10s} {'P(year<0)':>9s}")
for tag in sys.argv[2:]:
    mn=[];wd=[];c3=c4=c45=0;pm=[];neg=[];ny=0
    for f in sorted(glob.glob(f'{d}/{tag}_2*_eq.csv')):
        try:
            if 'end_balance' not in open(f.replace('_eq.csv','.log')).read(): continue
        except Exception: continue
        E=pd.read_csv(f).dropna()
        if len(E)<50: continue
        w=E.wd.values; ref=np.maximum(np.r_[1e5,E.bal.values[:-1]],np.r_[1e5,E.eq_close.values[:-1]])-np.r_[0,np.diff(w)]
        dl=(E.eq_min.values-ref)/1e3; mn.append(E.eq_min.min()); wd.append(dl.min()); c3+=(dl<=-3).sum(); c4+=(dl<=-4).sum(); c45+=(dl<=-4.5).sum()
        months=(E.day.iloc[-1]-E.day.iloc[0])/30.44; tot=E.eq_close.values+w; pm.append((tot[-1]-1e5)/1e3/months)
        for k in range(0,len(E)-252,252): ny+=1; neg.append(tot[min(k+252,len(E)-1)]-tot[k]<0)
    mn=np.array(mn)
    print(f"{tag:7s} {len(mn):3d} {mn.min():7.0f}/{np.percentile(mn,10):6.0f}/{np.median(mn):6.0f} {min(wd):10.2f} {c3:9d} {c4:6d} {c45:7d} {np.mean(pm):9.2f} {np.median(pm):5.2f} {min(pm):10.2f} {100*np.mean(neg):9.1f}")
