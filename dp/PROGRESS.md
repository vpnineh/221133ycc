# DonchianProp research progress (v13.13 -> v13.14)
Recovery: `sh dp/setup.sh` (downloads HistData + Dropbox US30, builds bins and harness). Then from dp/bt:
`export DATADIR=$PWD/data/bin/ OUTDIR=$PWD/out`.

## Data
- HistData 2013..2026-11 (8 symbols) + holdout zips 2009..2012 in data/zips_h (NAS100/SPX500 start 2010).
- US30 from user Dropbox export -> sha256 identical to MANIFEST.
- Other bins differ slightly from MANIFEST (HistData revised the 2026 monthly files; reference ended 2026-10-09).

## Baseline validation (CALR, 100k funded, 2016-01..2026-10)
| run | %/mo | Sharpe | MaxDD | worst day | README |
|---|---|---|---|---|---|
| v13.13 | 2.28 | 2.26 | 8.37 | -3.84 | 2.30 / 8.37 / -3.84 |
| v13.12 | 2.21 | 2.17 | 9.92 | -3.65 | 2.23 / 9.92 / -3.65 |
| v13.13 VF=0 OVR=P.7=0.10,SLK.3=2.0 | identical to v13.12 | | | | |
