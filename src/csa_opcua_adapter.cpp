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
#include "void_type.h"

csa_opcua_adapter::csa_opcua_adapter(boost::shared_ptr<ControlSystemPVManager> csManager, string configFile) {
  this->csManager = std::move(csManager);

  // Create new server adapter
  this->adapter = new ua_uaadapter(std::move(configFile));

  // Pre-Processing - get list of PV's to skip excluding
  vector<string> mappedPvSources = adapter->getAllMappedPvSourceNames();
  cout << "List of mapped process variables: " << endl;
  for(const auto& f : mappedPvSources) {
    cout << "\t" << f << endl;
  }

  vector<ProcessVariable::SharedPtr> allProcessVariables = this->csManager->getAllProcessVariables();

  // start implicit var mapping
  bool skip_var = false;
  for(const ProcessVariable::SharedPtr& oneProcessVariable : allProcessVariables) {
    //cout << "Info: " << oneProcessVariable->getName() << endl;
    for(auto e : this->adapter->exclude){
      string suffix_1 = "/*", suffix_2 ="*";
      if(e.rfind(suffix_1) == e.size() - suffix_1.size()){
        if(oneProcessVariable->getName().rfind(e.substr(0, e.size()-1)) == 0){
          bool pv_used = false;
          for(const auto& ele: mappedPvSources){
            if(ele == oneProcessVariable->getName().substr(1, oneProcessVariable->getName().size()-1)){
              cout << "Warning: Skip exclude node - Used in pv-mapping (directory match) PV:" << oneProcessVariable->getName() << endl;
              pv_used = true;
              break;
            }
          }
          for(auto folder :this->adapter->folder_with_history) {
            if(oneProcessVariable->getName().substr(1, folder.size()-1) == folder){
              cout << "Warning: Skip exclude node - Used in folder history setting:" << oneProcessVariable->getName() << endl;
              pv_used = true;
              break;
            }
          }
          if(!pv_used){
            cout << "Info: directory var exclude (/*) from mapping " << oneProcessVariable->getName() << endl;
            skip_var = true;
          }
        }
      } else if(e.rfind(suffix_2) == e.size() - suffix_2.size()){
        if(oneProcessVariable->getName().rfind(e.substr(0, e.size()-1), 0) == 0){
          bool pv_used = false;
          for(const auto& ele: mappedPvSources){
            if(ele == oneProcessVariable->getName().substr(1, oneProcessVariable->getName().size()-1)){
              cout << "Warning: Skip exclude node - Used in pv-mapping (greedy match) PV:" << oneProcessVariable->getName() << endl;
              pv_used = true;
              break;
            }
          }
          for(auto folder :this->adapter->folder_with_history) {
            if(oneProcessVariable->getName().substr(1, folder.size()-1) == folder){
              cout << "Warning: Skip exclude node - Used in folder history setting:" << oneProcessVariable->getName() << endl;
              pv_used = true;
              break;
            }
          }
          if(!pv_used){
            cout << "Info: greedy var exclude (*) from mapping " << oneProcessVariable->getName() << endl;
            skip_var = true;
          }
        }
      } else if(oneProcessVariable->getName() == e){
           //check if this node is mapped later
          if (std::find(mappedPvSources.begin(), mappedPvSources.end(), e.substr(1, e.size() - 1)) != mappedPvSources.end()){
            cout << "Warning: Skip exclude node - Used in pv-mapping (direct match) PV:" << oneProcessVariable->getName() << endl; //Print namen
          } else {
            cout << "Info: direct exclude var (direct match) from mapping" << oneProcessVariable->getName() << endl;
            skip_var = true;
          }
          for(auto folder :this->adapter->folder_with_history) {
            if(oneProcessVariable->getName().substr(1, folder.size()-1) == folder){
              cout << "Warning: Skip exclude node - Used in folder history setting:" << oneProcessVariable->getName() << endl;
              skip_var = false;
              break;
            }
          }
      }
    }
    if(!skip_var){
        adapter->implicitVarMapping(oneProcessVariable->getName(), this->csManager);
    } else {
        //variable is skipped and therefore unused
        this->unusedVariables.insert(oneProcessVariable->getName());
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
  //start void observer loop
  cout << "Start the void observer thread" << endl;
  void_observer_data *data  =  (void_observer_data*) UA_calloc(1, sizeof(void_observer_data));
  vector<ua_processvariable*> vars = adapter->getVariables();
  auto conf = adapter->get_server_config();
  data->rootFolder = conf.rootFolder;
  data->adapter = this;
  cout << "application_name: " << data->rootFolder << endl;
  for(auto & variable : vars){
    auto type = variable->getType();
    if(type == "Void"){
      data->pvs.insert(data->pvs.end(), variable->getName());
    }
  }
  cout << "Nbr void: " << data->pvs.size() << endl;
  if(!data->pvs.empty()){
    data->mappedServer = adapter->getMappedServer();
    data->csManager = csManager;
    std::thread observer_thread(&startVoidObserverThread, data);
    observer_thread.detach();
  }
  else{
    UA_free(data);
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
const set<string>& csa_opcua_adapter::getUnusedVariables() const {
  return unusedVariables;
}
