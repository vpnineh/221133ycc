# chparse3.py OUTDIR TAG... : phases from the daily equity file: P1 = first day eq_max >= 1.08 x start; P2 base = that day's
# closing balance, P2 = first later day eq_max >= 1.05 x base; fail = eq_min <= 0.91 x start (EA guard at 10% - 1% buffer)
import sys,glob,numpy as np,pandas as pd
o=sys.argv[1]
for tag in sys.argv[2:]:
    R=[]
    for f in sorted(glob.glob(f'{o}/{tag}_2*_eq.csv')):
        E=pd.read_csv(f); b0=pd.read_csv(f.replace('_eq.csv','_deals.csv'),nrows=1).profit.iloc[0]
        i1=np.argmax(E.eq_max.values>=1.08*b0) if (E.eq_max.values>=1.08*b0).any() else None
        fl=np.argmax(E.eq_min.values<=0.91*b0) if (E.eq_min.values<=0.91*b0).any() else None
        if i1 is None or (fl is not None and fl<i1): R.append((None,None,'fail' if fl is not None else 'timeout',None)); continue
        B1=E.bal.values[i1]; sub=E.eq_max.values[i1+1:]>=1.05*B1
        mn=E.eq_min.values[i1+1:]<=B1-0.09*B1
        i2=i1+1+np.argmax(sub) if sub.any() else None; f2=i1+1+np.argmax(mn) if mn.any() else None
        d1=int(E.day.values[i1]-E.day.values[0])+1
        if i2 is None or (f2 is not None and f2<i2): R.append((d1,None,'fail' if f2 is not None else 'timeout',None)); continue
        d2=int(E.day.values[i2]-E.day.values[i1]); R.append((d1,d2,'pass',d1+d2))
    n=len(R); tot=np.array([r[3] for r in R if r[2]=='pass']); p1=np.array([r[0] for r in R if r[0] is not None])
    print(f"{tag:8s} n={n} pass={len(tot)/max(n,1)*100:5.1f}% fail={sum(r[2]=='fail' for r in R)} timeout={sum(r[2]=='timeout' for r in R)} | days to funded med={np.median(tot):.0f} p75={np.percentile(tot,75):.0f} p90={np.percentile(tot,90):.0f} | P1 med={np.median(p1):.0f}")
