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
#include "unistd.h"
}

#include "ControlSystemAdapterOPCUA.h"

#include <iostream>
#include <math.h>
#include <typeinfo>       // std::bad_cast

#include "ipc_manager.h"

#include "mtca_uaadapter.h"
#include "mtca_processvariable.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"

/**
 * This class provide the two parts of the OPCUA Adapter. First of all the OPCUA server starts with a random port number (recommended 16664),
 * following the mapping process start. For this, the ProcessVariable from ControlSystemPVManager will be mapped to the OPCUA Model 
 * 
 */ 
ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA(uint16_t opcuaPort, shCSysPVManager csManager) {
	this->opcuaPort = opcuaPort;
	this->csManager = csManager;
    this->ControlSystemAdapterOPCUA_InitServer(opcuaPort);
    this->ControlSystemAdapterOPCUA_InitVarMapping(csManager);
}

// Init the OPCUA server
void ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA_InitServer(uint16_t opcuaPort) {
    this->mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

    // Create new server adapter
    this->adapter = new mtca_uaadapter(opcuaPort);
  
    this->mgr->addObject(adapter);
    this->mgr->doStart(); // Implicit: Startet Worker-Threads aller ipc_managed_objects, die mit addObject registriert wurden  
}

// Mapping the ProcessVariables to the server
void ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA_InitVarMapping(shCSysPVManager csManager) {
    // Get all ProcessVariables
    vector<ProcessVariable::SharedPtr> allProcessVariables = csManager->getAllProcessVariables();
  
    for(ProcessVariable::SharedPtr oneProcessVariable : allProcessVariables) {
        adapter->addVariable(oneProcessVariable->getName(), csManager);
    }
}
    
void ControlSystemAdapterOPCUA::updateOPCUAValues() {
	for(mtca_processvariable *oneMTCA_ProcessVariable: this->adapter->getVariables()) {
		for(ProcessVariable::SharedPtr oneProcessVariable: csManager->getAllProcessVariables()) {
			if(oneProcessVariable->getName().compare(oneMTCA_ProcessVariable->getName()) == 0) {
				std::cout << "Name: " << oneProcessVariable->getName() << std::endl;
			}
		}		
	}	
}

// Maybe needed, return the ControlSystemPVManager
shCSysPVManager const & ControlSystemAdapterOPCUA::getControlSystemPVManager() const {
    return this->csManager;
}

mtca_uaadapter* ControlSystemAdapterOPCUA::getUAAdapter() {
	return this->adapter;
}

// Maybe needed, return the OPCUA Portnumber
uint32_t ControlSystemAdapterOPCUA::getOPCUAPort() {
	return this->opcuaPort;
}

ipc_manager* ControlSystemAdapterOPCUA::getIPCManager() {
    return this->mgr;
}


void ControlSystemAdapterOPCUA::start() {
    this->mgr->startAll();
    return;
}

void ControlSystemAdapterOPCUA::stop() {
    this->mgr->stopAll();
    return;
}

void ControlSystemAdapterOPCUA::terminate() {
    this->mgr->terminate();
    return;
}

bool ControlSystemAdapterOPCUA::isRunning() {
    return this->mgr->isRunning();
}