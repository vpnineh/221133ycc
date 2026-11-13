# reg.py : weak-regime diagnosis of candidate F. Monthly portfolio result and sleeve-group R by regime (IS 2016-26 + LH 2010-15)
import numpy as np,pandas as pd,sys,os
import os; S=os.environ.get('PKG','.')
sys.path.insert(0,S+'/w/tools2'); from trades import trades
def daily(s):
    fr=[]
    for dn in ['binold','binref']:
        f=f'{S}/data/{dn}/{s}_utc.npy'
        if not os.path.exists(f): continue
        a=np.load(f); t=pd.to_datetime(a['t'],unit='s'); d=pd.DataFrame({'h':a['h'],'l':a['l'],'c':a['c']},index=t-pd.Timedelta(hours=22)+pd.Timedelta(days=1))
        fr.append(d.resample('D').agg({'h':'max','l':'min','c':'last'}).dropna())
    D=pd.concat(fr); D=D[~D.index.duplicated(keep='last')].sort_index(); return D[D.index.dayofweek<5]
def feats(s):
    D=daily(s); c=D.c; pc=c.shift(); tr=pd.concat([D.h-D.l,(D.h-pc).abs(),(D.l-pc).abs()],axis=1).max(axis=1)
    F=pd.DataFrame(index=D.index); F['r60']=c/c.shift(60)-1; F['a200']=c/c.rolling(200).mean()-1
    rv=np.log(c/pc).rolling(20).std(); F['volp']=rv.rolling(250).rank(pct=True)
    er=(c-c.shift(60)).abs()/ (c.diff().abs().rolling(60).sum()); F['er']=er   # Kaufman efficiency: trend vs range
    return F
