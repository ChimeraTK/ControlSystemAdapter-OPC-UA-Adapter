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

#include "csa_opcua_adapter.h"

#include <iostream>
#include <math.h>
#include <typeinfo>       // std::bad_cast

#include "ipc_manager.h"
#include "ua_adapter.h"
#include "ua_processvariable.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ApplicationBase.h"

csa_opcua_adapter::csa_opcua_adapter(boost::shared_ptr<ControlSystemPVManager> csManager, string configFile) {
	
	this->csManager = csManager; 	
	this->csa_opcua_adapter_InitServer(configFile);
	this->csa_opcua_adapter_InitVarMapping();
	
}

void csa_opcua_adapter::csa_opcua_adapter_InitServer(string configFile) {
	this->mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

	// Create new server adapter
	this->adapter = new ua_uaadapter(configFile);
	this->mgr->addObject(this->adapter);
	this->mgr->doStart(); // Implicit: Startet Worker-Threads aller ipc_managed_objects, die mit addObject registriert wurden  
}

void csa_opcua_adapter::csa_opcua_adapter_InitVarMapping() {
    // Get all ProcessVariables
    vector<ProcessVariable::SharedPtr> allProcessVariables = this->csManager->getAllProcessVariables();
  
    for(ProcessVariable::SharedPtr oneProcessVariable : allProcessVariables) {
        adapter->addVariable(oneProcessVariable->getName(), this->csManager);
    }
    
    vector<string> allNotMappedVariables = adapter->getAllNotMappableVariablesNames();
		if(allNotMappedVariables.size() > 0) {
			cout << "The following VariableNodes cant be mapped, because they are not member in PV-Manager:" << endl;
			for(string var:allNotMappedVariables) {
				cout << var << endl;
			}
		}
}

csa_opcua_adapter::~csa_opcua_adapter() {
	
	//free(this->mgr);
	//delete this->mgr;
	//this->mgr = NULL;
	this->adapter->~ua_uaadapter();
	//free(this->adapter);	
	//delete this->adapter;
	//this->adapter = NULL;
	
	this->mgr->~ipc_manager();
}

boost::shared_ptr<ControlSystemPVManager> const & csa_opcua_adapter::getControlSystemManager() const {
    return this->csManager;
}

ua_uaadapter* csa_opcua_adapter::getUAAdapter() {
	return this->adapter;
}

ipc_manager* csa_opcua_adapter::getIPCManager() {
    return this->mgr;
}

void csa_opcua_adapter::start() {
    this->mgr->startAll();
    return;
}

void csa_opcua_adapter::stop() {
    this->mgr->stopAll();
    return;
}

void csa_opcua_adapter::terminate() {
    this->mgr->terminate();
    return;
}

bool csa_opcua_adapter::isRunning() {
    return this->mgr->isRunning();
}