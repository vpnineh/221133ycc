# uplog.py LOG : summary of an MT5 tester journal (untrusted input; text only)
import re,sys,collections
b=open(sys.argv[1],'rb').read()
try: t=b.decode('utf-16')
except Exception: t=b.decode('utf-8','replace')
L=t.splitlines(); print('lines',len(L))
starts=[k for k,l in enumerate(L) if 'testing of Experts' in l]
for k in starts: print(k, L[k][:200])
i=starts[-1] if starts else 0; F=L[i:]
for l in F:
    if re.search(r'ticks generating|initial deposit|Test passed|total ticks|final balance|OnTester|Tester\s+(\S+): generate',l): print(l[:210])
want=('InpPreset','InpRiskMode','InpTesterChain','InpDemoMode','InpAccountType','InpTesterPayoutDays','InpTesterPayoutFirstDay','InpPayoutBufferPct','InpGoldVolFilter','InpInitialBalance','InpFastRiskFunded','InpTurboRiskPhase1','InpTurboRiskPhase2')
for l in F[:400]:
    if any(w+'=' in l for w in want): print('  ',l.split('\t')[-1].strip())
print('--- stage / payout lines')
for l in F:
    if 'STAGE SUMMARY' in l or 'tester payouts:' in l or 'TARGET' in l and 'v13' in l: print(l.split('\t',3)[-1][:330])
n=sum('tester payout:' in l for l in F); print('tester payout lines',n)
for l in [x for x in F if 'tester payout:' in x][:3]+[x for x in F if 'tester payout:' in x][-2:]: print(l.split('\t',3)[-1][:200])
print('--- errors / warnings')
c=collections.Counter()
for l in F:
    if re.search(r'error|failed|invalid|not enough|rejected|halt|guard|no money|off quotes|frozen|breach',l,re.I):
        c[re.sub(r'[\d.]+','#',l.split('\t',3)[-1])[:110]]+=1
for k,v in c.most_common(25): print(v,k)
