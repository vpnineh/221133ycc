# diag.py OUT NAME [BAL] : monthly portfolio % (on initial balance), negative months, per-sleeve monthly $ and correlations
import sys,os,numpy as np,pandas as pd
sys.path.insert(0,os.path.join(os.path.dirname(__file__),'..','bt','tools2'))
from trades import trades
NM=["GOLD T60","GOLD T240","GOLD MOM","GOLD TOM","SILV T240","SILV MA","DOW RSI2","DOW IBS","NAS RSI2","NAS IBS","AUDJPY BB","SILV H4-55","JPY GOTOBI","NAS 3RD-FRI","JPY FIX-REV","GOLD LOTTERY","NAS LOTTERY","JPY LOTTERY","EURGBP MR","NAS GAP-LONG","GOLD IBS","SPX RSI2","FED POST US30","FED POST SPX","DXY-JPY MR","GOLD Z-CONT"]
o,n=sys.argv[1],sys.argv[2]; B=float(sys.argv[3]) if len(sys.argv)>3 else 100000
E=pd.read_csv(f'{o}/{n}_eq.csv'); E['d']=pd.to_datetime(E.day,unit='D'); E['m']=E.d.dt.to_period('M')
me=E.groupby('m').eq_close.last(); prev=me.shift(1); prev.iloc[0]=B
mr=(me-prev)/B*100
print(f'months {len(mr)} mean {mr.mean():.2f} med {mr.median():.2f} std {mr.std():.2f}  neg {int((mr<0).sum())} (<0.5%: {int((mr<0.5).sum())}, <1%: {int((mr<1).sum())}, >=3%: {int((mr>=3).sum())})  worst {mr.min():.2f}')
print('worst months:', ', '.join(f'{k}:{v:.2f}' for k,v in mr.nsmallest(12).items()))
yr=mr.groupby(mr.index.year); print('per year mean %/mo, neg months:'); print(pd.DataFrame({'mean':yr.mean().round(2),'neg':yr.apply(lambda x:(x<0).sum())}).T.to_string())
T=trades(f'{o}/{n}_deals.csv'); T['m']=pd.to_datetime(T.t_out,unit='s').dt.to_period('M'); T['nm']=T.k.map(lambda k:NM[k] if 0<=k<len(NM) else str(k))
P=T.pivot_table(index='m',columns='nm',values='net',aggfunc='sum').fillna(0).reindex(mr.index).fillna(0)/B*100
P.to_csv(f'{o}/{n}_sleeve_month.csv'); mr.to_csv(f'{o}/{n}_month.csv')
neg=mr<0
s=pd.DataFrame({'tot%':P.sum().round(1),'mean%/mo':P.mean().round(3),'std':P.std().round(2),'negMo%':(P<0).mean().round(2),
  'inNegMonths%':P[neg.values].sum().round(1),'corrPort':P.corrwith(mr).round(2),'n':T.groupby('nm').size()}).sort_values('tot%',ascending=False)
print(s.to_string())
if len(sys.argv)>4: print(P.corr().round(2).to_string())
