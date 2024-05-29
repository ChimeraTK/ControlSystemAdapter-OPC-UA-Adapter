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
  static UA_StatusCode fire_void_event(void_observer_data* data, const string& pv_name) {
    UA_NodeId outId;
    UA_NodeId void_event_NodeId = UA_NODEID_STRING(1, "VoidEventType");
    UA_StatusCode retval = UA_Server_createEvent(data->mappedServer, void_event_NodeId, &outId);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Creation of the void Event failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    auto pv = data->csManager->getProcessVariable(pv_name);
    auto validity = static_cast<UA_Int32>(pv->dataValidity());
    retval = UA_Server_writeObjectProperty_scalar(
        data->mappedServer, outId, UA_QUALIFIEDNAME(1, "Validity"), &validity, &UA_TYPES[UA_TYPES_INT32]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting PV validity failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_String pvname = UA_String_fromChars(pv_name.c_str());
    retval = UA_Server_writeObjectProperty_scalar(
        data->mappedServer, outId, UA_QUALIFIEDNAME(1, "Name"), &pvname, &UA_TYPES[UA_TYPES_STRING]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting PV Name failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_String_clear(&pvname);
    UA_NodeId id;
    UA_NodeId_init(&id);
    id.namespaceIndex = 1;
    id.identifierType = UA_NODEIDTYPE_STRING;
    id.identifier.string = UA_String_fromChars((data->rootFolder + "/" + pv_name).c_str());
    retval = UA_Server_writeObjectProperty_scalar(
        data->mappedServer, outId, UA_QUALIFIEDNAME(1, "cs_path"), &id, &UA_TYPES[UA_TYPES_NODEID]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting cs_path failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_NodeId_clear(&id);
    UA_String desvalue = UA_String_fromChars(pv->getDescription().c_str());
    const UA_String null_str = UA_STRING_NULL;
    if(UA_String_equal(&desvalue, &null_str) != 0) {
      retval = UA_Server_writeObjectProperty_scalar(
          data->mappedServer, outId, UA_QUALIFIEDNAME(1, "Description"), &desvalue, &UA_TYPES[UA_TYPES_STRING]);
      if(retval != UA_STATUSCODE_GOOD) {
        string error_message = "Setting PV description failed with StatusCode ";
        error_message.append(UA_StatusCode_name(retval));
        throw std::logic_error(error_message);
      }
    }
    const auto* type = "/Type";
    UA_Variant type_val;
    UA_Variant_init(&type_val);
    retval = UA_Server_readValue(
        data->mappedServer, UA_NODEID_STRING(1, (char*)(data->rootFolder + "/" + pv_name + type).c_str()), &type_val);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Failed to read Type value of PV with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    auto* typevalue = (UA_String*)type_val.data;
    retval = UA_Server_writeObjectProperty_scalar(
        data->mappedServer, outId, UA_QUALIFIEDNAME(1, "Type"), typevalue, &UA_TYPES[UA_TYPES_STRING]);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Setting PV Type failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_Variant_clear(&type_val);
    retval = UA_Server_triggerEvent(data->mappedServer, outId, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), nullptr, UA_TRUE);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Triggering event failed with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Fire Void Event for PV with ID %s", pv_name.c_str());
    return retval;
  }

  static void updateLoop(void_observer_data* data) {
    ReadAnyGroup group;                                   ///< This group will collect all Void type PVs
    std::map<TransferElementID, std::string> idToNameMap; ///< Map that connects TransferID to PV name string
    for(auto& pv : data->pvs) {
      group.add(data->csManager->getProcessArray<Void>(pv));
      idToNameMap[data->csManager->getProcessArray<Void>(pv)->getId()] = pv;
    }
    group.finalise();
    while(data->adapter->isRunning()) {
      // wait for next event
      auto id = group.readAny();
      fire_void_event(data, idToNameMap.at(id));
    }
    UA_free(data);
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
    vattr.displayName = UA_LOCALIZEDTEXT("en-us", property_name);
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
    attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"VoidEventType");
    attr.description = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"The EventType for void nodes");
    UA_NodeId void_event_NodeId = UA_NODEID_STRING(1, (char*)"VoidEventType");
    UA_StatusCode retval = UA_Server_addObjectTypeNode(server, void_event_NodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE), UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, (char*)"VoidEventType"), attr, nullptr, nullptr);
    if(retval != UA_STATUSCODE_GOOD) {
      string error_message = "Failed to add the VoidEventType with StatusCode ";
      error_message.append(UA_StatusCode_name(retval));
      throw std::logic_error(error_message);
    }
    addEventProperty(server, void_event_NodeId, (char*)"cs_path", UA_TYPES[UA_TYPES_NODEID].typeId);
    addEventProperty(server, void_event_NodeId, (char*)"Description", UA_TYPES[UA_TYPES_STRING].typeId);
    addEventProperty(server, void_event_NodeId, "Name", UA_TYPES[UA_TYPES_STRING].typeId);
    addEventProperty(server, void_event_NodeId, "Type", UA_TYPES[UA_TYPES_STRING].typeId);
    addEventProperty(server, void_event_NodeId, "Validity", UA_TYPES[UA_TYPES_INT32].typeId);
    return retval;
  }
} // namespace ChimeraTK
