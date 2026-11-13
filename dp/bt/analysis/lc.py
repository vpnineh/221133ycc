# lc.py DIR TAG... : lifecycle economics (rules in u_prereg.txt, L-series). Challenge outcomes from out/chF (abandon -6%).
import sys,glob,numpy as np,pandas as pd
import os
CHD=os.environ.get("CHDIR","out")
CH=sys.argv[1]; d=sys.argv[2]; SPLIT=0.80; DAYB=float(os.environ.get('DAYB','5'))*1000   # a day loss >= DAYB% counts as a breach
def ch_outcome(E,b0,A=6):
    prev=np.r_[b0,E.eq_close.values[:-1]]; mn=E.eq_min.values; mx=E.eq_max.values; cl=E.eq_close.values; day=E.day.values-E.day.values[0]+1
    base=b0; ph=1; tgt=1.08
    for i in range(len(E)):
        if mn[i]<=base*0.90 or (prev[i]-mn[i])>=0.05*base: return (0,day[i])
        if mx[i]>=base*tgt:
            if ph==1: ph=2; base=E.bal.values[i]; tgt=1.05; continue
            return (1,day[i])
        if cl[i]<=b0*(1-A/100): return (0,day[i])
    return (0,365)
C=[]
for f in sorted(glob.glob(f'{CHD}/{CH}_2*_eq.csv')):
    E=pd.read_csv(f); b0=pd.read_csv(f.replace('_eq.csv','_deals.csv'),nrows=1).profit.iloc[0]; E=E[E.day<=E.day.iloc[0]+365]; C.append(ch_outcome(E,b0))
C=np.array(C)
def funded(tag):
    R=[]
    for f in sorted(glob.glob(f'{d}/{tag}_2*_eq.csv')):
        lg=f.replace('_eq.csv','.log')
        try:
            if 'end_balance' not in open(lg).read(): continue
        except Exception: continue
        E=pd.read_csv(f).dropna()
        if len(E)<50 or 'wd' not in E: continue
        day=E.day.values-E.day.values[0]+1; wd=E.wd.values; mn=E.eq_min.values
        ref=np.maximum(np.r_[1e5,E.bal.values[:-1]],np.r_[1e5,E.eq_close.values[:-1]])-np.r_[0,np.diff(wd)]
        dead=np.where((mn<=91200)|(ref-mn>=DAYB))[0]
        if len(dead): i=dead[0]; R.append((1,day[i],wd[i]/1e3,int(mn[i]>91200)))
        else: R.append((0,day[-1],(wd[-1]+E.eq_close.values[-1]-1e5)/1e3,0))
    return np.array(R)
rng=np.random.default_rng(7)
def life(R,fee,years=30,n=400):
    inc=[];dth=[];cht=[]
    for _ in range(n):
        t=0;w=0;fees=0;deaths=0;tc=0
        while t<years*365:
            while True:
                s,dd=C[rng.integers(len(C))]; t+=dd; tc+=dd; fees+=fee
                if s: break
            while t<years*365:
                dead,dd,W,_=R[rng.integers(len(R))]; t+=dd; w+=SPLIT*W
                if dead: deaths+=1; break
        inc.append((w-fees)/(t/30.44)); dth.append(deaths/(t/365)); cht.append(tc/t)
    return np.mean(inc),np.mean(dth),np.mean(cht)
print(f'challenge {CH}: pass/attempt {C[:,0].mean()*100:.0f}%, mean days per attempt {C[:,1].mean():.0f}')
print(f"{'cell':8s} {'n':>3s} {'dead%/24m':>9s} {'(daily)':>7s} {'%/mo alive':>10s} | net income %/mo of account (after 80% split): fee 0.8%  fee 0.55% | deaths/yr  time in challenge")
for tag in sys.argv[3:]:
    R=funded(tag)
    if not len(R): print(tag,'none'); continue
    alive=R[:,2].sum()/(R[:,1].sum()/30.44)
    a=life(R,0.8); b=life(R,0.55)
    print(f"{tag:8s} {len(R):3d} {100*R[:,0].mean():9.1f} {int(R[:,3].sum()):7d} {alive:10.2f} | {a[0]:28.2f} {b[0]:9.2f} | {a[1]:9.2f} {100*a[2]:10.0f}%")
