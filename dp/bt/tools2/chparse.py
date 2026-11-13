# chparse.py OUTDIR TAG... : per start: P1 days, P2 days, total, outcome; summary median/p75/p90, pass%, worst day
import sys,re,glob,os,numpy as np
o=sys.argv[1]
for tag in sys.argv[2:]:
    R=[]
    for f in sorted(glob.glob(f'{o}/{tag}_2*.log')):
        s=open(f).read(); st=re.findall(r'STAGE SUMMARY (\w+): ([^|]+)\| (\d+) calendar days.*?worst closed day ([-+\d.]+)%.*?max drawdown ([\d.]+)%',s)
        p1=p2=None; wd=0; out='timeout'
        for name,what,d,w,dd in st:
            wd=min(wd,float(w))
            if name=='PHASE1' and 'TARGET' in what: p1=int(d)
            elif name=='PHASE2' and 'TARGET' in what: p2=int(d); out='pass'
            elif 'FAILED' in what: out='fail'
        if 'HALT' in s and out!='pass' and re.search(r'max-loss guard',s): out='fail'
        R.append((os.path.basename(f),p1,p2,out,wd))
    n=len(R); ps=[r for r in R if r[3]=='pass']; tot=np.array([r[1]+r[2] for r in ps]) if ps else np.array([0])
    p1s=np.array([r[1] for r in R if r[1] is not None])
    print(f"{tag:10s} n={n} pass={len(ps)/max(n,1)*100:5.1f}% fail={sum(r[3]=='fail' for r in R)} timeout={sum(r[3]=='timeout' for r in R)} | days to funded med={np.median(tot):.0f} p75={np.percentile(tot,75):.0f} p90={np.percentile(tot,90):.0f} | P1 med={np.median(p1s):.0f} | worst day {min(r[4] for r in R):.2f}%")
