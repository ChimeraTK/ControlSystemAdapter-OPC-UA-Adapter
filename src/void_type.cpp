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

#include "../include/void_type.h"

#include "csa_processvariable.h"
#include "open62541/plugin/log_stdout.h"

#include <functional>
using namespace std;
namespace ChimeraTK {
  static UA_StatusCode fireVoidEvent(void_observer_data* data, const string& pv_name) {
    UA_NodeId outId;
    UA_NodeId void_event_NodeId = UA_NODEID_STRING(1, const_cast<char*>("VoidEventType"));
    UA_StatusCode retval = UA_Server_createEvent(data->mappedServer, void_event_NodeId, &outId);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Creation of the void Event failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    auto pv = data->csManager->getProcessVariable(pv_name);
    auto eventValidity = static_cast<UA_Int32>(pv->dataValidity());
    retval = UA_Server_writeObjectProperty_scalar(data->mappedServer, outId,
        UA_QUALIFIEDNAME(1, const_cast<char*>("Validity")), &eventValidity, &UA_TYPES[UA_TYPES_INT32]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting void event validity failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_String eventSourceName = UA_String_fromChars(pv_name.c_str());
    retval = UA_Server_writeObjectProperty_scalar(data->mappedServer, outId,
        UA_QUALIFIEDNAME(0, const_cast<char*>("SourceName")), &eventSourceName, &UA_TYPES[UA_TYPES_STRING]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting void event SourceName failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_String_clear(&eventSourceName);
    UA_LocalizedText eventMessage =
        UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>("ChimeraTK::Void written."));
    retval = UA_Server_writeObjectProperty_scalar(data->mappedServer, outId,
        UA_QUALIFIEDNAME(0, const_cast<char*>("Message")), &eventMessage, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting void event Message failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_UInt16 eventSeverity = 100;
    retval = UA_Server_writeObjectProperty_scalar(data->mappedServer, outId,
        UA_QUALIFIEDNAME(0, const_cast<char*>("Severity")), &eventSeverity, &UA_TYPES[UA_TYPES_UINT16]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting void event Severity failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_UInt16_clear(&eventSeverity);
    UA_NodeId id;
    UA_NodeId_init(&id);
    id.namespaceIndex = 1;
    id.identifierType = UA_NODEIDTYPE_STRING;
    id.identifier.string = UA_String_fromChars((data->rootFolder + /*"/" +*/ pv_name).c_str());
    retval = UA_Server_writeObjectProperty_scalar(
        data->mappedServer, outId, UA_QUALIFIEDNAME(1, const_cast<char*>("cs_path")), &id, &UA_TYPES[UA_TYPES_NODEID]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting void event cs_path failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_NodeId_clear(&id);
    UA_String eventDescription = UA_String_fromChars(pv->getDescription().c_str());
    const UA_String null_str = UA_STRING_NULL;
    if(UA_String_equal(&eventDescription, &null_str) != true) {
      retval = UA_Server_writeObjectProperty_scalar(data->mappedServer, outId,
          UA_QUALIFIEDNAME(1, const_cast<char*>("Description")), &eventDescription, &UA_TYPES[UA_TYPES_STRING]);
      if(retval != UA_STATUSCODE_GOOD) {
        string error_message = "Setting void event description failed with StatusCode ";
        error_message.append(UA_StatusCode_name(retval));
        throw std::logic_error(error_message);
      }
    }
    UA_String_clear(&eventDescription);
    UA_String eventType = UA_String_fromChars("Void");
    retval = UA_Server_writeObjectProperty_scalar(data->mappedServer, outId,
        UA_QUALIFIEDNAME(1, const_cast<char*>("Type")), &eventType, &UA_TYPES[UA_TYPES_STRING]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting void event Type failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_String_clear(&eventType);
    UA_DateTime eventTime = UA_DateTime_now();
    retval = UA_Server_writeObjectProperty_scalar(data->mappedServer, outId,
        UA_QUALIFIEDNAME(0, const_cast<char*>("Time")), &eventTime, &UA_TYPES[UA_TYPES_DATETIME]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting void event Time failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_DateTime_clear(&eventTime);

    retval = UA_Server_triggerEvent(data->mappedServer, outId, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), nullptr, UA_TRUE);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Triggering void event failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_LOG_TRACE(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Fire Void Event for PV with ID %s", pv_name.c_str());
    return retval;
  }

  static void updateLoop(void_observer_data* data) {
    try {
      ReadAnyGroup group;                                   ///< This group will collect all Void type PVs
      std::map<TransferElementID, std::string> idToNameMap; ///< Map that connects TransferID to PV name string
      for(auto pv : data->pvs) {
        group.add(data->csManager->getProcessArray<Void>(pv));
        idToNameMap[data->csManager->getProcessArray<Void>(pv)->getId()] = pv;
        //}
      }
      /*vector<ProcessVariable::SharedPtr> allProcessVariables = data->csManager->getAllProcessVariables();
        for(const ProcessVariable::SharedPtr& oneProcessVariable : allProcessVariables) {
          std::type_info const& valueType = oneProcessVariable->getValueType();
          if(valueType == typeid(Void)) {
            // Check if PV is writable - if not assume it is an VoidInput
            if(!oneProcessVariable->isWriteable()) {
              group.add(data->csManager->getProcessArray<Void>(oneProcessVariable->getName()));
              idToNameMap[data->csManager->getProcessArray<Void>(oneProcessVariable->getName())->getId()] =
                  oneProcessVariable->getName();
            }
            else {
              UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                  "Ignoring Void input %s. Void inputs are not yet supported.", oneProcessVariable->getName().c_str());
            }
          }
        }*/
      group.finalise();

      while(data->adapter->isRunning()) {
        // wait for next event
        auto id = group.readAny();
        fireVoidEvent(data, idToNameMap.at(id));
      }
      UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Void observer thread terminated.");
      UA_free(data);
    }
    catch(boost::thread_interrupted&) {
      UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Void observer thread terminated.");
      UA_free(data);
    }
  }

  void startVoidObserverThread(void_observer_data* data) {
    updateLoop(data);
  }

  /*
   * declare the eventtype for void nodes and add it to the server
   * */
  UA_StatusCode addEventProperty(UA_Server* server, UA_NodeId eventNodeId, char* property_name, UA_NodeId dtype) {
    UA_NodeId propertyNodeId;
    UA_VariableAttributes vattr = UA_VariableAttributes_default;
    vattr.dataType = dtype;
    vattr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en-us"), property_name);
    vattr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_StatusCode retval = UA_Server_addVariableNode(server, UA_NODEID_NULL, eventNodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(1, property_name),
        UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), vattr, nullptr, &propertyNodeId);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Failed to add the property ";
      error_message.append(property_name);
      error_message.append(" with StatusCode ");
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    retval = UA_Server_addReference(server, propertyNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
        UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Failed to set the reference to property ";
      error_message.append(property_name);
      error_message.append(" mandatory with StatusCode ");
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    return retval;
  }

  UA_StatusCode add_void_event_type(UA_Server* server) {
    UA_ObjectTypeAttributes attr = UA_ObjectTypeAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>("VoidEventType"));
    attr.description = UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>("The EventType for void nodes"));
    UA_NodeId void_event_NodeId = UA_NODEID_STRING(1, const_cast<char*>("VoidEventType"));
    UA_StatusCode retval = UA_Server_addObjectTypeNode(server, void_event_NodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE), UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, const_cast<char*>("VoidEventType")), attr, nullptr, nullptr);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Failed to add the VoidEventType with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    retval |=
        addEventProperty(server, void_event_NodeId, const_cast<char*>("cs_path"), UA_TYPES[UA_TYPES_NODEID].typeId);
    retval |=
        addEventProperty(server, void_event_NodeId, const_cast<char*>("Description"), UA_TYPES[UA_TYPES_STRING].typeId);
    retval |= addEventProperty(server, void_event_NodeId, const_cast<char*>("Type"), UA_TYPES[UA_TYPES_STRING].typeId);
    retval |=
        addEventProperty(server, void_event_NodeId, const_cast<char*>("Validity"), UA_TYPES[UA_TYPES_INT32].typeId);
    return retval;
  }
} // namespace ChimeraTK
