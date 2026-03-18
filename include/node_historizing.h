// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2023-2024 Fraunhofer IOSB (Author: Florian Düwel)
// SPDX-License-Identifier: LGPL-3.0-or-later
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
  struct AdapterFolderHistorySetup {
    string folder_historizing;
    UA_NodeId folder_id;
  };

  struct AdapterPVHistorySetup {
    string variable_historizing;
    UA_NodeId variable_id;
  };

  struct AdapterHistorySetup {
    string name;
    size_t buffer_length{100};
    size_t entries_per_response{100};
    size_t interval{1000};
  };

  UA_HistoryDataGathering add_historizing_nodes(vector<UA_NodeId>& historizing_nodes, vector<string>& historizing_setup,
      UA_Server* mappedServer, UA_ServerConfig* server_config, vector<AdapterHistorySetup> history,
      vector<AdapterFolderHistorySetup> historyfolders, vector<AdapterPVHistorySetup> historyvariables);

  void clear_history(UA_HistoryDataGathering gathering, vector<UA_NodeId>& historizing_nodes,
      vector<string>& historizing_setup, UA_Server* mappedServer, vector<AdapterFolderHistorySetup> historyfolders,
      vector<AdapterPVHistorySetup> historyvariables, UA_ServerConfig* server_config);

  /**
   * This assumes both lists have the same size.
   * Avoid multiple setups for one node.
   * Because first nodes from pv settings in the mapping file are added,
   * those settings will be used instead of a setting added by mapping
   * a folder.
   *
   * @param historizing_nodes List of nodes with history.
   * @param historizing_setup List of setups.
   * @param server_config Server configuration used to get the log level.
   */
  void check_historizing_nodes(
      vector<UA_NodeId>& historizing_nodes, vector<string>& historizing_setup, UA_ServerConfig* server_config);

  void remove_nodes_with_incomplete_historizing_setup(vector<UA_NodeId>& historizing_nodes,
      vector<string>& historizing_setup, UA_ServerConfig* server_config, vector<AdapterHistorySetup> history);
} // namespace ChimeraTK
