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
#include "csa_processvariable.h"
#include "ua_adapter.h"

#include <iostream>

csa_opcua_adapter::csa_opcua_adapter(boost::shared_ptr<ControlSystemPVManager> csManager, string configFile) {
  this->csManager = csManager;

  // Create new server adapter
  this->adapter = new ua_uaadapter(configFile);

  // Initialize the process variables
  // This internally starts the managed threads in the mgr...
  vector<ProcessVariable::SharedPtr> allProcessVariables = this->csManager->getAllProcessVariables();

  //start implicit var mapping
  for(ProcessVariable::SharedPtr oneProcessVariable : allProcessVariables) {
    //adapter->addVariable(oneProcessVariable->getName(), this->csManager);
    adapter->implicitVarMapping(oneProcessVariable->getName(), this->csManager);
  }

  adapter->applyMapping(this->csManager);

  vector<string> allNotMappedVariables = adapter->getAllNotMappableVariablesNames();
  if(!allNotMappedVariables.empty()) {
    cout << "The following VariableNodes cant be mapped, because they are not member in PV-Manager:" << endl;
    for(string var : allNotMappedVariables) {
      cout << var << endl;
    }
  }
}

csa_opcua_adapter::~csa_opcua_adapter() {
  this->stop();
  this->adapter->~ua_uaadapter();
}

boost::shared_ptr<ControlSystemPVManager> const& csa_opcua_adapter::getControlSystemManager() const {
  return this->csManager;
}

ua_uaadapter* csa_opcua_adapter::getUAAdapter() {
  return this->adapter;
}

void csa_opcua_adapter::start() {
  if(!this->adapter_thread.joinable()){
    this->adapter_thread = std::thread(&ua_uaadapter::workerThread, this->adapter);
  }
}

void csa_opcua_adapter::stop() {
  if(this->adapter_thread.joinable()) {
    this->adapter->running = false;
    this->adapter_thread.join();
  }
}

bool csa_opcua_adapter::isRunning() {
  return this->adapter_thread.joinable();
}
