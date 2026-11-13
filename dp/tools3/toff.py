# toff.py : offline T-series pricing on the 39 fresh funded payout runs (RF=1 base) -- linear scaling, trim, room^e, 4% guard
import glob,numpy as np,pandas as pd
fs=sorted(glob.glob('/home/user/221133ycc/dp/bt/out/mo/mf_2*_eq.csv')); B=100000
runs=[]
for f in fs:
    E=pd.read_csv(f); wd=np.diff(np.r_[0,E.wd.values]); prev=np.r_[B,E.eq_close.values[:-1]]-wd
    runs.append(((E.eq_close.values-prev)/B*100,(E.eq_min.values-prev)/B*100,wd>0))
def sim(RF,T=None,e=None,cpl=0.2):
    inc=[];dead=0;wday=0;n4=0;mins=[]
    for cl,lo,pay in runs:
        eq=100.0;wtot=0;mn=100;alive=True
        for i in range(len(cl)):
            if pay[i] and eq>100: wtot+=eq-100; eq=100
            room=(eq-90)/10; s=RF*(min(1,max(cpl,room**e)) if e else 1)
            l=s*lo[i]; c=s*cl[i]
            if T is not None and l<=-T:
                l=-T+0.5*(l+T); c=-T+0.5*(c+T)
            if l<=-4: l=max(l,-4.25); c=l          # guard closes everything at -4 (+0.25 slippage)
            wday=min(wday,l); n4+= l<=-4
            mn=min(mn,eq+l)
            if eq+l<=91 or l<=-5: dead+=1; alive=False; break
            eq+=c
        months=len(cl)/21.0/1.0
        inc.append((wtot+eq-100)/ (i+1) * 21)   # %/mo over the alive span (approx trading days)
        mins.append(mn)
    return np.mean(inc),dead,wday,n4,np.min(mins)
print(f"{'cell':26s} %/mo  dead  wDay  days<=-4  minEq")
for lab,kw in [('F RF1.0',dict(RF=1.0)),('RF1.0 trim3',dict(RF=1.0,T=3)),('RF1.25',dict(RF=1.25)),('RF1.25 trim3',dict(RF=1.25,T=3)),
               ('RF1.5',dict(RF=1.5)),('RF1.5 trim3',dict(RF=1.5,T=3)),('RF1.5 trim2.5',dict(RF=1.5,T=2.5)),('RF2.0 trim2.5',dict(RF=2.0,T=2.5)),
               ('Lb RF2 e2',dict(RF=2.0,e=2)),('Lb + trim3',dict(RF=2.0,e=2,T=3)),('Lb + trim2.5',dict(RF=2.0,e=2,T=2.5)),
               ('RF1.5 e2 trim3',dict(RF=1.5,e=2,T=3)),('RF1.5 e1 trim2.5',dict(RF=1.5,e=1,T=2.5)),('RF1.25 e1 trim2.5',dict(RF=1.25,e=1,T=2.5)),('RF1.25 e1 trim3',dict(RF=1.25,e=1,T=3)),('RF1.25 trim2.5',dict(RF=1.25,T=2.5)),('RF1.5 e1 trim2',dict(RF=1.5,e=1,T=2)),('RF2 e1 trim2.5',dict(RF=2.0,e=1,T=2.5)),('RF1.5 e1 trim3',dict(RF=1.5,e=1,T=3)),('RF1.25 e2 trim3',dict(RF=1.25,e=2,T=3))]:
    r=sim(**kw); print(f"{lab:26s} {r[0]:5.2f} {r[1]:4d} {r[2]:6.2f} {r[3]:6d} {r[4]:7.2f}")
