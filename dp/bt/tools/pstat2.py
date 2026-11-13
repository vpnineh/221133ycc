import sys,os,numpy as np,pandas as pd,warnings;warnings.filterwarnings('ignore')
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)));import chsim
O=os.environ.get('OUTDIR',os.path.dirname(os.path.abspath(__file__))+'/../out')+'/'
CUTS=[('2016-01-01','2019-01-01'),('2019-01-01','2023-01-01'),('2023-01-01','2027-01-01'),('2016-01-01','2027-01-01')]
def load(n):
    e=pd.read_csv(f'{O}{n}_eq.csv');idx=pd.to_datetime(e.day*86400,unit='s')
    c=pd.Series(e.eq_close.values,index=idx);m=pd.Series(e.eq_min.values,index=idx);k=c.index.dayofweek<5;return c[k],m[k]
def stat(n):
    c,m=load(n);out=f'{n:10s}'
    for a,b in CUTS:
        cc=c[a:b];mm=m[a:b]
        if len(cc)<20: continue
        yrs=(cc.index[-1]-cc.index[0]).days/365.25;ret=(cc.iloc[-1]-cc.iloc[0])/1000/yrs
        pk=cc.cummax().shift(1).fillna(cc.iloc[0]);dd=((pk-mm).clip(lower=0)/1000).max()
        dr=cc.diff().dropna()/1e5;sh=dr.mean()/dr.std()*np.sqrt(252)
        out+=f' | {a[:4]}-{b[2:4]} {ret:5.2f}%/y DD{dd:5.2f} S{sh:4.2f}'
    y=c.resample('YE').last().diff();y.iloc[0]=c.resample('YE').last().iloc[0]-1e5
    out+='\n  yrs: '+' '.join(f'{d.year%100}:{v/1000:+.1f}' for d,v in y.items())
    # ex-covid: drop Feb20-May20 pnl
    pn=c.diff().fillna(0);ex=pn[(pn.index<'2020-02-15')|(pn.index>'2020-05-31')]
    out+=f'\n  exCOVID total {ex.sum()/1000:+.1f}%  COVID window {(pn.sum()-ex.sum())/1000:+.1f}%'
    pnv=pn.values;dip=(m-c.shift(1)).fillna(0).clip(upper=0).values;D=c.index
    for r in (1.4,2.0):
        x=chsim.challenge(pnv,dip,D,r,r*0.85,dict(ddb=4,ddm=0.5),step=5)
        out+=f" | x{r}: pass{x['pass_']:3.0f}% fail{x['fail']:3.0f}% med{x['med']:3.0f}d"
    return out
def sleeves(n):
    d=pd.read_csv(f'{O}{n}_deals.csv');d=d[d.entry==1].copy();d['y']=pd.to_datetime(d.time,unit='s').dt.year
    d['pl']=d.profit+d.comm+d.swap;g=d.groupby(['magic']).agg(n=('pl','size'),pl=('pl','sum'),sw=('swap','sum'))
    t=d.pivot_table(index='magic',columns='y',values='pl',aggfunc='sum').fillna(0)/1000
    print((t.round(1)).assign(n=g.n,tot=g.pl/1000,swap=g.sw/1000).round(1).to_string())
if __name__=='__main__':
    for n in sys.argv[1:]: print(stat(n));sleeves(n)
