#!/bin/sh
# r.sh OUTSUB VER NAME ARGS... : run harness from bt with DATADIR=data/bin (override DATADIR env), OUTDIR=out/OUTSUB
cd /home/user/221133ycc/dp/bt || exit 2
o=$1; shift; mkdir -p out/$o
DATADIR=${DATADIR:-$PWD/data/bin/} OUTDIR=$PWD/out/$o harness/run.sh "$@"
