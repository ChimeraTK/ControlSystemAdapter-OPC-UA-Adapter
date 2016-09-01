/*
 * Copyright (c) 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>, Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 * Chair for Process Systems Engineering
 * Technical University of Dresden
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */


extern "C" {
    #include <unistd.h>
    #include <signal.h>
	#include "mtca_namespaceinit_generated.h" // Output des pyUANamespacecompilers
}


#include <iostream>

#include "open62541.h"
#include "ControlSystemAdapterOPCUA.h"
#include "XMLFileHandler.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/IndependentControlCore.h"
#include "ipc_manager.h"

#include "runTimeValueGenerator.cpp"

using std::cout;
using std::endl;
using namespace ChimeraTK;

/* GLOBAL VAR */
ControlSystemAdapterOPCUA *csaOPCUA;

/* FUNCTIONS */
static void SigHandler_Int(int sign) {
    cout << "Received SIGINT... terminating" << endl;
    csaOPCUA->stop();
    csaOPCUA->terminate();
    cout << "terminated threads" << endl;
}

int main() {
    signal(SIGINT,  SigHandler_Int); // Registriert CTRL-C/SIGINT
    signal(SIGTERM, SigHandler_Int); // Registriert SIGTERM
    
    // Create the managers
    std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
    
    boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;
    boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
    
    ControlSystemSynchronizationUtility syncUtil(csManager);
    
    IndependentControlCore controlCore(devManager);
    
    /*
    * Generate dummy data
    */
    ProcessScalar<int8_t>::SharedPtr intAdev = devManager->createProcessScalar<int8_t>(controlSystemToDevice, "intA");
    ProcessScalar<uint8_t>::SharedPtr intBdev = devManager->createProcessScalar<uint8_t>(deviceToControlSystem, "intB");
	ProcessScalar<int16_t>::SharedPtr intCdev = devManager->createProcessScalar<int16_t>(controlSystemToDevice, "intC");
	ProcessScalar<uint16_t>::SharedPtr intDdev = devManager->createProcessScalar<uint16_t>(controlSystemToDevice, "intD");
    ProcessScalar<int32_t>::SharedPtr intEdev = devManager->createProcessScalar<int32_t>(controlSystemToDevice, "intE");
    ProcessScalar<uint32_t>::SharedPtr intFdev = devManager->createProcessScalar<uint32_t>(controlSystemToDevice, "intF");
    ProcessScalar<float>::SharedPtr intGdev = devManager->createProcessScalar<float>(controlSystemToDevice, "intG");
    ProcessScalar<double>::SharedPtr intHdev = devManager->createProcessScalar<double>(controlSystemToDevice, "intH");
    
    ProcessArray<int32_t>::SharedPtr intC1dev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "intArray15", 15);
    ProcessArray<int32_t>::SharedPtr intC2dev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "intArray10", 10);
    ProcessArray<int32_t>::SharedPtr intC3dev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "intArray5", 5);
    

        
    ProcessScalar<int>::SharedPtr targetVoltage = csManager->getProcessScalar<int>("TARGET_VOLTAGE");
    ProcessScalar<int>::SharedPtr monitorVoltage = csManager->getProcessScalar<int>("MONITOR_VOLTAGE");
        
    *targetVoltage = 42;
    targetVoltage->send();

    csManager->getProcessArray<int32_t>("intArray15")->get().at(0) = 21;
    csManager->getProcessArray<int32_t>("intArray10")->get().at(1) = 22;
    csManager->getProcessArray<int32_t>("intArray5")->get().at(2) = 23;
	
    int x[5] = {1, 2, 3, 4, 5};
    std::vector<int> v(x, x + sizeof x / sizeof x[0]);
    csManager->getProcessArray<int32_t>("intArray5")->set(v);    
	
	    
    std::cout << "Dummy Daten geschrieben..." << std::endl;	
	
	// Only for ValueGenerator
    ipc_manager *mgr;
    csaOPCUA = new ControlSystemAdapterOPCUA(16664, csManager, "../uamapping.xml");
	mgr = csaOPCUA->getIPCManager();
	runTimeValueGenerator *valGen = new runTimeValueGenerator(csManager);
	mgr->addObject(valGen);
	mgr->doStart();	

    while(csaOPCUA->isRunning()) {

		sleep(100);
	}
    
    return 0;
}