//#include "sight.h"
#include "thread_local_storage.h"
#include <map>

#ifndef DepthCounter
class DepthCounter {
  public:
  static ThreadLocalStorage1<int, int> funcDepth;
  DepthCounter()
  { funcDepth++; }
  ~DepthCounter()
  { funcDepth--; }
}; 
#endif

// #define SightMainFile 
#ifdef SIGHT_MAIN_FILE
   ThreadLocalStorage1<int, int> DepthCounter::funcDepth(0);
//#define SightNonMainFile 
//#else
//  extern ThreadLocalStorage1<int, int> funcDepth;
#endif

// State transition graph
#ifdef SIGHT_MAIN_FILE
sight::structure::graph* stateTransGraph;
sight::structure::anchor lastStateAnchor=anchor::noAnchor;
#else
extern sight::structure::graph* stateTransGraph;
extern anchor lastStateAnchor;
#endif

// Def-use graph
#ifdef SIGHT_VARDEFUSE
#ifdef SIGHT_MAIN_FILE
sight::structure::graph* duGraph=NULL;
#else
extern sight::structure::graph* duGraph;
void SightInitializeDUGraph();
#endif
#define SightInitializeDU(varUID) \
  graph duGraphMain; \
  duGraph = &duGraphMain;
#else
#define SightInitializeDU(varUID)
#endif //SIGHT_VARDEFUSE

// ----- SightInitialize -----
#if defined(SIGHT_MODULE) || defined(SIGHT_STATES)
#define SightInitializeModule(varUID) \
  sight::structure::modularApp SightModularApp ## varUID("app", namedMeasures("time", new timeMeasure()));
#else
#define SightInitializeModule(varUID)
#endif

#ifdef SIGHT_SCOPE
#define SightInitializeScope(varUID) \
  graph stateTransGraphStatic(false); \
  stateTransGraph = &stateTransGraphStatic;
#else
#define SightInitializeScope(varUID)
#endif

//#ifdef MPI_COMM_WORLD
#define SightInitialize(varUID)                            \
  { /*int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank); */             \
  SightInit("Application", sight::txt()<<"out.process_"<<getpid()); }  \
  SightInitializeModule(varUID); \
  SightInitializeScope(varUID); \
  SightInitializeDU(varUID); \
/*#else
#define SightInitialize(varUID)                           \
  SightInit("Application", "out");                       / * \
  sight::structure::modularApp SightModularApp ## varUID("app", namedMeasures("time", new timeMeasure())); * /
#endif*/

// ----- SightCreateCtxt -----
#ifdef SIGHT_MODULE
#define SightCreateCtxtModule(varUID, funcName) \
  sight::structure::module::context SightContext ## varUID;
#else
#define SightCreateCtxtModule(varUID, funcName)
#endif

#ifdef SIGHT_SCOPE
#define SightCreateCtxtScope(varUID, funcName) \
  DepthCounter counter ## varUID;                           \
  std::stringstream SightScopeLabel ## varUID;              \
  SightScopeLabel ## varUID << funcName << "(";
#else
#define SightCreateCtxtScope(varUID, funcName)
#endif

#define SightCreateCtxt(varUID, funcName)                   \
  /*cout << "DepthCounter::funcDepth="<<DepthCounter::funcDepth<<endl;*/ \
  /*attrIf SightIf ## varUID((DepthCounter::funcDepth<2? (universalAttrOp*)new attrTrue(): (universalAttrOp*)new attrFalse()));*/ \
  SightCreateCtxtModule(varUID, funcName) \
  SightCreateCtxtScope(varUID, funcName)

// ----- SightAddCtxt -----
#ifdef SIGHT_MODULE
#define SightAddCtxtModule(varUID, key, val) \
  SightContext ## varUID.add(key, val);
#else
#define SightAddCtxtModule(varUID, key, val)
#endif

#ifdef SIGHT_SCOPE
#define SightAddCtxtScope(varUID, key, val) \
  SightScopeLabel ## varUID << ", " << key << "=" << val;
#else
#define SightAddCtxtScope(varUID, key, val)
#endif

#define SightAddCtxt(varUID, key, val) \
  SightAddCtxtModule(varUID, key, val) \
  SightAddCtxtScope(varUID, key, val)

// ----- SightEnterFunc -----
#ifdef SIGHT_MODULE
#define SightEnterFuncModule(varUID, funcName) \
  sight::structure::module SightModule ## varUID(instance(funcName, 1, 0), inputs(port(SightContext ## varUID)));
#else
#define SightEnterFuncModule(varUID, funcName)
#endif

#ifdef SIGHT_SCOPE
#define SightEnterFuncScope(varUID, funcName) \
  SightScopeLabel ## varUID << ")"; \
  sight::structure::scope SightScope ## varUID(SightScopeLabel ## varUID.str(), \
                                               DepthCounter::funcDepth%10==2? sight::structure::scope::high: sight::structure::scope::medium, \
                                               (DepthCounter::funcDepth<=10? (const universalAttrOp&)attrTrue(): (const universalAttrOp&)attrFalse()));
#else
#define SightEnterFuncScope(varUID, funcName)
#endif


#define SightEnterFunc(varUID, funcName) \
  SightEnterFuncModule(varUID, funcName) \
  SightEnterFuncScope(varUID, funcName)
      
#define SightExitFunc(varUID) 
//  }

// ----- States -----

#ifdef SIGHT_STATES

#ifdef SIGHT_MAIN_FILE
sight::structure::port controlPred(context("Predecessor", "START"));

void sightState(std::string label) {
  std::vector<port> externalOutputs;
  sight::structure::module m(instance(label, 1, 1),
                             controlPred, externalOutputs);
  m.setOutCtxt(0, sight::structure::context("Predecessor", label));
  controlPred=externalOutputs[0];
  scope s(label);
  if(lastStateAnchor!=anchor::noAnchor)
    stateTransGraph->addDirEdge(lastStateAnchor, s.getAnchor());
  lastStateAnchor = s.getAnchor();
}

#else
extern sight::structure::port controlPred;
void sightState(std::string label);
#endif

#define SIGHT_STATE1(arg1)                                                                     \
  sightState(std::string(sight::txt()<<"State("<<#arg1<<"=>"<<(arg1)<<")")); 

#define SIGHT_STATE2(arg1, arg2)                                                 \
  sightState(std::string(sight::txt()<<"State("<<#arg1<<"=>"<<(arg1)<<", "<<#arg2<<"=>"<<(arg2)<<")")); 

#define SIGHT_STATE3(arg1, arg2, arg3)                                                         \
  sightState(std::string(sight::txt()<<"State("<<#arg1<<"=>"<<(arg1)<<", "<<#arg2<<"=>"<<(arg2)<<", "<<#arg3<<"=>"<<(arg3)<<")")); 

// ----- Variable Change Tracking -----
#define SightStartTrack(var, UID) \
  sight::structure::trace sightTrace ## UID (sight::txt()<<"SightTrace_d"<<sight::structure::getSOStackDepth()<<"_v"<<#var<<UID, sight::structure::trace::showBegin, sight::structure::trace::lines); \
  int sightTraceCounter ## UID=0;

#ifdef SIGHT_VARTRACK
/*#define SIGHT_TRACK(var, type) \
  sight::structure::dbg << #var << "=" << var << endl;*/
template<typename T>
void SIGHT_TRACK(T&, char*) {}

#define TrackVarChange(var, anchorExp, UID) \
  (sight::structure::dbg << #var << "=" << var << endl, \
   traceAttr(&sightTrace ## UID, \
             sight::structure::trace::ctxtVals("index", sightTraceCounter ## UID), \
             sight::structure::trace::observation(#var, var)), \
   ++sightTraceCounter ## UID,\
   var)

#endif

// ----- Def-Use Graphs -----
#ifdef SIGHT_VARDEFUSE
template<typename T>
void SIGHT_DEFUSE(T&, char*) {}

#ifdef SIGHT_MAIN_FILE
std::map<long, long> lastVarDef;
std::map<long, sight::structure::anchor> lastVarDefAnchor;
//sight::structure::anchor lastDUAnchor = sight::structure::anchor::noAnchor;
std::map<long, sight::structure::anchor> lastVarAccessAnchor;
long useCntr=0;
std::list<sight::box*> udBoxStack;
#else
extern std::map<long, long> lastVarDef;
extern std::map<long, sight::structure::anchor> lastVarDefAnchor;
//extern sight::structure::anchor lastDUAnchor;
extern std::map<long, sight::structure::anchor> lastVarAccessAnchor;
extern long useCntr;
extern std::list<sight::box*> udBoxStack;
#endif

#define SightUDStart(var, anchorExp, UID) \
  udBoxStack.push_back(new sight::box("border-style:solid; border-width:1px; border-color: #000000; display:inline-block"))

#define SightUDEnd(var, anchorExp, UID) \
  (delete udBoxStack.back(), udBoxStack.pop_back(), dbg<<endl)

#define SightUDOp(var, anchorExp, UID) \
  (dbg << #anchorExp)

#ifdef SIGHT_MAIN_FILE
void SightVarDefFunc(char* varName, char* anchorExp, long UID) {
  if(lastVarDef.find(UID)==lastVarDef.end()) lastVarDef[UID]=0;
  else lastVarDef[UID]++;
  //dbg << "Def "<<varName<<": send=SightDef_"<<varName<<"_"<<UID<<"_"<<lastVarDef[UID]<<endl;
  //{ sight::box b(sight::txt()<<"margin-left:"<<(UID*20)<<"; margin-top:0; margin-bottom:0; margin-right:0; width:40; height:20; border-style:solid; border-color: #0000ff;"); 
  { sight::box b(sight::txt()<<"margin-left:"<<(UID*0)<<"; margin-top:0; margin-bottom:10; margin-right:0; border-style:solid; border-color: #0000ff; display:inline-block",
                 sight::txt()<<"Def "<<varName); 
  dbg << "Def "<<varName/*<<" ("<<anchorExp<<")"*/<<endl;
  sight::structure::commSend s("", //sight::txt()<<"Def "<<varName,
                               sight::txt()<<"SightDef_"<<varName<<"_"<<UID<<"_"<<lastVarDef[UID], "");

  if(lastVarAccessAnchor.find(UID) != lastVarAccessAnchor.end())
    duGraph->addInvisDepEdge(lastVarAccessAnchor[UID], b.getAnchor());
  /*if(lastDUAnchor != sight::structure::anchor::noAnchor)
    duGraph->addUndirEdge(lastDUAnchor, b.getAnchor());*/
  
  lastVarDefAnchor[UID] = b.getAnchor();
  lastVarAccessAnchor[UID] = b.getAnchor();
//  lastDUAnchor = b.getAnchor();
  }
}
#else
void SightVarDefFunc(char* varName, long UID);
#endif

#define SightVarDef(var, anchorExp, UID) \
  SightVarDefFunc(#var, #anchorExp, UID)

#ifdef SIGHT_MAIN_FILE
void SightVarUseFunc(char* varName, char* anchorExp, long UID) {
  //dbg << "Use "<<varName<<": send="<<"SightDef_"<<varName<<"_"<<UID<<"_"<<lastVarDef[UID]<<", recv="<<"SightUse_"<<useCntr<<endl;
  //{ sight::box b(sight::txt()<<"margin-left:"<<(UID*20)<<"; margin-top:0; margin-bottom:0; margin-right:0; width:40; height:20; iborder-style:solid; border-color: #ff0000;"); 
  { sight::box b(sight::txt()<<"margin-left:"<<(UID*0)<<"; margin-top:0; margin-bottom:10; margin-right:0; border-style:solid; border-color: #ff0000; display:inline-block",
                 sight::txt()<<"Use "<<varName); 
  dbg << "Use "<<varName/*<<" ("<<anchorExp<<")"*/<<endl;
  if(lastVarDefAnchor.find(UID) != lastVarDefAnchor.end())
    duGraph->addDirEdge(lastVarDefAnchor[UID], b.getAnchor());
  else if(lastVarAccessAnchor.find(UID) != lastVarAccessAnchor.end())
    duGraph->addInvisDepEdge(lastVarAccessAnchor[UID], b.getAnchor());
  /*else if(lastDUAnchor != sight::structure::anchor::noAnchor)
    duGraph->addUndirEdge(lastDUAnchor, b.getAnchor());*/
  sight::structure::commRecv s("",//sight::txt()<<"Use "<<varName,
                               sight::txt()<<"SightDef_"<<varName<<"_"<<UID<<"_"<<lastVarDef[UID],
                               sight::txt()<<"SightUse_"<<useCntr); 
  lastVarAccessAnchor[UID] = b.getAnchor();
  //lastDUAnchor = b.getAnchor();
  }
  ++useCntr;
}
#else
void SightVarUseFunc(char* varName, long UID);
#endif

#define SightVarUse(var, anchorExp, UID) \
  SightVarUseFunc(#var, #anchorExp, UID)

#define SightVarDefUse(var, anchorExp, UID) \
  SightVarUse(var, anchorExp, UID), SightVarDef(var, anchorExp, UID)

#endif // #ifdef SIGHT_DEFUSE

#endif
