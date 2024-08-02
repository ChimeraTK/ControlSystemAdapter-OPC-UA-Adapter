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

#pragma once

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "csa_additionalvariable.h"
#include "csa_processvariable.h"
#include "ua_mapped_class.h"
#include "xml_file_handler.h"
#include <open62541/plugin/historydata/history_data_backend_memory.h>
#include <open62541/plugin/historydata/history_data_gathering.h>
#include <open62541/plugin/historydata/history_data_gathering_default.h>
#include <open62541/plugin/historydata/history_database_default.h>

#include <vector>

namespace ChimeraTK {
  typedef struct {
    string folder_historizing;
    UA_NodeId folder_id;
  } AdapterFolderHistorySetup;

  typedef struct {
    string variable_historizing;
    UA_NodeId variable_id;
  } AdapterPVHistorySetup;

  typedef struct {
    string name;
    size_t buffer_length{100};
    size_t entries_per_response{100};
    size_t interval{1000};
  } AdapterHistorySetup;

  UA_HistoryDataGathering add_historizing_nodes(vector<UA_NodeId>& historizing_nodes, vector<string>& historizing_setup,
      UA_Server* mappedServer, UA_ServerConfig* server_config, vector<AdapterHistorySetup> history,
      vector<AdapterFolderHistorySetup> historyfolders, vector<AdapterPVHistorySetup> historyvariables);

  void clear_history(UA_HistoryDataGathering gathering, vector<UA_NodeId>& historizing_nodes,
      vector<string>& historizing_setup, UA_Server* mappedServer, vector<AdapterFolderHistorySetup> historyfolders,
      vector<AdapterPVHistorySetup> historyvariables);
} // namespace ChimeraTK
