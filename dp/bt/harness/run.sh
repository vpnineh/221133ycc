#!/bin/sh
# run.sh VER NAME [@PROFILE ...] [KEY=VALUE ...]
#   VER: v131|v132|v133 (bin/t_VER from build.sh). NAME: output prefix -> $OUTDIR/NAME_{deals,eq}.csv + NAME.log
#   @PROFILE expands profiles/PROFILE.env (one KEY=VALUE per line, # comments). Later keys override earlier ones.
#   DATADIR (default <pkg>/data/bin/) and OUTDIR (default <pkg>/out) may be set in the environment.
H="$(cd "$(dirname "$0")" && pwd)"
V=$1; N=$2; shift 2
DATADIR=${DATADIR:-$H/../data/bin/}; OUTDIR=${OUTDIR:-$H/../out}; mkdir -p "$OUTDIR"
OUTDIR=$(cd "$OUTDIR" && pwd); DATADIR=$(cd "$DATADIR" && pwd)/   # absolute: the binary runs from harness/
set -- PRESET=2 DEMO=0 TZS=4 TZW=2 SWAPM=1 FROM=1451606400 TO=1792000000 PREDAYS=1100 \
  SYMS=XAUUSD,XAGUSD,US30,NAS100,AUDJPY,USDJPY,EURGBP,SPX500,GBPUSD "$@"
A=""
for a in "$@"; do
  case "$a" in
    @*) f="$H/profiles/${a#@}.env"; [ -f "$f" ] || { echo "no profile $f" >&2; exit 2; }
        while IFS= read -r l || [ -n "$l" ]; do case "$l" in ''|'#'*) ;; *) A="$A
$l";; esac; done < "$f" ;;
    *) A="$A
$a" ;;
  esac
done
cd "$H" || exit 2
IFS='
'
env DATADIR="$DATADIR" OUT="$OUTDIR/$N" $A ./bin/t_$V > "$OUTDIR/$N.log" 2>&1
rc=$?; tail -1 "$OUTDIR/$N.log"; exit $rc
