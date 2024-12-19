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
#include "void_type.h"

#include <iostream>
#include <utility>

namespace ChimeraTK {
  csa_opcua_adapter::csa_opcua_adapter(boost::shared_ptr<ControlSystemPVManager> csManager, string configFile) {
    this->csManager = std::move(csManager);

    // Create new server adapter
    adapter = std::make_shared<ua_uaadapter>(std::move(configFile));
    UA_LOG_INFO(this->getLogger(), UA_LOGCATEGORY_USERLAND, "Start the mapping of %s", configFile.c_str());
    UA_LOG_INFO(this->getLogger(), UA_LOGCATEGORY_USERLAND, "Create the adapter");

    // Pre-Processing - get list of PV's to skip excluding
    vector<string> mappedPvSources = adapter->getAllMappedPvSourceNames();
    if(mappedPvSources.size() > 0) {
      std::stringstream ss;
      for(const auto& f : mappedPvSources) {
        ss << "\n\t\t - " << f;
      }
      UA_LOG_INFO(this->getLogger(), UA_LOGCATEGORY_USERLAND, "List of mapped process variables: %s", ss.str().c_str());
    }

    vector<ProcessVariable::SharedPtr> allProcessVariables = this->csManager->getAllProcessVariables();

    // start implicit var mapping
    bool skip_var = false;
    for(const ProcessVariable::SharedPtr& oneProcessVariable : allProcessVariables) {
      std::type_info const& valueType = oneProcessVariable->getValueType();
      if(valueType == typeid(Void)) {
        /*skip_var = true;*/
        UA_LOG_DEBUG(this->getLogger(), UA_LOGCATEGORY_USERLAND, "Skip Variable: Variable: %s has a void type",
            oneProcessVariable->getName().c_str());
        continue;
      }
      for(auto e : this->adapter->exclude) {
        string suffix_1 = "/*", suffix_2 = "*";
        if(e.rfind(suffix_1) == e.size() - suffix_1.size()) {
          if(oneProcessVariable->getName().rfind(e.substr(0, e.size() - 1)) == 0) {
            bool pv_used = false;
            for(const auto& ele : mappedPvSources) {
              if(ele == oneProcessVariable->getName().substr(1, oneProcessVariable->getName().size() - 1)) {
                UA_LOG_WARNING(this->getLogger(), UA_LOGCATEGORY_USERLAND,
                    "Skip exclude node - Used in pv-mapping (directory match) PV: %s",
                    oneProcessVariable->getName().c_str());
                pv_used = true;
                break;
              }
            }
            if(!pv_used) {
              for(auto folder : this->adapter->folder_with_history) {
                if(oneProcessVariable->getName().substr(0, folder.size()) == folder) {
                  UA_LOG_WARNING(this->getLogger(), UA_LOGCATEGORY_USERLAND,
                      "Skip exclude node - Used in folder history setting: %s", oneProcessVariable->getName().c_str());
                  pv_used = true;
                  break;
                }
              }
            }
            if(!pv_used) {
              UA_LOG_INFO(this->getLogger(), UA_LOGCATEGORY_USERLAND, "Directory var exclude (/*) from mapping %s",
                  oneProcessVariable->getName().c_str());
              skip_var = true;
            }
          }
        }
        else if(e.rfind(suffix_2) == e.size() - suffix_2.size()) {
          if(oneProcessVariable->getName().rfind(e.substr(0, e.size() - 1), 0) == 0) {
            bool pv_used = false;
            for(const auto& ele : mappedPvSources) {
              if(ele == oneProcessVariable->getName().substr(1, oneProcessVariable->getName().size() - 1)) {
                UA_LOG_WARNING(this->getLogger(), UA_LOGCATEGORY_USERLAND,
                    "Skip exclude node - Used in pv-mapping (greedy match) PV: %s",
                    oneProcessVariable->getName().c_str());
                pv_used = true;
                break;
              }
            }
            for(auto folder : this->adapter->folder_with_history) {
              if(oneProcessVariable->getName().substr(1, folder.size() - 1) == folder) {
                UA_LOG_WARNING(this->getLogger(), UA_LOGCATEGORY_USERLAND,
                    "Skip exclude node - Used in folder history setting: %s", oneProcessVariable->getName().c_str());
                pv_used = true;
                break;
              }
            }
            if(!pv_used) {
              UA_LOG_INFO(this->getLogger(), UA_LOGCATEGORY_USERLAND, "Greedy var exclude (*) from mapping %s",
                  oneProcessVariable->getName().c_str());
              skip_var = true;
            }
          }
        }
        else if(oneProcessVariable->getName() == e) {
          // check if this node is mapped later
          if(std::find(mappedPvSources.begin(), mappedPvSources.end(), e.substr(1, e.size() - 1)) !=
              mappedPvSources.end()) {
            UA_LOG_WARNING(this->getLogger(), UA_LOGCATEGORY_USERLAND,
                "Skip exclude node - Used in pv-mapping (direct match) PV: %s", oneProcessVariable->getName().c_str());
          }
          else {
            UA_LOG_INFO(this->getLogger(), UA_LOGCATEGORY_USERLAND, "Direct exclude var (direct match) from mapping %s",
                oneProcessVariable->getName().c_str());
            skip_var = true;
          }
          for(auto folder : this->adapter->folder_with_history) {
            if(oneProcessVariable->getName().substr(1, folder.size() - 1) == folder) {
              UA_LOG_WARNING(this->getLogger(), UA_LOGCATEGORY_USERLAND,
                  "Skip exclude node - Used in folder history setting: %s", oneProcessVariable->getName().c_str());
              skip_var = false;
              break;
            }
          }
        }
      }
      if(!skip_var) {
        adapter->implicitVarMapping(oneProcessVariable->getName(), this->csManager);
      }
      else {
        // variable is skipped and therefore unused
        this->unusedVariables.insert(oneProcessVariable->getName());
        skip_var = false;
      }
    }

    adapter->applyMapping(this->csManager);
    vector<string> allNotMappedVariables = adapter->getAllNotMappableVariablesNames();
    if(!allNotMappedVariables.empty()) {
      std::stringstream ss;
      for(const string& var : allNotMappedVariables) {
        ss << "\t" << var << endl;
      }
      UA_LOG_WARNING(this->getLogger(), UA_LOGCATEGORY_USERLAND,
          "The following VariableNodes cant be mapped, because they are not member in PV-Manager: \n%s",
          ss.str().c_str());
    }
  }

  csa_opcua_adapter::~csa_opcua_adapter() {
    this->stop();
  }

  boost::shared_ptr<ControlSystemPVManager> const& csa_opcua_adapter::getControlSystemManager() const {
    return this->csManager;
  }

  std::shared_ptr<ua_uaadapter> csa_opcua_adapter::getUAAdapter() {
    return adapter;
  }

  void csa_opcua_adapter::start() {
    if(!this->adapter_thread.joinable()) {
      this->adapter_thread = std::thread(&ua_uaadapter::workerThread, this->adapter.get());
    }
    // start void observer loop
    void_observer_data* data = (void_observer_data*)UA_calloc(1, sizeof(void_observer_data));
    vector<ProcessVariable::SharedPtr> allProcessVariables = csManager->getAllProcessVariables();
    for(const ProcessVariable::SharedPtr& oneProcessVariable : allProcessVariables) {
      std::type_info const& valueType = oneProcessVariable->getValueType();
      if(valueType == typeid(Void)) {
        // Check if PV is writable - if not assume it is an VoidInput
        if(!oneProcessVariable->isWriteable()) {
          data->pvs.insert(data->pvs.end(), oneProcessVariable->getName());
          UA_LOG_INFO(this->getLogger(), UA_LOGCATEGORY_USERLAND, "Adding variable %s to void thread.",
              oneProcessVariable->getName().c_str());
        }
        else {
          UA_LOG_WARNING(this->getLogger(), UA_LOGCATEGORY_USERLAND,
              "Ignoring Void input %s. Void inputs are not yet supported.", oneProcessVariable->getName().c_str());
        }
      }
    }
    if(!data->pvs.empty()) {
      auto conf = adapter->get_server_config();
      data->rootFolder = conf.rootFolder;
      data->adapter = this;
      data->mappedServer = adapter->getMappedServer();
      data->csManager = csManager;
      this->observer_thread = std::thread(&startVoidObserverThread, data, this->getLogger());
      this->observer_thread.detach();
    }
    else {
      UA_free(data);
    }
  }

  void csa_opcua_adapter::stop() {
    if(this->adapter_thread.joinable()) {
      adapter->running = false;
      this->adapter_thread.join();
    }
    if(this->observer_thread.joinable()) {
      this->observer_thread.join();
    }
  }

  bool csa_opcua_adapter::isRunning() {
    return this->adapter_thread.joinable();
  }
  const set<string>& csa_opcua_adapter::getUnusedVariables() const {
    return unusedVariables;
  }

  const UA_Logger* csa_opcua_adapter::getLogger() {
    return adapter->server_config->logging;
  }
} // namespace ChimeraTK
