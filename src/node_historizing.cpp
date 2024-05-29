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
 * Copyright (c) 2023-2024 Fraunhofer IOSB (Author: Florian Düwel)
 */

#include "node_historizing.h"

#include "open62541/plugin/log_stdout.h"

namespace ChimeraTK {
  typedef struct {
    UA_Server* server;
    string history;
    vector<UA_NodeId>* historizing_nodes;
    vector<string>* historizing_setup;
  } HandleFolderVariables;

  UA_StatusCode get_child_variables(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void* handle) {
    if(isInverse) return UA_STATUSCODE_GOOD;
    auto* handler = (HandleFolderVariables*)handle;
    UA_NodeId organizes = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    // get variables of nested folders
    if(UA_NodeId_equal(&referenceTypeId, &organizes)) {
      UA_Server_forEachChildNodeCall(handler->server, childId, get_child_variables, handler);
    }
    UA_NodeClass outNodeClass;
    UA_NodeClass_init(&outNodeClass);
    UA_Server_readNodeClass(handler->server, childId, &outNodeClass);
    if(outNodeClass == UA_NODECLASS_VARIABLE) {
      UA_NodeId* temp = UA_NodeId_new();
      UA_NodeId_copy(&childId, temp);
      handler->historizing_nodes->insert(handler->historizing_nodes->end(), *temp);
      handler->historizing_setup->insert(handler->historizing_setup->end(), handler->history);
    }
    return UA_STATUSCODE_GOOD;
  }

  void add_folder_historizing(vector<UA_NodeId>* historizing_nodes, vector<string>* historizing_setup,
      vector<AdapterFolderHistorySetup> historyfolders, UA_Server* mappedServer, UA_ServerConfig* server_config) {
    for(size_t i = 0; i < historyfolders.size(); i++) {
      UA_NodeId* temp = UA_NodeId_new();
      UA_StatusCode retval = UA_Server_readNodeId(mappedServer, historyfolders[i].folder_id, temp);
      UA_NodeId_clear(temp);
      if(retval == UA_STATUSCODE_GOOD) {
        HandleFolderVariables handle;
        handle.server = mappedServer;
        handle.history = historyfolders[i].folder_historizing;
        handle.historizing_nodes = historizing_nodes;
        handle.historizing_setup = historizing_setup;
        UA_Server_forEachChildNodeCall(mappedServer, historyfolders[i].folder_id, get_child_variables, &handle);
      }
      else {
        UA_LOG_WARNING(server_config->logging, UA_LOGCATEGORY_USERLAND,
            "Warning! Folder is not mapped in the server. StatusCode: %s ", UA_StatusCode_name(retval));
      }
    }
  }

  void add_variable_historizing(vector<UA_NodeId>* historizing_nodes, vector<string>* historizing_setup,
      vector<AdapterPVHistorySetup> historyvariables, UA_Server* mappedServer, UA_ServerConfig* server_config) {
    for(size_t i = 0; i < historyvariables.size(); i++) {
      UA_NodeId* temp = UA_NodeId_new();
      UA_NodeId id = historyvariables[i].variable_id;
      UA_StatusCode retval = UA_Server_readNodeId(mappedServer, id, temp);
      if(retval == UA_STATUSCODE_GOOD) {
        UA_String out = UA_STRING_NULL;
        UA_print(&id, &UA_TYPES[UA_TYPES_NODEID], &out);
        UA_LOG_INFO(server_config->logging, UA_LOGCATEGORY_USERLAND, "add Node %.*s ", (int)out.length, out.data);
        UA_String_clear(&out);
        historizing_nodes->insert(historizing_nodes->end(), historyvariables[i].variable_id);
        historizing_setup->insert(historizing_setup->end(), historyvariables[i].variable_historizing);
      }
      else {
        UA_String out = UA_STRING_NULL;
        UA_print(&id, &UA_TYPES[UA_TYPES_NODEID], &out);
        UA_LOG_WARNING(server_config->logging, UA_LOGCATEGORY_USERLAND,
            "Warning! Node %.*s has a history configuration but is not mapped to the server. StatusCode: %s ",
            (int)out.length, out.data, UA_StatusCode_name(retval));
        UA_String_clear(&out);
      }
      UA_NodeId_clear(temp);
    }
  }

  void set_variable_access_level_historizing(UA_NodeId id, UA_Server* mappedServer) {
    UA_Byte temp;
    UA_Byte_init(&temp);
    UA_Server_readAccessLevel(mappedServer, id, &temp);
    if(temp & UA_ACCESSLEVELMASK_READ) {
      temp |= UA_ACCESSLEVELMASK_HISTORYREAD;
      UA_Server_writeAccessLevel(mappedServer, id, temp);
    }
    UA_Server_writeHistorizing(mappedServer, id, true);
    UA_Byte_clear(&temp);
  }

  // avoid multiple setups for one node
  void check_historizing_nodes(vector<UA_NodeId>& historizing_nodes, vector<string>& historizing_setup) {
    bool repeat = true;
    /*size_t position = 0;*/
    while(repeat) {
      repeat = false;
      for(size_t i = /*position*/ 0; i < historizing_nodes.size(); i++) {
        for(size_t j = i + 1; j < historizing_nodes.size(); j++) {
          if(UA_NodeId_equal(reinterpret_cast<UA_NodeId*>(&historizing_nodes[i]),
                 reinterpret_cast<UA_NodeId*>(&historizing_nodes[j]))) {
            UA_String out = UA_STRING_NULL;
            UA_print(reinterpret_cast<UA_NodeId*>(&historizing_nodes[j]), &UA_TYPES[UA_TYPES_NODEID], &out);
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Warning! Node %.*s has multiple history settings.",
                (int)out.length, out.data);
            UA_String_clear(&out);
            UA_NodeId_clear(reinterpret_cast<UA_NodeId*>(&historizing_nodes[j]));
            historizing_nodes.erase(historizing_nodes.begin() + j);
            historizing_setup.erase(historizing_setup.begin() + j);
            repeat = true;
            /*position = i;*/
            break;
          }
        }
      }
    }
  }

  static void remove_nodes_with_incomplete_historizing_setup(
      vector<UA_NodeId>& historizing_nodes, vector<string>& historizing_setup, vector<AdapterHistorySetup> history) {
    bool repeat = true;
    size_t position = 0;
    while(repeat) {
      repeat = false;
      for(size_t i = position; i < historizing_nodes.size(); i++) {
        bool found = false;
        for(auto& j : history) {
          if(historizing_setup[i] == j.name) {
            found = true;
            break;
          }
        }
        if(!found) {
          UA_String out = UA_STRING_NULL;
          UA_print(reinterpret_cast<UA_NodeId*>(&historizing_nodes[i]), &UA_TYPES[UA_TYPES_NODEID], &out);
          UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
              "Warning! Remove node %.*s from historizing because the setup %s  is missing.", (int)out.length, out.data,
              historizing_setup[i].c_str());
          UA_String_clear(&out);
          // UA_NodeId_clear(&historizing_nodes[j]);
          historizing_nodes.erase(historizing_nodes.begin() + i);
          historizing_setup.erase(historizing_setup.begin() + i);
          repeat = true;
          position = i;
          break;
        }
      }
    }
  }

  UA_HistoryDataGathering add_historizing_nodes(vector<UA_NodeId>& historizing_nodes, vector<string>& historizing_setup,
      UA_Server* mappedServer, UA_ServerConfig* server_config, vector<AdapterHistorySetup> history,
      vector<AdapterFolderHistorySetup> historyfolders, vector<AdapterPVHistorySetup> historyvariables) {
    add_variable_historizing(&historizing_nodes, &historizing_setup, historyvariables, mappedServer, server_config);
    add_folder_historizing(&historizing_nodes, &historizing_setup, historyfolders, mappedServer, server_config);
    check_historizing_nodes(historizing_nodes, historizing_setup);
    // search nodes with incomplete history config
    remove_nodes_with_incomplete_historizing_setup(historizing_nodes, historizing_setup, history);
    UA_HistoryDataGathering gathering = UA_HistoryDataGathering_Default(historizing_nodes.size());
    server_config->historyDatabase = UA_HistoryDatabase_default(gathering);
    for(size_t i = 0; i < historizing_nodes.size(); i++) {
      UA_HistorizingNodeIdSettings setting;
      setting.historizingUpdateStrategy = UA_HISTORIZINGUPDATESTRATEGY_POLL;
      AdapterHistorySetup hist;
      for(auto& j : history) {
        if(historizing_setup[i] == j.name) {
          hist = j;
        }
      }
      set_variable_access_level_historizing(historizing_nodes[i], mappedServer);
      setting.historizingBackend = UA_HistoryDataBackend_Memory_Circular(historizing_nodes.size(), hist.buffer_length);
      setting.maxHistoryDataResponseSize = hist.entries_per_response;
      setting.pollingInterval = hist.interval;
      UA_StatusCode retval = gathering.registerNodeId(mappedServer, gathering.context, &historizing_nodes[i], setting);
      if(retval != UA_STATUSCODE_GOOD) {
        UA_String out = UA_STRING_NULL;
        UA_print(&historizing_nodes[i], &UA_TYPES[UA_TYPES_NODEID], &out);
        UA_LOG_WARNING(server_config->logging, UA_LOGCATEGORY_USERLAND,
            "Failed to add historizing for Node %.*s with StatusCode %s", (int)out.length, out.data,
            UA_StatusCode_name(retval));
        UA_String_clear(&out);
      }
      retval = gathering.startPoll(mappedServer, gathering.context, &historizing_nodes[i]);
      if(retval != UA_STATUSCODE_GOOD) {
        UA_String out = UA_STRING_NULL;
        UA_print(&historizing_nodes[i], &UA_TYPES[UA_TYPES_NODEID], &out);
        UA_LOG_WARNING(server_config->logging, UA_LOGCATEGORY_USERLAND,
            "Failed to start the poll for Node %.*s with StatusCode %s", (int)out.length, out.data,
            UA_StatusCode_name(retval));
        UA_String_clear(&out);
      }
    }
    return gathering;
  }

  void clear_history(UA_HistoryDataGathering gathering, vector<UA_NodeId>& historizing_nodes,
      vector<string>& historizing_setup, UA_Server* mappedServer, vector<AdapterFolderHistorySetup> historyfolders,
      vector<AdapterPVHistorySetup> historyvariables) {
    // stop the poll for all historizing variables
    for(auto& historizing_node : historizing_nodes) {
      UA_StatusCode retval = gathering.stopPoll(mappedServer, gathering.context, &historizing_node);
      UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "stopPoll %s", UA_StatusCode_name(retval));
    }
    // clear the list of valid historizing nodes
    historizing_nodes.clear();
    historizing_setup.clear();
    // clear the nodeis lists (variables + server from the xml config)
    for(auto& historyfolder : historyfolders) {
      UA_NodeId_clear(&historyfolder.folder_id);
    }
    for(auto& historyvariable : historyvariables) {
      UA_NodeId_clear(&historyvariable.variable_id);
    }
  }
} // namespace ChimeraTK
