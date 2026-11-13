# uprep.py REPORT.html LOG : MT5 tester report summary + deals table stats (untrusted input; text only)
import re,html,sys,collections
def rd(p):
    b=open(p,'rb').read()
    try: return b.decode('utf-16')
    except Exception: return b.decode('utf-8','replace')
t=rd(sys.argv[1])
txt=html.unescape(re.sub(r'<[^>]+>','|',t)); txt=re.sub(r'\|[\s|]*','|',txt)
for k in ['Initial Deposit','Total Net Profit','Gross Profit','Gross Loss','Profit Factor','Balance Drawdown Maximal','Equity Drawdown Maximal','Total Trades','Withdrawal','Sharpe Ratio','Period:','Symbol:','Modelling','History Quality','Bars','Ticks']:
    m=re.search(re.escape(k)+r':?\|([^|]*)\|?([^|]*)',txt)
    if m: print(f'{k:28s} {m.group(1)[:70]} {m.group(2)[:40]}')
rows=re.findall(r'<tr[^>]*>(.*?)</tr>',t,flags=re.S); hdr=None; D=[]
for r in rows:
    c=[html.unescape(re.sub(r'<[^>]+>','',x)).strip() for x in re.findall(r'<t[dh][^>]*>(.*?)</t[dh]>',r,flags=re.S)]
    if 'Commission' in c and 'Swap' in c and 'Deal' in c: hdr=c; continue
    if hdr and len(c)==len(hdr): D.append(dict(zip(hdr,c)))
print('deals',len(D)); f=lambda s: float(s.replace(' ','').replace(',','') or 0)
bal=[d for d in D if d['Type']=='balance']; print('balance deals',len(bal),'sum',round(sum(f(d['Profit']) for d in bal),2)); 
for d in bal[:3]+bal[-3:]: print('  ',d['Time'],d['Profit'],d['Balance'],d['Comment'][:30])
print('last deal',D[-1]['Time'],'balance',D[-1]['Balance']); print('first trade deal',[d for d in D if d['Type']!='balance'][0]['Time'])
c=collections.Counter(); p=collections.defaultdict(float)
for d in D:
    if d['Type']!='balance': c[d['Symbol']]+=1; p[d['Symbol']]+=f(d['Profit'])+f(d['Commission'])+f(d['Swap'])
for s in sorted(c): print(f'  {s:8s} deals {c[s]:4d} net {p[s]:9.2f}')
y=collections.defaultdict(float)
for d in D:
    if d['Type']!='balance': y[d['Time'][:4]]+=f(d['Profit'])+f(d['Commission'])+f(d['Swap'])
print('net by year',{k:round(v,1) for k,v in sorted(y.items())})
L=rd(sys.argv[2]).splitlines(); i=max(k for k,l in enumerate(L) if 'testing of Experts' in l); F=L[i:]
print('balance-operation lines:',sum('balance operation' in l for l in F)); 
for l in [x for x in F if 'balance operation' in x][:3]: print('  ',l.split('\t',3)[-1][:200])
print('waiting-history / skipped:'); cc=collections.Counter()
for l in F:
    m=re.search(r'v13\S* (.{0,40}?)(signal skipped: [a-z\- ]+|waiting for daily history)',l)
    if m: cc[m.group(2)]+=1
print(dict(cc.most_common(8)))
