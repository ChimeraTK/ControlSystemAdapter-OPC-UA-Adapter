#include <iostream>

extern "C"{
#include <unistd.h>
#include <signal.h>
}

#include "ipc_manager.h"
#include "mtca_uaadapter.h"

ipc_manager  *mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

/* FUNCTIONS */
static void SigHandler_Int(int sign) {
  cout << "Received SIGINT... terminating" << endl;
  mgr->stopAll();
  mgr->terminate();
  cout << "terminated threads" << endl;
}

int main(int argc, char **argv) {
  signal(SIGINT,  SigHandler_Int); // Registriert CTRL-C/SIGINT
  signal(SIGTERM, SigHandler_Int); // Registriert SIGTERM
  
  mtca_uaadapter *adapter = new mtca_uaadapter(16664);
  std::cout << "Hello, world!" << std::endl;
  
  mgr->addObject(adapter);
  mgr->doStart(); // Implicit: Startet Worker-Threads aller ipc_managed_objects, die mit addObject registriert wurden
  
  // Add a new variable to the adapter (should appear in UA Namespace dynamically)
  adapter->addVariable("Variable X", "mHensel/Iatrou");
 
  while(mgr->isRunning()) {
    sleep(1);
  }
  
  return 0;
}
