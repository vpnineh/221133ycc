# HistData M1 ASCII downloader: hd.py OUTDIR SYM Y0 Y1   (years < current: yearly zip; current year: monthly)
import sys,os,re,time,urllib.request,urllib.parse,http.cookiejar
out,sym,y0,y1=sys.argv[1],sys.argv[2],int(sys.argv[3]),int(sys.argv[4])
CUR=2026
cj=http.cookiejar.CookieJar(); op=urllib.request.build_opener(urllib.request.HTTPCookieProcessor(cj))
UA={'User-Agent':'Mozilla/5.0'}
def get(y,m=None):
    fn=f"{out}/{sym}_{y}{'' if m is None else '%02d'%m}.zip"
    if os.path.exists(fn) and os.path.getsize(fn)>1000: return 'have'
    page=f"https://www.histdata.com/download-free-forex-historical-data/?/ascii/1-minute-bar-quotes/{sym.lower()}/{y}"+('' if m is None else f"/{m}")
    for a in range(4):
        try:
            h=op.open(urllib.request.Request(page,headers=UA),timeout=60).read().decode('utf-8','ignore')
            tk=re.search(r'id="tk" value="([^"]+)"',h)
            if not tk: return 'notk'
            d=urllib.parse.urlencode({'tk':tk.group(1),'date':str(y),'datemonth':f"{y}{'' if m is None else '%02d'%m}",'platform':'ASCII','timeframe':'M1','fxpair':sym}).encode()
            r=op.open(urllib.request.Request('https://www.histdata.com/get.php',data=d,headers={**UA,'Referer':page}),timeout=300).read()
            if len(r)<1000: return 'small'
            open(fn,'wb').write(r); return 'ok %d'%len(r)
        except Exception as e: time.sleep(3*(a+1)); err=str(e)
    return 'fail '+err
for y in range(y0,y1+1):
    if y<CUR: print(sym,y,get(y),flush=True)
    else:
        for m in range(1,11): print(sym,y,m,get(y,m),flush=True)
