/* 
 * This file is part of ChimeraTKs ControlSystem-OPC-UA-Adapter.
 *
 * ChimeraTKs ControlSystem-OPC-UA-Adapter is free software: you can 
 * redistribute it and/or modify it under the terms of the Lesser GNU 
 * General Public License as published by the Free Software Foundation, 
 * either version 3 of the License, or (at your option) any later version.
 *
 * ChimeraTKs ControlSystem-OPC-UA-Adapter is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty ofMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 * See the Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see https://www.gnu.org/licenses/lgpl.html
 * 
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 * Copyright (c) 2016 Julian Rahm  <Julian.Rahm@tu-dresden.de>
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
	
	this->mgr->doStart(); // Implicit: Startet Worker-Threads aller ipc_managed_objects, die mit addObject registriert wurden  
}

void csa_opcua_adapter::csa_opcua_adapter_InitServer(string configFile) {
	this->mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

	// Create new server adapter
	this->adapter = new ua_uaadapter(configFile);
	this->mgr->addObject(this->adapter);
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
	
	this->adapter->~ua_uaadapter();
	
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

bool csa_opcua_adapter::isRunning() {
    return this->mgr->isRunning();
}
