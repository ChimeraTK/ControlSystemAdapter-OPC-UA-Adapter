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
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 * Copyright (c) 2016 Julian Rahm  <Julian.Rahm@tu-dresden.de>
 * Copyright (c) 2018-2023 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
 */

#include "csa_processvariable.h"

#include "csa_config.h"

extern "C" {
#include "csa_namespace.h"
}

#include "open62541/plugin/log_stdout.h"
#include "ua_map_types.h"

#include <iostream>
#include <utility>
#include <vector>
namespace ChimeraTK {
  ua_processvariable::ua_processvariable(UA_Server* server, UA_NodeId basenodeid, const string& namePV,
      boost::shared_ptr<ControlSystemPVManager> csManager, const UA_Logger* logger, string overwriteNodeString)
  : namePV(namePV), nameNew(namePV), csManager(std::move(csManager)),
    nodeStringIdOverwrite(std::move(overwriteNodeString)), array(false), ua_mapped_class(server, basenodeid) {
    this->mapSelfToNamespace(logger);
  }

  ua_processvariable::~ua_processvariable() {
    //* Our ua_mapped_class destructor will take care of deleting our opcua footprint as long as all variables are mapped
    // in this->ownedNodes
    UA_NodeId_clear(&this->ownNodeId);
  }

  UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getName(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      UA_Boolean includeSourceTimeStamp, const UA_NumericRange* /*range*/, UA_DataValue* value) {
    auto* thisObj = static_cast<ua_processvariable*>(nodeContext);

    UA_String ua_val = UA_String_fromChars((char*)thisObj->getName().c_str());
    UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&ua_val);
    value->hasValue = true;
    if(includeSourceTimeStamp) {
      value->sourceTimestamp = thisObj->getSourceTimeStamp();
      value->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
  }

  string ua_processvariable::getName() {
    return this->namePV;
  }

  // EngineeringUnit
  UA_StatusCode ua_processvariable::ua_writeproxy_ua_processvariable_setEngineeringUnit(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      const UA_NumericRange* /*range*/, const UA_DataValue* value) {
    auto* theClass = static_cast<ua_processvariable*>(nodeContext);
    std::string cpps;
    char* s;
    s = (char*)malloc(((UA_String) * ((UA_String*)value->value.data)).length + 1);
    memset(s, 0, ((UA_String) * ((UA_String*)value->value.data)).length + 1);
    memcpy(s, ((UA_String) * ((UA_String*)value->value.data)).data,
        ((UA_String) * ((UA_String*)value->value.data)).length);
    cpps.assign(s, ((UA_String) * ((UA_String*)value->value.data)).length);
    free(s);
    theClass->setEngineeringUnit(cpps);
    return UA_STATUSCODE_GOOD;
  }

  void ua_processvariable::setEngineeringUnit(string engineeringUnit) {
    this->engineeringUnit = std::move(engineeringUnit);
  }

  UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getEngineeringUnit(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      UA_Boolean includeSourceTimeStamp, const UA_NumericRange* /*range*/, UA_DataValue* value) {
    auto* thisObj = static_cast<ua_processvariable*>(nodeContext);

    UA_String ua_val = UA_String_fromChars((char*)thisObj->getEngineeringUnit().c_str());
    UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&ua_val);
    value->hasValue = true;
    if(includeSourceTimeStamp) {
      value->sourceTimestamp = thisObj->getSourceTimeStamp();
      value->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
  }

  string ua_processvariable::getEngineeringUnit() {
    if(!this->engineeringUnit.empty()) {
      return this->engineeringUnit;
    }
    else {
      this->engineeringUnit = this->csManager->getProcessVariable(this->namePV)->getUnit();
      return this->engineeringUnit;
    }
  }

  // Description
  UA_StatusCode ua_processvariable::ua_writeproxy_ua_processvariable_setDescription(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      const UA_NumericRange* /*range*/, const UA_DataValue* value) {
    auto* theClass = static_cast<ua_processvariable*>(nodeContext);
    std::string cpps;
    char* s;
    s = (char*)malloc(((UA_String) * ((UA_String*)value->value.data)).length + 1);
    memset(s, 0, ((UA_String) * ((UA_String*)value->value.data)).length + 1);
    memcpy(s, ((UA_String) * ((UA_String*)value->value.data)).data,
        ((UA_String) * ((UA_String*)value->value.data)).length);
    cpps.assign(s, ((UA_String) * ((UA_String*)value->value.data)).length);
    free(s);
    theClass->setDescription(cpps);
    return UA_STATUSCODE_GOOD;
  }

  void ua_processvariable::setDescription(string description) {
    this->description = std::move(description);
  }

  UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getDescription(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      UA_Boolean includeSourceTimeStamp, const UA_NumericRange* /*range*/, UA_DataValue* value) {
    auto* thisObj = static_cast<ua_processvariable*>(nodeContext);

    UA_String ua_val = UA_String_fromChars((char*)thisObj->getDescription().c_str());
    UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&ua_val);
    value->hasValue = true;
    if(includeSourceTimeStamp) {
      value->sourceTimestamp = thisObj->getSourceTimeStamp();
      value->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
  }

  UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getValidity(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      UA_Boolean includeSourceTimeStamp, const UA_NumericRange* /*range*/, UA_DataValue* value) {
    auto* thisObj = static_cast<ua_processvariable*>(nodeContext);
    DataValidity dv = thisObj->csManager->getProcessVariable(thisObj->namePV)->dataValidity();
    UA_Int32 validity;
    switch(dv) {
      case DataValidity::ok:
        validity = 1;
        break;
      case DataValidity::faulty:
        validity = 0;
        break;
      default:
        validity = -1;
    }
    UA_Variant_setScalarCopy(&value->value, &validity, &UA_TYPES[UA_TYPES_INT32]);
    value->hasValue = true;
    if(includeSourceTimeStamp) {
      value->sourceTimestamp = thisObj->getSourceTimeStamp();
      value->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
  }

  string ua_processvariable::getDescription() {
    if(!this->description.empty()) {
      return this->description;
    }
    else {
      this->description = this->csManager->getProcessVariable(this->namePV)->getDescription();
      return this->description;
    }
  }

  // Type
  UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getType(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      UA_Boolean includeSourceTimeStamp, const UA_NumericRange* /*range*/, UA_DataValue* value) {
    auto* thisObj = static_cast<ua_processvariable*>(nodeContext);

    UA_String ua_val = UA_String_fromChars((char*)thisObj->getType().c_str());
    UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&ua_val);
    value->hasValue = true;
    if(includeSourceTimeStamp) {
      value->sourceTimestamp = thisObj->getSourceTimeStamp();
      value->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
  }

  string ua_processvariable::getType() {
    // Note: typeid().name() may return the name; may as well return the symbol's name from the binary though...
    std::type_info const& valueType = this->csManager->getProcessVariable(this->namePV)->getValueType();
    if(valueType == typeid(int8_t))
      return "int8_t";
    else if(valueType == typeid(uint8_t))
      return "uint8_t";
    else if(valueType == typeid(int16_t))
      return "int16_t";
    else if(valueType == typeid(uint16_t))
      return "uint16_t";
    else if(valueType == typeid(int32_t))
      return "int32_t";
    else if(valueType == typeid(uint32_t))
      return "uint32_t";
    else if(valueType == typeid(int64_t))
      return "int64_t";
    else if(valueType == typeid(uint64_t))
      return "uint64_t";
    else if(valueType == typeid(float))
      return "float";
    else if(valueType == typeid(double))
      return "double";
    else if(valueType == typeid(string))
      return "string";
    else if(valueType == typeid(Boolean))
      return "Boolean";
    else if(valueType == typeid(Void))
      return "Void";
    else
      return "Unsupported type";
  }

  template<typename T>
  UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getValue(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      UA_Boolean includeSourceTimeStamp, const UA_NumericRange* range, UA_DataValue* value) {
    auto* thisObj = static_cast<ua_processvariable*>(nodeContext);
    UA_StatusCode rv = UA_STATUSCODE_GOOD;
    rv = thisObj->getValue<T>(&value->value, range);
    if(rv == UA_STATUSCODE_GOOD) {
      value->hasValue = true;
      if(includeSourceTimeStamp) {
        value->sourceTimestamp = thisObj->getSourceTimeStamp();
        value->hasSourceTimestamp = true;
      }
    }
    return rv;
  }

  template<typename T>
  UA_StatusCode ua_processvariable::getValue(UA_Variant* v, const UA_NumericRange* range) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<T>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0).size() == 1) {
      T ival = this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0).at(0);
      UA_Variant_setScalar(v, &ival, &UA_TYPES[fusion::at_key<T>(typesMap)]);
    }
    else {
      std::vector<T> iarr = this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0);
      v->type = &UA_TYPES[fusion::at_key<T>(typesMap)];
      if(range != nullptr) {
        // v->type = &UA_TYPES[fusion::at_key<T>(typesMap)];
        // rv = UA_Variant_setRangeCopy(v, iarr.data(), iarr.size(), *range);
        UA_Variant tmpVariant;
        UA_Variant_setArray(&tmpVariant, iarr.data(), iarr.size(), &UA_TYPES[fusion::at_key<T>(typesMap)]);
        rv = UA_Variant_copyRange(&tmpVariant, v, *range);
        UA_Variant_clear(&tmpVariant);
      }
      else {
        rv = UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[fusion::at_key<T>(typesMap)]);
      }
    }
    return rv;
  }

  template<>
  UA_StatusCode ua_processvariable::getValue<string>(UA_Variant* v, const UA_NumericRange* /*range*/) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<string>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).size() == 1) {
      string sval = this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).at(0);
      UA_String ua_val = UA_String_fromChars((char*)sval.c_str());
      rv = UA_Variant_setScalarCopy(v, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
      UA_String_clear(&ua_val);
    }
    else {
      std::vector<string> sarr = this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0);
      auto* sarrayval = new UA_String[sarr.size()];
      for(size_t i = 0; i < sarr.size(); i++) {
        sarrayval[i] = UA_String_fromChars((char*)sarr[i].c_str());
      }
      rv = UA_Variant_setArrayCopy(v, sarrayval, sarr.size(), &UA_TYPES[UA_TYPES_STRING]);
      delete[] sarrayval;
    }
    return rv;
  }

  template<typename T>
  UA_StatusCode ua_processvariable::ua_writeproxy_ua_processvariable_setValue(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      const UA_NumericRange* /*range*/, const UA_DataValue* value) {
    UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;
    auto* theClass = static_cast<ua_processvariable*>(nodeContext);
    retval = theClass->setValue<T>(&value->value);
    return retval;
  }

  template<typename T>
  UA_StatusCode ua_processvariable::setValue(const UA_Variant* data) {
    UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<T> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        T value = *((T*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (T*)data->data;
        valueArray.resize(data->arrayLength);
        for(size_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0).size() != data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<T>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }

    return retval;
  }

  template<>
  UA_StatusCode ua_processvariable::setValue<std::string>(const UA_Variant* data) {
    UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<string> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        string cpps;
        UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)data->data)), cpps)
        // string value = *((string *)data->data);
        valueArray.push_back(cpps);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        // Array
        auto* vdata = (UA_String*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          string cpps;
          UASTRING_TO_CPPSTRING(vdata[i], cpps)
          valueArray.at(i) = cpps;
        }
        if(this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).size() != data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<string>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }

    return retval;
  }

  UA_StatusCode ua_processvariable::mapSelfToNamespace(const UA_Logger* logger) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_NodeId createdNodeId = UA_NODEID_NULL;
    if(!nodeStringIdOverwrite.empty()) this->nameNew = nodeStringIdOverwrite;

    if(UA_NodeId_equal(&this->baseNodeId, &createdNodeId))
      return UA_STATUSCODE_GOOD; // initializer should have set this!

    UA_LocalizedText description;
    description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"),
        const_cast<char*>(this->csManager->getProcessVariable(this->namePV)->getDescription().c_str()));

    // Create our toplevel instance
    UA_VariableAttributes attr;
    UA_VariableAttributes_init(&attr);
    attr = UA_VariableAttributes_default;
    // Allow negative sampling intervals -> used by Labview and handled by open62541-interface 1.3.3.-2
    attr.minimumSamplingInterval = -1.;
    attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(this->nameNew.c_str()));
    attr.description = description;
    attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE);

    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      attr.writeMask = UA_ACCESSLEVELMASK_READ ^ UA_ACCESSLEVELMASK_WRITE;
      attr.accessLevel = UA_ACCESSLEVELMASK_READ ^ UA_ACCESSLEVELMASK_WRITE;
    }

    // Append the app and application folder names to the string nodeId, this is needed
    // because the mapping allows one pv linked in more folder -> only pv path is not unique
    string baseNodeIdName;
    if(this->baseNodeId.identifierType == UA_NODEIDTYPE_STRING) {
      UA_STRING_TO_CPPSTRING_COPY(&this->baseNodeId.identifier.string, &baseNodeIdName)
    }
    if(!baseNodeIdName.empty()) {
      baseNodeIdName.resize(baseNodeIdName.size() - 3);
    }

    // check if the nodeId is used by another mapping and find next free NodeId
    UA_NodeId result;
    if(UA_Server_readDataType(this->mappedServer,
           UA_NODEID_STRING(1, const_cast<char*>((baseNodeIdName + "/" + this->nameNew).c_str())),
           &result) == UA_STATUSCODE_GOOD) {
      return UA_STATUSCODE_BADNODEIDEXISTS;
    }
    retval = UA_Server_addVariableNode(this->mappedServer,
        UA_NODEID_STRING(1, const_cast<char*>((baseNodeIdName + "/" + this->nameNew).c_str())), this->baseNodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(1, const_cast<char*>(this->nameNew.c_str())),
        UA_NODEID_NUMERIC(CSA_NSID, 1001), attr, (void*)this, &createdNodeId);
    UA_NodeId_copy(&createdNodeId, &this->ownNodeId);
    ua_mapInstantiatedNodes(createdNodeId, UA_NODEID_NUMERIC(CSA_NSID, 1001), &this->ownedNodes);

    /* Use a datasource map to map any local getter/setter functions to OPC UA variables nodes */
    UA_DataSource_Map mapDs;
    this->addPVChildNodes(createdNodeId, baseNodeIdName, mapDs);

    UA_Variant arrayDimensions;
    UA_Variant_init(&arrayDimensions);

    std::type_info const& valueType = this->csManager->getProcessVariable(this->namePV)->getValueType();

    UA_DataSource_Map_Element mapElem;
    mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE);
    mapElem.description = description;
    // Read is possible for all elements
    UA_Variant uaArrayDimensions;
    UA_UInt32 arrayDims[1];

    if(valueType == typeid(int8_t)) {
      arrayDims[0] = typeSpecificSetup<int8_t>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(uint8_t)) {
      arrayDims[0] = typeSpecificSetup<uint8_t>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(int16_t)) {
      arrayDims[0] = typeSpecificSetup<int16_t>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(uint16_t)) {
      arrayDims[0] = typeSpecificSetup<uint16_t>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(int32_t)) {
      arrayDims[0] = typeSpecificSetup<int32_t>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(uint32_t)) {
      arrayDims[0] = typeSpecificSetup<uint32_t>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(int64_t)) {
      arrayDims[0] = typeSpecificSetup<int64_t>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(uint64_t)) {
      arrayDims[0] = typeSpecificSetup<uint64_t>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(float)) {
      arrayDims[0] = typeSpecificSetup<float>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(double)) {
      arrayDims[0] = typeSpecificSetup<double>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(string)) {
      arrayDims[0] = typeSpecificSetup<std::string>(mapElem, createdNodeId);
    }
    else if(valueType == typeid(Boolean)) {
      arrayDims[0] = typeSpecificSetup<Boolean>(mapElem, createdNodeId);
    }
    else if(valueType != typeid(Void)) {
      int status;
      auto* demangledName = abi::__cxa_demangle(valueType.name(), nullptr, nullptr, &status);

      if(status == 0) {
        UA_LOG_WARNING(logger, UA_LOGCATEGORY_USERLAND, "Cannot proxy unknown type %s for variable %s", demangledName,
            this->namePV.c_str());
        free(demangledName);
      }
      else {
        UA_LOG_WARNING(logger, UA_LOGCATEGORY_USERLAND,
            "Cannot proxy unknown type %s (demangling failed) for variable %s", valueType.name(), this->namePV.c_str());
      }
    }

    if(!this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      mapElem.dataSource.write = nullptr;
    }
    if(this->array) {
      UA_Server_writeValueRank(this->mappedServer, createdNodeId, UA_VALUERANK_ONE_DIMENSION);
      UA_Variant_setArray(&uaArrayDimensions, arrayDims, 1, &UA_TYPES[UA_TYPES_UINT32]);
      UA_Server_writeArrayDimensions(this->mappedServer, createdNodeId, uaArrayDimensions);
    }
    else {
      UA_Server_writeValueRank(this->mappedServer, createdNodeId, UA_VALUERANK_SCALAR);
    }

    // add variable data source map element to the list
    UA_NodeId_copy(&createdNodeId, &mapElem.concreteNodeId);
    mapDs.push_back(mapElem);

    UA_NodeId nodeIdVariableType = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE);
    NODE_PAIR_PUSH(this->ownedNodes, nodeIdVariableType, createdNodeId)

    for(auto i : mapDs) {
      retval |= UA_Server_setVariableNode_dataSource(this->mappedServer, i.concreteNodeId, i.dataSource);
      UA_NodeId_clear(&i.concreteNodeId);
      UA_NodeId_clear(&i.typeTemplateId);
    }
    UA_NodeId_clear(&createdNodeId);
    return retval;
  }

  UA_StatusCode ua_processvariable::addPVChildNodes(
      UA_NodeId pvNodeId, const string& baseNodePath, UA_DataSource_Map& map) {
    UA_NodeId createdNodeId = UA_NODEID_NULL;
    UA_VariableAttributes attr;
    UA_StatusCode addResult;

    // Adding the 'name' node to the PV
    UA_DataSource_Map_Element mapElemName;
    mapElemName.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_NAME);
    mapElemName.description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(""));
    mapElemName.dataSource.read = ua_readproxy_ua_processvariable_getName;
    mapElemName.dataSource.write = nullptr;

    UA_VariableAttributes_init(&attr);
    attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>("Name"));
    attr.description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(""));
    attr.valueRank = UA_VALUERANK_SCALAR;
    attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    UA_String opcua_node_variable_t_ns_2_i_6004_variant_DataContents = UA_STRING_ALLOC(const_cast<char*>(""));
    UA_Variant_setScalar(
        &attr.value, &opcua_node_variable_t_ns_2_i_6004_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
    addResult = UA_Server_addVariableNode(this->mappedServer,
        UA_NODEID_STRING(1, const_cast<char*>((baseNodePath + "/" + this->nameNew + "/Name").c_str())), pvNodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(1, const_cast<char*>("Name")),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, this, &createdNodeId);
    if(addResult == UA_STATUSCODE_GOOD) {
      UA_NodeId nameVariable = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_NAME);
      NODE_PAIR_PUSH(this->ownedNodes, nameVariable, createdNodeId)
      UA_NodeId_copy(&createdNodeId, &mapElemName.concreteNodeId);
      map.push_back(mapElemName);
    }
    else {
      UA_NodeId_clear(&createdNodeId);
      return addResult;
    }
    UA_NodeId_clear(&createdNodeId);
    createdNodeId = UA_NODEID_NULL;

    // Adding the 'Description' node to the PV
    UA_DataSource_Map_Element mapElemDesc;
    mapElemDesc.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_DESC);
    mapElemDesc.description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(""));
    mapElemDesc.dataSource.read = ua_readproxy_ua_processvariable_getDescription;
    mapElemDesc.dataSource.write = nullptr;

    UA_VariableAttributes_init(&attr);
    attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>("Description"));
    attr.description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(""));
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    attr.userAccessLevel = UA_ACCESSLEVELMASK_READ;
    attr.valueRank = UA_VALUERANK_SCALAR;
    attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    UA_String opcua_node_variable_t_ns_2_i_6001_variant_DataContents = UA_STRING(const_cast<char*>(""));
    UA_Variant_setScalar(
        &attr.value, &opcua_node_variable_t_ns_2_i_6001_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
    addResult = UA_Server_addVariableNode(this->mappedServer,
        UA_NODEID_STRING(1, const_cast<char*>((baseNodePath + "/" + this->nameNew + "/Description").c_str())), pvNodeId,
        UA_NODEID_NUMERIC(0, 47), UA_QUALIFIEDNAME(1, const_cast<char*>("Description")), UA_NODEID_NUMERIC(0, 63), attr,
        this, &createdNodeId);
    if(addResult == UA_STATUSCODE_GOOD) {
      UA_NodeId descVariable = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_DESC);
      NODE_PAIR_PUSH(this->ownedNodes, descVariable, createdNodeId)
      UA_NodeId_copy(&createdNodeId, &mapElemDesc.concreteNodeId);
      map.push_back(mapElemDesc);
    }
    else {
      UA_NodeId_clear(&createdNodeId);
      return addResult;
    }

    UA_NodeId_clear(&createdNodeId);
    createdNodeId = UA_NODEID_NULL;

    // Adding the 'EngineeringUnit' node to the PV
    UA_DataSource_Map_Element mapElemEU;
    mapElemEU.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_UNIT);
    mapElemEU.description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(""));
    mapElemEU.dataSource.read = ua_readproxy_ua_processvariable_getEngineeringUnit;
    mapElemEU.dataSource.write = nullptr;
    UA_VariableAttributes_init(&attr);
    attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>("EngineeringUnit"));
    attr.description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(""));
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    attr.userAccessLevel = UA_ACCESSLEVELMASK_READ;
    attr.valueRank = UA_VALUERANK_SCALAR;
    attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    UA_String defaultEngineeringUnit = UA_STRING(const_cast<char*>(""));
    UA_Variant_setScalar(&attr.value, &defaultEngineeringUnit, &UA_TYPES[UA_TYPES_STRING]);
    addResult = UA_Server_addVariableNode(this->mappedServer,
        UA_NODEID_STRING(1, const_cast<char*>((baseNodePath + "/" + this->nameNew + "/EngineeringUnit").c_str())),
        pvNodeId, UA_NODEID_NUMERIC(0, 47), UA_QUALIFIEDNAME(1, const_cast<char*>("EngineeringUnit")),
        UA_NODEID_NUMERIC(0, 63), attr, this, &createdNodeId);
    if(addResult == UA_STATUSCODE_GOOD) {
      UA_NodeId engineeringunitVariable = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_UNIT);
      NODE_PAIR_PUSH(this->ownedNodes, engineeringunitVariable, createdNodeId)
      UA_NodeId_copy(&createdNodeId, &mapElemEU.concreteNodeId);
      map.push_back(mapElemEU);
    }
    else {
      UA_NodeId_clear(&createdNodeId);
      return addResult;
    }

    UA_NodeId_clear(&createdNodeId);
    createdNodeId = UA_NODEID_NULL;

    // Adding the 'Type' node to the PV
    UA_DataSource_Map_Element mapElemType;
    mapElemType.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TYPE);
    mapElemType.description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(""));
    mapElemType.dataSource.read = ua_readproxy_ua_processvariable_getType;
    mapElemType.dataSource.write = nullptr;
    UA_VariableAttributes_init(&attr);
    attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>("Type"));
    attr.description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>("Data type used in ChimeraTK"));
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    attr.userAccessLevel = UA_ACCESSLEVELMASK_READ;
    attr.valueRank = UA_VALUERANK_SCALAR;
    attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    UA_String opcua_node_variable_t_ns_2_i_6012_variant_DataContents = UA_STRING(const_cast<char*>(""));
    UA_Variant_setScalar(
        &attr.value, &opcua_node_variable_t_ns_2_i_6012_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
    UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 6012);
    UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, 63);
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(2, 1001);
    addResult = UA_Server_addVariableNode(this->mappedServer,
        UA_NODEID_STRING(1, const_cast<char*>((baseNodePath + "/" + this->nameNew + "/Type").c_str())), pvNodeId,
        UA_NODEID_NUMERIC(0, 47), UA_QUALIFIEDNAME(1, const_cast<char*>("Type")), UA_NODEID_NUMERIC(0, 63), attr, this,
        &createdNodeId);
    if(addResult == UA_STATUSCODE_GOOD) {
      UA_NodeId typeVariable = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TYPE);
      NODE_PAIR_PUSH(this->ownedNodes, typeVariable, createdNodeId)
      UA_NodeId_copy(&createdNodeId, &mapElemType.concreteNodeId);
      map.push_back(mapElemType);
    }
    else {
      UA_NodeId_clear(&createdNodeId);
      return addResult;
    }

    UA_NodeId_clear(&createdNodeId);
    createdNodeId = UA_NODEID_NULL;

    // Adding the Validity node to the PV
    UA_DataSource_Map_Element mapElemValidity;
    mapElemValidity.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALIDITY);
    mapElemValidity.description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(""));
    mapElemValidity.dataSource.read = ua_readproxy_ua_processvariable_getValidity;
    mapElemValidity.dataSource.write = nullptr;
    UA_VariableAttributes_init(&attr);
    attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>("Validity"));
    attr.description =
        UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>("Data validity. 0: faulty, 1: ok"));
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    attr.userAccessLevel = UA_ACCESSLEVELMASK_READ;
    attr.valueRank = UA_VALUERANK_SCALAR;
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    UA_Int32 defaultValidity = -1;
    UA_Variant_setScalar(&attr.value, &defaultValidity, &UA_TYPES[UA_TYPES_INT32]);
    addResult = UA_Server_addVariableNode(this->mappedServer,
        UA_NODEID_STRING(1, const_cast<char*>((baseNodePath + "/" + this->nameNew + "/Validity").c_str())), pvNodeId,
        UA_NODEID_NUMERIC(0, 47), UA_QUALIFIEDNAME(1, const_cast<char*>("Validity")), UA_NODEID_NUMERIC(0, 63), attr,
        this, &createdNodeId);
    if(addResult == UA_STATUSCODE_GOOD) {
      UA_NodeId vadilityVariable = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALIDITY);
      NODE_PAIR_PUSH(this->ownedNodes, vadilityVariable, createdNodeId)
      UA_NodeId_copy(&createdNodeId, &mapElemValidity.concreteNodeId);
      map.push_back(mapElemValidity);
    }
    else {
      UA_NodeId_clear(&createdNodeId);
      return addResult;
    }

    UA_NodeId_clear(&createdNodeId);

    return addResult;
  }

  /** @brief Reimplement the SourceTimeStamp to timestamp of csa_config
   *
   */
  UA_DateTime ua_processvariable::getSourceTimeStamp() {
    auto t = this->csManager->getProcessVariable(this->namePV)->getVersionNumber().getTime();
    auto microseconds = std::chrono::time_point_cast<std::chrono::microseconds>(t).time_since_epoch().count();
    // For the initial value from ChimeraTK microseconds will be 0 -> use current time in this case
    if(microseconds == 0) {
      microseconds = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now())
                         .time_since_epoch()
                         .count();
    }
    return (microseconds * UA_DATETIME_USEC) + UA_DATETIME_UNIX_EPOCH;
  }

  UA_NodeId ua_processvariable::getOwnNodeId() {
    UA_NodeId outputNode;
    UA_NodeId_copy(&this->ownNodeId, &outputNode);
    return outputNode;
  }

  template<typename T>
  UA_UInt32 ua_processvariable::typeSpecificSetup(UA_DataSource_Map_Element& mapElem, const UA_NodeId nodeId) {
    UA_Int32 arrayDims;
    mapElem.dataSource.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue<T>;
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      mapElem.dataSource.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue<T>;
    }
    if(this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims = this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, nodeId, UA_TYPES[fusion::at_key<T>(typesMap)].typeId);
    return arrayDims;
  }
} // namespace ChimeraTK
