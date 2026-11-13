# chabandon.py OUT TAG... : challenge with an abandon rule (restart a new challenge when the closing equity of a phase falls to
# <= start - A%, or after D days without passing). Retries bootstrapped. Reports median / p75 days, P(<=90d), mean attempts.
import sys,glob,numpy as np,pandas as pd
o=sys.argv[1]; rng=np.random.default_rng(5)
def outcome(E,b0,A,D):
    prev=np.r_[b0,E.eq_close.values[:-1]]; mn=E.eq_min.values; mx=E.eq_max.values; cl=E.eq_close.values; day=E.day.values-E.day.values[0]+1
    base=b0; ph=1; tgt=1.08
    for i in range(len(E)):
        if mn[i]<=base*0.90 or (prev[i]-mn[i])>=0.05*base: return ('fail',day[i])
        if mx[i]>=base*tgt:
            if ph==1: ph=2; base=E.bal.values[i]; tgt=1.05; continue
            return ('pass',day[i])
        if A and cl[i]<=b0*(1-A/100): return ('abandon',day[i])
        if D and day[i]>=D: return ('abandon',day[i])
    return ('timeout',365)
for tag in sys.argv[2:]:
    fs=sorted(glob.glob(f'{o}/{tag}_2*_eq.csv')); Es=[(pd.read_csv(f)) for f in fs]; B0=[pd.read_csv(f.replace('_eq.csv','_deals.csv'),nrows=1).profit.iloc[0] for f in fs]; Es=[E[E.day<=E.day.iloc[0]+365].reset_index(drop=True) for E in Es]
    for A,D in [(0,0),(5,0),(6,0),(7,0),(0,180),(6,180),(5,150)]:
        R=[outcome(E,b,A,D) for E,b in zip(Es,B0)]; n=len(R)
        tot=[];att=[]
        for _ in range(20000):
            t=0;a=0
            while True:
                s,d=R[rng.integers(n)]; t+=d; a+=1
                if s=='pass' or t>3000: break
            tot.append(t); att.append(a)
        tot=np.array(tot)
        print(f"{tag} abandon<=-{A}% / {D or '-'}d: pass/attempt {np.mean([s=='pass' for s,d in R])*100:4.0f}% fail {sum(s=='fail' for s,d in R)} | median {np.median(tot):4.0f}d p75 {np.percentile(tot,75):4.0f} P(<=90d) {np.mean(tot<=90)*100:4.1f}% P(<=120d) {np.mean(tot<=120)*100:4.1f}% attempts {np.mean(att):.2f}")
