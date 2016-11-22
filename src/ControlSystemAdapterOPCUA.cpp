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

/**
* @mainpage Example program
*
* Description of the project
* You can force new lines in the HTML documentation with <br>
*
* With the <img-keyword you can add own pictures
* <img src="../images/application_screenshot.jpg" alt="Screenshot">
*
* @author Jack Pot
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
 */ 
ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA(shCSysPVManager csManager, string configFile) {
	this->configId = configId;
	this->csManager = csManager;
	this->ControlSystemAdapterOPCUA_InitServer(configFile);
	this->ControlSystemAdapterOPCUA_InitVarMapping(csManager);
}

// Init the OPCUA server
void ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA_InitServer(string configFile) {
    this->mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

    // Create new server adapter
     this->adapter = new mtca_uaadapter(configFile);
  
    this->mgr->addObject(this->adapter);
    this->mgr->doStart(); // Implicit: Startet Worker-Threads aller ipc_managed_objects, die mit addObject registriert wurden  
}

// Mapping the ProcessVariables to the server
void ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA_InitVarMapping(shCSysPVManager csManager) {
    // Get all ProcessVariables
    vector<ProcessVariable::SharedPtr> allProcessVariables = csManager->getAllProcessVariables();
  
    for(ProcessVariable::SharedPtr oneProcessVariable : allProcessVariables) {
        adapter->addVariable(oneProcessVariable->getName(), csManager);
    }
    
    vector<string> allNotMappedVariables = adapter->getAllNotMappableVariablesNames();
		cout << "The following VariableNodes cant be mapped, because they are not member in the PV-Manager:" << endl;
		for(string var:allNotMappedVariables) {
			cout << var << endl;
		}
}

ControlSystemAdapterOPCUA::~ControlSystemAdapterOPCUA() {
	this->mgr->~ipc_manager();
	free(this->mgr);
	this->adapter->~mtca_uaadapter();
	free(this->adapter);
}


// Maybe needed, return the ControlSystemPVManager
shCSysPVManager const & ControlSystemAdapterOPCUA::getControlSystemPVManager() const {
    return this->csManager;
}

mtca_uaadapter* ControlSystemAdapterOPCUA::getUAAdapter() {
	return this->adapter;
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