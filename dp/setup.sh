#!/bin/sh
# Idempotent recovery: download data, build bins, build harness binaries.  usage: sh dp/setup.sh  (from repo root or anywhere)
set -e
R="$(cd "$(dirname "$0")" && pwd)"; B="$R/bt"; cd "$B"
mkdir -p work data/bin
# 1) HistData M1 zips (resumable)
python3 -I data/scripts/histdata_get.py data/zips XAUUSD,XAGUSD,NAS100,SPX500,USDJPY,AUDJPY,EURGBP,GBPUSD 2013 >> work/dl.log 2>&1
# 2) US30 from the user's Dropbox MT5 export
if [ ! -f data/bin/US30_m1.bin ]; then
  mkdir -p work/us30 work/us30w/bin work/us30w/out
  [ -f work/us30/DJIUSD_M1.csv ] || curl -sSL -o work/us30/DJIUSD_M1.csv "https://www.dropbox.com/scl/fi/cpd1ajlnjbkdtkmjrvswt/DJIUSD.prop_M1_201601040200_202610150216?rlkey=kxzqaoo2k9kyl5ixopndmyva5&st=40hzflju&dl=1"
  python3 data/scripts/us30/dji_clean.py work/us30/DJIUSD_M1.csv work/us30w > work/us30_clean.log 2>&1
  cp work/us30w/bin/US30_m1.bin data/bin/
fi
# 3) bins + QA
python3 -I data/scripts/build_bin.py data/zips data/bin XAUUSD,XAGUSD,NAS100,SPX500,USDJPY,AUDJPY,EURGBP,GBPUSD
python3 -I data/scripts/dataqa.py data/bin > data/qa_report.txt || true
# 4) harness
for v in $(ls harness/ea); do sh harness/build.sh $v --conv; done
echo "setup done; export DATADIR=$B/data/bin/"
