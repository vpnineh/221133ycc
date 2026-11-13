#!/usr/bin/env python3
"""Download Dukascopy daily M1 candle files (free datafeed; BID or ASK) -> OUTDIR/<DSYM>/<SIDE>/YYYYMMDD.bi5 (LZMA, raw).
usage: dukas_get.py DSYM YYYY-MM-DD YYYY-MM-DD OUTDIR [BID|ASK] [--days-file f]
  DSYM examples: XAUUSD XAGUSD USDJPY AUDJPY EURGBP GBPUSD USA30IDXUSD USATECHIDXUSD USA500IDXUSD
  Polite: one request at a time with a pause (DUKAS_PAUSE, default 1.2 s), User-Agent set, exponential back-off on HTTP 429.
  Weekends are skipped. Existing files are kept (resumable)."""
import sys,os,time,datetime as dt,urllib.request,urllib.error
def get(url,tries=8):
    p=float(os.environ.get('DUKAS_PAUSE','1.2'))
    for k in range(tries):
        try:
            r=urllib.request.urlopen(urllib.request.Request(url,headers={'User-Agent':'Mozilla/5.0 (X11; Linux x86_64)'}),timeout=60); b=r.read(); time.sleep(p); return b
        except urllib.error.HTTPError as e:
            if e.code==404: time.sleep(p); return b''
            time.sleep(min(300,p*(2**(k+2))))
        except Exception: time.sleep(min(300,p*(2**(k+2))))
    return None
if __name__=='__main__':
    sym,a,b,out=sys.argv[1:5]; side=sys.argv[5] if len(sys.argv)>5 and not sys.argv[5].startswith('--') else 'BID'
    days=None
    if '--days-file' in sys.argv: days=[l.strip() for l in open(sys.argv[sys.argv.index('--days-file')+1]) if l.strip()]
    d0=dt.date.fromisoformat(a); d1=dt.date.fromisoformat(b); D=os.path.join(out,sym,side); os.makedirs(D,exist_ok=True)
    todo=[dt.date.fromisoformat(x) for x in days] if days else [d0+dt.timedelta(n) for n in range((d1-d0).days+1)]
    ok=fail=0
    for d in todo:
        if d.weekday()==5: continue
        f=os.path.join(D,d.strftime('%Y%m%d')+'.bi5')
        if os.path.exists(f): continue
        u=f'https://datafeed.dukascopy.com/datafeed/{sym}/{d.year}/{d.month-1:02d}/{d.day:02d}/{side}_candles_min_1.bi5'
        r=get(u)
        if r is None: fail+=1; print('FAIL',u,flush=True); continue
        open(f,'wb').write(r); ok+=1
    print(sym,side,'downloaded',ok,'failed',fail)
