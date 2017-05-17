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
		#include "csa_namespaceinit_generated.h" // Output des pyUANamespacecompilers
}

#include <iostream>
#include <math.h>

#include <chrono>
#include <time.h>
#include <errno.h>
#include <linux/unistd.h>       /* for _syscallX macros/related stuff */
#include <linux/kernel.h>       /* for struct sysinfo */
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <time.h>

#include "open62541.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PersistentDataStorage.h"
#include "ChimeraTK/ControlSystemAdapter/ProcessArray.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"

#include "ipc_manager.h"
#include "csa_opcua_adapter.h"
#include "runtime_value_generator.h"

using namespace std;
using namespace ChimeraTK;

	ipc_manager *mgr;
	runtime_value_generator *valGen;
		csa_opcua_adapter *csaOPCUA;
	
/* FUNCTIONS */
static void SigHandler_Int(int sign) {
    cout << "Received SIGINT... terminating" << endl;
		valGen->doStop();
		valGen->~runtime_value_generator();
		csaOPCUA->stop();
		csaOPCUA->~csa_opcua_adapter();
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
	ProcessArray<double>::SharedPtr periodDev = devManager->createProcessArray<double>(controlSystemToDevice, "period", 1);
	ProcessArray<double>::SharedPtr amplitudeDev = devManager->createProcessArray<double>(controlSystemToDevice, "amplitude", 1);
	ProcessArray<double>::SharedPtr double_sineDev = devManager->createProcessArray<double>(deviceToControlSystem, "double_sine", 1);
	ProcessArray<int32_t>::SharedPtr int_sineDev = devManager->createProcessArray<int32_t>(deviceToControlSystem, "int_sine", 1);
	
	// example mapping variable
	ProcessArray<int8_t>::SharedPtr intB15A8devMap = devManager->createProcessArray<int8_t>(controlSystemToDevice, "Mein/Name_ist#int8Array_s15", 15, "Iatrou^2/Rahm");
	ProcessArray<uint8_t>::SharedPtr intB10Au8devMap1 = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "/Dein/Name//ist/uint8Array_s10", 10, "Iatrou^2/Rahm", "Beschreibung");
	ProcessArray<uint8_t>::SharedPtr intB10Au8devMap2 = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "Unser/Name/ist_uint8Array_s10", 10);
	ProcessArray<uint32_t>::SharedPtr intAu32devMap = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "Ist/Name/dieser/uint32Scalar", 1);
	ProcessArray<int32_t>::SharedPtr intA32devMap = devManager->createProcessArray<int32_t>(controlSystemToDevice, "Ist/Name/dieser/int32Scalar", 1);
	ProcessArray<double>::SharedPtr doubledevMap = devManager->createProcessArray<double>(controlSystemToDevice, "Ist/Name/dieser/doubleScalar", 1);
	
	/* Some big size arrays for test purpose 
		* 
		*/
		time_t t;
		time(&t);
		srand((unsigned int)t);
		for(int32_t i=1000; i < 65535; i=i+1000) {
			string nameDouble = "testDoubleArray_" + to_string(i);
			string nameInt = "testIntArray_" + to_string(i);
			ProcessArray<double>::SharedPtr testDoubleArray = devManager->createProcessArray<double>(deviceToControlSystem, nameDouble, i);
			ProcessArray<int32_t>::SharedPtr testIntArray = devManager->createProcessArray<int32_t>(deviceToControlSystem, nameInt, i);
			for(int32_t k = 0; k < i; k++) {
				if(k % 2 == 0) {
					testDoubleArray->get().at(k) = rand() % 6 + 1;
					testIntArray->get().at(k) = rand() % 6 + 1;
				}
				else {
					testDoubleArray->get().at(k) = rand() % 50 + 10;
					testIntArray->get().at(k) = rand() % 50 + 10;
				}
			}
			testDoubleArray->write();
			testIntArray->write();
		}
		
		ProcessArray<double>::SharedPtr testDoubleArray = devManager->createProcessArray<double>(deviceToControlSystem, "testDoubleArray_65535", 65535);
		ProcessArray<int32_t>::SharedPtr testIntArray = devManager->createProcessArray<int32_t>(deviceToControlSystem, "testIntArray_65535", 65535);
		for(int32_t i = 0; i < 65535; i++) {
			if(i % 2 == 0) {
				testDoubleArray->get().at(i) = rand() % 6 + 1;
				testIntArray->get().at(i) = rand() % 6 + 1;
			}
			else {
				testDoubleArray->get().at(i) = rand() % 50 + 10;
				testIntArray->get().at(i) = rand() % 50 + 10;
			}
		}
		testDoubleArray->write();
		testIntArray->write();
	
	// start values
	int32_t microseconds = 1000000;
	double period = 3.141;
	double amplitude = 10;
		
	// Set values
	csManager->getProcessArray<int32_t>("dt")->set(vector<int32_t> {microseconds});
	csManager->getProcessArray<int32_t>("dt")->write();
	csManager->getProcessArray<double>("period")->set(vector<double> {period});
	csManager->getProcessArray<double>("period")->write();
	csManager->getProcessArray<double>("amplitude")->set(vector<double> {amplitude});
	csManager->getProcessArray<double>("amplitude")->write();

	csManager->getProcessArray<int8_t>("int8Scalar")->set(vector<int8_t> {12});
	cout << "write dummy Data..." << std::endl;	
	
	string pathToConfig = "opcuaAdapter_mapping.xml";
	csaOPCUA = new csa_opcua_adapter(csManager, pathToConfig);
	
	// Only for Sin ValueGenerator
	mgr = new ipc_manager();
	valGen = new runtime_value_generator(devManager, syncDevUtility);
	mgr->addObject(valGen);
	mgr->doStart();	
	
	// Server is running
	std::cout << "server is running..." << std::endl;
	while(csaOPCUA->isRunning()) {
		sleep(2);
	}
	
	return 0;
}