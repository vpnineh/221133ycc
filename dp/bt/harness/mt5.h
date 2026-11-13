// Minimal MQL5 runtime + broker simulator used to compile and execute the EA source in g++.
#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cfloat>
#include <algorithm>
#include <functional>
#include <cstring>
#include <ctime>
#include <type_traits>
using std::string;
typedef long llong;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef long datetime;
struct McCount{ long objset=0, objcreate=0, redraw=0, objdel=0; }; extern McCount g_mc;
typedef unsigned int color;
#define INVALID_HANDLE (-1)
#define EMPTY_VALUE DBL_MAX
#define clrNONE 0
#define MQL_DT(y,m,d,hh,mm) ((datetime)MqlMakeTime(y,m,d,hh,mm))
#define MQL_RGB(r,g,b) ((color)(((b)<<16)|((g)<<8)|(r)))
inline llong MqlMakeTime(int y,int m,int d,int hh,int mm){
  llong yy=y,mo=m; if(mo<=2){yy-=1;mo+=12;} llong era=(yy>=0?yy:yy-399)/400, yoe=yy-era*400, doy=(153*(mo-3)+2)/5+d-1, doe=yoe*365+yoe/4-yoe/100+doy;
  return (era*146097+doe-719468)*86400+hh*3600+mm*60; }
enum { clrDodgerBlue=1,clrMediumOrchid,clrDarkOrange,clrSilver,clrTomato,clrMediumSeaGreen,clrIndianRed,clrDarkGray };
// ---- dynamic array
template<class T> struct MArr { std::vector<T> v; bool series=false;
  T& operator[](int i){ if(i<0||i>=(int)v.size()){ fprintf(stderr,"ARRAY OUT OF RANGE %d/%zu\n",i,v.size()); abort(); } return v[i]; }
  const T& operator[](int i) const { if(i<0||i>=(int)v.size()){ fprintf(stderr,"ARRAY OUT OF RANGE %d/%zu\n",i,v.size()); abort(); } return v[i]; } };
template<class T> int ArrayResize(MArr<T>&a,int n){ a.v.resize(n); return n; }
template<class T> int ArraySize(const MArr<T>&a){ return (int)a.v.size(); }
template<class T> bool ArraySetAsSeries(MArr<T>&a,bool s){ a.series=s; return true; }
template<class T> T* GetPointer(T&x){ return &x; }
// ---- structs
struct MqlRates { datetime time=0; double open=0,high=0,low=0,close=0; llong tick_volume=0; int spread=0; llong real_volume=0; };
struct MqlDateTime { int year=0,mon=0,day=0,hour=0,min=0,sec=0,day_of_week=0,day_of_year=0; };
struct MqlTick { datetime time=0; double bid=0,ask=0,last=0; ulong volume=0; llong time_msc=0; uint flags=0; double volume_real=0; };
struct MqlTradeRequest { int action=0; ulong magic=0; ulong order=0; string symbol; double volume=0,price=0,stoplimit=0,sl=0,tp=0; ulong deviation=0; int type=0; int type_filling=0; int type_time=0; datetime expiration=0; string comment; ulong position=0; ulong position_by=0; };
struct MqlTradeResult { uint retcode=0; ulong deal=0,order=0; double volume=0,price=0,bid=0,ask=0; string comment; uint request_id=0; };
struct MqlTradeCheckResult { uint retcode=0; double balance=0,equity=0,profit=0,margin=0,margin_free=0,margin_level=0; string comment; };
struct MqlTradeTransaction { ulong deal=0,order=0; string symbol; int type=0; int order_type=0; int order_state=0; int deal_type=0; double price=0,volume=0; ulong position=0; };
struct MqlCalendarValue { ulong id=0, event_id=0; datetime time=0; };
struct MqlCalendarEvent { ulong id=0; int importance=0; string event_code, name; };
enum { CALENDAR_IMPORTANCE_NONE=0, CALENDAR_IMPORTANCE_LOW, CALENDAR_IMPORTANCE_MODERATE, CALENDAR_IMPORTANCE_HIGH };
// ---- enums / consts
enum ENUM_TIMEFRAMES { PERIOD_CURRENT=0,PERIOD_M1=1,PERIOD_M5=5,PERIOD_M15=15,PERIOD_M30=30,PERIOD_H1=16385,PERIOD_H4=16388,PERIOD_D1=16408 };
enum ENUM_ORDER_TYPE { ORDER_TYPE_BUY=0,ORDER_TYPE_SELL=1 };
enum ENUM_POSITION_TYPE { POSITION_TYPE_BUY=0,POSITION_TYPE_SELL=1 };
enum { POSITION_SYMBOL,POSITION_MAGIC,POSITION_TYPE,POSITION_VOLUME,POSITION_PRICE_OPEN,POSITION_SL,POSITION_TP,POSITION_PROFIT,POSITION_SWAP,POSITION_TIME,POSITION_IDENTIFIER,POSITION_PRICE_CURRENT,POSITION_COMMENT };
enum { ORDER_MAGIC,ORDER_SYMBOL };
enum { DEAL_TYPE=100,DEAL_ENTRY,DEAL_MAGIC,DEAL_TIME,DEAL_SYMBOL,DEAL_PRICE,DEAL_PROFIT,DEAL_COMMISSION,DEAL_SWAP,DEAL_FEE,DEAL_VOLUME,DEAL_POSITION_ID };
enum { DEAL_TYPE_BUY=0,DEAL_TYPE_SELL=1,DEAL_TYPE_BALANCE=2,DEAL_TYPE_CREDIT=3,DEAL_TYPE_BONUS=6 };
enum { DEAL_ENTRY_IN=0,DEAL_ENTRY_OUT=1,DEAL_ENTRY_INOUT=2,DEAL_ENTRY_OUT_BY=3 };
enum { ACCOUNT_BALANCE=200,ACCOUNT_EQUITY,ACCOUNT_MARGIN_FREE,ACCOUNT_LOGIN,ACCOUNT_TRADE_MODE,ACCOUNT_MARGIN_MODE,ACCOUNT_TRADE_ALLOWED,ACCOUNT_TRADE_EXPERT,ACCOUNT_CURRENCY,ACCOUNT_SERVER };
enum { ACCOUNT_TRADE_MODE_DEMO=0,ACCOUNT_TRADE_MODE_CONTEST=1,ACCOUNT_TRADE_MODE_REAL=2 };
enum { ACCOUNT_MARGIN_MODE_RETAIL_NETTING=0,ACCOUNT_MARGIN_MODE_EXCHANGE=1,ACCOUNT_MARGIN_MODE_RETAIL_HEDGING=2 };
enum { SYMBOL_SWAP_LONG=400,SYMBOL_SWAP_SHORT,SYMBOL_SWAP_MODE,SYMBOL_TIME,SYMBOL_SWAP_MODE_POINTS=1,SYMBOL_SWAP_MODE_CURRENCY_SYMBOL=2,SYMBOL_SWAP_MODE_CURRENCY_MARGIN=3,SYMBOL_SWAP_MODE_CURRENCY_DEPOSIT=4 };
enum ENUM_DAY_OF_WEEK { SUNDAY=0,MONDAY,TUESDAY,WEDNESDAY,THURSDAY,FRIDAY,SATURDAY };
enum { SYMBOL_TRADE_CONTRACT_SIZE=300,SYMBOL_TRADE_TICK_SIZE,SYMBOL_VOLUME_MIN,SYMBOL_VOLUME_MAX,SYMBOL_VOLUME_STEP,SYMBOL_TRADE_STOPS_LEVEL,SYMBOL_DIGITS,SYMBOL_TRADE_MODE,SYMBOL_FILLING_MODE,SYMBOL_CURRENCY_BASE,SYMBOL_BID,SYMBOL_ASK,SYMBOL_TRADE_TICK_VALUE,SYMBOL_POINT,SYMBOL_SPREAD,SYMBOL_DESCRIPTION,SYMBOL_PATH,SYMBOL_CUSTOM };
enum { SYMBOL_TRADE_TICK_VALUE_LOSS=340,SYMBOL_CURRENCY_PROFIT,SYMBOL_TRADE_CALC_MODE };   // h132
enum { SYMBOL_CALC_MODE_FOREX=0,SYMBOL_CALC_MODE_FUTURES=1,SYMBOL_CALC_MODE_CFD=2,SYMBOL_CALC_MODE_CFDINDEX=3,SYMBOL_CALC_MODE_CFDLEVERAGE=4,SYMBOL_CALC_MODE_FOREX_NO_LEVERAGE=5 };
enum { SERIES_TERMINAL_FIRSTDATE=1, SERIES_FIRSTDATE=2 };
enum { SYMBOL_TRADE_MODE_DISABLED=0,SYMBOL_TRADE_MODE_LONGONLY=1,SYMBOL_TRADE_MODE_SHORTONLY=2,SYMBOL_TRADE_MODE_CLOSEONLY=3,SYMBOL_TRADE_MODE_FULL=4 };
enum { SYMBOL_FILLING_FOK=1,SYMBOL_FILLING_IOC=2 };
enum { ORDER_FILLING_FOK=0,ORDER_FILLING_IOC=1,ORDER_FILLING_RETURN=2 };
enum { TRADE_ACTION_DEAL=1 };
enum { TERMINAL_TRADE_ALLOWED=400,MQL_TRADE_ALLOWED,MQL_TESTER,MQL_VISUAL_MODE,MQL_OPTIMIZATION,TERMINAL_CONNECTED };
enum { TRADE_TRANSACTION_DEAL_ADD=6 };
enum { TRADE_RETCODE_DONE=10009,TRADE_RETCODE_DONE_PARTIAL=10010,TRADE_RETCODE_PLACED=10008,TRADE_RETCODE_TIMEOUT=10012,TRADE_RETCODE_NO_CHANGES=10025,TRADE_RETCODE_REJECT=10006,TRADE_RETCODE_NO_MONEY=10019,TRADE_RETCODE_INVALID_VOLUME=10014,TRADE_RETCODE_MARKET_CLOSED=10018 };
enum { INIT_SUCCEEDED=0,INIT_FAILED=1,INIT_PARAMETERS_INCORRECT=2 };
enum { FILE_READ=1,FILE_WRITE=2,FILE_BIN=4,FILE_CSV=8,FILE_ANSI=16,FILE_COMMON=32,FILE_SHARE_READ=64,FILE_SHARE_WRITE=128 };
#undef SEEK_END
enum { SEEK_END=2 };
enum { CP_UTF8=65001 };
enum { CHARTEVENT_OBJECT_CLICK=1,CHARTEVENT_CHART_CHANGE=9 };
enum { OBJ_TREND=1,OBJ_TEXT,OBJ_ARROW,OBJ_VLINE,OBJ_RECTANGLE_LABEL,OBJ_LABEL,OBJ_BUTTON };
typedef int ENUM_OBJECT;
inline void ResetLastError(){}
inline int GetLastError(){ return 0; }
enum { OBJPROP_RAY_RIGHT=1,OBJPROP_SELECTABLE,OBJPROP_BACK,OBJPROP_TIME,OBJPROP_PRICE,OBJPROP_COLOR,OBJPROP_STYLE,OBJPROP_WIDTH,OBJPROP_TEXT,OBJPROP_FONT,OBJPROP_FONTSIZE,OBJPROP_ANCHOR,OBJPROP_ARROWCODE,OBJPROP_CORNER,OBJPROP_XDISTANCE,OBJPROP_YDISTANCE,OBJPROP_XSIZE,OBJPROP_YSIZE,OBJPROP_BGCOLOR,OBJPROP_STATE,OBJPROP_BORDER_TYPE,OBJPROP_ZORDER,OBJPROP_HIDDEN,OBJPROP_READONLY,OBJPROP_ALIGN,OBJPROP_TOOLTIP,OBJPROP_BORDER_COLOR };
enum { STYLE_SOLID=0,STYLE_DASH,STYLE_DOT,STYLE_DASHDOT };
enum { TIME_DATE=1,TIME_MINUTES=2,TIME_SECONDS=4 };
enum { CHART_WIDTH_IN_PIXELS=106 };
enum { ANCHOR_LEFT_LOWER=1,ANCHOR_LEFT_UPPER,ANCHOR_RIGHT_UPPER,ANCHOR_CENTER,ANCHOR_UPPER };
enum { CORNER_LEFT_UPPER=0 };
enum { BORDER_FLAT=1 };
enum { MODE_HIGH=2,MODE_LOW=1 };
// ---- math
inline double MathAbs(double x){return std::fabs(x);} inline int MathAbs(int x){return x<0?-x:x;} inline llong MathAbs(llong x){return x<0?-x:x;}
template<class A,class B> inline typename std::common_type<A,B>::type MathMax(A a,B b){return a>b?a:b;}
template<class A,class B> inline typename std::common_type<A,B>::type MathMin(A a,B b){return a<b?a:b;}
inline double MathFloor(double x){return std::floor(x);} inline double MathCeil(double x){return std::ceil(x);}
inline double MathRound(double x){return std::round(x);} inline double MathPow(double a,double b){return std::pow(a,b);}
inline double MathSqrt(double x){return std::sqrt(x);} inline double MathLog(double x){return std::log(x);} inline double MathExp(double x){return std::exp(x);}
inline bool MathIsValidNumber(double x){return std::isfinite(x);}
inline double NormalizeDouble(double x,int d){double p=std::pow(10.0,d);return std::round(x*p)/p;}
// ---- strings
inline int StringLen(const string&s){return (int)s.size();}
inline int StringFind(const string&s,const string&f,int st=0){size_t p=s.find(f,st);return p==string::npos?-1:(int)p;}
inline string StringSubstr(const string&s,int a,int n=-1){if(a>=(int)s.size())return "";return n<0?s.substr(a):s.substr(a,n);}
inline int StringTrimLeft(string&s){size_t i=s.find_first_not_of(" \t\r\n");s=(i==string::npos)?"":s.substr(i);return (int)s.size();}
inline int StringTrimRight(string&s){size_t i=s.find_last_not_of(" \t\r\n");s=(i==string::npos)?"":s.substr(0,i+1);return (int)s.size();}
inline bool StringToUpper(string&s){for(auto&c:s)c=toupper(c);return true;}
inline int StringGetCharacter(const string&s,int i){return (i>=0&&i<(int)s.size())?(unsigned char)s[i]:0;}
inline llong StringToInteger(const string&s){return atoll(s.c_str());}
inline int StringSplit(const string&s,int sep,MArr<string>&out){out.v.clear();string cur;for(char c:s){if((unsigned char)c==sep){out.v.push_back(cur);cur.clear();}else cur+=c;}out.v.push_back(cur);return (int)out.v.size();}
inline string IntegerToString(llong v,int=0,int=' '){return std::to_string(v);}
inline string DoubleToString(double v,int d=8){char b[64];snprintf(b,64,"%.*f",d,v);return b;}
inline string TimeToString(datetime t,int=0){time_t tt=t;struct tm m;gmtime_r(&tt,&m);char b[64];strftime(b,64,"%Y.%m.%d %H:%M",&m);return b;}
struct FmtArg{ int k; llong i; double d; string s; };
inline FmtArg FA(const string&v){return {2,0,0,v};} inline FmtArg FA(const char*v){return {2,0,0,v};}
inline FmtArg FA(double v){return {1,0,v,""};} inline FmtArg FA(float v){return {1,0,v,""};}
template<class T> inline typename std::enable_if<std::is_integral<T>::value||std::is_enum<T>::value,FmtArg>::type FA(T v){return {0,(llong)v,0,""};}
inline string MqlFormat(const string&f,const std::vector<FmtArg>&a){
  string o; size_t ai=0;
  for(size_t i=0;i<f.size();i++){
    if(f[i]!='%'){o+=f[i];continue;}
    if(i+1<f.size()&&f[i+1]=='%'){o+='%';i++;continue;}
    string spec="%"; i++;
    while(i<f.size()&&strchr("-+ #0123456789.",f[i])) spec+=f[i++];
    bool l64=false; if(f.compare(i,3,"I64")==0){l64=true;i+=3;} else if(f[i]=='l'){i++;}
    char c=f[i]; char buf[512]; FmtArg x=ai<a.size()?a[ai++]:FmtArg{0,0,0,""};
    if(c=='d'||c=='i'){spec+="lld";snprintf(buf,512,spec.c_str(),x.k==1?(llong)x.d:x.i);}
    else if(c=='u'){spec+="llu";snprintf(buf,512,spec.c_str(),(unsigned long long)(x.k==1?(llong)x.d:x.i));}
    else if(c=='x'||c=='X'){spec+=string("ll")+c;snprintf(buf,512,spec.c_str(),(unsigned long long)x.i);}
    else if(c=='f'||c=='e'||c=='g'){spec+=c;snprintf(buf,512,spec.c_str(),x.k==1?x.d:(double)x.i);}
    else if(c=='s'){spec+="s";snprintf(buf,512,spec.c_str(),x.s.c_str());}
    else if(c=='c'){snprintf(buf,512,"%c",(int)x.i);}
    else buf[0]=0;
    (void)l64; o+=buf; }
  return o; }
template<class...A> string StringFormat(const string&f,A...a){return MqlFormat(f,{FA(a)...});}
template<class...A> void PrintFormat(const string&f,A...a){ extern bool g_mock_quiet; if(!g_mock_quiet) printf("[PF] %s\n",MqlFormat(f,{FA(a)...}).c_str()); }
template<class...A> void Print(A...a){ extern bool g_mock_quiet; if(g_mock_quiet) return; string o; std::vector<FmtArg> v={FA(a)...}; for(auto&x:v){ if(x.k==2)o+=x.s; else if(x.k==1)o+=DoubleToString(x.d,8); else o+=std::to_string(x.i);} printf("[PRINT] %s\n",o.c_str()); }
template<class...A> void Alert(A...a){ Print(a...); }
inline bool SendNotification(const string&){ return true; }
// ---- time/terminal state (defined in sim)
struct Sim;
extern Sim* S;
datetime TimeCurrent(); datetime TimeTradeServer(); datetime TimeGMT();
uint GetTickCount();
llong MQLInfoInteger(int); llong TerminalInfoInteger(int);
llong AccountInfoInteger(int); double AccountInfoDouble(int); string AccountInfoString(int);
llong SymbolInfoInteger(const string&,int); double SymbolInfoDouble(const string&,int); string SymbolInfoString(const string&,int);
bool SymbolInfoTick(const string&,MqlTick&);
bool OrderCalcProfit(int type,const string&sym,double vol,double po,double pc,double&profit);
bool OrderCalcMargin(int type,const string&sym,double vol,double price,double&margin);
bool OrderCheck(const MqlTradeRequest&,MqlTradeCheckResult&);
int PositionsTotal(); ulong PositionGetTicket(int); string PositionGetString(int); llong PositionGetInteger(int); double PositionGetDouble(int);
bool PositionSelect(const string&); bool PositionSelectByTicket(ulong);
int OrdersTotal(); ulong OrderGetTicket(int); llong OrderGetInteger(int); string OrderGetString(int);
bool HistorySelect(datetime,datetime); bool HistorySelectByPosition(ulong); int HistoryDealsTotal(); ulong HistoryDealGetTicket(int); bool HistoryDealSelect(ulong); bool TesterWithdrawal(double);
llong HistoryDealGetInteger(ulong,int); double HistoryDealGetDouble(ulong,int); string HistoryDealGetString(ulong,int);
bool GlobalVariableGet(const string&,double&); bool GlobalVariableDel(const string&); datetime GlobalVariableSet(const string&,double); bool GlobalVariablesFlush(); int GlobalVariablesDeleteAll(const string&p="");
int FileOpen(const string&,int,int=0,int=0); void FileClose(int); void FileFlush(int); llong FileSize(int); bool FileSeek(int,llong,int);
template<class...A> uint FileWrite(int h,A...a);
void EventSetTimer(int); void EventKillTimer(); void ChartRedraw(llong=0); bool EventSetMillisecondTimer(int);
llong ChartID(); string Symbol(); int ObjectsTotal(llong,int=-1,int=-1); string ObjectName(llong,int,int=-1,int=-1); bool ObjectDelete(llong,const string&);
bool SymbolSelect(const string&,bool); int SymbolsTotal(bool); string SymbolName(int,bool);
inline bool IsStopped(){ return false; }
bool TimeToStruct(datetime,MqlDateTime&); datetime StringToTime(const string&); llong ObjectGetInteger(llong,const string&,int,int=0);

datetime iTime(const string&,int tf,int shift); double iHigh(const string&,int,int); double iLow(const string&,int,int);
int iBarShift(const string&,int,datetime,bool=false); int iHighest(const string&,int,int,int,int); int iLowest(const string&,int,int,int,int);
int CopyRates(const string&,int,int,int,MArr<MqlRates>&); int CopyRates(const string&,int,datetime,datetime,MArr<MqlRates>&);
inline int CalendarValueHistory(MArr<MqlCalendarValue>&v,datetime,datetime,const char*,const char*){ v.v.clear(); return 0; }
inline bool CalendarEventById(ulong,MqlCalendarEvent&){ return false; }
// v13.5+ mocks: no calendar / web / text files in the harness (the EA then uses its built-in FOMC table, as in the MT5 tester)
inline int CalendarEventByCountry(const string&,MArr<MqlCalendarEvent>&e){ e.v.clear(); return 0; }
inline int CalendarValueHistoryByEvent(ulong,MArr<MqlCalendarValue>&v,datetime,datetime){ v.v.clear(); return 0; }
enum { FILE_TXT=512, WHOLE_ARRAY=-1 };
inline int StringReplace(string&s,const string&a,const string&b){ if(a.empty()) return 0; int n=0; size_t p=0; while((p=s.find(a,p))!=string::npos){ s.replace(p,a.size(),b); p+=b.size(); n++; } return n; }
inline uint FileWriteString(int,const string&s,int=-1){ return (uint)s.size(); }
inline string FileReadString(int,int=-1){ return ""; }
inline bool FileIsEnding(int){ return true; }
inline int WebRequest(const string&,const string&,const string&,int,MArr<char>&,MArr<char>&,string&){ return -1; }
inline string CharArrayToString(MArr<char>&a,int=0,int=-1,int=0){ return string(a.v.begin(),a.v.end()); }
// objects
bool ObjectCreate(int,const string&,int,int,datetime,double,datetime=0,double=0); int ObjectFind(int,const string&);
bool ObjectMove(int,const string&,int,datetime,double);
bool ObjectSetInteger(int,const string&,int,llong); bool ObjectSetInteger(int,const string&,int,int,llong);
bool ObjectSetDouble(int,const string&,int,double); bool ObjectSetDouble(int,const string&,int,int,double);
bool ObjectSetString(int,const string&,int,const string&); int ObjectsDeleteAll(int,const string&);
// ---- CTrade
class CTrade { public:
  void SetDeviationInPoints(ulong d); void SetAsyncMode(bool); void SetMarginMode(); bool SetTypeFillingBySymbol(const string&); void SetExpertMagicNumber(ulong m);
  bool Buy(double v,const string&s,double p,double sl,double tp,const string&c=""); bool Sell(double v,const string&s,double p,double sl,double tp,const string&c="");
  bool PositionClose(ulong t); bool PositionClosePartial(ulong t,double v); bool PositionModify(ulong t,double sl,double tp); double ResultPrice(); bool OrderDelete(ulong t); uint ResultRetcode(); string ResultRetcodeDescription(); ulong magic=0; ulong dev=0; MqlTradeResult res; };
inline string ShortToString(int c){ string o; if(c<0x80) o+=(char)c; else if(c<0x800){ o+=(char)(0xC0|(c>>6)); o+=(char)(0x80|(c&63)); } else { o+=(char)(0xE0|(c>>12)); o+=(char)(0x80|((c>>6)&63)); o+=(char)(0x80|(c&63)); } return o; }

// ---- v11.8 chart / instance helpers (one chart in the harness: the EA's own)
llong ChartFirst(); llong ChartNext(llong); string ChartSymbol(llong); int ChartPeriod(llong); llong ChartGetInteger(llong,int,int=0);
inline int PeriodSeconds(int tf){ return tf==PERIOD_H1?3600:tf==PERIOD_H4?14400:tf==PERIOD_D1?86400:tf==PERIOD_M30?1800:tf==PERIOD_M15?900:tf==PERIOD_M5?300:60; }
datetime TimeLocal(); void ExpertRemove();
inline void MathSrand(int x){ srand(x); } inline int MathRand(){ return rand()%32768; }
enum { CHART_BRING_TO_TOP=107 };
llong ChartOpen(const string&,int); bool ChartSetInteger(llong,int,int,llong);

llong SeriesInfoInteger(const string&,int,int);

#include <vector>
inline bool SymbolInfoSessionTrade(const string&,ENUM_DAY_OF_WEEK,uint,datetime&,datetime&);
inline int iADX(const string&,int,int); inline int iATR(const string&,int,int);
inline int CopyBuffer(int,int,int,int,MArr<double>&); inline int CopyBuffer(int,int,datetime,int,MArr<double>&); inline void IndicatorRelease(int);
