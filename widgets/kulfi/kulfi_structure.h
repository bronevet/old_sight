#pragma once
#include "sight.h"

namespace sight {
namespace structure {
/*#ifdef __cplusplus
extern "C" {
#endif*/
	void EnableKulfi();
	void DisableKulfi();
/*#ifdef __cplusplus
}
#endif*/

extern long next_fault_countdown;

class kulfiConfHandlerInstantiator : common::confHandlerInstantiator {
  public:
  kulfiConfHandlerInstantiator();
};
extern kulfiConfHandlerInstantiator kulfiConfHandlerInstance;

class kulfiModularApp : public compModularApp
{
  friend class kulfiModule;
  long bufSize;
  char* data;
  pthread_t interfThread;

  public:
  kulfiModularApp(const std::string& appName,                                                        sight::properties* props=NULL);
  kulfiModularApp(const std::string& appName, const attrOp& onoffOp,                                 sight::properties* props=NULL);
  kulfiModularApp(const std::string& appName,                        const compNamedMeasures& cMeas, sight::properties* props=NULL);
  kulfiModularApp(const std::string& appName, const attrOp& onoffOp, const compNamedMeasures& cMeas, sight::properties* props=NULL);

  void init();

  ~kulfiModularApp();

  // Maps each signal number that we've overridden to the signal handler originally mapped to it
  static std::map<int, struct sigaction> originalHandler;

  static void termination_handler (int signum);

  static void overrideSignal(int signum, struct sigaction& new_action);

  // Returns a pointer to the current instance of modularApp
  static kulfiModularApp* getInstance() {
    kulfiModularApp* kulfiActiveMA = dynamic_cast<kulfiModularApp*>(modularApp::getInstance());
    assert(kulfiActiveMA);
    return kulfiActiveMA;
  }
  
  // Returns whether fault injection is currently enabled
  static bool isFIEnabled() {
    //return KulfiRuntime::next_fault_countdown >= 0;
    if(getenv("NEXT_FAULT_COUNTDOWN"))
      return atoi(getenv("NEXT_FAULT_COUNTDOWN")) >= 0;
    else
      return false;
  }

  static void recordFaultInjection(const char* error_type, unsigned bPos, int fault_index, int ef, int tf);
  
  // Directly calls the destructor of this object. This is necessary because when an application crashes
  // Sight must clean up its state by calling the destructors of all the currently-active sightObjs. Since 
  // there is no way to directly call the destructor of a given object when it may have several levels
  // of inheritance above sightObj, each object must enable Sight to directly call its destructor by calling
  // it inside the destroy() method. The fact that this method is virtual ensures that calling destroy() on 
  // an object will invoke the destroy() method of the most-derived class.
  void destroy();

  // -------------------------
  // ----- Configuration -----
  // -------------------------
  // The amount of time the app may run before it is aborted due to a timeout
  static double timeoutLimitS;

  static void* timeoutWatcher(void*);

  public:
  class KulfiModularAppConfiguration : public CompModularAppConfiguration {
    public:
    KulfiModularAppConfiguration(properties::iterator props) : CompModularAppConfiguration(props.next()) {
      timeoutLimitS = props.getFloat("timeoutLimitS");
      pthread_t thread;
      int ret = pthread_create(&thread, NULL, kulfiModularApp::timeoutWatcher, (void*)NULL);
      if(ret) { std::cerr << "ERROR creating timeoutWatcher thread! Error code "<<ret<<"."<<std::endl; exit(-1); }
    }
  };
  static common::Configuration* configure(properties::iterator props) {
    // Create a ModuleConfiguration object, using the invocation of the constructor hierarchy to
    // record the configuration details with the respective widgets from which modules inherit
    KulfiModularAppConfiguration* c = new KulfiModularAppConfiguration(props);
    delete c;
    return NULL;
  } 
}; // class kulfiModularApp

class kulfiModule: public compModule {
  public:

  // The types of outcomes of a given module's execution
  typedef enum {completed, aborted} moduleOutcomeT;

  // options: The context that describes the configuration options of this module
  // meas: The measurements that should be performed during the execution of this kulfiModule
  // derivInfo: Information that describes the class that derives from this on. It includes a pointer to a properties
  //    object that can be used to create a tag for the derived object that includes info about its parents. Further,
  //    it includes fields that must be included within the context of any trace observations made during the execution
  //    of this module.
  kulfiModule(const instance& inst, const std::vector<port>& inputs, 
               const context& options,
                                                               sight::properties* props=NULL);
  kulfiModule(const instance& inst, const std::vector<port>& inputs, 
             const context& options,
             const attrOp& onoffOp,                            sight::properties* props=NULL);
  kulfiModule(const instance& inst, const std::vector<port>& inputs, 
             const context& options,
                                    const compNamedMeasures& meas, sight::properties* props=NULL);
  kulfiModule(const instance& inst, const std::vector<port>& inputs, 
             const context& options,
             const attrOp& onoffOp, const compNamedMeasures& meas, sight::properties* props=NULL);

  kulfiModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
               const context& options,
                                                               sight::properties* props=NULL);
  kulfiModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
             const context& options,
             const attrOp& onoffOp,                            sight::properties* props=NULL);
  kulfiModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
             const context& options,
                                    const compNamedMeasures& meas, sight::properties* props=NULL);
  kulfiModule(const instance& inst, const std::vector<port>& inputs, std::vector<port>& externalOutputs,
             const context& options,
             const attrOp& onoffOp, const compNamedMeasures& meas, sight::properties* props=NULL);

  // Return an instance that is identical to inst but has 1 extra output for the outcome of the module
  instance modifyInst(const instance& inst);

  // Sets the properties of this object
  sight::properties* setProperties(const instance& inst, bool isReference, context options, const attrOp* onoffOp, sight::properties* props);

  void init();

  ~kulfiModule();

  // Returns the context attributes to be used in this module's measurements by combining the context provided by the classes
  // that this object derives from with its own unique context attributes.
  virtual std::map<std::string, sight::attrValue> getTraceCtxt();
  
  // Sets the context of the given output port. This variant ensures that KULFI fault injection
  // is disabled while setting the output context
  virtual void setOutCtxt(int idx, const compContext& c);
}; // class kulfiModule

}; // namespace sight
}; // namespace structure

