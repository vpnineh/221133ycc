# mkfresh.py TAG "KEY=VAL ..." > jobs : 39 fresh funded accounts (quarterly starts 2016Q1..2025Q3), 24 months each,
# funded mode (PRESET=2) with monthly payout of all profit (PAYD=30 PAY1=30 PAYB=0) - the FundedNext real-world case.
import sys,pandas as pd
tag=sys.argv[1]; extra=sys.argv[2] if len(sys.argv)>2 else ''
for q in pd.date_range('2016-01-01','2025-07-01',freq='QS'):
    f=int(q.value//10**9); t=int((q+pd.DateOffset(months=24)).value//10**9)
    print(f"{tag}_{q:%Y%m} PRESET=2 FROM={f} TO={min(t,1792000000)} PAYD=30 PAY1=30 PAYB=0 {extra}")
