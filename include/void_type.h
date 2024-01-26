//
// Created by flo47663 on 08.01.2024.
//

#ifndef CHIMERATK_CONTROLSYSTEMADAPTER_OPCUAADAPTER_VOID_TYPE_H
#define CHIMERATK_CONTROLSYSTEMADAPTER_OPCUAADAPTER_VOID_TYPE_H

#include "ChimeraTK/ReadAnyGroup.h"
#include "csa_processvariable.h"
#include "open62541/plugin/log_stdout.h"
#include "ua_mapped_class.h"
#include "csa_opcua_adapter.h"

typedef struct{
  vector<std::string> pvs;
  boost::shared_ptr<ControlSystemPVManager> csManager;
  UA_Server *mappedServer;
  /*UA_Boolean *running;*/
}void_observer_data;

void startVoidObserverThread(void_observer_data *data);
UA_StatusCode add_void_event_type(UA_Server *server);

#endif // CHIMERATK_CONTROLSYSTEMADAPTER_OPCUAADAPTER_VOID_TYPE_H
