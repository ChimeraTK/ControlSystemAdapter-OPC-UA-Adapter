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
 * Copyright (c) 2018-2021 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
 */

#include "csa_config.h"
#include "csa_processvariable.h"

extern "C" {
#include "csa_namespace.h"
}

#include "ua_proxies.h"

#include <iostream>
#include <utility>
#include <vector>

ua_processvariable::ua_processvariable(UA_Server* server, UA_NodeId basenodeid, const string& namePV,
                                       boost::shared_ptr<ControlSystemPVManager> csManager, string overwriteNodeString)
    : ua_mapped_class(server, basenodeid) {
  // FIXME Check if name member of a csManager Parameter
  this->namePV = namePV;
  this->nameNew = namePV;
  this->csManager = std::move(csManager);
  this->nodeStringIdOverwrite = std::move(overwriteNodeString);
  this->type = UA_PV_UNKNOWN;
  this->array = false;

  this->mapSelfToNamespace();
}

ua_processvariable::~ua_processvariable() {
  //* Our ua_mapped_class destructor will take care of deleting our opcua footprint as long as all variables are mapped in this->ownedNodes
  UA_NodeId_clear(&this->ownNodeId);
}

UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getName(UA_Server* server, const UA_NodeId* sessionId,
                                                                          void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
                                                                          const UA_NumericRange* range, UA_DataValue* value) {
  auto* thisObj = static_cast<ua_processvariable*>(nodeContext);
  UA_String ua_val;
  char* s = (char*)malloc(thisObj->getName().length() + 1);
  strncpy(s, (char*)thisObj->getName().c_str(), thisObj->getName().length());
  ua_val.length = thisObj->getName().length();
  ua_val.data = (UA_Byte*)malloc(ua_val.length);
  memcpy(ua_val.data, s, ua_val.length);
  free(s);
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
UA_StatusCode ua_processvariable::ua_writeproxy_ua_processvariable_setEngineeringUnit(UA_Server* server,
                                                                                      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId, void* nodeContext,
                                                                                      const UA_NumericRange* range, const UA_DataValue* value) {
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

UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getEngineeringUnit(UA_Server* server,
                                                                                     const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId, void* nodeContext,
                                                                                     UA_Boolean includeSourceTimeStamp, const UA_NumericRange* range, UA_DataValue* value) {
  auto* thisObj = static_cast<ua_processvariable*>(nodeContext);
  UA_String ua_val;
  char* s = (char*)malloc(thisObj->getEngineeringUnit().length() + 1);
  strncpy(s, (char*)thisObj->getEngineeringUnit().c_str(), thisObj->getEngineeringUnit().length());
  ua_val.length = thisObj->getEngineeringUnit().length();
  ua_val.data = (UA_Byte*)malloc(ua_val.length);
  memcpy(ua_val.data, s, ua_val.length);
  free(s);

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
UA_StatusCode ua_processvariable::ua_writeproxy_ua_processvariable_setDescription(UA_Server* server,
                                                                                  const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId, void* nodeContext,
                                                                                  const UA_NumericRange* range, const UA_DataValue* value) {
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

UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getDescription(UA_Server* server,
                                                                                 const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId, void* nodeContext,
                                                                                 UA_Boolean includeSourceTimeStamp, const UA_NumericRange* range, UA_DataValue* value) {
  auto* thisObj = static_cast<ua_processvariable*>(nodeContext);
  UA_String ua_val;
  char* s = (char*)malloc(thisObj->getDescription().length() + 1);
  strncpy(s, (char*)thisObj->getDescription().c_str(), thisObj->getDescription().length());
  ua_val.length = thisObj->getDescription().length();
  ua_val.data = (UA_Byte*)malloc(ua_val.length);
  memcpy(ua_val.data, s, ua_val.length);
  free(s);
  UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
  UA_String_clear(&ua_val);
  value->hasValue = true;
  if(includeSourceTimeStamp) {
    value->sourceTimestamp = thisObj->getSourceTimeStamp();
    value->hasSourceTimestamp = true;
  }
  return UA_STATUSCODE_GOOD;
}

UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getValidity(UA_Server* server,
                                                                              const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId, void* nodeContext,
                                                                              UA_Boolean includeSourceTimeStamp, const UA_NumericRange* range, UA_DataValue* value) {
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
UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getType(UA_Server* server, const UA_NodeId* sessionId,
                                                                          void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
                                                                          const UA_NumericRange* range, UA_DataValue* value) {
  auto* thisObj = static_cast<ua_processvariable*>(nodeContext);
  UA_String ua_val;
  char* s = (char*)malloc(thisObj->getType().length() + 1);
  strncpy(s, (char*)thisObj->getType().c_str(), thisObj->getType().length());
  ua_val.length = thisObj->getType().length();
  ua_val.data = (UA_Byte*)malloc(ua_val.length);
  memcpy(ua_val.data, s, ua_val.length);
  free(s);
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
  else
    return "Unsupported type";
}

UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getValue(UA_Server* server,
                                                                           const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId, void* nodeContext,
                                                                           UA_Boolean includeSourceTimeStamp, const UA_NumericRange* range, UA_DataValue* value) {
  auto* thisObj = static_cast<ua_processvariable*>(nodeContext);
  UA_StatusCode rv = UA_STATUSCODE_GOOD;
  switch(thisObj->type) {
    case UA_PV_INT8:
      rv = thisObj->getValue_int8(&value->value);
      break;

    case UA_PV_UINT8:
      rv = thisObj->getValue_uint8(&value->value);
      break;

    case UA_PV_INT16:
      rv = thisObj->getValue_int16(&value->value);
      break;

    case UA_PV_UINT16:
      rv = thisObj->getValue_uint16(&value->value);
      break;

    case UA_PV_INT32:
      rv = thisObj->getValue_int32(&value->value);
      break;

    case UA_PV_UINT32:
      rv = thisObj->getValue_uint32(&value->value);
      break;

    case UA_PV_INT64:
      rv = thisObj->getValue_int64(&value->value);
      break;

    case UA_PV_UINT64:
      rv = thisObj->getValue_uint64(&value->value);
      break;

    case UA_PV_FLOAT:
      rv = thisObj->getValue_float(&value->value);
      break;

    case UA_PV_DOUBLE:
      rv = thisObj->getValue_double(&value->value);
      break;

    case UA_PV_STRING:
      rv = thisObj->getValue_string(&value->value);
      break;

    case UA_PV_BOOL:
      rv = thisObj->getValue_bool(&value->value);
      break;

    default:
      std::cout << "ua_readproxy_ua_processvariable_getValue: Unknown type: " << thisObj->type;
      break;
  }

  if(rv == UA_STATUSCODE_GOOD) {
    value->hasValue = true;
    if(includeSourceTimeStamp) {
      value->sourceTimestamp = thisObj->getSourceTimeStamp();
      value->hasSourceTimestamp = true;
    }
  }
  return rv;
}

UA_StatusCode ua_processvariable::getValue_int8(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_INT8) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<int8_t>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<int8_t>(this->namePV)->accessChannel(0).size() == 1) {
      uint8_t ival = this->csManager->getProcessArray<int8_t>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_SBYTE]);
    } else {
      std::vector<int8_t> iarr = this->csManager->getProcessArray<int8_t>(this->namePV)->accessChannel(0);
      rv = UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_SBYTE]);
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_uint8(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_UINT8) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<uint8_t>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<uint8_t>(this->namePV)->accessChannel(0).size() == 1) {
      uint8_t ival = this->csManager->getProcessArray<uint8_t>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_BYTE]);
    } else {
      std::vector<uint8_t> iarr = this->csManager->getProcessArray<uint8_t>(this->namePV)->accessChannel(0);
      rv = UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_BYTE]);
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_int16(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_INT16) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<int16_t>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<int16_t>(this->namePV)->accessChannel(0).size() == 1) {
      uint16_t ival = this->csManager->getProcessArray<int16_t>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_INT16]);
    } else {
      std::vector<int16_t> iarr = this->csManager->getProcessArray<int16_t>(this->namePV)->accessChannel(0);
      rv = UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_INT16]);
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_uint16(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_UINT16) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<uint16_t>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<uint16_t>(this->namePV)->accessChannel(0).size() == 1) {
      uint16_t ival = this->csManager->getProcessArray<uint16_t>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_UINT16]);
    } else {
      std::vector<uint16_t> iarr = this->csManager->getProcessArray<uint16_t>(this->namePV)->accessChannel(0);
      rv = UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_UINT16]);
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_int32(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_INT32) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<int32_t>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<int32_t>(this->namePV)->accessChannel(0).size() == 1) {
      int32_t ival = this->csManager->getProcessArray<int32_t>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_INT32]);
    } else {
      std::vector<int32_t> iarr = this->csManager->getProcessArray<int32_t>(this->namePV)->accessChannel(0);
      rv = UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_INT32]);
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_uint32(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_UINT32) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<uint32_t>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<uint32_t>(this->namePV)->accessChannel(0).size() == 1) {
      uint32_t ival = this->csManager->getProcessArray<uint32_t>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_UINT32]);
    } else {
      std::vector<uint32_t> iarr = this->csManager->getProcessArray<uint32_t>(this->namePV)->accessChannel(0);
      rv = UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_UINT32]);
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_int64(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_INT64) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<int64_t>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<int64_t>(this->namePV)->accessChannel(0).size() == 1) {
      int64_t ival = this->csManager->getProcessArray<int64_t>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_INT64]);
    } else {
      std::vector<int64_t> iarr = this->csManager->getProcessArray<int64_t>(this->namePV)->accessChannel(0);
      rv = UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_INT64]);
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_uint64(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_UINT64) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<uint64_t>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<uint64_t>(this->namePV)->accessChannel(0).size() == 1) {
      uint64_t ival = this->csManager->getProcessArray<uint64_t>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_UINT64]);
    } else {
      std::vector<uint64_t> iarr = this->csManager->getProcessArray<uint64_t>(this->namePV)->accessChannel(0);
      rv = UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_UINT64]);
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_float(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_FLOAT) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      while(this->csManager->getProcessArray<float>(this->namePV)->readNonBlocking()) {      }
    }
    if(this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0).size() == 1) {
      float dval = this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &dval, &UA_TYPES[UA_TYPES_FLOAT]);
    }
    else {
      // Array
      std::vector<float> darr = this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0);
      rv = UA_Variant_setArrayCopy(v, darr.data(), darr.size(), &UA_TYPES[UA_TYPES_FLOAT]);
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_double(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_DOUBLE) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      while(this->csManager->getProcessArray<double>(this->namePV)->readNonBlocking()) {
      }
    }
    if(this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0).size() == 1) {
      double dval = this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &dval, &UA_TYPES[UA_TYPES_DOUBLE]);
    } else {
      std::vector<double> darr = this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0);
      rv = UA_Variant_setArrayCopy(v, darr.data(), darr.size(), &UA_TYPES[UA_TYPES_DOUBLE]);
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_string(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_STRING) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<string>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).size() == 1) {
      string sval = this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).at(0);
      UA_String ua_val = CPPSTRING_TO_UASTRING(sval);
      rv = UA_Variant_setScalarCopy(v, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
      UA_String_clear(&ua_val);
    } else {
      std::vector<string> sarr = this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0);
      auto* sarrayval = new UA_String[sarr.size()];
      for(size_t i = 0; i < sarr.size(); i++) {
        sarrayval[i] = CPPSTRING_TO_UASTRING(sarr[i]);
      }
      rv = UA_Variant_setArrayCopy(v, sarrayval, sarr.size(), &UA_TYPES[UA_TYPES_STRING]);
      delete[] sarrayval;
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::getValue_bool(UA_Variant* v) {
  UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

  if(this->type == UA_PV_BOOL) {
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      while(this->csManager->getProcessArray<Boolean>(this->namePV)->readNonBlocking()) {
      }
    }
    if(this->csManager->getProcessArray<Boolean>(this->namePV)->accessChannel(0).size() == 1) {
      Boolean bval = this->csManager->getProcessArray<Boolean>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &bval, &UA_TYPES[UA_TYPES_BOOLEAN]);
    } else {
      std::vector<Boolean> bvector = this->csManager->getProcessArray<Boolean>(this->namePV)->accessChannel(0);
      auto* barr = new Boolean[bvector.size()];
      for(size_t i = 0; i < bvector.size(); i++) {
        barr[i] = bvector[i];
      }
      delete[] barr;
    }
  }

  return rv;
}

UA_StatusCode ua_processvariable::ua_writeproxy_ua_processvariable_setValue(UA_Server* server,
                                                                            const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId, void* nodeContext,
                                                                            const UA_NumericRange* range, const UA_DataValue* value) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;
  auto* theClass = static_cast<ua_processvariable*>(nodeContext);
  switch(theClass->type) {
    case UA_PV_INT8:
      retval = theClass->setValue_int8(&value->value);
      break;

    case UA_PV_UINT8:
      retval = theClass->setValue_uint8(&value->value);
      break;

    case UA_PV_INT16:
      retval = theClass->setValue_int16(&value->value);
      break;

    case UA_PV_UINT16:
      retval = theClass->setValue_uint16(&value->value);
      break;

    case UA_PV_INT32:
      retval = theClass->setValue_int32(&value->value);
      break;

    case UA_PV_UINT32:
      retval = theClass->setValue_uint32(&value->value);
      break;

    case UA_PV_INT64:
      retval = theClass->setValue_int64(&value->value);
      break;

    case UA_PV_UINT64:
      retval = theClass->setValue_uint64(&value->value);
      break;

    case UA_PV_FLOAT:
      retval = theClass->setValue_float(&value->value);
      break;

    case UA_PV_DOUBLE:
      retval = theClass->setValue_double(&value->value);
      break;

    case UA_PV_STRING:
      retval = theClass->setValue_string(&value->value);
      break;

    case UA_PV_BOOL:
      retval = theClass->setValue_bool(&value->value);
      break;

    default:
      std::cout << "Unknown type: " << theClass->type;
      break;
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_int8(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_INT8) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<int8_t> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        int8_t value = *((int8_t*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (int8_t*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<int8_t>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<int8_t>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<int8_t>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_uint8(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_UINT8) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<uint8_t> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        uint8_t value = *((uint8_t*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (uint8_t*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<uint8_t>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<uint8_t>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<uint8_t>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_int16(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_INT16) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<int16_t> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        int16_t value = *((int16_t*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (int16_t*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<int16_t>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<int16_t>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<int16_t>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_uint16(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_UINT16) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<uint16_t> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        uint16_t value = *((uint16_t*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (uint16_t*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<uint16_t>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<uint16_t>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<uint16_t>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_int32(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_INT32) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<int32_t> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        int32_t value = *((int32_t*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (int32_t*)data->data;
        valueArray.resize(data->arrayLength);
        //std::vector<int32_t> vectorizedValue(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<int32_t>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<int32_t>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<int32_t>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_uint32(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_UINT32) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<uint32_t> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        uint32_t value = *((uint32_t*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (uint32_t*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<uint32_t>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<uint32_t>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<uint32_t>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_int64(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_INT64) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<int64_t> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        int64_t value = *((int64_t*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (int64_t*)data->data;
        valueArray.resize(data->arrayLength);
        //std::vector<int64_t> vectorizedValue(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<int64_t>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<int64_t>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<int64_t>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_uint64(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_UINT64) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<uint64_t> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        uint64_t value = *((uint64_t*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (uint64_t*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<uint64_t>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<uint64_t>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<uint64_t>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_float(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_FLOAT) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<float> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        float value = *((float*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (float*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<float>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_double(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_DOUBLE) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<double> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        double value = *((double*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (double*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }

      this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<double>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_string(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_STRING) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<string> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        string cpps;
        UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)data->data)), cpps)
        //string value = *((string *)data->data);
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
        if(this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).size() !=
           data->arrayLength) {
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
  }

  return retval;
}

UA_StatusCode ua_processvariable::setValue_bool(const UA_Variant* data) {
  UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

  if(type == UA_PV_BOOL) {
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<Boolean> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        Boolean value = *((Boolean*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (Boolean*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<Boolean>(this->namePV)->accessChannel(0).size() != data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<Boolean>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<Boolean>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }
  }

  return retval;
}

UA_StatusCode ua_processvariable::mapSelfToNamespace() {
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  UA_NodeId createdNodeId = UA_NODEID_NULL;
  if(!nodeStringIdOverwrite.empty())
    this->nameNew = nodeStringIdOverwrite;

  if(UA_NodeId_equal(&this->baseNodeId, &createdNodeId))
    return UA_STATUSCODE_GOOD; // initializer should have set this!

  UA_LocalizedText description;
  description = UA_LOCALIZEDTEXT(
      (char*)"en_US", (char*)this->csManager->getProcessVariable(this->namePV)->getDescription().c_str());

  // Create our toplevel instance
  UA_VariableAttributes attr;
  UA_VariableAttributes_init(&attr);
  attr = UA_VariableAttributes_default;
  // Allow negative sampling intervals -> used by Labview and handled by open62541-interface 1.3.3.-2
  attr.minimumSamplingInterval = -1.;
  attr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)this->nameNew.c_str());
  attr.description = description;
  attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE);

  if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
    attr.writeMask = UA_ACCESSLEVELMASK_READ ^ UA_ACCESSLEVELMASK_WRITE;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ ^ UA_ACCESSLEVELMASK_WRITE;
  }

  //Append the app and application folder names to the string nodeId, this is needed
  //because the mapping allows one pv linked in more folder -> only pv path is not unique
  string baseNodeIdName;
  if(this->baseNodeId.identifierType == UA_NODEIDTYPE_STRING) {
    UA_STRING_TO_CPPSTRING_COPY(&this->baseNodeId.identifier.string, &baseNodeIdName)
  }
  if (!baseNodeIdName.empty()) {
    baseNodeIdName.resize(baseNodeIdName.size() - 3);
  }

  //check if the nodeId is used by another mapping and find next free NodeId
  UA_NodeId result;
  if(UA_Server_readDataType(this->mappedServer,
                            UA_NODEID_STRING(1, (char*)(baseNodeIdName + "/" + this->nameNew).c_str()),
                            &result) == UA_STATUSCODE_GOOD) {
    return UA_STATUSCODE_BADNODEIDEXISTS;
  }

  retval = UA_Server_addVariableNode(this->mappedServer,
                            UA_NODEID_STRING(1, (char*)(baseNodeIdName + "/" + this->nameNew).c_str()),
                            this->baseNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(1, (char*)this->nameNew.c_str()), UA_NODEID_NUMERIC(CSA_NSID, 1001), attr, (void*) this,
                            &createdNodeId);
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
  //Read is possible for all elements
  mapElem.dataSource.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
  UA_Variant uaArrayDimensions;
  UA_UInt32 arrayDims[1];

  if(valueType == typeid(int8_t)) {
    type = UA_PV_INT8;
    if(this->csManager->getProcessArray<int8_t>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<int8_t>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_BYTE].typeId);
  }
  else if(valueType == typeid(uint8_t)) {
    type = UA_PV_UINT8;
    if(this->csManager->getProcessArray<uint8_t>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<uint8_t>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_SBYTE].typeId);
  }
  else if(valueType == typeid(int16_t)) {
    type = UA_PV_INT16;
    if(this->csManager->getProcessArray<int16_t>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<int16_t>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_INT16].typeId);
  }
  else if(valueType == typeid(uint16_t)) {
    type = UA_PV_UINT16;
    if(this->csManager->getProcessArray<uint16_t>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<uint16_t>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_UINT16].typeId);
  }
  else if(valueType == typeid(int32_t)) {
    type = UA_PV_INT32;
    if(this->csManager->getProcessArray<int32_t>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<int32_t>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_INT32].typeId);
  }
  else if(valueType == typeid(uint32_t)) {
    type = UA_PV_UINT32;
    if(this->csManager->getProcessArray<uint32_t>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<uint32_t>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_UINT32].typeId);
  }
  else if(valueType == typeid(int64_t)) {
    type = UA_PV_INT64;
    if(this->csManager->getProcessArray<int64_t>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<int64_t>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_INT64].typeId);
  }
  else if(valueType == typeid(uint64_t)) {
    type = UA_PV_UINT64;
    if(this->csManager->getProcessArray<uint64_t>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<uint64_t>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_UINT64].typeId);
  }
  else if(valueType == typeid(float)) {
    type = UA_PV_FLOAT;
    if(this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_FLOAT].typeId);
  }
  else if(valueType == typeid(double)) {
    type = UA_PV_DOUBLE;
    if(this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_DOUBLE].typeId);
  }
  else if(valueType == typeid(string)) {
    type = UA_PV_STRING;
    if(this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_STRING].typeId);
  }
  else if(valueType == typeid(Boolean)) {
    type = UA_PV_BOOL;
    if(this->csManager->getProcessArray<Boolean>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims[0] = this->csManager->getProcessArray<Boolean>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, createdNodeId, UA_TYPES[UA_TYPES_BOOLEAN].typeId);
  }
  else {
    int status;
    auto* demangledName = abi::__cxa_demangle(valueType.name(), nullptr, nullptr, &status);
    std::cout << "Cannot proxy unknown type ";
    if(status == 0) {
      std::cout << demangledName;
      free(demangledName);
    }
    else {
      std::cout << " (demangling failed) " << valueType.name();
    }
    std::cout << " for variable " << this->namePV << std::endl;
  }

  if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
    mapElem.dataSource.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
  } else {
    mapElem.dataSource.write = nullptr;
  }
  if(this->array){
    UA_Server_writeValueRank(this->mappedServer, createdNodeId, UA_VALUERANK_ONE_DIMENSION);
    UA_Variant_setArray(&uaArrayDimensions, arrayDims, 1, &UA_TYPES[UA_TYPES_UINT32]);
    UA_Server_writeArrayDimensions(this->mappedServer, createdNodeId, uaArrayDimensions);
  } else {
    UA_Server_writeValueRank(this->mappedServer, createdNodeId, UA_VALUERANK_SCALAR);
  }

  //add variable data source map element to the list
  UA_NodeId_copy(&createdNodeId, &mapElem.concreteNodeId);
  mapDs.push_back(mapElem);

  UA_NodeId nodeIdVariableType = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE);
  NODE_PAIR_PUSH(this->ownedNodes, nodeIdVariableType, createdNodeId)

  for(auto i: mapDs){
    retval |= UA_Server_setVariableNode_dataSource(this->mappedServer, i.concreteNodeId, i.dataSource);
  }
  UA_NodeId_clear(&createdNodeId);
  return retval;
}

UA_StatusCode ua_processvariable::addPVChildNodes(UA_NodeId pvNodeId, const string& baseNodePath, UA_DataSource_Map &map) {
  UA_NodeId createdNodeId = UA_NODEID_NULL;
  UA_VariableAttributes attr;
  UA_StatusCode addResult;

  // Adding the 'name' node to the PV
  UA_DataSource_Map_Element mapElemName;
  mapElemName.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_NAME);
  mapElemName.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"");
  mapElemName.dataSource.read = ua_readproxy_ua_processvariable_getName;
  mapElemName.dataSource.write = nullptr;

  UA_VariableAttributes_init(&attr);
  attr = UA_VariableAttributes_default;
  attr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"Name");
  attr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"");
  attr.valueRank = UA_VALUERANK_SCALAR;
  attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
  UA_String opcua_node_variable_t_ns_2_i_6004_variant_DataContents = UA_STRING_ALLOC((char*)"");
  UA_Variant_setScalar(
      &attr.value, &opcua_node_variable_t_ns_2_i_6004_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
  addResult = UA_Server_addVariableNode(this->mappedServer,
                                        UA_NODEID_STRING(1, (char*)(baseNodePath + "/" + this->nameNew + "/Name").c_str()), pvNodeId,
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(1, (char*)"Name"),
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, this, &createdNodeId);
  if(addResult == UA_STATUSCODE_GOOD) {
    UA_NodeId nameVariable = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_NAME);
    NODE_PAIR_PUSH(this->ownedNodes, nameVariable, createdNodeId)
    UA_NodeId_copy(&createdNodeId, &mapElemName.concreteNodeId);
    map.push_back(mapElemName);
  }
  else
    return addResult;

  UA_NodeId_clear(&createdNodeId);
  createdNodeId = UA_NODEID_NULL;

  //Adding the 'Description' node to the PV
  UA_DataSource_Map_Element mapElemDesc;
  mapElemDesc.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_DESC);
  mapElemDesc.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"");
  mapElemDesc.dataSource.read = ua_readproxy_ua_processvariable_getDescription;
  mapElemDesc.dataSource.write = nullptr;

  UA_VariableAttributes_init(&attr);
  attr = UA_VariableAttributes_default;
  attr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"Description");
  attr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"");
  attr.accessLevel = UA_ACCESSLEVELMASK_READ ;
  attr.userAccessLevel = UA_ACCESSLEVELMASK_READ ;
  attr.valueRank = UA_VALUERANK_SCALAR;
  attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
  UA_String opcua_node_variable_t_ns_2_i_6001_variant_DataContents = UA_STRING((char*)"");
  UA_Variant_setScalar(
      &attr.value, &opcua_node_variable_t_ns_2_i_6001_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
  addResult = UA_Server_addVariableNode(this->mappedServer,
                                        UA_NODEID_STRING(1, (char*)(baseNodePath + "/" + this->nameNew + "/Description").c_str()), pvNodeId,
                                        UA_NODEID_NUMERIC(0, 47), UA_QUALIFIEDNAME(1, (char*)"Description"), UA_NODEID_NUMERIC(0, 63), attr, this,
                                        &createdNodeId);
  if(addResult == UA_STATUSCODE_GOOD) {
    UA_NodeId descVariable = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_DESC);
    NODE_PAIR_PUSH(this->ownedNodes, descVariable, createdNodeId)
    UA_NodeId_copy(&createdNodeId, &mapElemDesc.concreteNodeId);
    map.push_back(mapElemDesc);
  }
  else
    return addResult;

  UA_NodeId_clear(&createdNodeId);
  createdNodeId = UA_NODEID_NULL;

  //Adding the 'EngineeringUnit' node to the PV
  UA_DataSource_Map_Element mapElemEU;
  mapElemEU.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_UNIT);
  mapElemEU.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"");
  mapElemEU.dataSource.read = ua_readproxy_ua_processvariable_getEngineeringUnit;
  mapElemEU.dataSource.write = nullptr;
  UA_VariableAttributes_init(&attr);
  attr = UA_VariableAttributes_default;
  attr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"EngineeringUnit");
  attr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"");
  attr.accessLevel = UA_ACCESSLEVELMASK_READ ;
  attr.userAccessLevel = UA_ACCESSLEVELMASK_READ ;
  attr.valueRank = UA_VALUERANK_SCALAR;
  attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
  UA_String defaultEngineeringUnit = UA_STRING((char*)"");
  UA_Variant_setScalar(&attr.value, &defaultEngineeringUnit, &UA_TYPES[UA_TYPES_STRING]);
  addResult = UA_Server_addVariableNode(this->mappedServer,
                                        UA_NODEID_STRING(1, (char*)(baseNodePath + "/" + this->nameNew + "/EngineeringUnit").c_str()), pvNodeId,
                                        UA_NODEID_NUMERIC(0, 47), UA_QUALIFIEDNAME(1, (char*)"EngineeringUnit"), UA_NODEID_NUMERIC(0, 63), attr, this,
                                        &createdNodeId);
  if(addResult == UA_STATUSCODE_GOOD) {
    UA_NodeId engineeringunitVariable = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_UNIT);
    NODE_PAIR_PUSH(this->ownedNodes, engineeringunitVariable, createdNodeId)
    UA_NodeId_copy(&createdNodeId, &mapElemEU.concreteNodeId);
    map.push_back(mapElemEU);
  }
  else
    return addResult;

  UA_NodeId_clear(&createdNodeId);
  createdNodeId = UA_NODEID_NULL;

  //Adding the 'Type' node to the PV
  UA_DataSource_Map_Element mapElemType;
  mapElemType.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TYPE);
  mapElemType.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"");
  mapElemType.dataSource.read = ua_readproxy_ua_processvariable_getType;
  mapElemType.dataSource.write = nullptr;
  UA_VariableAttributes_init(&attr);
  attr = UA_VariableAttributes_default;
  attr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"Type");
  attr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"");
  attr.accessLevel = UA_ACCESSLEVELMASK_READ ;
  attr.userAccessLevel = UA_ACCESSLEVELMASK_READ ;
  attr.valueRank = UA_VALUERANK_SCALAR;
  attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
  UA_String opcua_node_variable_t_ns_2_i_6012_variant_DataContents = UA_STRING((char*)"");
  UA_Variant_setScalar(
      &attr.value, &opcua_node_variable_t_ns_2_i_6012_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
  UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 6012);
  UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, 63);
  UA_NodeId parentNodeId = UA_NODEID_NUMERIC(2, 1001);
  addResult = UA_Server_addVariableNode(this->mappedServer,
                                        UA_NODEID_STRING(1, (char*)(baseNodePath + "/" + this->nameNew + "/Type").c_str()), pvNodeId,
                                        UA_NODEID_NUMERIC(0, 47), UA_QUALIFIEDNAME(1, (char*)"Type"), UA_NODEID_NUMERIC(0, 63), attr, this,
                                        &createdNodeId);
  if(addResult == UA_STATUSCODE_GOOD) {
    UA_NodeId typeVariable = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TYPE);
    NODE_PAIR_PUSH(this->ownedNodes, typeVariable, createdNodeId)
    UA_NodeId_copy(&createdNodeId, &mapElemType.concreteNodeId);
    map.push_back(mapElemType);
  }
  else
    return addResult;

  UA_NodeId_clear(&createdNodeId);
  createdNodeId = UA_NODEID_NULL;

  //Adding the Validity node to the PV
  UA_DataSource_Map_Element mapElemValidity;
  mapElemValidity.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALIDITY);
  mapElemValidity.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"");
  mapElemValidity.dataSource.read = ua_readproxy_ua_processvariable_getValidity;
  mapElemValidity.dataSource.write = nullptr;
  UA_VariableAttributes_init(&attr);
  attr = UA_VariableAttributes_default;
  attr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"Validity");
  attr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"");
  attr.accessLevel = UA_ACCESSLEVELMASK_READ ;
  attr.userAccessLevel = UA_ACCESSLEVELMASK_READ ;
  attr.valueRank = UA_VALUERANK_SCALAR;
  attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
  UA_Int32 defaultValidity = -1;
  UA_Variant_setScalar(&attr.value, &defaultValidity, &UA_TYPES[UA_TYPES_INT32]);
  addResult = UA_Server_addVariableNode(this->mappedServer,
                                        UA_NODEID_STRING(1, (char*)(baseNodePath + "/" + this->nameNew + "/Validity").c_str()), pvNodeId,
                                        UA_NODEID_NUMERIC(0, 47), UA_QUALIFIEDNAME(1, (char*)"Validity"), UA_NODEID_NUMERIC(0, 63), attr, this,
                                        &createdNodeId);
  if(addResult == UA_STATUSCODE_GOOD) {
    UA_NodeId vadilityVariable = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALIDITY);
    NODE_PAIR_PUSH(this->ownedNodes, vadilityVariable, createdNodeId)
    UA_NodeId_copy(&createdNodeId, &mapElemValidity.concreteNodeId);
    map.push_back(mapElemValidity);
  }
  else
    return addResult;

  UA_NodeId_clear(&createdNodeId);

  return addResult;
}

/** @brief Reimplement the SourceTimeStamp to timestamp of csa_config
 *
 */
UA_DateTime ua_processvariable::getSourceTimeStamp() {
  auto t = this->csManager->getProcessVariable(this->namePV)->getVersionNumber().getTime();
  auto microseconds = std::chrono::time_point_cast<std::chrono::microseconds>(t).time_since_epoch().count();
  return (microseconds * UA_DATETIME_USEC) + UA_DATETIME_UNIX_EPOCH;
}

UA_NodeId ua_processvariable::getOwnNodeId() {
  UA_NodeId outputNode;
  UA_NodeId_copy(&this->ownNodeId, &outputNode);
  return outputNode;
}
