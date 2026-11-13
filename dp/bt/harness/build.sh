#!/bin/sh
# build harness binary for one EA version: build.sh VER [--conv]
#   VER = v131 | v132 | v133 (EA sources in ea/VER). --conv regenerates ea/VER/ea.cpp from ea/VER/ea.mq5 first.
set -e
cd "$(dirname "$0")"
V=${1:-v133}; mkdir -p bin
if [ "$2" = "--conv" ]; then
  python3 conv2.py ea/$V/ea.mq5 ea/$V/ea.cpp '^string GvKey\(' >/dev/null
  sed -i 's/MqlCalendarValue vals\[\];/MArr<MqlCalendarValue> vals;/; s/string parts\[\];/MArr<string> parts;/; s/string al\[\];/MArr<string> al;/; s/MqlCalendarEvent evs\[\];/MArr<MqlCalendarEvent> evs;/' ea/$V/ea.cpp
fi
Q=""; case $V in v13[3-9]*|v131*|v1312*|v1313*|vp*|vf*) Q="-DQCNT";; esac
${CXX:-g++} -O2 -std=c++17 -w -I. -DEA_SRC="\"ea/$V/ea.cpp\"" -DINPUTS_SRC="\"ea/$V/inputs119.h\"" $Q t114_run.cpp -o bin/t_$V
echo "built bin/t_$V"
