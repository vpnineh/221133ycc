#!/bin/sh
# Replicate the 5 MT5 tester runs with the harness. usage: [FNP=profile] [MQP=profile] [DATADIR=...] tools/reps.sh TAG [KEY=VALUE ...]
# MQ22: MAXLOSS=100 because the 10x-metal-sizing replica trips the 10% guard in 2022-05 while the tester did not.
# outputs $OUTDIR/{T23,T24,T25,MQ22,MQ18}_TAG_*; then: python3 tools/evalall.py TAG
H="$(cd "$(dirname "$0")/../harness" && pwd)"; T=$1; shift
J=${JOBS:-3}
r(){ "$H/run.sh" "$@" >/dev/null; }
r v131 T23_$T @${FNP:-FNT} PRESET=0 FROM=1704067200 PREDAYS=0 PREDAYS_AUDJPY=365 "$@" &
r v131 T24_$T @${FNP:-FNT} PRESET=2 FROM=1704067200 PREDAYS=0 PREDAYS_AUDJPY=365 "$@" &
r v131 T25_$T @${FNP:-FNT} PRESET=2 FROM=1735689600 PREDAYS=366 "$@" &
[ "$J" -lt 5 ] && wait
r v131 MQ22_$T @${MQP:-MQ} FROM=1640995200 PREDAYS=362 PREDAYS_NAS100=-186 MAXLOSS=100 "$@" &
r v132 MQ18_$T @${MQP:-MQ} FROM=1514764800 PREDAYS=1100 PREDAYS_NAS100=-1647 "$@" &
wait
