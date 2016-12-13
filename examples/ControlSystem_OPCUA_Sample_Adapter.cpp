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
#include <math.h>

#include <chrono>
#include <time.h>
#include <errno.h>
#include <linux/unistd.h>       /* for _syscallX macros/related stuff */
#include <linux/kernel.h>       /* for struct sysinfo */
#include <sys/sysinfo.h>

#include "open62541.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PersistentDataStorage.h"
#include "ChimeraTK/ControlSystemAdapter/ProcessArray.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"

#include "ipc_manager.h"
#include "ControlSystemAdapterOPCUA.h"
#include "runtimeValueGenerator.h"

using namespace std;
using namespace ChimeraTK;

	ipc_manager *mgr;
	runtimeValueGenerator *valGen;
		ControlSystemAdapterOPCUA *csaOPCUA;
	
/* FUNCTIONS */
static void SigHandler_Int(int sign) {
    cout << "Received SIGINT... terminating" << endl;
		valGen->doStop();
		valGen->terminate();
		valGen->~runtimeValueGenerator();
		csaOPCUA->stop();
		csaOPCUA->terminate();
		csaOPCUA->~ControlSystemAdapterOPCUA();
    cout << "terminated threads" << endl;
}
	
int main() {
	signal(SIGINT,  SigHandler_Int); // Registriert CTRL-C/SIGINT
	signal(SIGTERM, SigHandler_Int); // Registriert SIGTERM

	// Create the managers
	std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
    
	boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;
	boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;
	
	boost::shared_ptr<ControlSystemSynchronizationUtility> syncCsUtility;
	syncCsUtility.reset(new ChimeraTK::ControlSystemSynchronizationUtility(csManager));
	
	boost::shared_ptr<DeviceSynchronizationUtility> syncDevUtility;
	syncDevUtility.reset(new ChimeraTK::DeviceSynchronizationUtility(devManager));

	//csManager->enablePersistentDataStorage();
	
	/*
	 * Generate dummy data
	 */
 	ProcessArray<int8_t>::SharedPtr intA8dev = devManager->createProcessArray<int8_t>(controlSystemToDevice, "int8Scalar", 1, "Iatrou^2/Rahm");
 	ProcessArray<uint8_t>::SharedPtr intAu8dev = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "uint8Scalar", 1, "mIatrou*Rahm");
 	ProcessArray<int16_t>::SharedPtr intA16dev = devManager->createProcessArray<int16_t>(controlSystemToDevice, "int16Scalar", 1);
 	ProcessArray<uint16_t>::SharedPtr intAu16dev = devManager->createProcessArray<uint16_t>(controlSystemToDevice, "uint16Scalar", 1);
 	ProcessArray<int32_t>::SharedPtr intA32dev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "int32Scalar", 1);
 	ProcessArray<uint32_t>::SharedPtr intAu32dev = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "uint32Scalar", 1);
 	ProcessArray<float>::SharedPtr intAfdev = devManager->createProcessArray<float>(controlSystemToDevice, "floatScalar", 1);
 	ProcessArray<double>::SharedPtr intAddev = devManager->createProcessArray<double>(controlSystemToDevice, "doubleScalar", 1);
	
	ProcessArray<int8_t>::SharedPtr intB15A8dev = devManager->createProcessArray<int8_t>(controlSystemToDevice, "int8Array_s15", 15, "mIatrou*Rahm", "Die Einheit ist essentiel und sollte SI Einheit sein...");
	ProcessArray<uint8_t>::SharedPtr intB10Au8dev = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "uint8Array_s10", 10, "1/Rahm");
	ProcessArray<int16_t>::SharedPtr intB15A16dev = devManager->createProcessArray<int16_t>(controlSystemToDevice, "int16Array_s15", 15, "Iatrou", "Beschreibung eines Iatrous");
	ProcessArray<uint16_t>::SharedPtr intB10Au16dev = devManager->createProcessArray<uint16_t>(controlSystemToDevice, "uint16Array_s10", 10);
	ProcessArray<int32_t>::SharedPtr intB15A32dev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "int32Array_s15", 15);
	ProcessArray<uint32_t>::SharedPtr intB10Au32dev = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "uint32Array_s10", 10);
	ProcessArray<double>::SharedPtr intB15Afdev = devManager->createProcessArray<double>(controlSystemToDevice, "doubleArray_s15", 15);
	ProcessArray<float>::SharedPtr intB10Addev = devManager->createProcessArray<float>(controlSystemToDevice, "floatArray_s10", 10);	
	// data generation cycle time in ms
	ProcessArray<int32_t>::SharedPtr dtDev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "dt", 1);
	// time since server start in ms
	ProcessArray<int32_t>::SharedPtr tDev = devManager->createProcessArray<int32_t>(deviceToControlSystem, "t", 1);
	ProcessArray<int32_t>::SharedPtr periodDev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "period", 1);
	ProcessArray<double>::SharedPtr amplitudeDev = devManager->createProcessArray<double>(controlSystemToDevice, "amplitude", 1);
	ProcessArray<double>::SharedPtr double_sineDev = devManager->createProcessArray<double>(deviceToControlSystem, "double_sine", 1);
	ProcessArray<int32_t>::SharedPtr int_sineDev = devManager->createProcessArray<int32_t>(deviceToControlSystem, "int_sine", 1);
	
	// example mapping variable
	ProcessArray<int8_t>::SharedPtr intB15A8devMap = devManager->createProcessArray<int8_t>(controlSystemToDevice, "Mein/Name_ist#int8Array_s15", 15, "Iatrou^2/Rahm");
	ProcessArray<uint8_t>::SharedPtr intB10Au8devMap1 = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "Dein/Name/ist/uint8Array_s10", 10, "Iatrou^2/Rahm", "Beschreibung");
	ProcessArray<uint8_t>::SharedPtr intB10Au8devMap2 = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "Unser/Name/ist_uint8Array_s10", 10);
	ProcessArray<uint32_t>::SharedPtr intAu32devMap = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "Ist/Name/dieser/uint32Scalar", 1);
	ProcessArray<int32_t>::SharedPtr intA32devMap = devManager->createProcessArray<int32_t>(controlSystemToDevice, "Ist/Name/dieser/int32Scalar", 1);
	ProcessArray<double>::SharedPtr doubledevMap = devManager->createProcessArray<double>(controlSystemToDevice, "Ist/Name/dieser/doubleScalar", 1);
	
	csManager->getProcessArray<int32_t>("int32Array_s15")->get().at(0) = 12;
	csManager->getProcessArray<int32_t>("int32Array_s15")->get().at(1) = 13;
	csManager->getProcessArray<int32_t>("int32Array_s15")->get().at(2) = 14;
	csManager->getProcessArray<int32_t>("int32Array_s15")->get().at(3) = 15;
	csManager->getProcessArray<int32_t>("int32Array_s15")->get().at(4) = 16;
	csManager->getProcessArray<int32_t>("int32Array_s15")->write();
	
	// start values
	int32_t microseconds = 1000000;
	int32_t period = 3.141;
	double amplitude = 10;
		
	// Set values
	csManager->getProcessArray<int32_t>("dt")->set(vector<int32_t> {microseconds});
	csManager->getProcessArray<int32_t>("dt")->write();
	csManager->getProcessArray<int32_t>("period")->set(vector<int32_t> {period});
	csManager->getProcessArray<int32_t>("period")->write();
	csManager->getProcessArray<double>("amplitude")->set(vector<double> {amplitude});
	csManager->getProcessArray<double>("amplitude")->write();

	cout << "Dummy Daten geschrieben..." << std::endl;	
	
	string pathToConfig = "opcuaAdapter_mapping.xml";
	csaOPCUA = new ControlSystemAdapterOPCUA(csManager, pathToConfig);
	
	// Only for Sin ValueGenerator
	mgr = new ipc_manager();
	valGen = new runtimeValueGenerator(devManager, syncDevUtility);
	mgr->addObject(valGen);
	mgr->doStart();	
	
	// Server is running
	std::cout << "Server läuft..." << std::endl;
	int32_t i = 0;
	while(csaOPCUA->isRunning()) {
// 		do something crazy shit
// 		cout << "Part1: " << devManager->getProcessArray<int32_t>("int32Array_s15")->get().at(0) << endl;
// 		cout << "Part2: " << csManager->getProcessArray<int32_t>("int32Array_s15")->get().at(0) << endl;
// 		
// 		cout << "Part3: " << csManager->getProcessArray<int32_t>("int_sine")->get().at(0) << endl;
// 		cout << "Part4: " << devManager->getProcessArray<int32_t>("int_sine")->get().at(0) << endl;
// 		
// 		cout << "Part5: " << csManager->getProcessArray<int16_t>("int16Scalar")->get().at(0) << endl;
// 		cout << "Part6: " << devManager->getProcessArray<int16_t>("int16Scalar")->get().at(0) << endl;
		
		sleep(2);
	}
	
	return 0;
}