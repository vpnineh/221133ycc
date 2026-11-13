# chparse2.py OUTDIR TAG... : challenge outcome from deals (no LOUD needed): 'target reached' closes mark P1 / P2 passes
import sys,glob,os,numpy as np,pandas as pd
o=sys.argv[1]
for tag in sys.argv[2:]:
    R=[]
    for f in sorted(glob.glob(f'{o}/{tag}_2*_deals.csv')):
        D=pd.read_csv(f); t0=D.time.iloc[0]; w=D.why.astype(str)
        tc=sorted(set(D.time[w.str.contains('target reached')]//86400))
        fail=w.str.contains('max-loss').any()
        p1=(tc[0]*86400-t0)/86400 if len(tc)>0 else None; p2=(tc[1]-tc[0]) if len(tc)>1 else None
        out='pass' if len(tc)>1 else ('fail' if fail else 'timeout')
        R.append((p1,p2,out))
    n=len(R); ps=[r for r in R if r[2]=='pass']; tot=np.array([r[0]+r[1] for r in ps]) if ps else np.array([np.nan])
    p1=np.array([r[0] for r in R if r[0] is not None])
    print(f"{tag:8s} n={n} pass={len(ps)/max(n,1)*100:5.1f}% fail={sum(r[2]=='fail' for r in R)} timeout={sum(r[2]=='timeout' for r in R)} | days to funded med={np.nanmedian(tot):.0f} p75={np.nanpercentile(tot,75):.0f} p90={np.nanpercentile(tot,90):.0f} | P1 med={np.median(p1):.0f}")
