# runner.py JOBFILE OUTDIR P BIN COMMON...   job line: NAME KEY=VAL ...   (skips finished runs; DATADIR from env)
import sys,os,subprocess,concurrent.futures as cf
jf,out,P,ver=sys.argv[1],sys.argv[2],int(sys.argv[3]),sys.argv[4]; common=sys.argv[5:]
out=os.path.abspath(out)
H=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','harness')
os.makedirs(out,exist_ok=True)
jobs=[l.split() for l in open(jf) if l.strip() and not l.startswith('#')]
def run(j):
    n=j[0]; lg=f'{out}/{n}.log'
    if os.path.exists(lg) and '"end_balance"' in open(lg).read(): return n+' have'
    env=dict(os.environ,OUTDIR=out)
    subprocess.run(['sh',f'{H}/run.sh',ver,n]+common+j[1:],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    ok='"end_balance"' in open(lg).read()
    return n+(' ok' if ok else ' FAIL')
with cf.ThreadPoolExecutor(P) as ex:
    for r in ex.map(run,jobs): print(r,flush=True)
print('ALLDONE',flush=True)
