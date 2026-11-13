if(getenv("MAXLOSS")) InpMaxLossPct=atof(getenv("MAXLOSS"));
{ const char*v; if((v=getenv("TZS"))) { g_tz_s=atoi(v); g_tz_w=atoi(getenv("TZW")); g_tz_ok=true; } if((v=getenv("GIBS"))) InpUseGoldIbs=atoi(v); if((v=getenv("DEMO"))) InpDemoMode=atoi(v); if((v=getenv("GMTOFF"))) InpServerGMTOffset=atoi(v);
  if((v=getenv("RISK"))){ double r=atof(v); InpRiskPhase1=r; InpRiskPhase2=r; InpRiskFunded=r; InpRiskCustom=r; InpFastRiskPhase1=r; InpFastRiskPhase2=r; InpFastRiskFunded=r; }
  if((v=getenv("NOLOT"))&&atoi(v)){ InpUseGoldLottery=false; InpUseNasLottery=false; InpUseJpyLottery=false; }
  if((v=getenv("NG2"))&&atoi(v)){ InpDailyLossPct=1000; InpMaxLossPct=1000; }
  if((v=getenv("MAXOR"))) InpMaxOpenRiskR=atof(v);
  if((v=getenv("DDB"))) InpDDBrakePct=atof(v);
  if((v=getenv("DDM"))) InpDDBrakeMult=atof(v);
  if((v=getenv("SOFT"))) InpSoftDailyStopPct=atof(v);
  if((v=getenv("MAXRT"))) InpMaxRiskPerTradeR=atof(v);
  if((v=getenv("MLOR"))) InpMinLotOverRisk=atof(v);
  if((v=getenv("ROOM"))) InpDailyRoomMult=atof(v);
  if((v=getenv("LOCK"))) InpLockOpenProfit=atoi(v);
  if((v=getenv("FRS"))) InpFundedRampStartPct=atof(v);
  if((v=getenv("FRA"))) InpFundedRampAtPct=atof(v);
  if((v=getenv("RP1"))) InpFastRiskPhase1=atof(v);
  if((v=getenv("RP2"))) InpFastRiskPhase2=atof(v);
  if((v=getenv("RF"))) InpFastRiskFunded=atof(v);
  if((v=getenv("RAMP"))) InpP1RampStartPct=atof(v);
  if((v=getenv("ACCT"))) InpAccountType=(ENUM_ACCTYPE)atoi(v);
  if((v=getenv("PRISK"))) InpPersRiskPct=atof(v);
  if((v=getenv("PDAY"))) InpPersDailyStopPct=atof(v);
  if((v=getenv("PDD"))) InpPersMaxDDPct=atof(v);
  if((v=getenv("PCOMP"))) InpPersCompound=atoi(v);
  if((v=getenv("FTC"))) InpFundedTightCushionPct=atof(v);
  if((v=getenv("NOGUARD"))&&atoi(v)){ InpDailyLossPct=1000; InpMaxLossPct=1000; InpSoftDailyStopPct=1000; InpMaxOpenRiskR=1000; }
  if((v=getenv("SLV"))){ int m=atoi(v); InpUseGoldT60=m&1; InpUseGoldT240=m&2; InpUseGoldMOM=m&4; InpUseGoldTOM=m&8; InpUseSilverT240=m&16; InpUseSilverMA=m&32; InpUseDowRSI2=m&64; InpUseDowIBS=m&128; InpUseNasRSI2=m&256; InpUseNasIBS=m&512; InpUseAudJpyBB=m&1024; InpUseSilverH4=m&2048; InpUseUsdJpyGotobi=m&4096; InpUseNas3FPS=m&8192; InpUseUsdJpyFixRev=m&16384; InpUseGoldLottery=m&32768; InpUseNasLottery=m&65536; InpUseJpyLottery=m&131072; InpUseEurGbpMR=m&262144; InpUseSpxRSI2=m&2097152; }
  if((v=getenv("LWG"))) { SL_W[15]=atof(v); InpUseGoldLottery=SL_W[15]>0; }
  if((v=getenv("LWN"))) { SL_W[16]=atof(v); InpUseNasLottery=SL_W[16]>0; }
  if((v=getenv("LWJ"))) { SL_W[17]=atof(v); InpUseJpyLottery=SL_W[17]>0; }
  if((v=getenv("SILVMA"))) InpUseSilverMA=atoi(v);   // h133
  if((v=getenv("STOPP"))) InpStopProfile=(ENUM_STOPPROF)atoi(v);   // h133
  if((v=getenv("RMODE"))) InpRiskMode=(ENUM_RISKMODE)atoi(v);
  if((v=getenv("NOEG"))&&atoi(v)) InpUseEurGbpMR=false;
  if((v=getenv("GCAP"))) InpGoldDirCapR=atof(v);
  if((v=getenv("VT"))) InpVolTarget=atoi(v);
  if((v=getenv("VTP"))) InpVolTargetPct=atof(v);
  if((v=getenv("VMIN"))) InpVolGMin=atof(v);
  if((v=getenv("VMAX"))) InpVolGMax=atof(v);
  if((v=getenv("NGL"))) InpUseNasGapLong=atoi(v);
  if((v=getenv("NGLW"))) SL_W[19]=atof(v);
  if((v=getenv("T60ADD"))) SL_ADDS[0]=atoi(v);
  if((v=getenv("GBSL"))) InpGotobiStopPips=atoi(v);
  if((v=getenv("TFSL"))) InpTfpsStopPct=atof(v);
  if((v=getenv("GR"))) InpGoldRegime=atoi(v); if((v=getenv("GRT"))) InpGRTrendMult=atof(v); if((v=getenv("GRM"))) InpGRMRMult=atof(v); if((v=getenv("GRA"))) InpGRAdx=atof(v);
  if((v=getenv("OVR"))){ string o=v; size_t a=0; while(a<o.size()){ size_t b=o.find(',',a); if(b==string::npos) b=o.size(); string it=o.substr(a,b-a); a=b+1;
      size_t d=it.find('.'), e=it.find('='); if(d==string::npos||e==string::npos) continue; string nm=it.substr(0,d); int k=atoi(it.substr(d+1,e-d-1).c_str()); double x=atof(it.substr(e+1).c_str());
      if(nm=="N") SL_N[k]=(int)x; else if(nm=="SIDE") SL_SIDE[k]=(int)x; else if(nm=="REG") SL_REG[k]=(int)x; else if(nm=="P") SL_P[k]=x; else if(nm=="SLK") SL_SLK[k]=x; else if(nm=="TRK") SL_TRK[k]=x;
      else if(nm=="TPK") SL_TPK[k]=x; else if(nm=="BEK") SL_BEK[k]=x; else if(nm=="W") SL_W[k]=x; else if(nm=="ADDS") SL_ADDS[k]=(int)x; else if(nm=="MAXB") SL_MAXB[k]=(int)x; else if(nm=="RTF") SL_RTF[k]=(int)x; else if(nm=="ADXT") SL_ADXT[k]=x; else if(nm=="X") SL_X[k]=x; else if(nm=="BER") SL_BER[k]=x; else if(nm=="BEO") SL_BEO[k]=x; else if(nm=="LKA") SL_LKA[k]=x; else if(nm=="LKB") SL_LKB[k]=x; else if(nm=="RTR") SL_RTR[k]=x; else if(nm=="PTR") SL_PTR[k]=x; else if(nm=="PFR") SL_PFR[k]=x; else if(nm=="STB") SL_STB[k]=x; else if(nm=="STR") SL_STR[k]=x; else if(nm=="WKD") SL_WKD[k]=x; else if(nm=="ROLE") SL_ROLE[k]=(int)x; else if(nm=="PT2") SL_PT2[k]=x; else if(nm=="PF2") SL_PF2[k]=x; else fprintf(stderr,"bad OVR %s\n",nm.c_str()); } }
}
{ const char*v; if((v=getenv("VF"))) InpGoldVolFilter=atof(v); }
