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
* Copyright (c) 2023 Florian Düwel <florian.duewel@iosb.fraunhofer.de>
*/

#ifndef CHIMERATK_CONTROLSYSTEMADAPTER_OPCUAADAPTER_NODE_HISTORIZING_H
#define CHIMERATK_CONTROLSYSTEMADAPTER_OPCUAADAPTER_NODE_HISTORIZING_H

#include <open62541/plugin/historydata/history_data_gathering.h>
#include <open62541/plugin/historydata/history_data_gathering_default.h>
#include <open62541/plugin/historydata/history_database_default.h>
#include <open62541/plugin/historydata/history_data_backend_memory.h>
#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "csa_additionalvariable.h"
#include "csa_processvariable.h"
#include "ua_mapped_class.h"
#include "xml_file_handler.h"
#include <vector>

typedef struct{
  string folder_historizing;
  UA_NodeId folder_id;
}AdapterFolderHistorySetup;

typedef struct{
  string variable_historizing;
  UA_NodeId variable_id;
}AdapterPVHistorySetup;

typedef struct{
  string name;
  size_t max_length;
  size_t entries_per_response;
  size_t interval;
}AdapterHistorySetup;

void add_historizing_nodes(vector<UA_NodeId>& historizing_nodes,
                           vector<string>& historizing_setup,
                           UA_HistoryDataGathering gathering,
                           UA_Server *mappedServer,
                           UA_ServerConfig *server_config,
                           vector<AdapterHistorySetup> history,
                           vector<AdapterFolderHistorySetup> historyfolders,
                           vector<AdapterPVHistorySetup> historyvariables);


#endif // CHIMERATK_CONTROLSYSTEMADAPTER_OPCUAADAPTER_NODE_HISTORIZING_H