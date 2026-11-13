# payx.py DIR BAL NAME... : payout runs of any size: %/mo (withdrawn+eq-BAL)/BAL, worst day vs prior close, min equity vs BAL, neg months
import sys,numpy as np,pandas as pd
d=sys.argv[1]; B=float(sys.argv[2])
print(f"{'run':16s} %/mo  wDay%  minEq%  negMo  months  <1%mo")
for n in sys.argv[3:]:
    E=pd.read_csv(f'{d}/{n}_eq.csv'); w=E.wd if 'wd' in E else 0*E.eq_close
    tot=E.eq_close+w; prev=np.r_[B,E.eq_close.values[:-1]]-np.diff(np.r_[0,w.values])
    wday=((E.eq_min-prev)/B).min()*100; mn=(E.eq_min.min()-B)/B*100
    m=pd.to_datetime(E.day,unit='D').dt.to_period('M'); mt=tot.groupby(m).last(); mr=np.diff(np.r_[B,mt.values])/B*100
    print(f"{n:16s} {mr.mean():5.2f} {wday:6.2f} {mn:7.2f} {int((mr<0).sum()):5d} {len(mr):6d} {int((mr<1).sum()):5d}")
