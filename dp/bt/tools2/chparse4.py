# chparse4.py OUTDIR TAG... : 12-month challenge windows from eq files; fail = max loss (eq_min <= start-10%... EA guard -9%) or a day
# loss >= 5% of the initial (vs previous close); time incl. retries by bootstrap (failed / timed-out attempt costs its days, retry)
import sys,glob,numpy as np,pandas as pd
o=sys.argv[1]; rng=np.random.default_rng(3)
for tag in sys.argv[2:]:
    R=[]
    for f in sorted(glob.glob(f'{o}/{tag}_2*_eq.csv')):
        E=pd.read_csv(f); b0=pd.read_csv(f.replace('_eq.csv','_deals.csv'),nrows=1).profit.iloc[0]
        E=E[E.day<=E.day.iloc[0]+365].reset_index(drop=True)
        prev=np.r_[b0,E.eq_close.values[:-1]]; mn=E.eq_min.values; mx=E.eq_max.values; day=E.day.values-E.day.values[0]+1
        res=None; base=b0; ph=1; tgt=1.08
        for i in range(len(E)):
            ref=max(prev[i],base) if i else b0
            if mn[i]<=base*0.90+(0 if ph==1 else 0) or (prev[i]-mn[i])>=0.05*base: res=('fail',day[i]); break
            if mx[i]>=base*tgt:
                if ph==1: ph=2; base=E.bal.values[i]; tgt=1.05; p1=day[i]
                else: res=('pass',day[i]); break
        if res is None: res=('timeout',365)
        R.append(res)
    n=len(R); pas=[d for s,d in R if s=='pass']; fl=[d for s,d in R if s!='pass']
    tot=[]
    for _ in range(20000):
        t=0
        while True:
            s,d=R[rng.integers(n)]; t+=d
            if s=='pass' or t>3000: break
        tot.append(t)
    tot=np.array(tot)
    print(f"{tag:10s} pass1st {len(pas)/n*100:5.1f}% fail {sum(s=='fail' for s,d in R)} timeout {sum(s=='timeout' for s,d in R)} | single pass median {np.median(pas) if pas else np.nan:4.0f} | incl. retries: median {np.median(tot):4.0f} p75 {np.percentile(tot,75):4.0f} P(<=90d) {np.mean(tot<=90)*100:4.1f}% attempts {np.mean([1]) if False else ''}")
