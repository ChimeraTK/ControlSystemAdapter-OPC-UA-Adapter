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

#include "ChimeraTK/ReadAnyGroup.h"
#include "csa_opcua_adapter.h"
#include "csa_processvariable.h"
#include "open62541/plugin/log_stdout.h"
#include "ua_mapped_class.h"

namespace ChimeraTK {
typedef struct{
  boost::shared_ptr<ControlSystemPVManager> csManager;
  UA_Server *mappedServer;
  string rootFolder;
  csa_opcua_adapter *adapter;
}void_observer_data;

  void startVoidObserverThread(void_observer_data* data);
  UA_StatusCode add_void_event_type(UA_Server* server);
} // namespace ChimeraTK
