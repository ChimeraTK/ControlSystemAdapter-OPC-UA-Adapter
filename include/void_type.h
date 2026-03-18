// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2023-2024 Fraunhofer IOSB (Author: Florian Düwel)
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "csa_opcua_adapter.h"

#include <ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h>

#include <open62541/plugin/log.h>
#include <open62541/types.h>
#include <open62541/util.h>

#include <vector>

namespace ChimeraTK {
  typedef struct {
    boost::shared_ptr<ControlSystemPVManager> csManager;
    UA_Server* mappedServer;
    string rootFolder;
    csa_opcua_adapter* adapter;
    vector<std::string> pvs;
  } void_observer_data;

  void startVoidObserverThread(void_observer_data* data, const UA_Logger* logger);
  UA_StatusCode add_void_event_type(UA_Server* server);
} // namespace ChimeraTK
