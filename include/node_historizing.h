// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2023-2024 Fraunhofer IOSB (Author: Florian Düwel)
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "data_structs.h"
#include "history_backend/InfluxClient.h"

#include <open62541/plugin/historydata/history_data_backend_memory.h>
#include <open62541/plugin/historydata/history_data_gathering.h>
#include <open62541/plugin/historydata/history_data_gathering_default.h>
#include <open62541/plugin/historydata/history_database_default.h>

#include <memory>
#include <string>
#include <vector>

namespace ChimeraTK {

  UA_HistoryDataGathering add_historizing_nodes(std::vector<UA_NodeId>& historizing_nodes,
      std::vector<std::string>& historizing_setup, UA_Server* mappedServer, UA_ServerConfig* server_config,
      const ServerConfig& config, std::unique_ptr<InfluxClient>& influxClient);

  void clear_history(UA_HistoryDataGathering gathering, std::vector<UA_NodeId>& historizing_nodes,
      std::vector<std::string>& historizing_setup, UA_Server* mappedServer,
      std::vector<AdapterFolderHistorySetup> historyfolders, std::vector<AdapterPVHistorySetup> historyvariables,
      UA_ServerConfig* server_config);

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
  void check_historizing_nodes(std::vector<UA_NodeId>& historizing_nodes, std::vector<std::string>& historizing_setup,
      UA_ServerConfig* server_config);

  void remove_nodes_with_incomplete_historizing_setup(std::vector<UA_NodeId>& historizing_nodes,
      std::vector<std::string>& historizing_setup, UA_ServerConfig* server_config,
      std::vector<AdapterHistorySetup> history);
} // namespace ChimeraTK
