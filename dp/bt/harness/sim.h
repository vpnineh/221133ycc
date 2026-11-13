// Multi-symbol mock broker (hedging/netting, SL/TP on ticks, commission per side, USD account).
#pragma once
#include "mt5.h"
McCount g_mc;
struct SymSpec{ string name="XAUUSD"; int digits=2; double point=0.01,tick=0.01,contract=100,vmin=0.01,vmax=100,vstep=0.01; int stops=0; string base="XAU",quote="USD"; double comm_side=3.5; double comm_pct=0; double swl=0, sws=0; double spread=0.30; double slip=0; double slip_e=-1,slip_x=-1,slip_s=-1; double swl_usd=0,sws_usd=0,mL=0,mS=0; int tmode=4; int fill=3; string desc, path; bool custom=false; long first=0; double tvm=1; int calc=0;
  double sk=0,smin=0,smax=1e9,sr=0; int swq=0; };   // rx: sk = spread per unit of 20-day mean D1 range (SPRM), smin/smax clamps, sr = rollover spread multiplier, swq=1: swl/sws in quote ccy per lot (MT5 'points' swaps)
struct Bar{ datetime t; double o,h,l,c; };
struct TfSeries{ int sec=60; std::vector<Bar> closed; Bar cur{0,0,0,0,0}; bool has=false; };
struct SymState{ SymSpec sp; double bid=0,ask=0,prev_bid=0; bool have_tick=false; datetime tick_t=0; double spr_now=-1, rng=0, bo=0; bool open_tick=false; std::map<int,TfSeries> tf;
  SymState(){ tf[1].sec=60; tf[5].sec=300; tf[15].sec=900; tf[30].sec=1800; tf[16385].sec=3600; tf[16388].sec=14400; tf[16408].sec=86400; } };
struct Pos{ ulong ticket,id; string sym; int type; double vol,open,sl,tp; ulong magic; datetime time; double swap=0; string comment; };
struct Deal{ ulong ticket,order,pos; datetime time; int type,entry; ulong magic; string sym; double price,profit,comm,swap,fee,vol; string why; double sl=0; };
struct Obj{ std::map<string,double> d; std::map<string,string> s; };
struct Sim{
  std::map<string,SymState> sy; string chart="XAUUSD";
  double balance=10000; bool hedging=true; llong login=900001; string server="Sim-Server"; int trade_mode=ACCOUNT_TRADE_MODE_DEMO;
  bool tester=true, visual=false; int server_offset_min=0; double leverage=100;
  bool chart_overlay=false, removed=false; int connected=1;
  datetime now=0; std::vector<Pos> pos; std::vector<Deal> deals; ulong next_ticket=1000;
  std::vector<int> hsel; std::map<string,double> gv; std::map<int,string> files; std::map<string,int> open_names; std::map<int,std::vector<string>> filelines; int next_h=1;
  std::map<string,Obj> objs; int cur_pos=-1; bool reject_next=false; uint reject_rc=0; bool block_closes=false; bool trade_allowed=true; bool ea_on=true;
  std::vector<ulong> pending_tx;
  // h132: MCLOSE_<SYM>="HH:MM-HH:MM[,HH:MM-HH:MM]" server-time windows in which trade requests on SYM get 10018 (market closed), quotes still run
  bool mkt_closed(const string&sym){ std::map<string,std::vector<std::pair<int,int>>> W; { const char*e=getenv(("MCLOSE_"+sym).c_str()); if(e){ string t=e; size_t a=0; while(a<t.size()){ size_t b=t.find(',',a); if(b==string::npos) b=t.size(); int h1,m1,h2,m2; if(sscanf(t.substr(a,b-a).c_str(),"%d:%d-%d:%d",&h1,&m1,&h2,&m2)==4) W[sym].push_back({h1*60+m1,h2*60+m2}); a=b+1; } } }
    auto it=W.find(sym); if(it==W.end()) return false; int md=(int)((now%86400)/60); for(auto&w:it->second) if(md>=w.first&&md<w.second) return true; return false; }
  long n_closed_rej=0;
  SymState* G(const string&n){ auto it=sy.find(n); return it==sy.end()?nullptr:&it->second; }
  double q2usd(const SymSpec&s,double price){ const string&qq=s.quote; if(qq=="USD"||qq.empty()) return 1.0; if(s.base=="USD") return 1.0/price;
    if(qq=="JPY"){ auto it=sy.find("USDJPY"); if(getenv("OLDQ2USD")) return 1.0/price; return (it!=sy.end()&&it->second.bid>0)?1.0/it->second.bid:1.0/140.0; }
    { auto it=sy.find(qq+"USD"); if(it!=sy.end()&&it->second.bid>0) return it->second.bid; }
    { auto it=sy.find("USD"+qq); if(it!=sy.end()&&it->second.bid>0) return 1.0/it->second.bid; }
    if(qq=="GBP") return 1.30; if(qq=="EUR") return 1.10; if(qq=="CAD") return 0.75; if(qq=="CHF") return 1.10; if(qq=="AUD") return 0.70; if(qq=="NZD") return 0.62; return 1.0; }
  double profit_of(const string&sym,int type,double vol,double po,double pc){ SymState*s=G(sym); if(!s) return 0; double d=(type==0? pc-po : po-pc); return d*vol*s->sp.contract*q2usd(s->sp,pc); }
  double cur_px(const Pos&p){ SymState*s=G(p.sym); return p.type==0?s->bid:s->ask; }
  double floating(){ double f=0; for(auto&p:pos) f+=profit_of(p.sym,p.type,p.vol,p.open,cur_px(p))+p.swap; return f; }
  double equity(){ return balance+floating(); }
  double margin(){ double m=0; for(auto&p:pos){ SymState*s=G(p.sym); m+=p.vol*s->sp.contract*p.open/leverage*q2usd(s->sp,p.open);} return m; }
  static double ffr(long t){ static const double F[]={0.18,0.10,0.14,0.11,0.09,0.13,0.40,1.00,1.83,2.16,0.38,0.08,1.68,5.02,5.14,4.20,3.70}; int y=1970+(int)(t/31556952); int i=y-2010; if(i<0)i=0; if(i>16)i=16; return F[i]; }
  void rollover(int dow_prev){ static int m=getenv("SWAPM")?atoi(getenv("SWAPM")):0; static double mk=getenv("SWAPMK")?atof(getenv("SWAPMK")):2.5; static double sx=getenv("SWAPX")?atof(getenv("SWAPX")):1.0;
    for(auto&p:pos){ SymState*s=G(p.sym); double r=(p.type==0?s->sp.swl:s->sp.sws); int mult=(dow_prev==3?3:1);
      if(m==3){ double usd=(p.type==0?s->sp.swl_usd:s->sp.sws_usd); if(s->sp.swq) usd*=q2usd(s->sp,s->bid); if(s->sp.base=="US30") mult=(dow_prev==5?3:1); p.swap+=usd*p.vol*mult*sx; continue; }
      if(m==4){ bool cfd4=(s->sp.base=="XAU"||s->sp.base=="XAG"||s->sp.base=="US30"); if(!cfd4) continue; double f=ffr(now)/100.0; double rr=(p.type==0? f+s->sp.mL/100.0 : s->sp.mS/100.0-f); if(s->sp.base=="US30") mult=(dow_prev==5?3:1);
        double px=(p.type==0?s->bid:s->ask); p.swap-=rr*sx*px*s->sp.contract*p.vol/365.0*mult*q2usd(s->sp,px); continue; }
      if(m){ bool cfd=(s->sp.base=="XAU"||s->sp.base=="XAG"||s->sp.base=="US30"); double f=ffr(now)/100.0, k=mk/100.0;
        if(cfd){ r=(p.type==0? f+k : std::max(0.0025,k-f)); if(s->sp.base=="US30") mult=(dow_prev==5?3:1); }
        r*=sx; }
      double px=(p.type==0?s->bid:s->ask); p.swap-=r*px*s->sp.contract*p.vol/365.0*mult*q2usd(s->sp,px); } }
  void add_deal(Deal d){ d.ticket=next_ticket++; deals.push_back(d); }
  void deposit(double amt,datetime t){ Deal d{}; d.time=t; d.type=DEAL_TYPE_BALANCE; d.profit=amt; d.entry=DEAL_ENTRY_IN; add_deal(d); balance=amt; }
  void begin_minute(SymState&S_,datetime st,double open){ for(auto&kv:S_.tf){ TfSeries&s=kv.second; datetime bt=st/s.sec*s.sec; if(!s.has){ s.cur={bt,open,open,open,open}; s.has=true; } else if(s.cur.t!=bt){ s.closed.push_back(s.cur); s.cur={bt,open,open,open,open}; } } }
  void on_m1_close(SymState&S_,const Bar&b){ for(auto&kv:S_.tf){ TfSeries&s=kv.second; s.cur.h=std::max(s.cur.h,b.h); s.cur.l=std::min(s.cur.l,b.l); s.cur.c=b.c; } }
  bool bar_at(const string&sym,int tfid,int shift,Bar&out){ SymState*S_=G(sym); if(!S_) return false; TfSeries&s=S_->tf[tfid]; int n=(int)s.closed.size(); if(shift==0){ if(!s.has) return false; out=s.cur; return true;} int idx=n-shift; if(idx<0||idx>=n) return false; out=s.closed[idx]; return true; }
  void close_part(int idx,double vol,double price,const char*why){
    Pos&p=pos[idx]; SymState*s=G(p.sym); double pr=profit_of(p.sym,p.type,vol,p.open,price); double comm=-(s->sp.comm_side+s->sp.comm_pct*s->sp.contract*price)*vol;
    Deal d{}; d.order=next_ticket; d.pos=p.id; d.time=now; d.type=(p.type==0?DEAL_TYPE_SELL:DEAL_TYPE_BUY); d.entry=DEAL_ENTRY_OUT; d.magic=p.magic; d.sym=p.sym; d.price=price; d.profit=pr; d.comm=comm; d.swap=0; d.fee=0; d.vol=vol; d.why=why;
    balance+=pr+comm+p.swap; d.swap=p.swap; add_deal(d); pending_tx.push_back(deals.back().ticket);
    if(vol<p.vol-1e-9){ balance-=p.swap; deals.back().swap=0; } if(vol>=p.vol-1e-9) pos.erase(pos.begin()+idx); else p.vol=NormalizeDouble(p.vol-vol,2); }
  bool open_pos(const string&sym,int type,double vol,double sl,double tp,ulong magic,const string&cm,double&fillp){
    SymState*s=G(sym); if(!s||!s->have_tick) return false;
    double px=(type==0? s->ask+s->sp.slip_e : s->bid-s->sp.slip_e);
    if(!hedging){ for(size_t i=0;i<pos.size();i++) if(pos[i].sym==sym) return false; }
    Pos p{}; p.ticket=next_ticket++; p.id=p.ticket; p.sym=sym; p.type=type; p.vol=vol; p.open=px; p.sl=sl; p.tp=tp; p.magic=magic; p.time=now; p.comment=cm; pos.push_back(p); fillp=px;
    Deal d{}; d.order=next_ticket; d.pos=p.id; d.time=now; d.type=type==0?DEAL_TYPE_BUY:DEAL_TYPE_SELL; d.entry=DEAL_ENTRY_IN; d.magic=magic; d.sym=sym; d.price=px; d.profit=0; d.comm=-(s->sp.comm_side+s->sp.comm_pct*s->sp.contract*px)*vol; d.vol=vol; d.why=cm; d.sl=sl;
    balance+=d.comm; add_deal(d); pending_tx.push_back(deals.back().ticket); return true; }
  // SL/TP fill: gap through the level -> fill at the tick price; otherwise at the level (no slippage on TP)
  // rx GAPFILL=1 (live realism; the MT5 tester does NOT do this - all 1420 tester SL fills in the 5 logs are exactly at the SL):
  //   when the bar OPEN is already beyond SL/TP (weekend/overnight/data gap) the order fills at the open price (worse for SL, better for TP).
  // rx SLIPV=k: extra stop slippage = k x range of the triggering M1 bar (fast-market slippage), on top of slip_s.
  void check_stops(const string&sym){ static int gf=getenv("GAPFILL")?atoi(getenv("GAPFILL")):0; static double kv=getenv("SLIPV")?atof(getenv("SLIPV")):0;
    SymState*s=G(sym); double sv=s->sp.slip_s+kv*s->rng; for(int i=(int)pos.size()-1;i>=0;i--){ Pos&p=pos[i]; if(p.sym!=sym) continue;
      if(p.type==0){ if(p.sl>0&&s->bid<=p.sl){ double f=(s->prev_bid>p.sl?p.sl-sv:s->bid); if(gf&&s->open_tick) f=std::min(s->bid,p.sl)-sv; close_part(i,p.vol,f,"SL"); }
        else if(p.tp>0&&s->bid>=p.tp){ double f=p.tp; if(gf&&s->open_tick) f=std::max(s->bid,p.tp); close_part(i,p.vol,f,"TP"); } }
      else { if(p.sl>0&&s->ask>=p.sl){ double f=((s->prev_bid+(s->ask-s->bid))<p.sl?p.sl+sv:s->ask); if(gf&&s->open_tick) f=std::max(s->ask,p.sl)+sv; close_part(i,p.vol,f,"SL"); }
        else if(p.tp>0&&s->ask<=p.tp){ double f=p.tp; if(gf&&s->open_tick) f=std::min(s->ask,p.tp); close_part(i,p.vol,f,"TP"); } } } }
};
Sim* S=nullptr;
bool g_mock_quiet=true;
static SymState* GS(const string&n){ return S->G(n); }
datetime TimeCurrent(){return S->now;} datetime TimeTradeServer(){return S->now;} datetime TimeGMT(){return S->now-(datetime)S->server_offset_min*60;}
uint GetTickCount(){ return (uint)(S->now*1000); }
llong MQLInfoInteger(int p){ if(p==MQL_TESTER) return S->tester; if(p==MQL_VISUAL_MODE) return S->visual; if(p==MQL_TRADE_ALLOWED) return S->trade_allowed; return 0; }
llong TerminalInfoInteger(int p){ if(p==TERMINAL_TRADE_ALLOWED) return S->trade_allowed; if(p==TERMINAL_CONNECTED) return S->connected; return 0; }
llong AccountInfoInteger(int p){ switch(p){case ACCOUNT_LOGIN:return S->login;case ACCOUNT_TRADE_MODE:return S->trade_mode;case ACCOUNT_MARGIN_MODE:return S->hedging?ACCOUNT_MARGIN_MODE_RETAIL_HEDGING:ACCOUNT_MARGIN_MODE_RETAIL_NETTING;case ACCOUNT_TRADE_ALLOWED:case ACCOUNT_TRADE_EXPERT:return S->trade_allowed;} return 0; }
double AccountInfoDouble(int p){ switch(p){case ACCOUNT_BALANCE:return S->balance;case ACCOUNT_EQUITY:return S->equity();case ACCOUNT_MARGIN_FREE:return S->equity()-S->margin();} return 0; }
string AccountInfoString(int p){ if(p==ACCOUNT_CURRENCY) return "USD"; if(p==ACCOUNT_SERVER) return S->server; return ""; }
llong SymbolInfoInteger(const string&n,int p){ SymState*s=GS(n); if(!s) return 0; switch(p){case SYMBOL_DIGITS:return s->sp.digits;case SYMBOL_TRADE_STOPS_LEVEL:return s->sp.stops;case SYMBOL_TRADE_MODE:return s->sp.tmode;case SYMBOL_FILLING_MODE:return s->sp.fill;case SYMBOL_SPREAD:return (llong)std::round((s->ask-s->bid)/s->sp.point);case SYMBOL_SWAP_MODE:return 1;case SYMBOL_TIME:return (llong)S->now;case SYMBOL_CUSTOM:return s->sp.custom;case SYMBOL_TRADE_CALC_MODE:return s->sp.calc;} return 0; }
double SymbolInfoDouble(const string&n,int p){ SymState*s=GS(n); if(!s) return 0; switch(p){case SYMBOL_TRADE_CONTRACT_SIZE:return s->sp.contract;case SYMBOL_TRADE_TICK_SIZE:return s->sp.tick;case SYMBOL_VOLUME_MIN:return s->sp.vmin;case SYMBOL_VOLUME_MAX:return s->sp.vmax;case SYMBOL_VOLUME_STEP:return s->sp.vstep;case SYMBOL_BID:return s->bid;case SYMBOL_ASK:return s->ask;case SYMBOL_POINT:return s->sp.point;case SYMBOL_TRADE_TICK_VALUE:case SYMBOL_TRADE_TICK_VALUE_LOSS:return s->sp.tick*s->sp.contract*S->q2usd(s->sp,s->bid)*s->sp.tvm;} return 0; }
string SymbolInfoString(const string&n,int p){ SymState*s=GS(n); if(!s) return ""; if(p==SYMBOL_CURRENCY_BASE) return s->sp.base; if(p==SYMBOL_CURRENCY_PROFIT) return s->sp.quote.empty()?string("USD"):s->sp.quote; if(p==SYMBOL_DESCRIPTION) return s->sp.desc; if(p==SYMBOL_PATH) return s->sp.path; return ""; }
bool SymbolInfoTick(const string&n,MqlTick&t){ SymState*s=GS(n); if(!s||!s->have_tick) return false; static bool tn=getenv("TICKNOW")!=nullptr; t.time=(s->tick_t&&!tn)?s->tick_t:S->now;   /* TICKNOW=1: every quote counts as fresh (diagnostic) */   // h133: per-symbol last tick time
  t.bid=s->bid; t.ask=s->ask; t.time_msc=S->now*1000; return true; }
bool SymbolSelect(const string&n,bool){ return GS(n)!=nullptr; }
int SymbolsTotal(bool){ return (int)S->sy.size(); }
string SymbolName(int i,bool){ int k=0; for(auto&kv:S->sy){ if(k++==i) return kv.first; } return ""; }
bool OrderCalcProfit(int type,const string&n,double vol,double po,double pc,double&pr){ if(!GS(n)) return false; pr=S->profit_of(n,type,vol,po,pc); return true; }
bool OrderCalcMargin(int,const string&n,double vol,double price,double&m){ SymState*s=GS(n); if(!s) return false; m=vol*s->sp.contract*price/S->leverage*S->q2usd(s->sp,price); return true; }
bool OrderCheck(const MqlTradeRequest&r,MqlTradeCheckResult&c){ c.retcode=10009; c.comment="ok"; SymState*s=GS(r.symbol); if(!s){c.retcode=TRADE_RETCODE_REJECT;c.comment="no symbol";return false;} if(r.volume<s->sp.vmin-1e-9||r.volume>s->sp.vmax+1e-9){c.retcode=TRADE_RETCODE_INVALID_VOLUME;c.comment="Invalid volume";return false;} double m;OrderCalcMargin(r.type,r.symbol,r.volume,r.price,m); if(m>AccountInfoDouble(ACCOUNT_MARGIN_FREE)){c.retcode=TRADE_RETCODE_NO_MONEY;c.comment="No money";return false;} return true; }
int PositionsTotal(){return (int)S->pos.size();}
ulong PositionGetTicket(int i){ if(i<0||i>=(int)S->pos.size()) return 0; S->cur_pos=i; return S->pos[i].ticket; }
bool PositionSelect(const string&s){ for(size_t i=0;i<S->pos.size();i++) if(S->pos[i].sym==s){S->cur_pos=(int)i;return true;} return false; }
bool PositionSelectByTicket(ulong t){ for(size_t i=0;i<S->pos.size();i++) if(S->pos[i].ticket==t){S->cur_pos=(int)i;return true;} return false; }
static Pos* CP(){ return (S->cur_pos>=0&&S->cur_pos<(int)S->pos.size())?&S->pos[S->cur_pos]:nullptr; }
string PositionGetString(int p){ Pos*q=CP(); if(!q) return ""; return p==POSITION_COMMENT?q->comment:q->sym; }
llong PositionGetInteger(int p){ Pos*q=CP(); if(!q) return 0; switch(p){case POSITION_MAGIC:return q->magic;case POSITION_TYPE:return q->type;case POSITION_TIME:return q->time;case POSITION_IDENTIFIER:return q->id;} return 0; }
double PositionGetDouble(int p){ Pos*q=CP(); if(!q) return 0; switch(p){case POSITION_VOLUME:return q->vol;case POSITION_PRICE_OPEN:return q->open;case POSITION_SL:return q->sl;case POSITION_TP:return q->tp;case POSITION_SWAP:return q->swap;case POSITION_PRICE_CURRENT:return S->cur_px(*q);case POSITION_PROFIT:return S->profit_of(q->sym,q->type,q->vol,q->open,S->cur_px(*q));} return 0; }
int OrdersTotal(){return 0;} ulong OrderGetTicket(int){return 0;} llong OrderGetInteger(int){return 0;} string OrderGetString(int){return "";}
bool HistorySelect(datetime a,datetime b){ S->hsel.clear(); size_t lo=0,hi=S->deals.size(); while(lo<hi){ size_t m=(lo+hi)/2; if(S->deals[m].time<a) lo=m+1; else hi=m; } for(size_t i=lo;i<S->deals.size()&&S->deals[i].time<=b;i++) S->hsel.push_back((int)i); return true; }
bool HistorySelectByPosition(ulong id){ S->hsel.clear(); for(size_t i=0;i<S->deals.size();i++) if(S->deals[i].pos==id) S->hsel.push_back((int)i); return true; }
int HistoryDealsTotal(){return (int)S->hsel.size();}
ulong HistoryDealGetTicket(int i){ if(i<0||i>=(int)S->hsel.size()) return 0; return S->deals[S->hsel[i]].ticket; }
static Deal* FindDeal(ulong t){ size_t lo=0,hi=S->deals.size(); while(lo<hi){ size_t m=(lo+hi)/2; if(S->deals[m].ticket<t) lo=m+1; else hi=m; } return (lo<S->deals.size()&&S->deals[lo].ticket==t)?&S->deals[lo]:nullptr; }
bool HistoryDealSelect(ulong t){ return FindDeal(t)!=nullptr; }
double g_mock_wd=0; long g_mock_nwd=0;
bool TesterWithdrawal(double amt){ if(amt<=0||amt>S->balance) return false; Deal d{}; d.time=S->now; d.type=DEAL_TYPE_BALANCE; d.profit=-amt; d.entry=DEAL_ENTRY_IN; S->add_deal(d); S->balance-=amt; if(!getenv("TWNOTX")) S->pending_tx.push_back(S->deals.back().ticket); g_mock_wd+=amt; g_mock_nwd++; return true; }
llong HistoryDealGetInteger(ulong t,int p){ Deal*d=FindDeal(t); if(!d) return 0; switch(p){case DEAL_TYPE:return d->type;case DEAL_ENTRY:return d->entry;case DEAL_MAGIC:return d->magic;case DEAL_TIME:return d->time;case DEAL_POSITION_ID:return d->pos;} return 0; }
double HistoryDealGetDouble(ulong t,int p){ Deal*d=FindDeal(t); if(!d) return 0; switch(p){case DEAL_PRICE:return d->price;case DEAL_PROFIT:return d->profit;case DEAL_COMMISSION:return d->comm;case DEAL_SWAP:return d->swap;case DEAL_FEE:return d->fee;case DEAL_VOLUME:return d->vol;} return 0; }
string HistoryDealGetString(ulong t,int p){ Deal*d=FindDeal(t); if(!d) return ""; return p==DEAL_SYMBOL?d->sym:""; }
bool GlobalVariableGet(const string&n,double&v){ auto it=S->gv.find(n); if(it==S->gv.end()) return false; v=it->second; return true; }
datetime GlobalVariableSet(const string&n,double v){ S->gv[n]=v; return S->now; } bool GlobalVariablesFlush(){return true;}
int GlobalVariablesDeleteAll(const string&p){ if(getenv("LOAD")) return 0; int c=0; for(auto it=S->gv.begin();it!=S->gv.end();){ if(it->first.compare(0,p.size(),p)==0){it=S->gv.erase(it);c++;} else ++it; } return c; }
int FileOpen(const string&n,int fl,int,int){ if((fl&FILE_WRITE)&&!(fl&FILE_SHARE_WRITE)&&S->open_names.count(n)) return INVALID_HANDLE; int h=S->next_h++; S->files[h]=n; if(fl&FILE_WRITE) S->open_names[n]=h; return h; }
void FileClose(int h){ auto it=S->files.find(h); if(it==S->files.end()) return; auto o=S->open_names.find(it->second); if(o!=S->open_names.end()&&o->second==h) S->open_names.erase(o); S->files.erase(it); }
void FileFlush(int){} llong FileSize(int h){ return (llong)S->filelines[h].size(); } bool FileSeek(int,llong,int){return true;}
template<class...A> uint FileWrite(int h,A...a){ std::vector<FmtArg> v={FA(a)...}; string o; for(auto&x:v){ if(!o.empty())o+=","; o+= x.k==2?x.s:(x.k==1?DoubleToString(x.d,5):std::to_string(x.i)); } S->filelines[h].push_back(o); return 1; }
void EventSetTimer(int){} bool EventSetMillisecondTimer(int){return true;} void EventKillTimer(){} void ChartRedraw(llong){ g_mc.redraw++; }
llong ChartID(){ return 1234567; }
datetime iTime(const string&n,int tf,int sh){ Bar b; return S->bar_at(n,tf,sh,b)?b.t:0; }
double iHigh(const string&n,int tf,int sh){ Bar b; return S->bar_at(n,tf,sh,b)?b.h:0; }
double iLow(const string&n,int tf,int sh){ Bar b; return S->bar_at(n,tf,sh,b)?b.l:0; }
int iBarShift(const string&n,int tf,datetime t,bool exact){ SymState*ss=GS(n); if(!ss) return -1; TfSeries&s=ss->tf[tf]; int c=(int)s.closed.size(); if(s.has&&s.cur.t==t) return 0; int lo=0,hi=c-1; while(lo<=hi){ int m=(lo+hi)/2; if(s.closed[m].t==t) return c-m; if(s.closed[m].t<t) lo=m+1; else hi=m-1; } if(exact) return -1; return hi>=0? c-hi:-1; }
int iHighest(const string&n,int tf,int,int cnt,int start){ int best=-1; double bv=-1e300; for(int i=start;i<start+cnt;i++){ Bar b; if(!S->bar_at(n,tf,i,b)) break; if(b.h>bv){bv=b.h;best=i;} } return best; }
int iLowest(const string&n,int tf,int,int cnt,int start){ int best=-1; double bv=1e300; for(int i=start;i<start+cnt;i++){ Bar b; if(!S->bar_at(n,tf,i,b)) break; if(b.l<bv){bv=b.l;best=i;} } return best; }
static MqlRates ToR(const Bar&b){ MqlRates r; r.time=b.t; r.open=b.o; r.high=b.h; r.low=b.l; r.close=b.c; return r; }
int CopyRates(const string&n,int tfid,int start,int count,MArr<MqlRates>&out){ out.v.clear(); SymState*ss=GS(n); if(!ss) return -1; TfSeries&s=ss->tf[tfid]; int c=(int)s.closed.size();
  std::vector<Bar> all(s.closed); if(s.has) all.push_back(s.cur); int tot=(int)all.size(); int newest=tot-1-start; if(newest<0) return 0; int oldest=std::max(0,newest-count+1);
  for(int i=oldest;i<=newest;i++) out.v.push_back(ToR(all[i])); (void)c; return (int)out.v.size(); }
int CopyRates(const string&n,int tfid,datetime a,datetime b,MArr<MqlRates>&out){ out.v.clear(); SymState*ss=GS(n); if(!ss) return -1; TfSeries&s=ss->tf[tfid];
  auto it=std::lower_bound(s.closed.begin(),s.closed.end(),a,[](const Bar&x,datetime v){return x.t<v;}); for(;it!=s.closed.end()&&it->t<=b;++it) out.v.push_back(ToR(*it)); if(s.has&&s.cur.t>=a&&s.cur.t<=b) out.v.push_back(ToR(s.cur)); return (int)out.v.size(); }
bool ObjectCreate(int,const string&n,int type,int,datetime,double,datetime,double){ g_mc.objcreate++; S->objs[n].d["type"]=type; return true; }
int ObjectFind(int,const string&n){ return S->objs.count(n)?0:-1; }
bool ObjectMove(int,const string&n,int,datetime t,double p){ g_mc.objset++; S->objs[n].d["t0"]=t; S->objs[n].d["p0"]=p; return true; }
bool ObjectSetInteger(int,const string&n,int p,llong v){ g_mc.objset++; S->objs[n].d["i"+std::to_string(p)]=v; return true; }
bool ObjectSetInteger(int,const string&n,int p,int m,llong v){ g_mc.objset++; S->objs[n].d["i"+std::to_string(p)+"_"+std::to_string(m)]=v; return true; }
bool ObjectSetDouble(int,const string&n,int p,double v){ g_mc.objset++; S->objs[n].d["i"+std::to_string(p)]=v; return true; }
bool ObjectSetDouble(int,const string&n,int p,int m,double v){ g_mc.objset++; S->objs[n].d["i"+std::to_string(p)+"_"+std::to_string(m)]=v; return true; }
bool ObjectSetString(int,const string&n,int p,const string&v){ g_mc.objset++; S->objs[n].s[std::to_string(p)]=v; return true; }
llong ObjectGetInteger(llong,const string&n,int p,int){ auto it=S->objs.find(n); if(it==S->objs.end()) return 0; auto j=it->second.d.find("i"+std::to_string(p)); return j==it->second.d.end()?0:(llong)j->second; }
int ObjectsDeleteAll(int,const string&pf){ int c=0; for(auto it=S->objs.begin();it!=S->objs.end();){ if(it->first.compare(0,pf.size(),pf)==0){it=S->objs.erase(it);c++;} else ++it; } g_mc.objdel+=c; return c; }
int ObjectsTotal(llong,int,int){ return (int)S->objs.size(); }
string ObjectName(llong,int i,int,int){ int k=0; for(auto&kv:S->objs){ if(k++==i) return kv.first; } return ""; }
bool ObjectDelete(llong,const string&n){ g_mc.objdel++; return S->objs.erase(n)>0; }
bool TimeToStruct(datetime t,MqlDateTime&d){ time_t tt=t; struct tm m; gmtime_r(&tt,&m); d.year=m.tm_year+1900; d.mon=m.tm_mon+1; d.day=m.tm_mday; d.hour=m.tm_hour; d.min=m.tm_min; d.sec=m.tm_sec; d.day_of_week=m.tm_wday; d.day_of_year=m.tm_yday; return true; }
datetime StringToTime(const string&s){ int y=0,mo=0,d=0,h=0,mi=0; char c1,c2; if(sscanf(s.c_str(),"%d%c%d%c%d %d:%d",&y,&c1,&mo,&c2,&d,&h,&mi)<5) return 0; return (datetime)MqlMakeTime(y,mo,d,h,mi); }
void CTrade::SetDeviationInPoints(ulong d){dev=d;} void CTrade::SetAsyncMode(bool){} void CTrade::SetMarginMode(){} bool CTrade::SetTypeFillingBySymbol(const string&){return true;} void CTrade::SetExpertMagicNumber(ulong m){magic=m;}
// fi: REJP = probability that a trade request (open / close / partial / modify) is refused with a random retcode
// (10004 requote, 10006 rejected, 10018 market closed, 10021 no prices); FSEED = seed. Counts in g_fi_rej.
long g_fi_rej=0, g_fi_req=0;
static uint fi_fault(){ static double p=getenv("REJP")?atof(getenv("REJP")):0; if(p<=0) return 0; static unsigned long long x=getenv("FSEED")?atoll(getenv("FSEED")):88172645463325252ULL;
  g_fi_req++; x^=x<<13; x^=x>>7; x^=x<<17; double u=(x>>11)*(1.0/9007199254740992.0); if(u>=p) return 0; g_fi_rej++; static const uint rc[4]={10004,10006,10018,10021}; return rc[(x>>3)%4]; }
static bool DoTrade(CTrade&t,const string&sym,int type,double v,double sl,double tp,const string&cm){ t.res=MqlTradeResult(); if(S->mkt_closed(sym)){ S->n_closed_rej++; t.res.retcode=TRADE_RETCODE_MARKET_CLOSED; return false; } { uint f=fi_fault(); if(f){ t.res.retcode=f; return false; } } if(S->reject_next){S->reject_next=false;t.res.retcode=S->reject_rc?S->reject_rc:TRADE_RETCODE_REJECT;return false;} double f; if(!S->open_pos(sym,type,v,sl,tp,t.magic,cm,f)){t.res.retcode=TRADE_RETCODE_REJECT;return false;} t.res.retcode=TRADE_RETCODE_DONE; t.res.price=f; return true; }
bool CTrade::Buy(double v,const string&s,double,double sl,double tp,const string&c){return DoTrade(*this,s,0,v,sl,tp,c);} bool CTrade::Sell(double v,const string&s,double,double sl,double tp,const string&c){return DoTrade(*this,s,1,v,sl,tp,c);}
bool CTrade::PositionClose(ulong tk){ res=MqlTradeResult(); { uint f=fi_fault(); if(f){ res.retcode=f; return false; } } for(auto&q:S->pos) if(q.ticket==tk&&S->mkt_closed(q.sym)){ S->n_closed_rej++; res.retcode=TRADE_RETCODE_MARKET_CLOSED; return false; } if(S->block_closes){res.retcode=TRADE_RETCODE_REJECT;return false;} for(size_t i=0;i<S->pos.size();i++) if(S->pos[i].ticket==tk){ Pos&p=S->pos[i]; double px=S->cur_px(p); SymState*s=GS(p.sym); px+= (p.type==0?-s->sp.slip_x:s->sp.slip_x); S->close_part((int)i,p.vol,px,"close"); res.retcode=TRADE_RETCODE_DONE; res.price=px; return true;} res.retcode=TRADE_RETCODE_REJECT; return false; }
bool CTrade::PositionClosePartial(ulong tk,double v){ res=MqlTradeResult(); { uint f=fi_fault(); if(f){ res.retcode=f; return false; } } for(auto&q:S->pos) if(q.ticket==tk&&S->mkt_closed(q.sym)){ S->n_closed_rej++; res.retcode=TRADE_RETCODE_MARKET_CLOSED; return false; } for(size_t i=0;i<S->pos.size();i++) if(S->pos[i].ticket==tk){ Pos&p=S->pos[i]; SymState*s=GS(p.sym); double vol=std::floor(v/s->sp.vstep+1e-9)*s->sp.vstep; if(vol<s->sp.vmin-1e-9||vol>=p.vol-1e-9){ res.retcode=TRADE_RETCODE_INVALID_VOLUME; return false; } double px=S->cur_px(p)+(p.type==0?-s->sp.slip_x:s->sp.slip_x); S->close_part((int)i,NormalizeDouble(vol,2),px,"partial"); res.retcode=TRADE_RETCODE_DONE; return true; } res.retcode=TRADE_RETCODE_REJECT; return false; }
bool CTrade::PositionModify(ulong tk,double sl,double tp){ res=MqlTradeResult(); { uint f=fi_fault(); if(f){ res.retcode=f; return false; } } for(auto&q:S->pos) if(q.ticket==tk&&S->mkt_closed(q.sym)){ S->n_closed_rej++; res.retcode=TRADE_RETCODE_MARKET_CLOSED; return false; } for(auto&p:S->pos) if(p.ticket==tk){ SymState*s=GS(p.sym); double b=s->bid,a=s->ask; if(p.type==0&&((sl>0&&sl>=b)||(tp>0&&tp<=b))){res.retcode=TRADE_RETCODE_REJECT;return false;} if(p.type==1&&((sl>0&&sl<=a)||(tp>0&&tp>=a))){res.retcode=TRADE_RETCODE_REJECT;return false;} p.sl=sl; p.tp=tp; res.retcode=TRADE_RETCODE_DONE; return true; } res.retcode=TRADE_RETCODE_REJECT; return false; }
double CTrade::ResultPrice(){ return res.price; }
bool CTrade::OrderDelete(ulong){res.retcode=TRADE_RETCODE_DONE;return true;} uint CTrade::ResultRetcode(){return res.retcode;} string CTrade::ResultRetcodeDescription(){ return res.retcode==TRADE_RETCODE_DONE?"done":res.retcode==TRADE_RETCODE_MARKET_CLOSED?"market closed":"rejected"; }
string Symbol(){ return S->chart; }
bool GlobalVariableDel(const string&n){ return S->gv.erase(n)>0; }

llong ChartFirst(){ return S->chart_overlay ? ChartID() : -1; } llong ChartNext(llong){ return -1; } string ChartSymbol(llong){ return S->chart; } int ChartPeriod(llong){ return PERIOD_H1; }
llong ChartGetInteger(llong,int p,int){ return p==CHART_WIDTH_IN_PIXELS?1400:0; }
datetime TimeLocal(){ return TimeCurrent(); } void ExpertRemove(){ S->removed=true; }
llong ChartOpen(const string&,int){ return 0; } bool ChartSetInteger(llong,int,int,llong){ return true; }

llong SeriesInfoInteger(const string&n,int,int){ SymState*s=GS(n); return s?s->sp.first:0; }

// ---- v15.1 harness additions: Wilder ATR/ADX handles, no session table (open 24h), swap props
struct IndH { int kind; string sym; int tf; int per; };
static std::vector<IndH> g_ind;
inline int iADX(const string&n,int tf,int per){ g_ind.push_back({1,n,tf,per}); return (int)g_ind.size()-1; }
inline int iATR(const string&n,int tf,int per){ g_ind.push_back({0,n,tf,per}); return (int)g_ind.size()-1; }
inline void IndicatorRelease(int){}
// h133: trade sessions = the complement of the MCLOSE_<SYM> windows (same every day); no MCLOSE -> no session table (false)
inline bool SymbolInfoSessionTrade(const string&n,ENUM_DAY_OF_WEEK,uint idx,datetime&f,datetime&t){
  if(getenv(("MCLOSE_"+n+"_HIDE").c_str())) return false;   // unit test: broker without a session table
  static std::map<string,std::vector<std::pair<int,int>>> C; const char*e=getenv(("MCLOSE_"+n).c_str()); string key=n+"|"+(e?e:""); auto it=C.find(key);
  if(it==C.end()){ std::vector<std::pair<int,int>> W,O;
    if(e){ string s=e; size_t a=0; while(a<s.size()){ size_t b=s.find(',',a); if(b==string::npos) b=s.size(); int h1,m1,h2,m2; if(sscanf(s.substr(a,b-a).c_str(),"%d:%d-%d:%d",&h1,&m1,&h2,&m2)==4) W.push_back({h1*60+m1,h2*60+m2}); a=b+1; }
      std::sort(W.begin(),W.end()); int c=0; for(auto&w:W){ if(w.first>c) O.push_back({c,w.first}); c=std::max(c,w.second); } if(c<1440) O.push_back({c,1440}); }
    it=C.emplace(key,O).first; }
  if(idx>=it->second.size()) return false; f=(datetime)it->second[idx].first*60; t=(datetime)it->second[idx].second*60; return true; }
static double wilder_at(const IndH&h,MArr<MqlRates>&r,int last){
  int n=last+1; if(n<h.per*3) return 0;
  double atr=0,pdm=0,mdm=0,trs=0,adx=0; int cnt=0; int P=h.per;
  for(int i=1;i<n;i++){ double hi=r.v[i].high,lo=r.v[i].low,pc=r.v[i-1].close; double tr=std::max(hi-lo,std::max(std::fabs(hi-pc),std::fabs(lo-pc)));
    double up=hi-r.v[i-1].high,dn=r.v[i-1].low-lo; double pd=(up>dn&&up>0)?up:0,md=(dn>up&&dn>0)?dn:0;
    if(i<=P){ atr+=tr/P; trs+=tr; pdm+=pd; mdm+=md; } else { atr=(atr*(P-1)+tr)/P; trs=trs-trs/P+tr; pdm=pdm-pdm/P+pd; mdm=mdm-mdm/P+md; }
    if(i>=P){ double pdi=trs>0?100*pdm/trs:0,mdi=trs>0?100*mdm/trs:0; double dx=(pdi+mdi>0)?100*std::fabs(pdi-mdi)/(pdi+mdi):0; cnt++; if(cnt<=P) adx+=dx/P; else adx=(adx*(P-1)+dx)/P; } }
  return h.kind==0?atr:adx; }
inline int CopyBuffer(int h,int,int start,int count,MArr<double>&out){ out.v.clear(); if(h<0||h>=(int)g_ind.size()) return -1; IndH&H=g_ind[h]; MArr<MqlRates> r; int n=CopyRates(H.sym,H.tf,start,H.per*12+50,r); if(n<H.per*3) return 0; out.v.push_back(wilder_at(H,r,n-1)); return 1; }
inline int CopyBuffer(int h,int,datetime t,int count,MArr<double>&out){ out.v.clear(); if(h<0||h>=(int)g_ind.size()) return -1; IndH&H=g_ind[h]; MArr<MqlRates> r; datetime a=t-(datetime)((long)PeriodSeconds(H.tf)*(H.per*12+50)); int n=CopyRates(H.sym,H.tf,a,t,r); if(n<H.per*3) return 0; out.v.push_back(wilder_at(H,r,n-1)); return 1; }
