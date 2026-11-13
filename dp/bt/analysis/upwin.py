# upwin.py REPORT.html FROM TO : closed trades of an MT5 tester report inside a window, by symbol / sleeve comment (untrusted input; text only)
import re,html,sys,collections
b=open(sys.argv[1],'rb').read()
try: t=b.decode('utf-16')
except Exception: t=b.decode('utf-8','replace')
rows=re.findall(r'<tr[^>]*>(.*?)</tr>',t,flags=re.S); hdr=None; D=[]
for r in rows:
    c=[html.unescape(re.sub(r'<[^>]+>','',x)).strip() for x in re.findall(r'<t[dh][^>]*>(.*?)</t[dh]>',r,flags=re.S)]
    if 'Commission' in c and 'Swap' in c and 'Deal' in c: hdr=c; continue
    if hdr and len(c)==len(hdr): D.append(dict(zip(hdr,c)))
f=lambda s: float(s.replace(' ','').replace(',','') or 0)
a,bb=sys.argv[2],sys.argv[3]
W=[d for d in D if a<=d['Time'][:10]<=bb and d['Type']!='balance']
print('deals in window',len(W)); ins={}
agg=collections.defaultdict(lambda:[0,0.0,0]); 
for d in D:
    if d['Type']=='balance': continue
    if d['Direction']=='in': ins[d['Order']]=d
print('time              sym     dir  vol   price      net   comment')
tot=0
for d in W:
    net=f(d['Profit'])+f(d['Commission'])+f(d['Swap'])
    if d['Direction']=='out':
        tot+=net; k=d['Symbol']; agg[k][0]+=1; agg[k][1]+=net; agg[k][2]+=net>0
    print(d['Time'],f"{d['Symbol']:7s} {d['Type']:4s} {d['Direction']:3s} {d['Volume']:>5s} {d['Price']:>10s} {net:8.2f}  {d['Comment'][:28]}")
print('closed net in window',round(tot,2))
for k,(n,p,w) in sorted(agg.items(),key=lambda x:x[1][1]): print(f'  {k:8s} closed {n:3d} wins {w:2d} net {p:8.2f}')
