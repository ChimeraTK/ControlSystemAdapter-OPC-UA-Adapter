//
// Created by flo47663 on 08.01.2024.
//

#include "../include/void_type.h"
#include "csa_processvariable.h"
#include "open62541/plugin/log_stdout.h"


static UA_StatusCode fire_void_event(void_observer_data *data, const string& pv_name){
  UA_NodeId outId;
  UA_NodeId void_event_NodeId = UA_NODEID_STRING(1, "VoidEventType");
  UA_StatusCode retval = UA_Server_createEvent(data->mappedServer, void_event_NodeId, &outId);
  if (retval != UA_STATUSCODE_GOOD) {
    /*UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
        "createEvent failed. StatusCode %s", UA_StatusCode_name(retval));*/
    return retval;
  }

  auto pv = data->csManager->getProcessVariable(pv_name);
  auto validity = static_cast<UA_Int32>(pv->dataValidity());
  /*retval =*/ UA_Server_writeObjectProperty_scalar(data->mappedServer, outId, UA_QUALIFIEDNAME(1, "Validity"),
      &validity, &UA_TYPES[UA_TYPES_INT32]);
  /*if(retval != UA_STATUSCODE_GOOD){
    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "Setting PV validity failed with StatusCode %s", UA_StatusCode_name(retval));
  }*/

  UA_String pvname = UA_String_fromChars(pv_name.c_str());
  /*retval =*/ UA_Server_writeObjectProperty_scalar(data->mappedServer, outId, UA_QUALIFIEDNAME(1, "Name"),
      &pvname, &UA_TYPES[UA_TYPES_STRING]);
  /*if(retval != UA_STATUSCODE_GOOD){
    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "Setting PV Name failed with StatusCode %s", UA_StatusCode_name(retval));
  }*/
  UA_String_clear(&pvname);

  UA_NodeId id;
  UA_NodeId_init(&id);
  id.namespaceIndex = 1;
  id.identifierType = UA_NODEIDTYPE_STRING;
  id.identifier.string = UA_String_fromChars((data->rootFolder+"/"+pv_name).c_str());
  /*retval = */UA_Server_writeObjectProperty_scalar(data->mappedServer, outId, UA_QUALIFIEDNAME(1, "cs_path"),
      &id, &UA_TYPES[UA_TYPES_NODEID]);
  /*if(retval != UA_STATUSCODE_GOOD){
    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "Setting cs_path failed with StatusCode %s", UA_StatusCode_name(retval));
  }*/
  UA_NodeId_clear(&id);

  UA_String desvalue =  UA_String_fromChars(pv->getDescription().c_str());
  const UA_String null_str = UA_STRING_NULL;
  if(UA_String_equal(&desvalue, &null_str) != 0){
    /*retval =*/ UA_Server_writeObjectProperty_scalar(data->mappedServer, outId, UA_QUALIFIEDNAME(1, "Description"),
        &desvalue, &UA_TYPES[UA_TYPES_STRING]);
    /*if(retval != UA_STATUSCODE_GOOD){
      UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
          "Setting PV description failed with StatusCode %s", UA_StatusCode_name(retval));
    }*/
  }

  const auto *type = "/Type";
  UA_Variant type_val;
  UA_Variant_init(&type_val);
  /*retval =*/ UA_Server_readValue(data->mappedServer, UA_NODEID_STRING(1, (char*)(data->rootFolder+"/"+pv_name+type).c_str()), &type_val);
  /*if(retval != UA_STATUSCODE_GOOD){
    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "Failed to read Type value of PV with StatusCode %s", UA_StatusCode_name(retval));
  }*/
  auto *typevalue =  (UA_String*) type_val.data;
  /*retval =*/ UA_Server_writeObjectProperty_scalar(data->mappedServer , outId, UA_QUALIFIEDNAME(1, "Type"),
      typevalue, &UA_TYPES[UA_TYPES_STRING]);
  /*if(retval != UA_STATUSCODE_GOOD){
    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "Setting PV Type failed with StatusCode %s", UA_StatusCode_name(retval));
  }*/
  UA_Variant_clear(&type_val);

  retval = UA_Server_triggerEvent(data->mappedServer, outId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER),
      nullptr, UA_TRUE);
  /*if(retval != UA_STATUSCODE_GOOD){
    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "Triggering event failed. StatusCode %s", UA_StatusCode_name(retval));
  }
  else{*/
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "Fire Event for PV with ID %s", pv_name.c_str());
  /*}*/
  return retval;
}


static void updateLoop (void_observer_data *data){
  ReadAnyGroup group; ///< This group will collect all Void type PVs
  std::map<TransferElementID, std::string> idToNameMap; ///< Map that connects TransferID to PV name string
  for(auto &pv : data->pvs){
    group.add(data->csManager->getProcessArray<Void>(pv));
    idToNameMap[data->csManager->getProcessArray<Void>(pv)->getId()] = pv;
  }
  group.finalise();
  while(data->adapter->isRunning()) {
    // wait for next event
    auto id = group.readAny();
    UA_StatusCode retval = fire_void_event(data, idToNameMap.at(id));
    if(retval != UA_STATUSCODE_GOOD){
      UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
          "Failed to fire the Event with StatusCode %s", UA_StatusCode_name(retval));
    }
  }
  UA_free(data);
}

void startVoidObserverThread(void_observer_data *data){
  updateLoop(data );
}

/*
 * declare the eventtype for void nodes and add it to the server
 * */

UA_StatusCode addEventProperty(UA_Server *server, UA_NodeId eventNodeId, char *property_name, UA_NodeId dtype){
  UA_NodeId propertyNodeId;
  UA_VariableAttributes vattr = UA_VariableAttributes_default;
  vattr.dataType = dtype;
  vattr.displayName = UA_LOCALIZEDTEXT("en-us", property_name);
  vattr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

  UA_StatusCode retval = UA_Server_addVariableNode(server, UA_NODEID_NULL, eventNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(1, property_name),
      UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), vattr, nullptr, &propertyNodeId);
  if(retval != UA_STATUSCODE_GOOD){
    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add the %s property with StatusCode %s", property_name, UA_StatusCode_name(retval));
    return retval;
  }
  retval = UA_Server_addReference(server, propertyNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
      UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);
  if(retval != UA_STATUSCODE_GOOD){
    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to set the reference to %s property mandatory with StatusCode %s", property_name, UA_StatusCode_name(retval));
    return retval;
  }
  return retval;
}

UA_StatusCode add_void_event_type(UA_Server *server){
  UA_ObjectTypeAttributes attr = UA_ObjectTypeAttributes_default;
  attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"VoidEventType");
  attr.description = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"The EventType for void nodes");
  UA_NodeId void_event_NodeId = UA_NODEID_STRING(1, (char*)"VoidEventType");
  UA_StatusCode retval = UA_Server_addObjectTypeNode(server, void_event_NodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE), UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
      UA_QUALIFIEDNAME(1, (char*)"VoidEventType"), attr, nullptr, nullptr);
  if(retval != UA_STATUSCODE_GOOD){
    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add the VoidEventType %s", UA_StatusCode_name(retval));
    return retval;
  }
  retval = addEventProperty(server, void_event_NodeId, (char*) "cs_path", UA_TYPES[UA_TYPES_NODEID].typeId);
  if(retval != UA_STATUSCODE_GOOD){
    return retval;
  }
  retval = addEventProperty(server, void_event_NodeId, (char*)"Description", UA_TYPES[UA_TYPES_STRING].typeId);
  if(retval != UA_STATUSCODE_GOOD){
    return retval;
  }
  retval = addEventProperty(server, void_event_NodeId, "Name", UA_TYPES[UA_TYPES_STRING].typeId);
  if(retval != UA_STATUSCODE_GOOD){
    return retval;
  }
  retval = addEventProperty(server, void_event_NodeId, "Type", UA_TYPES[UA_TYPES_STRING].typeId);
  if(retval != UA_STATUSCODE_GOOD){
    return retval;
  }
  retval = addEventProperty(server, void_event_NodeId, "Validity", UA_TYPES[UA_TYPES_INT32].typeId);
  if(retval != UA_STATUSCODE_GOOD){
    return retval;
  }
  return retval;
}