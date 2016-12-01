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

#include "runtimeValueGenerator.h"

#include <iostream>
#include <math.h>
#include <sys/sysinfo.h> 
#include <unistd.h>

#include "open62541.h"
#include "ControlSystemAdapterOPCUA.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ipc_manager.h"

using std::cout;
using std::endl;
using namespace ChimeraTK;

typedef boost::shared_ptr<ControlSystemPVManager> shCSysPVManager;
	 
runtimeValueGenerator::runtimeValueGenerator(shCSysPVManager csManager) {
	this->csManager = csManager;
	this->doStart();
}

runtimeValueGenerator::~runtimeValueGenerator() {
	if (this->isRunning()) {
		this->doStop();
	}
}

void runtimeValueGenerator::generateValues(shCSysPVManager csManager) {
	// Time meassureing
	clock_t start, end;
	start = clock();
	end = clock();
	csManager->getProcessArray<int32_t>("t")->set(vector<int32_t> {(int32_t)start});
	
	while(true) {
//  FIXME -Or maybe not: The Const M_PI from math.h generate senceless values, hence I use fix value 3.141
// 	double double_sine = csManager->getProcessScalar<double>("amplitude")->get() * sin(((2*M_PI)/csManager->getProcessScalar<int32_t>("period")->get()) * csManager->getProcessScalar<int32_t>("t")->get());
		double double_sine = csManager->getProcessArray<double>("amplitude")->get().at(0) * sin((2*3.141)/csManager->getProcessArray<int32_t>("period")->get().at(0) * csManager->getProcessArray<int32_t>("t")->get().at(0));
		int32_t int_sine = round(double_sine);
		bool bool_sine = (double_sine > 0)? true : false;
		
// 	std::cout << "double_sine: " << double_sine << std::endl;
// 	std::cout << "int_sine: " << int_sine << std::endl;
// 	std::cout << "bool_sine: " << bool_sine << std::endl;
			
		csManager->getProcessArray<double>("double_sine")->set(vector<double> {double_sine});
		csManager->getProcessArray<int32_t>("int_sine")->set(vector<int32_t> {int_sine}); 			
		csManager->getProcessArray<int32_t>("t")->set(vector<int32_t> {(int32_t)((end - start)/(CLOCKS_PER_SEC/1000))});	
		
		usleep(csManager->getProcessArray<int32_t>("dt")->get().at(0));
		end = clock();
	}
}

void runtimeValueGenerator::workerThread() {
    bool run = true;
	
    thread *valueGeneratorThread = new std::thread(generateValues, this->csManager);

    while (run == true) {
        if (! this->isRunning()) {
					run = false;
        }
        sleep(1);
    }
    
    if (valueGeneratorThread->joinable()) {
		valueGeneratorThread->join();
	}
	delete valueGeneratorThread;
}


