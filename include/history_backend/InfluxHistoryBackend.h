// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "history_backend/InfluxClient.h"

#include <open62541/plugin/historydata/history_data_backend.h>

#include <string>

namespace influxdb {

  /**
   * @brief Creates an InfluxDB history data backend.
   * @param client Pointer to the InfluxClient instance.
   * @param influxFieldName Field name used in the database. It is set to the short node name of the PV by default.
   * @param nodeIdTagName The name of the tag for the node ID.
   * @param engineeringUnit The engineering unit of the variable. It is assigned to the "unit" tag in the database.
   * @param hostname The hostname of the application. It is assigned to the "host" tag in the database.
   * @param applicationName The name of the application. It is assigned to the "application" tag in the database.
   * @param port The port of the application. It is assigned to the "port" tag in the database.
   * @return The created history data backend.
   */
  UA_HistoryDataBackend UA_HistoryDataBackend_Influx(influxdb::InfluxClient* client, const std::string& influxFieldName,
      const std::string& nodeIdTagName, const std::string& engineeringUnit, const std::string& hostname,
      const std::string& applicationName, uint16_t port);
} // namespace influxdb