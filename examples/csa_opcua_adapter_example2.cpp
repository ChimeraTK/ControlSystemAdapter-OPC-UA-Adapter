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

#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PersistentDataStorage.h"
#include "ChimeraTK/ControlSystemAdapter/ProcessArray.h"
#include "ChimeraTK/ControlSystemAdapter/DeviceSynchronizationUtility.h"
#include "ChimeraTK/ControlSystemAdapter/ApplicationBase.h"

using namespace std;
using namespace ChimeraTK;
	
struct MyApp : public ApplicationBase {
    MyApp() : ApplicationBase("opcuaAdapter") {}
		~MyApp() { shutdown(); }

	void run() {
		cout << "Application run..." << endl;
	}
	
  void shutdown() {
		std::lock_guard<std::mutex> lock(instance_mutex);
    instance = nullptr;
    hasBeenShutdown = true;
		cout << "Application shutdown..." << endl;
	}
	
	void initialise() {
		boost::shared_ptr<DevicePVManager> devManager = this->_processVariableManager;
		boost::shared_ptr<DeviceSynchronizationUtility> syncDevUtility;
		syncDevUtility.reset(new ChimeraTK::DeviceSynchronizationUtility(this->_processVariableManager));
				
		/*
		 * Generate dummy data
		 */
		ProcessArray<int8_t>::SharedPtr intA8dev = this->_processVariableManager->createProcessArray<int8_t>(controlSystemToDevice, "int8Scalar", 1, "Iatrou^2/Rahm");
		ProcessArray<uint8_t>::SharedPtr intAu8dev = this->_processVariableManager->createProcessArray<uint8_t>(controlSystemToDevice, "uint8Scalar", 1, "mIatrou*Rahm");
		ProcessArray<int16_t>::SharedPtr intA16dev = this->_processVariableManager->createProcessArray<int16_t>(controlSystemToDevice, "int16Scalar", 1);
		ProcessArray<uint16_t>::SharedPtr intAu16dev = this->_processVariableManager->createProcessArray<uint16_t>(controlSystemToDevice, "uint16Scalar", 1);
		ProcessArray<int32_t>::SharedPtr intA32dev = this->_processVariableManager->createProcessArray<int32_t>(controlSystemToDevice, "int32Scalar", 1);
		ProcessArray<uint32_t>::SharedPtr intAu32dev = this->_processVariableManager->createProcessArray<uint32_t>(controlSystemToDevice, "uint32Scalar", 1);
		ProcessArray<float>::SharedPtr intAfdev = this->_processVariableManager->createProcessArray<float>(controlSystemToDevice, "floatScalar", 1);
		ProcessArray<double>::SharedPtr intAddev = this->_processVariableManager->createProcessArray<double>(controlSystemToDevice, "doubleScalar", 1);
		
		ProcessArray<int8_t>::SharedPtr intB15A8dev = this->_processVariableManager->createProcessArray<int8_t>(controlSystemToDevice, "int8Array_s15", 15, "mIatrou*Rahm", "Die Einheit ist essentiel und sollte SI Einheit sein...");
		ProcessArray<uint8_t>::SharedPtr intB10Au8dev = this->_processVariableManager->createProcessArray<uint8_t>(controlSystemToDevice, "uint8Array_s10", 10, "1/Rahm");
		ProcessArray<int16_t>::SharedPtr intB15A16dev = this->_processVariableManager->createProcessArray<int16_t>(controlSystemToDevice, "int16Array_s15", 15, "Iatrou", "Beschreibung eines Iatrous");
		ProcessArray<uint16_t>::SharedPtr intB10Au16dev = this->_processVariableManager->createProcessArray<uint16_t>(controlSystemToDevice, "uint16Array_s10", 10);
		ProcessArray<int32_t>::SharedPtr intB15A32dev = this->_processVariableManager->createProcessArray<int32_t>(controlSystemToDevice, "int32Array_s15", 15);
		ProcessArray<uint32_t>::SharedPtr intB10Au32dev = this->_processVariableManager->createProcessArray<uint32_t>(controlSystemToDevice, "uint32Array_s10", 10);
		ProcessArray<double>::SharedPtr intB15Afdev = this->_processVariableManager->createProcessArray<double>(controlSystemToDevice, "doubleArray_s15", 15);
		ProcessArray<float>::SharedPtr intB10Addev = this->_processVariableManager->createProcessArray<float>(controlSystemToDevice, "floatArray_s10", 10);	
		
		// example mapping variable
		ProcessArray<int8_t>::SharedPtr intB15A8devMap = devManager->createProcessArray<int8_t>(controlSystemToDevice, "Mein/Name_ist#int8Array_s15", 15, "Iatrou^2/Rahm");
		ProcessArray<uint8_t>::SharedPtr intB10Au8devMap1 = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "/Dein/Name//ist/uint8Array_s10", 10, "Iatrou^2/Rahm", "Beschreibung");
		ProcessArray<uint8_t>::SharedPtr intB10Au8devMap2 = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "Unser/Name/ist_uint8Array_s10", 10);
		ProcessArray<uint32_t>::SharedPtr intAu32devMap = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "Ist/Name/dieser/uint32Scalar", 1);
		ProcessArray<int32_t>::SharedPtr intA32devMap = devManager->createProcessArray<int32_t>(controlSystemToDevice, "Ist/Name/dieser/int32Scalar", 1);
		ProcessArray<double>::SharedPtr doubledevMap = devManager->createProcessArray<double>(controlSystemToDevice, "Ist/Name/dieser/doubleScalar", 1);
	}
};

MyApp myApp;