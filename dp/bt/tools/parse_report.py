import sys,openpyxl,warnings,math,json,collections
import pandas as pd
warnings.filterwarnings('ignore')
NAMES=["GOLD T60","GOLD T240","GOLD MOM","GOLD TOM","SILV T240","SILV MA","DOW RSI2","DOW IBS","NAS RSI2","NAS IBS","AUDJPY BB","SILV H4-55","JPY GOTOBI","NAS 3RD-FRI","JPY FIX-REV","GOLD LOTTERY","NAS LOTTERY","JPY LOTTERY","EURGBP MR","NAS GAP-LONG","GOLD IBS","SPX RSI2"]
def sleeve_of(c):
    if not c or not c.startswith('v13 '): return None
    s=c[4:]
    best=None
    for k,n in enumerate(NAMES):
        if s==n or s.startswith(n+' ') or s.startswith(n):
            if best is None or len(n)>len(NAMES[best]): best=k
    return best
def load(p):
    rows=list(openpyxl.load_workbook(p,read_only=True).worksheets[0].iter_rows(values_only=True))
    oi=[i for i,r in enumerate(rows) if r and r[0]=='Orders'][0]; di=[i for i,r in enumerate(rows) if r and r[0]=='Deals'][0]
    orders={}
    for r in rows[oi+2:di]:
        if r[1] is None or not isinstance(r[1],(int,float)): continue
        orders[int(r[1])]=dict(sl=r[7],comment=r[12])
    deals=[]
    for r in rows[di+2:]:
        if r[1] is None or not isinstance(r[1],(int,float)): continue
        deals.append(dict(time=pd.Timestamp(r[0].replace('.','-')),deal=int(r[1]),sym=r[2],type=r[3],dir=r[4],vol=float(r[5]) if r[5] else 0,price=r[6],order=r[7],comm=r[8] or 0,swap=r[9] or 0,profit=r[10] or 0,bal=r[11],comment=r[12]))
    return orders,deals
LOGC={}
def loadlog(f):
    import re
    for l in open(f):
        l=l.split('\t')[-1]
        m=re.match(r'(\S+ \S+)\s+(?:market (?:sell|buy) [\d.]+ \S+, close #(\d+)|(?:stop loss|take profit) triggered #(\d+))',l)
        if m: LOGC.setdefault(pd.Timestamp(m.group(1).replace('.','-')),set()).add(int(m.group(2) or m.group(3)))
def build(p,EXP=None):
    orders,deals=load(p)
    openpos=collections.defaultdict(list)  # sym -> list of pos dict
    pos=[];factors=collections.defaultdict(list);amb=0
    for d in deals:
        if d['type']=='balance': continue
        if d['dir']=='in':
            k=sleeve_of(d['comment'])
            o=orders.get(d['order'],{})
            ps=dict(id=d['deal'],sym=d['sym'],side=1 if d['type']=='buy' else -1,k=k,comment=d['comment'],t_in=d['time'],ep=d['price'],vol0=d['vol'],vol=d['vol'],sl=o.get('sl'),pnl=0.0,comm=d['comm'],swap=d['swap'],gross=0.0,t_out=None,outs=[],exit_comments=[])
            openpos[d['sym']].append(ps);pos.append(ps)
        else:
            side=-1 if d['type']=='buy' else 1   # closing a long uses sell
            cands=[q for q in openpos[d['sym']] if q['side']==side and q['vol']>=d['vol']-1e-9]
            if not cands: print('NO MATCH',d);continue
            def fac(q):
                dp=(d['price']-q['ep'])*side*d['vol']
                return d['profit']/dp if abs(dp)>1e-12 else None
            fl=[(q,fac(q)) for q in cands]
            exp=(EXP or {}).get(d['sym']) or (pd.Series(factors[d['sym']]).median() if factors[d['sym']] else None)
            if d['sym']=='USDJPY': exp=100000/d['price']
            if len(cands)==1: q=cands[0]
            else:
                amb+=1
                def score(x):
                    q,f=x
                    if f is None or f<=0: return 1e9 if abs(d['profit'])>0.02 else 1e6
                    return abs(math.log(f/exp)) if exp else 0
                lg=LOGC.get(d['time'],set())|LOGC.get(d['time']-pd.Timedelta(seconds=1),set())
                q=min(fl,key=lambda x:(score(x)+(0 if x[0]['id'] in lg else 5)))[0]
            f=fac(q)
            if len(cands)==1 and f and f>0 and abs(d['profit'])>5: factors[d['sym']].append(f)
            q['vol']-=d['vol']; q['gross']+=d['profit']; q['comm']+=d['comm']; q['swap']+=d['swap']; q['t_out']=d['time']; q['outs'].append(d['deal']); q['xp']=d['price']; q['exit_comments'].append(d['comment'] or '')
            if q['vol']<1e-9: openpos[d['sym']].remove(q)
    for q in pos:
        q['net']=q['gross']+q['comm']+q['swap']
        f=pd.Series(factors[q['sym']]).median() if factors[q['sym']] else None
        q['fac']=f
        q['risk']=abs(q['ep']-q['sl'])*q['vol0']*f if q['sl'] and f else None
        q['R']=q['net']/q['risk'] if q['risk'] else None
    df=pd.DataFrame(pos)
    df['name']=df.k.map(lambda k: NAMES[int(k)] if k==k and k is not None else 'UNK')
    bal=pd.DataFrame([d for d in deals])[['time','bal','profit','comm','swap','type']]
    return df,bal,amb,{s:pd.Series(v).median() for s,v in factors.items()}
if __name__=='__main__':
    for p,o,lf in zip(sys.argv[1::3],sys.argv[2::3],sys.argv[3::3]):
        LOGC.clear();loadlog(lf)
        df,bal,amb,fac=build(p)
        fac.update({'XAUUSD':100,'XAGUSD':5000,'USTEC':1,'US500':1,'US30':1})
        df,bal,amb,fac=build(p,fac)
        df.drop(columns=['outs']).to_csv(o+'_pos.csv',index=False); bal.to_csv(o+'_bal.csv',index=False)
        print(p,'positions',len(df),'ambiguous outs',amb,'open left',int((df.vol>1e-9).sum()),'net',round(df.net.sum(),2)); print(fac)
