// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "history_backend/InfluxClient.h"

#include <open62541/plugin/historydata/history_data_backend.h>

#include <string>

UA_HistoryDataBackend UA_HistoryDataBackend_Influx(InfluxClient* client, const std::string& fieldKey,
    const std::string& nodeIdTagName, const std::string& hostname, const std::string& applicationName, uint16_t port);
