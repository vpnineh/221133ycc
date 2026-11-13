import sys,re,html,collections,pandas as pd
rep,hd=sys.argv[1],sys.argv[2]
t=open(rep,encoding='utf-16').read(); t=t[t.find('>Deals<'):]
T=[]
for r in re.findall(r'<tr[^>]*>(.*?)</tr>',t,re.S):
    c=[html.unescape(re.sub(r'<[^>]+>','',x)).strip() for x in re.findall(r'<td[^>]*>(.*?)</td>',r,re.S)]
    if len(c)>=13 and re.match(r'\d{4}\.\d\d',c[0]) and c[3]!='balance':
        f=lambda s: float(s.replace(' ','') or 0)
        T.append((c[0][:4],c[2],c[4],f(c[8])+f(c[9])+f(c[10]),c[12],c[0]))
M={'US500':'SPX500','USTEC':'NAS100'}
H=pd.read_csv(hd); H=H[H.type<2]; H['y']=pd.to_datetime(H.time,unit='s').dt.year.astype(str); H['p']=H.profit+H.comm+H.swap
ty=collections.defaultdict(float); ts=collections.defaultdict(float); tn=collections.Counter()
for y,s,d,p,cm,tm in T: ty[y]+=p; ts[M.get(s,s)]+=p; tn[M.get(s,s)]+=(d=='in')
hy=H.groupby('y').p.sum(); hs=H.groupby('sym').p.sum(); hn=H[H.entry==0].groupby('sym').size()
print('year  tester  harness'); [print(y,round(ty[y]),round(hy.get(y,0))) for y in sorted(ty)]
print('sym  n_t n_h  pnl_t pnl_h'); [print(s,tn[s],hn.get(s,0),round(ts[s]),round(hs.get(s,0))) for s in sorted(ts)]
print('total',round(sum(ty.values())),round(H.p.sum()))
print('tester target closes:',sorted({tm[:10] for y,s,d,p,cm,tm in T if 'target' in cm.lower()})[:5])
print('harness target closes:',sorted({str(pd.to_datetime(x,unit='s'))[:10] for x,w in zip(H.time,H.why.astype(str)) if 'target' in w})[:5])
