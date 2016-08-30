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

#include "runTimeValueGenerator.h"

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

#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif

typedef boost::shared_ptr<ControlSystemPVManager> shCSysPVManager;
	 
runTimeValueGenerator::runTimeValueGenerator(shCSysPVManager csManager) {
	this->csManager = csManager;
	this->doStart();
}

runTimeValueGenerator::runTimeValueGenerator() {
	//this->doStart();
	//this->generateValues();
}

runTimeValueGenerator::~runTimeValueGenerator() {
	if (this->isRunning()) {
		this->doStop();
	}
}

void runTimeValueGenerator::generateValues(shCSysPVManager csManager) {
	unsigned int seconds = 10;
	
	while(true) {
		srand((unsigned)time(NULL));
		int32_t random_variable = rand()%101;
		for(ProcessVariable::SharedPtr oneProcessVar : csManager->getAllProcessVariables()) {	
			int32_t random_variable = rand()%101;
			if(!oneProcessVar->isArray()) {
				std::type_info const & valueType = csManager->getProcessVariable(oneProcessVar->getName())->getValueType();
				if (valueType == typeid(int8_t)) {
					csManager->getProcessScalar<int8_t>(oneProcessVar->getName())->set(random_variable);
				}
				else if (valueType == typeid(uint8_t)) {
					csManager->getProcessScalar<uint8_t>(oneProcessVar->getName())->set(random_variable);
				}
				else if (valueType == typeid(int16_t)) {
					csManager->getProcessScalar<int16_t>(oneProcessVar->getName())->set(random_variable);
				}
				else if (valueType == typeid(uint16_t)) {
					csManager->getProcessScalar<uint16_t>(oneProcessVar->getName())->set(random_variable);
				}
				else if (valueType == typeid(int32_t)) {
					csManager->getProcessScalar<int32_t>(oneProcessVar->getName())->set(random_variable);
				}
				else if (valueType == typeid(uint32_t)) {
					csManager->getProcessScalar<uint32_t>(oneProcessVar->getName())->set(random_variable);
				}
				else if (valueType == typeid(float)) {
					csManager->getProcessScalar<float>(oneProcessVar->getName())->set(random_variable);
				}
				else if (valueType == typeid(double)) {
					csManager->getProcessScalar<double>(oneProcessVar->getName())->set(random_variable);
				}
				else 
					std::cout << "Error occure" << std::endl;
				
				if(oneProcessVar->isSender()) {
					csManager->getProcessVariable(oneProcessVar->getName())->send();
				}
			}
		}
		sleep(seconds);
	}
}

void runTimeValueGenerator::workerThread() {
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


