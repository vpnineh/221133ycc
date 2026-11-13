// Multi-symbol driver: merges M1 streams, 4 ticks per bar (o, extreme1, extreme2, c), OnTick only on chart-symbol ticks.
#pragma once
#include "sim.h"
#include <array>
struct M1{ long t; double o,h,l,c; };
inline long last_sun(int y,int m){ long dn=MqlMakeTime(m==12?y+1:y,m==12?1:m+1,1,0,0)/86400-1; int w=(int)((dn+4)%7); return dn-w; }
// hcal DSHIFT=1: the M1 files are stamped in UK local time (GMT/BST); convert to true UTC so that the harness clock equals the broker server clock
inline bool uk_bst_local(long t){ time_t tt=t; struct tm g; gmtime_r(&tt,&g); int y=g.tm_year+1900; long a=last_sun(y,3)*86400+1*3600, b=last_sun(y,10)*86400+2*3600; return t>=a&&t<b; }
inline std::vector<M1> load_m1(const char*path,long from,long to){ std::vector<M1> v; FILE*f=fopen(path,"rb"); if(!f){fprintf(stderr,"no data %s\n",path);exit(2);} static int ds=getenv("DSHIFT")?atoi(getenv("DSHIFT")):0; M1 r;
  while(fread(&r,sizeof(M1),1,f)==1){ if(ds&&uk_bst_local(r.t)) r.t-=3600; if(r.t>=from&&r.t<to) v.push_back(r);} fclose(f);
  if(ds){ std::stable_sort(v.begin(),v.end(),[](const M1&a,const M1&b){return a.t<b.t;}); std::vector<M1> u; u.reserve(v.size()); for(auto&m:v){ if(!u.empty()&&u.back().t==m.t){ u.back().h=std::max(u.back().h,m.h); u.back().l=std::min(u.back().l,m.l); u.back().c=m.c; } else u.push_back(m);} v.swap(u); }
  return v; }
int OnInit(); void OnTick(); void OnTimer(); void OnDeinit(const int);
void OnTradeTransaction(const MqlTradeTransaction&,const MqlTradeRequest&,const MqlTradeResult&);
void OnChartEvent(const int,const long&,const double&,const string&);
inline void flush_tx(){ if(!S->ea_on){ S->pending_tx.clear(); return; } while(!S->pending_tx.empty()){ auto v=S->pending_tx; S->pending_tx.clear(); for(ulong d:v){ MqlTradeTransaction t; t.type=TRADE_TRANSACTION_DEAL_ADD; t.deal=d; MqlTradeRequest rq; MqlTradeResult rs; OnTradeTransaction(t,rq,rs);} } }
inline long nth_sun(int y,int m,int n){ long d0=MqlMakeTime(y,m,1,0,0)/86400; int w=(int)((d0+4)%7); long first=d0+((7-w)%7); return first+7*(n-1); }
inline int srv_off(long t){ if(S->server_offset_min>=0) return S->server_offset_min; time_t tt=t; struct tm g; gmtime_r(&tt,&g); int y=g.tm_year+1900;
  long a=nth_sun(y,3,2)*86400+7*3600, b=nth_sun(y,11,1)*86400+6*3600; return (t>=a&&t<b)?180:120; }
struct Stream{ string sym; std::vector<M1> v; size_t i=0; long last=0; bool gapped=false; };
inline void set_tick(SymState&s,datetime t,double bid){ s.prev_bid=s.have_tick?s.bid:bid; s.bid=bid; s.ask=bid+(s.spr_now>=0?s.spr_now:s.sp.spread); s.have_tick=true; s.tick_t=t; S->now=t; }
// ---- rx variable spread model (one spread per M1 bar, as the MT5 tester uses the bar's spread in "1 minute OHLC" mode) ----
// SPRM=1 : spread = clamp(sk x mean D1 range of the last 20 closed server days, smin, smax) for symbols with SPEC key sk>0 (others: constant spread)
// SPRT=file : broker spread history, CSV "SYM,YYYY-MM-DD,spread" plus optional hour-of-day multipliers "SYM,Hhh,mult" (step function, carried forward; before the first row the first value). Overrides SPRM.
// ROLL=1 : FX rollover widening: spread x sr (SPEC key; default ROLLM, 8) for server time ROLLA..ROLLB minutes after midnight (default 23:59-00:20 -> -1..20)
struct SprRow{ long t; double v; };
inline std::map<string,std::array<double,24>>& spr_hm(){ static std::map<string,std::array<double,24>> H; return H; }   // SPRT 'SYM,Hhh,mult' rows (default 1)
inline std::map<string,std::vector<SprRow>>& spr_tab(){ static std::map<string,std::vector<SprRow>> T; static bool ld=false; if(!ld){ ld=true; const char*f=getenv("SPRT"); if(f){ FILE*fp=fopen(f,"r"); if(!fp){ fprintf(stderr,"no SPRT %s\n",f); exit(2);} char sy[32]; int y,m,d; double v; char line[256];
      while(fgets(line,sizeof line,fp)){ int hh; if(sscanf(line,"%31[^,],H%d,%lf",sy,&hh,&v)==3&&hh>=0&&hh<24){ spr_hm()[sy][hh]=v; continue; }
        if(sscanf(line,"%31[^,],%d-%d-%d,%lf",sy,&y,&m,&d,&v)==5) T[sy].push_back({(long)MqlMakeTime(y,m,d,0,0),v}); } fclose(fp); for(auto&kv:T) std::sort(kv.second.begin(),kv.second.end(),[](const SprRow&a,const SprRow&b){return a.t<b.t;}); } } return T; }
inline bool is_fx(const SymSpec&p){ return p.base!="XAU"&&p.base!="XAG"&&p.base!="US30"&&p.name.size()==6; }
inline double spread_at(SymState&y,datetime st){
  static int m=getenv("SPRM")?atoi(getenv("SPRM")):0; static int roll=getenv("ROLL")?atoi(getenv("ROLL")):0; static double rm=getenv("ROLLM")?atof(getenv("ROLLM")):8.0;
  static int ra=getenv("ROLLA")?atoi(getenv("ROLLA")):-1, rb=getenv("ROLLB")?atoi(getenv("ROLLB")):20;
  double v=y.sp.spread;
  if(m==1&&y.sp.sk>0){ TfSeries&d=y.tf[16408]; int n=(int)d.closed.size(); if(n>=5){ int k=std::min(20,n); double a=0; for(int i=n-k;i<n;i++) a+=d.closed[i].h-d.closed[i].l; v=std::min(y.sp.smax,std::max(y.sp.smin,y.sp.sk*a/k)); } }
  auto&T=spr_tab(); if(!T.empty()){ auto it=T.find(y.sp.name); if(it!=T.end()&&!it->second.empty()){ auto&r=it->second; size_t lo=0,hi=r.size(); while(lo<hi){ size_t mm=(lo+hi)/2; if(r[mm].t<=st) lo=mm+1; else hi=mm; } v=r[lo?lo-1:0].v; auto h=spr_hm().find(y.sp.name); if(h!=spr_hm().end()){ double hv=h->second[(st%86400)/3600]; if(hv>0) v*=hv; } } }
  if(roll&&(is_fx(y.sp)||y.sp.sr>0)){ int md=(int)((st%86400)/60); if(md>=1440+ra) md-=1440; if(md>=ra&&md<rb) v*=(y.sp.sr>0?y.sp.sr:rm); }
  return v; }
// one merged minute; returns false when all streams are exhausted
inline bool step_minute(std::vector<Stream>&ss){
  long t=LONG_MAX; for(auto&s:ss) if(s.i<s.v.size()) t=std::min(t,s.v[s.i].t); if(t==LONG_MAX) return false;
  datetime st=t+(datetime)srv_off(t)*60; { static long lastd=-1; long dd=st/86400; if(lastd>=0&&dd!=lastd){ S->rollover((int)((lastd+4)%7)); } lastd=dd; } std::vector<Stream*> act; for(auto&s:ss) if(s.i<s.v.size()&&s.v[s.i].t==t) act.push_back(&s);
  // data holes (index history missing 2020-05..2023-12): a symbol without data for 5+ days is flattened at its last price
  for(auto&s:ss){ if(s.last>0 && !s.gapped && t-s.last>5L*86400){ for(int q=(int)S->pos.size()-1;q>=0;q--) if(S->pos[q].sym==s.sym){ SymState*y=S->G(s.sym); S->close_part(q,S->pos[q].vol,S->pos[q].type==0?y->bid:y->ask,"datagap"); } s.gapped=true; } }
  for(auto&s:ss){ if(s.last>0 && t-s.last>3L*86400){ SymState*y=S->G(s.sym); if(y) y->sp.tmode=0; } }   // stale feed (data ended): no new orders
  for(auto*a:act){ a->last=t; a->gapped=false; SymState*y=S->G(a->sym); if(y) y->sp.tmode=4; }
  for(auto*a:act){ SymState&y=*S->G(a->sym); S->begin_minute(y,st,a->v[a->i].o); }
  static const int offA[4]={0,15,30,59}, offB[4]={0,20,40,59};   // rx TICKSEC=1: MT5 "1 minute OHLC" tick seconds (journal: SL triggers at :00/:20/:40/:59)
  static const int*off=(getenv("TICKSEC")&&atoi(getenv("TICKSEC")))?offB:offA;
  static int vspr=getenv("SPRM")||getenv("SPRT")||getenv("ROLL");
  for(auto*a:act){ SymState&y=*S->G(a->sym); const M1&m=a->v[a->i]; y.rng=m.h-m.l; y.bo=m.o; if(vspr) y.spr_now=spread_at(y,st); }
  static int timer4=getenv("TIMER4")?atoi(getenv("TIMER4")):0;
  for(int k=0;k<4;k++){ for(auto*a:act){ const M1&m=a->v[a->i]; bool up=m.c>=m.o; double px= k==0?m.o : k==3?m.c : (k==1?(up?m.l:m.h):(up?m.h:m.l));
      SymState&y=*S->G(a->sym); y.open_tick=(k==0); set_tick(y,st+off[k],px); S->check_stops(a->sym); flush_tx(); if(S->ea_on&&a->sym==S->chart){ OnTick(); flush_tx(); } }
    if(timer4&&S->ea_on&&k<3){ S->now=st+off[k]; OnTimer(); flush_tx(); } }   // hcal: timer also fires on the open tick (MT5 1 s timer), like the tester
  for(auto*a:act){ const M1&m=a->v[a->i]; Bar b{st,m.o,m.h,m.l,m.c}; S->on_m1_close(*S->G(a->sym),b); a->i++; }
  if(S->ea_on){ OnTimer(); flush_tx(); } return true; }
