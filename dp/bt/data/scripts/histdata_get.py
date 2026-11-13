#!/usr/bin/env python3
"""Download HistData.com free M1 ASCII bars (bid, EST without DST labels) for the harness symbols.
usage: histdata_get.py OUTDIR [SYM,SYM,...] [FIRST_YEAR] [LAST_YEAR]
  Past years come as one yearly zip; the current year as monthly zips (OUTDIR/<SYM>/HISTDATA_COM_ASCII_<HD>_M1_<YYYY[MM]>.zip).
  Mechanism: GET the download page, read the hidden form (tk token etc.), POST it to https://www.histdata.com/get.php.
  Resumable (existing zips are skipped). US30 is NOT offered by HistData: supply an MT5 M1 export (see README)."""
import os,re,sys,time,datetime as dt,urllib.request,urllib.parse
HD={'XAUUSD':'XAUUSD','XAGUSD':'XAGUSD','NAS100':'NSXUSD','SPX500':'SPXUSD','DAX':'GRXEUR'}
UA='Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0 Safari/537.36'
BASE='https://www.histdata.com/download-free-forex-historical-data/?/ascii/1-minute-bar-quotes/'
def req(url,data=None,ref=None):
    h={'User-Agent':UA}
    if ref: h['Referer']=ref
    r=urllib.request.Request(url,data=urllib.parse.urlencode(data).encode() if data else None,headers=h)
    return urllib.request.urlopen(r,timeout=120).read()
def one(sym,y,m,out):
    hd=HD.get(sym,sym); tag=f'{y}{m:02d}' if m else f'{y}'
    f=os.path.join(out,sym,f'HISTDATA_COM_ASCII_{hd}_M1_{tag}.zip')
    if os.path.exists(f) and os.path.getsize(f)>1000: return 'skip'
    url=BASE+f'{hd.lower()}/{y}'+(f'/{m}' if m else '')
    page=req(url).decode('utf-8','replace')
    fm=re.search(r'<form[^>]*id="file_down"[^>]*>(.*?)</form>',page,re.S)
    if not fm: return 'noform'
    fields=dict(re.findall(r'name="([A-Za-z_]+)"[^>]*value="([^"]*)"',fm.group(1)))
    if 'tk' not in fields: return 'notoken'
    b=req('https://www.histdata.com/get.php',fields,url)
    if b[:2]!=b'PK': return 'notzip'
    os.makedirs(os.path.dirname(f),exist_ok=True); open(f,'wb').write(b); time.sleep(1.0); return f'ok {len(b)/1e6:.1f}MB'
if __name__=='__main__':
    out=sys.argv[1]; syms=(sys.argv[2] if len(sys.argv)>2 else 'XAUUSD,XAGUSD,NAS100,SPX500,USDJPY,AUDJPY,EURGBP,GBPUSD').split(',')
    y0=int(sys.argv[3]) if len(sys.argv)>3 else 2013; now=dt.date.today(); y1=int(sys.argv[4]) if len(sys.argv)>4 else now.year
    for s in syms:
        for y in range(y0,y1+1):
            jobs=[(y,0)] if y<now.year else [(y,m) for m in range(1,now.month+1)]
            for yy,m in jobs:
                try: r=one(s,yy,m,out)
                except Exception as e: r=f'error {e}'
                print(s,yy,m or '',r,flush=True)
