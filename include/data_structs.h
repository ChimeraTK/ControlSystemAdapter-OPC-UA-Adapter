// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <open62541/plugin/log.h>
#include <open62541/types.h>

#include <unistd.h>

#include <string>
#include <vector>

namespace ChimeraTK {
  struct AdapterFolderHistorySetup {
    std::string folder_historizing;
    UA_NodeId folder_id;
  };

  struct AdapterPVHistorySetup {
    std::string variable_historizing;
    UA_NodeId variable_id;
  };

  enum class HistorizingBackend { Circular, InfluxDB };
  struct AdapterHistorySetup {
    std::string name;
    size_t buffer_length{100};
    size_t entries_per_response{100};
    size_t interval{1000};
    HistorizingBackend backend{HistorizingBackend::Circular};
  };
  struct ServerConfig {
    std::string rootFolder;
    std::string descriptionFolder;
    UA_Boolean UsernamePasswordLogin = UA_FALSE;
    std::string password;
    std::string username;
    std::string applicationName = "OPCUA-Adapter";
    std::string hostname;
    uint16_t opcuaPort = 16664;
    bool enableSecurity = false;
    bool unsecure = false;
    bool registerLDS = false;
    bool useBoolAsVoid = false;
    std::string ldsAddress = "opc.tcp://localhost:4840";
    std::string ldsRegistryName;
    UA_LogLevel logLevel = UA_LOGLEVEL_INFO;
    std::string certPath;
    std::string keyPath;
    std::string allowListFolder;
    std::string blockListFolder;
    std::string issuerListFolder;
    std::vector<AdapterHistorySetup> history{};
    std::vector<AdapterFolderHistorySetup> historyfolders{};
    std::vector<AdapterPVHistorySetup> historyvariables{};
    explicit ServerConfig() {
      hostname.reserve(1024);
      gethostname(hostname.data(), 1024);
      // properly set length of the string after gethostname call
      hostname = std::string(hostname.c_str());
    }
  };
} // namespace ChimeraTK