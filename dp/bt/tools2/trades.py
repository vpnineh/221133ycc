# deals.csv -> per-position trades: sleeve k, sym, dir, t_in, t_out, entry, sl0, vol, net, R (net / initial risk money)
import sys,pandas as pd,numpy as np
def trades(path):
    D=pd.read_csv(path); D=D[D.type<2]
    rows=[]
    for pos,g in D.groupby('pos'):
        i=g[g.entry==0]
        if not len(i): continue
        i=i.iloc[0]; o=g[g.entry==1]
        if not len(o): continue
        d=1 if i.type==0 else -1
        net=(g.profit+g.comm+g.swap).sum()
        gross=o.profit.sum(); mv=((o.price-i.price)*d*o.vol).sum()
        upv=gross/mv if abs(mv)>1e-12 else np.nan          # money per (price unit x lot)
        risk=abs(i.price-i.sl)*i.vol*upv if i.sl>0 else np.nan
        rows.append(dict(pos=pos,k=int(i.magic)-110000,sym=i.sym,dir=d,t_in=int(i.time),t_out=int(o.time.max()),entry=i.price,sl0=i.sl,vol=i.vol,net=net,risk=risk,why=str(o.why.iloc[-1])))
    T=pd.DataFrame(rows); T['R']=T.net/T.risk; return T
if __name__=='__main__':
    T=trades(sys.argv[1]); T.to_csv(sys.argv[2],index=False); print(len(T)); print(T.groupby('k').agg(n=('R','size'),R=('R','mean'),sumR=('R','sum')).round(3))
