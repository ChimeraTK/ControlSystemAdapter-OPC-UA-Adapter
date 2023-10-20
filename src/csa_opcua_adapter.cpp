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
 * Copyright (c) 2023 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
 */

extern "C" {
#include "unistd.h"
}

#include "csa_opcua_adapter.h"
#include "csa_processvariable.h"
#include "ua_adapter.h"

#include <iostream>
#include <utility>

csa_opcua_adapter::csa_opcua_adapter(boost::shared_ptr<ControlSystemPVManager> csManager, string configFile) {
  this->csManager = std::move(csManager);

  // Create new server adapter
  this->adapter = new ua_uaadapter(std::move(configFile));

  // Pre-Processing - get list of PV's to skip excluding
  vector<string> mappedPvSources = adapter->getAllMappedPvSourceNames();
  for(const auto& f : mappedPvSources) {
    cout << f << endl;
  }

  // Initialize the process variables
  // This internally starts the managed threads in the mgr...
  vector<ProcessVariable::SharedPtr> allProcessVariables = this->csManager->getAllProcessVariables();

  vector<string> unusedVariables;

  // start implicit var mapping
  bool skip_var = false;
  for(const ProcessVariable::SharedPtr& oneProcessVariable : allProcessVariables) {
    //cout << "Info: " << oneProcessVariable->getName() << endl;
    for(auto e : this->adapter->exclude){
      string suffix_1 = "/*", suffix_2 ="*";
      if(e.rfind(suffix_1) == e.size() - suffix_1.size()){
        if(oneProcessVariable->getName().rfind(e.substr(0, e.size()-1)) == 0){
          if (std::find(mappedPvSources.begin(), mappedPvSources.end(), e.substr(1, e.size() - 1)) != mappedPvSources.end()){
            cout << "Warning: Skip exclude node - Used in pv-mapping" << endl;
          } else {
            cout << "Info: exclude var (/*) from mapping " << oneProcessVariable->getName() << endl;
            skip_var = true;
          }
        }
      } else if(e.rfind(suffix_2) == e.size() - suffix_2.size()){
        if(oneProcessVariable->getName().rfind(e.substr(0, e.size()-1), 0) == 0){
          if (std::find(mappedPvSources.begin(), mappedPvSources.end(), e.substr(1, e.size() - 1)) != mappedPvSources.end()){
            cout << "Warning: Skip exclude node - Used in pv-mapping" << endl;
          } else {
            cout << "Info: exclude var (/) from mapping " << oneProcessVariable->getName() << endl;
            skip_var = true;
          }
        }
      } else if(oneProcessVariable->getName() == e){
           //check if this node is mapped later
          if (std::find(mappedPvSources.begin(), mappedPvSources.end(), e.substr(1, e.size() - 1)) != mappedPvSources.end()){
            cout << "Warning: Skip exclude node - Used in pv-mapping" << endl; //Print namen
          } else {
            cout << "Info: exclude var (direct match) from mapping " << oneProcessVariable->getName() << endl;
            skip_var = true;
          }
      }

    }
    if(!skip_var){
        adapter->implicitVarMapping(oneProcessVariable->getName(), this->csManager);
    } else {
        //variable is skipped and therefore unused
        this->unusedVariables.push_back(oneProcessVariable->getName());
        skip_var = false;
    }
  }

  adapter->applyMapping(this->csManager);

  vector<string> allNotMappedVariables = adapter->getAllNotMappableVariablesNames();
  if(!allNotMappedVariables.empty()) {
    cout << "The following VariableNodes cant be mapped, because they are not member in PV-Manager:" << endl;
    for(const string& var : allNotMappedVariables) {
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
  if(!this->adapter_thread.joinable()) {
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
