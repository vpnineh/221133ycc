#include <climits>
#include "drv.h"
#include EA_SRC
static const char* E(const char*k,const char*d){ const char*v=getenv(k); return v?v:d; }

// ---- hcal cost profiles (fn3 calibration vs FundedNext MT5 tester, 3 runs 2024-26) ----
// PROFILE=CAL : tester-matched costs. PROFILE=CALR : CAL + 0.5*spread slippage on market entries and stop exits.
// per-symbol override: SPEC_<SYM>="spread=..:comm=..:commpct=..:contract=..:se=..:sx=..:ss=..:swl=..:sws=..:ml=..:ms=.."
//   comm = USD per lot per side, se/sx/ss = entry / market-exit / stop slippage in price units,
//   swl/sws = USD per lot per night (SWAPM=3), ml/ms = annual % margin over/under the Fed funds rate (SWAPM=4)
struct CalRow{ const char*sym; double spread,comm,contract,swl,sws,ml,ms; };
static const CalRow CAL_T[]={
  {"XAUUSD",0.44,1.5,100,-58.0,-15.8,1.13,5.10},
  {"XAGUSD",0.037,1.5,5000,-100.0,50.8,14.5,-1.1},
  {"US30",3.9,0,10,-92.75,0,3.24,2.5},
  {"NAS100",1.70,0,10,-33.0,0,0.95,2.5},
  {"SPX500",0.65,0,10,-10.46,0,1.87,2.5},
  {"USDJPY",0.0097,1.5,100000,0,0,0,0},
  {"AUDJPY",0.0115,1.5,100000,0,0,0,0},
  {"EURGBP",0.00010,1.5,100000,0,0,0,0},
  {"GBPUSD",0.00012,1.5,100000,0,0,0,0},
};
static void apply_profile(SymState&y){
  SymSpec&p=y.sp; if(p.slip_e<0) p.slip_e=p.slip; if(p.slip_x<0) p.slip_x=p.slip; if(p.slip_s<0) p.slip_s=p.slip;
  string pr=E("PROFILE","");
  if(pr=="CAL"||pr=="CALR"){ for(auto&r:CAL_T) if(p.name==r.sym){ double cx=atof(E("COSTX","1")); p.spread=r.spread*cx; p.comm_side=r.comm*cx; p.comm_pct=0; p.contract=r.contract; p.swl_usd=r.swl; p.sws_usd=r.sws; p.mL=r.ml; p.mS=r.ms; p.slip=p.slip_e=p.slip_x=p.slip_s=0; }
    if(pr=="CALR"){ double k=atof(E("CALR_K","0.5")); p.slip_e=k*p.spread; p.slip_s=k*p.spread; p.slip_x=atof(E("CALR_KX","0"))*p.spread; } }
  const char*o=getenv(("SPEC_"+p.name).c_str()); if(o){ string t=o; size_t a=0; while(a<t.size()){ size_t b=t.find(':',a); if(b==string::npos) b=t.size(); string it=t.substr(a,b-a); a=b+1; size_t e=it.find('='); if(e==string::npos) continue;
      string k=it.substr(0,e); double v=atof(it.substr(e+1).c_str());
      if(k=="spread") p.spread=v; else if(k=="comm") p.comm_side=v; else if(k=="commpct") p.comm_pct=v; else if(k=="contract") p.contract=v; else if(k=="se") p.slip_e=v; else if(k=="sx") p.slip_x=v; else if(k=="ss") p.slip_s=v;
      else if(k=="swl") p.swl_usd=v; else if(k=="sws") p.sws_usd=v; else if(k=="ml") p.mL=v; else if(k=="ms") p.mS=v; else if(k=="vstep") p.vstep=v; else if(k=="vmin") p.vmin=v; else if(k=="tvm") p.tvm=v; else if(k=="calc") p.calc=(int)v; else if(k=="sk") p.sk=v; else if(k=="smin") p.smin=v; else if(k=="smax") p.smax=v; else if(k=="sr") p.sr=v; else if(k=="swq") p.swq=(int)v; else fprintf(stderr,"bad SPEC key %s\n",k.c_str()); } }
  if(getenv("SHOWSPEC")) fprintf(stderr,"spec %s spread=%g comm=%g commpct=%g contract=%g se=%g sx=%g ss=%g swl=%g sws=%g ml=%g ms=%g vstep=%g vmin=%g tvm=%g calc=%d sk=%g smin=%g smax=%g sr=%g swq=%d\n",p.name.c_str(),p.spread,p.comm_side,p.comm_pct,p.contract,p.slip_e,p.slip_x,p.slip_s,p.swl_usd,p.sws_usd,p.mL,p.mS,p.vstep,p.vmin,p.tvm,p.calc,p.sk,p.smin,p.smax,p.sr,p.swq);
}
int main(){
  long from=atol(E("FROM","1293840000")), to=atol(E("TO","1790000000")); string syms=E("SYMS","XAUUSD,XAGUSD,US30,NAS100,AUDJPY,USDJPY,EURGBP,GBPUSD"); string out=E("OUT","/tmp/run");
  double cm=atof(E("COSTX","1"));
  Sim sim; S=&sim; sim.hedging=true; sim.tester=true; sim.visual=atoi(E("VISUAL","0")); sim.server_offset_min=atoi(E("SRVOFF","-1")); sim.ea_on=false;
  std::vector<Stream> ss; { string cur; for(char c:syms+","){ if(c==','){ if(!cur.empty()){ Stream s; s.sym=cur; ss.push_back(s);} cur.clear(); } else cur+=c; } }
  for(auto&s:ss){ SymState y; y.sp.name=s.sym; y.sp.comm_side=0;
    if(s.sym=="XAGUSD"){ y.sp.digits=3; y.sp.point=0.001; y.sp.tick=0.001; y.sp.contract=5000; y.sp.base="XAG"; y.sp.spread=0.025*cm; y.sp.comm_pct=0.000016*cm; y.sp.swl=0.04; y.sp.sws=0.01; y.sp.slip=0.003*cm; }
    else if(s.sym=="AUDJPY"){ y.sp.digits=3; y.sp.point=0.001; y.sp.tick=0.001; y.sp.contract=100000; y.sp.base="AUD"; y.sp.quote="JPY"; y.sp.spread=0.016*cm; y.sp.comm_side=3.0*cm; y.sp.swl=0.01; y.sp.sws=0.01; y.sp.slip=0.003*cm; }
    else if(s.sym=="USDJPY"){ y.sp.digits=3; y.sp.point=0.001; y.sp.tick=0.001; y.sp.contract=100000; y.sp.base="USD"; y.sp.quote="JPY"; y.sp.spread=0.010*cm; y.sp.comm_side=3.5*cm; y.sp.swl=0.01; y.sp.sws=0.01; y.sp.slip=0.002*cm; }
    else if(s.sym=="EURGBP"){ y.sp.digits=5; y.sp.point=0.00001; y.sp.tick=0.00001; y.sp.contract=100000; y.sp.base="EUR"; y.sp.quote="GBP"; y.sp.spread=0.00010*cm; y.sp.comm_side=3.0*cm; y.sp.swl=0.015; y.sp.sws=0.015; y.sp.slip=0.00002*cm; }
    else if(s.sym=="GBPUSD"){ y.sp.digits=5; y.sp.point=0.00001; y.sp.tick=0.00001; y.sp.contract=100000; y.sp.base="GBP"; y.sp.quote="USD"; y.sp.spread=0.00008*cm; y.sp.comm_side=3.0*cm; }
    else if(s.sym.size()==6 && s.sym!="NAS100" && s.sym!="SPX500" && s.sym!="XAUUSD"){ // generic FX
      string b=s.sym.substr(0,3), q=s.sym.substr(3,3); bool jp=(q=="JPY"); y.sp.digits=jp?3:5; y.sp.point=jp?0.001:0.00001; y.sp.tick=y.sp.point; y.sp.contract=100000; y.sp.base=b; y.sp.quote=q;
      double pip=jp?0.01:0.0001; y.sp.spread=atof(E(("SPR_"+s.sym).c_str(),"0.9"))*pip*cm; y.sp.comm_side=3.5*cm; y.sp.swl=0.01; y.sp.sws=0.01; y.sp.slip=0.2*pip*cm; }
    else if(s.sym=="SPX500"){ y.sp.digits=2; y.sp.point=0.01; y.sp.tick=0.01; y.sp.contract=1; y.sp.base="US30"; y.sp.spread=0.6*cm; y.sp.swl=0.05; y.sp.sws=0.01; y.sp.slip=0.15*cm; y.sp.vmax=atof(E("VMAX_US30","100")); }
    else if(s.sym=="DAX"){ y.sp.digits=2; y.sp.point=0.01; y.sp.tick=0.01; y.sp.contract=1; y.sp.base="US30"; y.sp.quote="EUR"; y.sp.spread=1.5*cm; y.sp.swl=0.05; y.sp.sws=0.01; y.sp.slip=0.5*cm; y.sp.vmax=atof(E("VMAX_US30","100")); }
    else if(s.sym=="US30"||s.sym=="NAS100"){ y.sp.digits=2; y.sp.point=0.01; y.sp.tick=0.01; y.sp.contract=1; y.sp.base="US30"; y.sp.spread=(s.sym=="US30"?2.0:1.5)*cm; y.sp.swl=0.05; y.sp.sws=0.01; y.sp.slip=0.5*cm; y.sp.vmax=atof(E("VMAX_US30","100")); }
    else { y.sp.spread=0.25*cm; y.sp.comm_pct=0.000016*cm; y.sp.swl=0.04; y.sp.sws=0.01; y.sp.slip=0.05*cm; y.sp.vmax=atof(E("VMAX_XAU","100")); }
    apply_profile(y);
    sim.sy[s.sym]=y; s.v=load_m1((string(E("DATADIR","../data/"))+s.sym+"_m1.bin").c_str(),from-atol(E(("PREDAYS_"+s.sym).c_str(),E("PREDAYS","1500")))*86400L,to); }
  // broker sessions: no weekend bars in server time (EET brokers have none; the data has a few stray Saturday minutes)
  for(auto&s:ss){ std::vector<M1> k; k.reserve(s.v.size()); for(auto&m:s.v){ long st=m.t+srv_off(m.t)*60; int dw=(int)((st/86400+4)%7); if(dw!=0&&dw!=6) k.push_back(m); } s.v.swap(k); }
  sim.chart=ss[0].sym;
  // warm-up without the EA (builds H1/D1 history), then deposit + OnInit at `from`
  long t0=LONG_MAX; for(auto&s:ss) if(!s.v.empty()) t0=std::min(t0,s.v[0].t); sim.now=t0;
  while(true){ long tn=LONG_MAX; for(auto&s:ss) if(s.i<s.v.size()) tn=std::min(tn,s.v[s.i].t); if(tn==LONG_MAX||tn>=from) break; step_minute(ss); }
  if(getenv("LOAD")){ FILE*fl=fopen(getenv("LOAD"),"r"); if(!fl){ fprintf(stderr,"no snapshot\n"); return 2; } char ln[4096]; char a[64],b[512],c[512];
    while(fgets(ln,sizeof ln,fl)){ if(ln[0]=='B'){ double bal; unsigned long nt; sscanf(ln,"B %lf %lu",&bal,&nt); sim.balance=bal; sim.next_ticket=nt; }
      else if(ln[0]=='G'){ double v; if(sscanf(ln,"G %511s %lf",b,&v)==2) sim.gv[b]=v; }
      else if(ln[0]=='P'){ Pos p{}; long tm; unsigned long tk,id,mg; if(sscanf(ln,"P %lu %lu %63s %d %lf %lf %lf %lf %lu %ld %lf %511[^\n]",&tk,&id,a,&p.type,&p.vol,&p.open,&p.sl,&p.tp,&mg,&tm,&p.swap,c)>=11){ p.ticket=tk; p.id=id; p.sym=a; p.magic=mg; p.time=tm; p.comment=c; sim.pos.push_back(p);} }
      else if(ln[0]=='D'){ Deal x{}; long tm; unsigned long tk,od,ps,mg; if(sscanf(ln,"D %lu %lu %lu %ld %d %d %lu %63s %lf %lf %lf %lf %lf %lf %lf %511s",&tk,&od,&ps,&tm,&x.type,&x.entry,&mg,a,&x.price,&x.profit,&x.comm,&x.swap,&x.fee,&x.vol,&x.sl,b)>=15){ x.ticket=tk; x.order=od; x.pos=ps; x.time=tm; x.magic=mg; x.sym=(string(a)=="-"?"":a); x.why=(string(b)=="-"?"":b); for(auto&ch:x.why) if(ch=='~') ch=' '; sim.deals.push_back(x);} } }
    fclose(fl); }
  else sim.deposit(atof(E("BAL","100000")),sim.now);
  InpPreset=(ENUM_PRESET)atoi(E("PRESET","2")); InpShowPanel=atoi(E("PANEL","0"));
  #ifdef INPUTS_SRC
  #include INPUTS_SRC
  #endif
  sim.ea_on=true; g_mock_quiet=!atoi(E("LOUD","0"));
  int rc=OnInit(); if(rc!=INIT_SUCCEEDED){ printf("OnInit failed %d\n",rc); return 1; }
  int payd=atoi(E("PAYD","0")), pay1=atoi(E("PAY1","21")); double payb=atof(E("PAYB","0")), wd=0, bal0=sim.balance; long pay_next=sim.now/86400+pay1; long n_pay=0; bool wcol=payd>0||atoi(E("TPD","0"))>0;
  FILE*fe=fopen((out+"_eq.csv").c_str(),"w"); fprintf(fe,wcol?"day,bal,eq_close,eq_min,eq_max,wd\n":"day,bal,eq_close,eq_min,eq_max\n");
  long cur_day=-1; double emin=1e18,emax=-1e18,elast=0,blast=0; long nmin=0;
  double offp=atof(E("OFFP","0"))/1440.0; int offm=atoi(E("OFFM","30")); long off_until=0; unsigned long long ox=atoll(E("FSEED","99991"))*2654435761ULL+1; long n_out=0;
  long snap_at=atol(E("SNAP_AT","0"));
  while(true){
    if(snap_at>0 && sim.now>=snap_at){ long nu=LONG_MAX; for(auto&s:ss) if(s.i<s.v.size()) nu=std::min(nu,s.v[s.i].t);
      FILE*fs=fopen(E("SNAP_OUT","snap.txt"),"w"); fprintf(fs,"N %ld\nB %.10f %lu\n",nu,sim.balance,(unsigned long)sim.next_ticket);
      for(auto&kv:sim.gv) fprintf(fs,"G %s %.17g\n",kv.first.c_str(),kv.second);
      for(auto&p:sim.pos) fprintf(fs,"P %lu %lu %s %d %.2f %.10f %.10f %.10f %lu %ld %.10f %s\n",(unsigned long)p.ticket,(unsigned long)p.id,p.sym.c_str(),p.type,p.vol,p.open,p.sl,p.tp,(unsigned long)p.magic,(long)p.time,p.swap,p.comment.empty()?"-":p.comment.c_str());
      for(auto&x:sim.deals){ string w=x.why.empty()?"-":x.why; for(auto&ch:w) if(ch==' ') ch='~'; fprintf(fs,"D %lu %lu %lu %ld %d %d %lu %s %.10f %.10f %.10f %.10f %.10f %.2f %.10f %s\n",(unsigned long)x.ticket,(unsigned long)x.order,(unsigned long)x.pos,(long)x.time,x.type,x.entry,(unsigned long)x.magic,x.sym.empty()?"-":x.sym.c_str(),x.price,x.profit,x.comm,x.swap,x.fee,x.vol,x.sl,w.c_str()); }
      fclose(fs); break; }
    bool out_now=false; if(offp>0){ if(sim.now<off_until) out_now=true; else { ox^=ox<<13; ox^=ox>>7; ox^=ox<<17; if((ox>>11)*(1.0/9007199254740992.0)<offp){ off_until=sim.now+offm*60; out_now=true; n_out++; } } }
    if(out_now) sim.ea_on=false; bool more=step_minute(ss); if(out_now) sim.ea_on=true; if(!more) break; nmin++; double eq=sim.equity(); long day=sim.now/86400; if(day!=cur_day){ if(cur_day>=0){ if(wcol) fprintf(fe,"%ld,%.2f,%.2f,%.2f,%.2f,%.2f\n",cur_day,blast,elast,emin,emax,wd+g_mock_wd); else fprintf(fe,"%ld,%.2f,%.2f,%.2f,%.2f\n",cur_day,blast,elast,emin,emax); }
      if(payd>0 && cur_day>=0 && day>=pay_next){   // payout: withdraw the closed profit above the kept buffer (never more than the equity allows)
        double keep=bal0*(1.0+payb/100.0), amt=std::min(sim.balance,eq)-keep;
        if(amt>=bal0*0.01){ Deal d{}; d.time=sim.now; d.type=DEAL_TYPE_BALANCE; d.profit=-amt; d.entry=DEAL_ENTRY_IN; sim.add_deal(d); sim.balance-=amt; sim.pending_tx.push_back(sim.deals.back().ticket); wd+=amt; n_pay++; pay_next=day+payd; eq=sim.equity(); } }
      cur_day=day; emin=emax=eq; } emin=std::min(emin,eq); emax=std::max(emax,eq); elast=eq; blast=sim.balance; }
  if(wcol){ fprintf(fe,"%ld,%.2f,%.2f,%.2f,%.2f,%.2f\n",cur_day,blast,elast,emin,emax,wd+g_mock_wd); printf("{\"payouts\":%ld,\"withdrawn\":%.2f}\n",n_pay+g_mock_nwd,wd+g_mock_wd); } else fprintf(fe,"%ld,%.2f,%.2f,%.2f,%.2f\n",cur_day,blast,elast,emin,emax); fclose(fe);
  FILE*fd=fopen((out+"_deals.csv").c_str(),"w"); fprintf(fd,"ticket,pos,time,sym,magic,type,entry,vol,price,profit,comm,swap,why,sl\n");
  for(auto&d:sim.deals) fprintf(fd,"%lu,%lu,%ld,%s,%lu,%d,%d,%.2f,%.5f,%.2f,%.2f,%.2f,%s,%.5f\n",d.ticket,d.pos,(long)d.time,d.sym.c_str(),d.magic,d.type,d.entry,d.vol,d.price,d.profit,d.comm,d.swap,d.why.c_str(),d.sl); fclose(fd);
  if(getenv("REJP")||getenv("OFFP")) printf("{\"fi_req\":%ld,\"fi_rej\":%ld,\"outages\":%ld}\n",g_fi_req,g_fi_rej,n_out);
  printf("{\"closed_rej\":%ld,\"minutes\":%ld,\"end_balance\":%.2f,\"halt\":%d,\"target\":%d,\"days\":%d,\"open_pos\":%zu,\"preset\":%d,\"tz\":\"%d/%d/%d\",\"init\":%.2f,\"mc\":{\"objset\":%ld,\"objcreate\":%ld,\"redraw\":%ld}}\n",sim.n_closed_rej,nmin,sim.balance,g_halt,(int)g_target_hit,g_trading_days,sim.pos.size(),g_preset,(int)g_tz_ok,g_tz_s,g_tz_w,g_init_bal,g_mc.objset,g_mc.objcreate,g_mc.redraw);
  #ifdef QCNT
  printf("{\"q_stop\":%ld,\"q_stop_sent\":%ld,\"q_close\":%ld,\"q_close_sent\":%ld,\"q_entry\":%ld,\"q_entry_sent\":%ld,\"q_entry_dropped\":%ld,\"q_partial\":%ld,\"skips\":%d}\n",g_q_cnt[0],g_q_cnt[1],g_q_cnt[2],g_q_cnt[3],g_q_cnt[4],g_q_cnt[5],g_q_cnt[6],g_q_cnt[7],g_nskip);
  #endif
  if(getenv("LOUD")){ for(int k=0;k<NSLV;k++) printf("sl%d use=%d status=%s\n",k,(int)g_use[k],g_ss[k].status.c_str()); for(int r=0;r<NROLE;r++) printf("role%d ok=%d dfok=%d atr=%.3f n_h1=%ld\n",r,(int)g_sym_ok[r],(int)g_df[r].ok,g_df[r].atr,(long)g_last_h1[r]); }
  OnDeinit(0); return 0; }
