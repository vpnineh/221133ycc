#!/usr/bin/env python3
"""Parse MT5 tester journal (UTF-8; convert UTF-16 logs with iconv first) -> market orders (bid/ask at fill) and SL/TP triggers.
usage: logparse.py LOG OUTPREFIX   -> OUTPREFIX_mkt.csv, OUTPREFIX_stops.csv (deduplicated over repeated passes)"""
import re,sys,pandas as pd
M=re.compile(r'\t(\d{4}\.\d\d\.\d\d \d\d:\d\d:\d\d)\s+market (buy|sell) ([\d.]+) (\S+) .*?\(([\d.]+) / ([\d.]+) / [\d.]+\)\s*$')
T=re.compile(r'\t(\d{4}\.\d\d\.\d\d \d\d:\d\d:\d\d)\s+(stop loss|take profit) triggered #\d+ (buy|sell) ([\d.]+) (\S+) ([\d.]+) sl: ([\d.]+)(?: tp: ([\d.]+))? \[#\d+ (?:buy|sell) [\d.]+ \S+ at ([\d.]+)\]')
mk=[];st=[]
for l in open(sys.argv[1],encoding='utf-8',errors='replace'):
    if 'triggered' not in l and 'market ' not in l: continue
    if 'OrderSend' in l: continue
    m=M.search(l)
    if m: t,s,v,y,b,a=m.groups();mk.append((t,s,float(v),y,float(b),float(a)));continue
    m=T.search(l)
    if m: t,k,s,v,y,op,sl,tp,px=m.groups();st.append((t,'SL' if k=='stop loss' else 'TP',s,float(v),y,float(op),float(sl),float(tp) if tp else 0.0,float(px)))
A=pd.DataFrame(mk,columns=['t','side','vol','sym','bid','ask']).drop_duplicates();A['t']=pd.to_datetime(A.t,format='%Y.%m.%d %H:%M:%S');A['spr']=A.ask-A.bid
B=pd.DataFrame(st,columns=['t','kind','side','vol','sym','open','sl','tp','px']).drop_duplicates();B['t']=pd.to_datetime(B.t,format='%Y.%m.%d %H:%M:%S')
A.sort_values('t').to_csv(sys.argv[2]+'_mkt.csv',index=False);B.sort_values('t').to_csv(sys.argv[2]+'_stops.csv',index=False)
print(sys.argv[1],'market',len(A),'stops',len(B))
