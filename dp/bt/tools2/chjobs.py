# chjobs.py TAG "KEY=VAL ..." > jobs : 39 quarterly starts 2016Q1..2025Q3, 24-month window, chain PRESET=0, LOUD
import sys,pandas as pd
tag=sys.argv[1]; extra=sys.argv[2] if len(sys.argv)>2 else ''
for q in pd.date_range('2016-01-01','2025-07-01',freq='QS'):
    f=int(q.value//10**9); t=int((q+pd.DateOffset(months=24)).value//10**9)
    print(f"{tag}_{q:%Y%m} PRESET=0 LOUD=1 FROM={f} TO={min(t,1792000000)} {extra}")
