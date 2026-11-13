# fstart.py OUT TAG... : new funded accounts started at each quarter (12 months): avg %/month, share of starts >= 2%/month,
# EA guard hit (eq_min <= start-9%), prop breach (<= -10% or a day >= 5%), worst start, negative years
import sys,glob,numpy as np,pandas as pd
o=sys.argv[1]
for tag in sys.argv[2:]:
    R=[]
    for f in sorted(glob.glob(f'{o}/{tag}_2*_eq.csv')):
        E=pd.read_csv(f); b0=1e5; prev=np.r_[b0,E.eq_close.values[:-1]]
        R.append(dict(m=(E.eq_close.values[-1]/b0-1)*100/12, low=(E.eq_min.min()/b0-1)*100, day=((E.eq_min.values-prev)/b0).min()*100))
    R=pd.DataFrame(R)
    print(f"{tag:5s} n={len(R)} avg {R.m.mean():.2f}%/mo median {R.m.median():.2f} | starts >=2%/mo {np.mean(R.m>=2)*100:4.0f}% | negative year {np.mean(R.m<0)*100:4.0f}% | EA guard -9% hit {np.mean(R.low<=-9)*100:4.1f}% | breach (-10% or day -5%) {np.mean((R.low<=-10)|(R.day<=-5))*100:4.1f}% | worst low {R.low.min():.1f}% worst day {R.day.min():.2f}%")
