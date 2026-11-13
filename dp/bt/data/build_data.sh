#!/bin/sh
# Rebuild the harness M1 data (data/bin) from HistData.com.  usage: data/build_data.sh [FIRST_YEAR]
#   1) downloads HistData M1 zips for XAUUSD XAGUSD NAS100 SPX500 USDJPY AUDJPY EURGBP GBPUSD into data/zips
#   2) converts them to data/bin/<SYM>_m1.bin (+ _utc.npy), repairing the 2026 duplicate-minute glitch
#   3) quality report (gaps / duplicates / spikes) -> data/qa_report.txt
# US30 is not on HistData: export DJIUSD (or your broker's US30) M1 history from MT5 and convert it with
#   python3 data/scripts/us30/dji_clean.py <export.csv> <workdir>   (clock rules are specific to that export, see docs/CLOCK_REPORT.md)
#   or any M1 source written in the same 40-byte format (int64 UK-local epoch seconds, float64 o,h,l,c).
set -e
D="$(cd "$(dirname "$0")" && pwd)"; Y=${1:-2013}
python3 -I "$D/scripts/histdata_get.py" "$D/zips" XAUUSD,XAGUSD,NAS100,SPX500,USDJPY,AUDJPY,EURGBP,GBPUSD $Y
python3 -I "$D/scripts/build_bin.py" "$D/zips" "$D/bin" XAUUSD,XAGUSD,NAS100,SPX500,USDJPY,AUDJPY,EURGBP,GBPUSD
python3 -I "$D/scripts/dataqa.py" "$D/bin" > "$D/qa_report.txt"; echo "QA report: $D/qa_report.txt"
echo "checksums of the reference build: $D/MANIFEST.sha256 (cd data/bin && sha256sum -c ../MANIFEST.sha256)"
