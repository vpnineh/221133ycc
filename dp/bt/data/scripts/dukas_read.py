"""Decode Dukascopy daily M1 candle files: records of 24 bytes big-endian (int32 sec offset from 00:00 UTC, open, close, low, high as int32 price/scale, float32 volume)."""
import lzma,struct,os,glob,numpy as np,datetime as dt
SCALE={'JPY':1e3,'XAU':1e3,'XAG':1e3,'IDX':1e3}
def scale_of(sym):
    if sym.endswith('JPY'): return 1e3
    if sym.startswith(('XAU','XAG')) or 'IDX' in sym: return 1e3
    return 1e5
def read_day(path,sym):
    b=open(path,'rb').read()
    if not b: return np.zeros(0,dtype=[('t','<i8'),('o','<f8'),('h','<f8'),('l','<f8'),('c','<f8'),('v','<f8')])
    raw=lzma.decompress(b); n=len(raw)//24; a=np.frombuffer(raw[:n*24],dtype='>i4').reshape(n,6)
    v=np.frombuffer(raw[:n*24],dtype='>f4').reshape(n,6)[:,5]
    d=os.path.basename(path)[:8]; t0=int(dt.datetime(int(d[:4]),int(d[4:6]),int(d[6:8]),tzinfo=dt.timezone.utc).timestamp()); s=scale_of(sym)
    out=np.zeros(n,dtype=[('t','<i8'),('o','<f8'),('h','<f8'),('l','<f8'),('c','<f8'),('v','<f8')])
    out['t']=t0+a[:,0]; out['o']=a[:,1]/s; out['c']=a[:,2]/s; out['l']=a[:,3]/s; out['h']=a[:,4]/s; out['v']=v
    return out[out['v']>0]      # Dukascopy writes flat zero-volume filler bars for closed minutes
def read_dir(D,sym):
    fs=sorted(glob.glob(os.path.join(D,'*.bi5')))
    return np.concatenate([read_day(f,sym) for f in fs]) if fs else None
