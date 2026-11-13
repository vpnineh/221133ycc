import sys,os,importlib.util
sp=importlib.util.spec_from_file_location('scalp','/home/user/221133ycc/dp/tools3/scalp.py'); S=importlib.util.module_from_spec(sp); sp.loader.exec_module(S)
rule=sys.argv[1]; K=float(os.environ.get('K','4'))
for s in os.environ.get('SYMS','XAUUSD,NAS100,SPX500,USDJPY,EURGBP').split(','):
    T=S.evaluate(rule,s,K,cx=0.0); T=T[T.t>=1451606400]; print(rule,s,'gross',S.stats(T),flush=True)
