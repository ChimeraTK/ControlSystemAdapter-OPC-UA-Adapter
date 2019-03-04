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
 */


#include "ua_processvariable.h"
#include "csa_config.h"

extern "C" {
#include "csa_namespaceinit_generated.h"
}

#include "ua_proxies.h"
//#include "ua_proxies_callback.h"

#include <iostream>
#include <vector>

ua_processvariable::ua_processvariable(UA_Server* server, UA_NodeId basenodeid, string namePV, boost::shared_ptr<ControlSystemPVManager> csManager) : ua_mapped_class(server, basenodeid) {

        // FIXME Check if name member of a csManager Parameter
        this->namePV = namePV;
        this->nameNew = namePV;
        this->csManager = csManager;
        this->type = UA_PV_UNKNOWN;
        this->array = false;

        this->mapSelfToNamespace();
}

ua_processvariable::~ua_processvariable()
{
  //* Our ua_mapped_class destructor will take care of deleting our opcua footprint as long as all variables are mapped in this->ownedNodes
}

// Name
//UA_RDPROXY_STRING(ua_processvariable, getName)
UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getName(void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value) {
    ua_processvariable *thisObj = static_cast<ua_processvariable *>(handle);
    UA_String ua_val;
    //{
    char *s = (char *) malloc(thisObj->getName().length() + 1);
    strncpy(s, (char*) thisObj->getName().c_str(), thisObj->getName().length());
    ua_val.length = thisObj->getName().length();
    ua_val.data = (UA_Byte *) malloc(ua_val.length);
    memcpy(ua_val.data, s, ua_val.length);
    free(s);
    //};
    UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_deleteMembers(&ua_val);
    value->hasValue = true;
    if (includeSourceTimeStamp) {
        value->sourceTimestamp = thisObj->getSourceTimeStamp();
        value->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
}

string ua_processvariable::getName() {
  return this->namePV;
}

// EngineeringUnit
UA_StatusCode ua_processvariable::ua_writeproxy_ua_processvariable_setEngineeringUnit(void *handle, const UA_NodeId nodeid, const UA_Variant *data, const UA_NumericRange *range) {
    ua_processvariable *theClass = static_cast<ua_processvariable *>(handle);
    std::string cpps;
    do {
        char *s;
        {
            s = (char *) malloc(((UA_String) *((UA_String *) data->data)).length + 1);
            memset(s, 0, ((UA_String) *((UA_String *) data->data)).length + 1);
            memcpy(s, ((UA_String) *((UA_String *) data->data)).data, ((UA_String) *((UA_String *) data->data)).length);
        }
        cpps.assign(s, ((UA_String) *((UA_String *) data->data)).length);
        free(s);
    }
    while (0);
    theClass->setEngineeringUnit(cpps);
    return UA_STATUSCODE_GOOD;
}

//UA_WRPROXY_STRING(ua_processvariable, setEngineeringUnit)
void ua_processvariable::setEngineeringUnit(string engineeringUnit) {
        this->engineeringUnit = engineeringUnit;
}

UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getEngineeringUnit(void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value) {
    ua_processvariable *thisObj = static_cast<ua_processvariable *>(handle);
    UA_String ua_val;
    {
        char *s = (char *) malloc(thisObj->getEngineeringUnit().length() + 1);
        strncpy(s, (char*) thisObj->getEngineeringUnit().c_str(), thisObj->getEngineeringUnit().length());
        ua_val.length = thisObj->getEngineeringUnit().length();
        ua_val.data = (UA_Byte *) malloc(ua_val.length);
        memcpy(ua_val.data, s, ua_val.length);
        free(s);
    };
    UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_deleteMembers(&ua_val);
    value->hasValue = true;
    if (includeSourceTimeStamp) {
        value->sourceTimestamp = thisObj->getSourceTimeStamp();
        value->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
}

//UA_RDPROXY_STRING(ua_processvariable, getEngineeringUnit)
string ua_processvariable::getEngineeringUnit() {
        if(!this->engineeringUnit.empty()) {
                return this->engineeringUnit;
        }
        else {
                this->engineeringUnit = this->csManager->getProcessVariable(this->namePV)->getUnit();
                return this->engineeringUnit;
        }
        return this->csManager->getProcessVariable(this->namePV)->getUnit();
}

// Description
UA_StatusCode ua_processvariable::ua_writeproxy_ua_processvariable_setDescription(void *handle, const UA_NodeId nodeid, const UA_Variant *data, const UA_NumericRange *range) {
    ua_processvariable *theClass = static_cast<ua_processvariable *>(handle);
    std::string cpps;
    do {
        char *s;
        {
            s = (char *) malloc(((UA_String) *((UA_String *) data->data)).length + 1);
            memset(s, 0, ((UA_String) *((UA_String *) data->data)).length + 1);
            memcpy(s, ((UA_String) *((UA_String *) data->data)).data, ((UA_String) *((UA_String *) data->data)).length);
        }
        cpps.assign(s, ((UA_String) *((UA_String *) data->data)).length);
        free(s);
    }
    while (0);
    theClass->setDescription(cpps);
    return UA_STATUSCODE_GOOD;
}

//UA_WRPROXY_STRING(ua_processvariable, setDescription)
void ua_processvariable::setDescription(string description) {
        this->description = description;
}

UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getDescription(void *handle, const UA_NodeId nodeid,
        UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value) {
    ua_processvariable *thisObj = static_cast<ua_processvariable *>(handle);
    UA_String ua_val;
    {
        char *s = (char *) malloc(thisObj->getDescription().length() + 1);
        strncpy(s, (char*) thisObj->getDescription().c_str(), thisObj->getDescription().length());
        ua_val.length = thisObj->getDescription().length();
        ua_val.data = (UA_Byte *) malloc(ua_val.length);
        memcpy(ua_val.data, s, ua_val.length);
        free(s);
    };
    UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_deleteMembers(&ua_val);
    value->hasValue = true;
    if (includeSourceTimeStamp) {
        value->sourceTimestamp = thisObj->getSourceTimeStamp();
        value->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
}

//UA_RDPROXY_STRING(ua_processvariable, getDescription)
string ua_processvariable::getDescription() {
        if(!this->description.empty()) {
                return this->description;
        }
        else {
                this->description = this->csManager->getProcessVariable(this->namePV)->getDescription();
                return this->description;
        }
        return this->csManager->getProcessVariable(this->namePV)->getDescription();
}

// Type
UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getType(void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value) {
    ua_processvariable *thisObj = static_cast<ua_processvariable *>(handle);
    UA_String ua_val;
    {
        char *s = (char *) malloc(thisObj->getType().length() + 1);
        strncpy(s, (char*) thisObj->getType().c_str(), thisObj->getType().length());
        ua_val.length = thisObj->getType().length();
        ua_val.data = (UA_Byte *) malloc(ua_val.length);
        memcpy(ua_val.data, s, ua_val.length);
        free(s);
    };
    UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[11]);
    UA_String_deleteMembers(&ua_val);
    value->hasValue = true;
    if (includeSourceTimeStamp) {
        value->sourceTimestamp = thisObj->getSourceTimeStamp();
        value->hasSourceTimestamp = true;
    }
    return UA_STATUSCODE_GOOD;
}

//UA_RDPROXY_STRING(ua_processvariable, getType)
string ua_processvariable::getType() {
    // Note: typeid().name() may return the name; may as well return the symbol's name from the binary though...
    std::type_info const & valueType = this->csManager->getProcessVariable(this->namePV)->getValueType();
    if (valueType == typeid(int8_t))        return "int8_t";
    else if (valueType == typeid(uint8_t))  return "uint8_t";
    else if (valueType == typeid(int16_t))  return "int16_t";
    else if (valueType == typeid(uint16_t)) return "uint16_t";
    else if (valueType == typeid(int32_t))  return "int32_t";
    else if (valueType == typeid(uint32_t)) return "uint32_t";
    else if (valueType == typeid(int64_t))  return "int64_t";
    else if (valueType == typeid(uint64_t)) return "uint64_t";
    else if (valueType == typeid(float))    return "float";
    else if (valueType == typeid(double))   return "double";
    else if (valueType == typeid(string))   return "string";
    else                                    return "Unsupported type";
}

/* Multivariant Read Functions for Value (without template-Foo) */
//#define CREATE_READ_FUNCTION(_p_type) \
//_p_type    ua_processvariable::getValue_##_p_type() { \
//    _p_type v = _p_type(); \
//    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return 0; \
//    if (this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).size() == 1) { \
//                        if(this->csManager->getProcessVariable(this->namePV)->isReadable()) { \
//                                while(this->csManager->getProcessArray<_p_type>(this->namePV)->readNonBlocking()) {} \
//                        } \
//                        v = this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).at(0); \
//                } \
//    return v; \
//} \


//#define CREATE_READ_FUNCTION_ARRAY(_p_type) \
//std::vector<_p_type>    ua_processvariable::getValue_Array_##_p_type() { \
//    std::vector<_p_type> v; \
//    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return v; \
//    if (this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).size() > 1) { \
//                        if(this->csManager->getProcessVariable(this->namePV)->isReadable()) { \
//                                while(this->csManager->getProcessArray<_p_type>(this->namePV)->readNonBlocking()) {} \
//                        } \
//                        v = this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0); \
//                } \
//    return v; \
//} \


//#define CREATE_WRITE_FUNCTION(_p_type) \
//void ua_processvariable::setValue_##_p_type(_p_type value) { \
//    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return; \
//    if (this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).size() == 1) {   \
//                        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) { \
//                                vector<_p_type> valueArray; \
//                                valueArray.push_back(value); \
//                                this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0) = valueArray; \
//                                this->csManager->getProcessArray<_p_type>(this->namePV)->write(); \
//                        } \
//                } \
//    return; \
//}


//#define CREATE_WRITE_FUNCTION_ARRAY(_p_type) \
//void ua_processvariable::setValue_Array_##_p_type(std::vector<_p_type> value) { \
//    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return; \
//    if (this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).size() <= 1) return; \
//                        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) { \
//                                int32_t valueSize = this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).size(); \
//                                value.resize(valueSize); \
//                                this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0) = value; \
//                                this->csManager->getProcessArray<_p_type>(this->namePV)->write(); \
//                } \
//        return; \
//}

UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getValue(void *handle,
    const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp,
    const UA_NumericRange *range, UA_DataValue *value) {
    ua_processvariable *thisObj = static_cast<ua_processvariable *>(handle);

    UA_StatusCode rv;
    switch (thisObj->type) {
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
    }

    if (rv == UA_STATUSCODE_GOOD) {
        value->hasValue = true;
        if (includeSourceTimeStamp) {
            value->sourceTimestamp = thisObj->getSourceTimeStamp();
            value->hasSourceTimestamp = true;
        }
    }
    return rv;
}

UA_StatusCode ua_processvariable::getValue_int8(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_INT8) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            this->csManager->getProcessArray <int8_t>(this->namePV)->readLatest();
        }
        if (this->csManager->getProcessArray <int8_t>(this->namePV)->accessChannel(0).size() == 1) {
            uint8_t ival = this->csManager->getProcessArray <int8_t>(this->namePV)->accessChannel(0).at(0);
            rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_SBYTE]);
        }
        else {
            // Array
            std::vector<int8_t> iarr = this->csManager->getProcessArray<int8_t>(this->namePV)->accessChannel(0);
            UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_SBYTE]);
            UA_NumericRange arrayRange;
            arrayRange.dimensionsSize = 1;
            UA_NumericRangeDimension
                scalarThisDimension = (UA_NumericRangeDimension)
            {
                .min = 0, .max = (unsigned)iarr.size()
            };
            arrayRange.dimensions = &scalarThisDimension;
            rv = UA_Variant_setRangeCopy(v, (UA_SByte *)iarr.data(), iarr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::getValue_uint8(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_UINT8) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            this->csManager->getProcessArray <uint8_t>(this->namePV)->readLatest();
        }
        if (this->csManager->getProcessArray <uint8_t>(this->namePV)->accessChannel(0).size() == 1) {
            uint8_t ival = this->csManager->getProcessArray <uint8_t>(this->namePV)->accessChannel(0).at(0);
            rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_BYTE]);
        }
        else {
            // Array
            std::vector<uint8_t> iarr = this->csManager->getProcessArray<uint8_t>(this->namePV)->accessChannel(0);
            UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_BYTE]);
            UA_NumericRange arrayRange;
            arrayRange.dimensionsSize = 1;
            UA_NumericRangeDimension
                scalarThisDimension = (UA_NumericRangeDimension)
            {
                .min = 0, .max = (unsigned)iarr.size()
            };
            arrayRange.dimensions = &scalarThisDimension;
            rv = UA_Variant_setRangeCopy(v, (UA_Byte *)iarr.data(), iarr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::getValue_int16(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_INT16) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            this->csManager->getProcessArray <int16_t>(this->namePV)->readLatest();
        }
        if (this->csManager->getProcessArray <int16_t>(this->namePV)->accessChannel(0).size() == 1) {
            uint16_t ival = this->csManager->getProcessArray <int16_t>(this->namePV)->accessChannel(0).at(0);
            rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_INT16]);
        }
        else {
            // Array
            std::vector<int16_t> iarr = this->csManager->getProcessArray<int16_t>(this->namePV)->accessChannel(0);
            UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_INT16]);
            UA_NumericRange arrayRange;
            arrayRange.dimensionsSize = 1;
            UA_NumericRangeDimension
                scalarThisDimension = (UA_NumericRangeDimension)
            {
                .min = 0, .max = (unsigned)iarr.size()
            };
            arrayRange.dimensions = &scalarThisDimension;
            rv = UA_Variant_setRangeCopy(v, (UA_Int16 *)iarr.data(), iarr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::getValue_uint16(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_UINT16) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            this->csManager->getProcessArray <uint16_t>(this->namePV)->readLatest();
        }
        if (this->csManager->getProcessArray <uint16_t>(this->namePV)->accessChannel(0).size() == 1) {
            uint16_t ival = this->csManager->getProcessArray <uint16_t>(this->namePV)->accessChannel(0).at(0);
            rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_UINT16]);
        }
        else {
            // Array
            std::vector<uint16_t> iarr = this->csManager->getProcessArray<uint16_t>(this->namePV)->accessChannel(0);
            UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_UINT16]);
            UA_NumericRange arrayRange;
            arrayRange.dimensionsSize = 1;
            UA_NumericRangeDimension
                scalarThisDimension = (UA_NumericRangeDimension)
            {
                .min = 0, .max = (unsigned)iarr.size()
            };
            arrayRange.dimensions = &scalarThisDimension;
            rv = UA_Variant_setRangeCopy(v, (UA_UInt16 *)iarr.data(), iarr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::getValue_int32(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_INT32) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            this->csManager->getProcessArray <int32_t>(this->namePV)->readLatest();
        }
        if (this->csManager->getProcessArray <int32_t>(this->namePV)->accessChannel(0).size() == 1) {
            int32_t ival = this->csManager->getProcessArray <int32_t>(this->namePV)->accessChannel(0).at(0);
            rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_INT32]);
        }
        else {
            // Array
            std::vector<int32_t> iarr = this->csManager->getProcessArray<int32_t>(this->namePV)->accessChannel(0);
            UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(),
                &UA_TYPES[UA_TYPES_INT32]);
            UA_NumericRange arrayRange;
            arrayRange.dimensionsSize = 1;
            UA_NumericRangeDimension
                scalarThisDimension = (UA_NumericRangeDimension)
            {
                .min = 0, .max = (unsigned)iarr.size()
            };
            arrayRange.dimensions = &scalarThisDimension;
            rv = UA_Variant_setRangeCopy(v, (UA_Int32 *)iarr.data(), iarr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::getValue_uint32(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_UINT32) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            this->csManager->getProcessArray <uint32_t>(this->namePV)->readLatest();
        }
        if (this->csManager->getProcessArray <uint32_t>(this->namePV)->accessChannel(0).size() == 1) {
            uint32_t ival = this->csManager->getProcessArray <uint32_t>(this->namePV)->accessChannel(0).at(0);
            rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_UINT32]);
        }
        else {
            // Array
            std::vector<uint32_t> iarr = this->csManager->getProcessArray<uint32_t>(this->namePV)->accessChannel(0);
            UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_UINT32]);
            UA_NumericRange arrayRange;
            arrayRange.dimensionsSize = 1;
            UA_NumericRangeDimension
                scalarThisDimension = (UA_NumericRangeDimension)
            {
                .min = 0, .max = (unsigned)iarr.size()
            };
            arrayRange.dimensions = &scalarThisDimension;
            rv = UA_Variant_setRangeCopy(v, (UA_UInt32 *)iarr.data(), iarr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::getValue_int64(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_INT64) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            this->csManager->getProcessArray <int64_t>(this->namePV)->readLatest();
        }
        if (this->csManager->getProcessArray <int64_t>(this->namePV)->accessChannel(0).size() == 1) {
            int64_t ival = this->csManager->getProcessArray <int64_t>(this->namePV)->accessChannel(0).at(0);
            rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_INT64]);
        }
        else {
            // Array
            std::vector<int64_t> iarr = this->csManager->getProcessArray<int64_t>(this->namePV)->accessChannel(0);
            UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_INT64]);
            UA_NumericRange arrayRange;
            arrayRange.dimensionsSize = 1;
            UA_NumericRangeDimension
                scalarThisDimension = (UA_NumericRangeDimension)
            {
                .min = 0, .max = (unsigned)iarr.size()
            };
            arrayRange.dimensions = &scalarThisDimension;
            rv = UA_Variant_setRangeCopy(v, (UA_Int64 *)iarr.data(), iarr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::getValue_uint64(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_UINT64) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            this->csManager->getProcessArray <uint64_t>(this->namePV)->readLatest();
        }
        if (this->csManager->getProcessArray <uint64_t>(this->namePV)->accessChannel(0).size() == 1) {
            uint64_t ival = this->csManager->getProcessArray <uint64_t>(this->namePV)->accessChannel(0).at(0);
            rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[UA_TYPES_UINT64]);
        }
        else {
            // Array
            std::vector<uint64_t> iarr = this->csManager->getProcessArray<uint64_t>(this->namePV)->accessChannel(0);
            UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[UA_TYPES_UINT64]);
            UA_NumericRange arrayRange;
            arrayRange.dimensionsSize = 1;
            UA_NumericRangeDimension
                scalarThisDimension = (UA_NumericRangeDimension)
            {
                .min = 0, .max = (unsigned)iarr.size()
            };
            arrayRange.dimensions = &scalarThisDimension;
            rv = UA_Variant_setRangeCopy(v, (UA_UInt64 *)iarr.data(), iarr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::getValue_float(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_FLOAT) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            this->csManager->getProcessArray<float>(this->namePV)->readLatest();
        }
        if (this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0).size() == 1) {
            float dval = this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0).at(0);
            rv = UA_Variant_setScalarCopy(v, &dval, &UA_TYPES[UA_TYPES_FLOAT]);
        }
        else {
            // Array
            std::vector<float> darr = this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0);
            UA_Variant_setArrayCopy(v, darr.data(), darr.size(), &UA_TYPES[UA_TYPES_FLOAT]);
            UA_NumericRange arrayRange;
            arrayRange.dimensionsSize = 1;
            UA_NumericRangeDimension scalarThisDimension = (UA_NumericRangeDimension)
            {
                .min = 0, .max = (unsigned)darr.size()
            };
            arrayRange.dimensions = &scalarThisDimension;
            rv = UA_Variant_setRangeCopy(v, (UA_Float *)darr.data(), darr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::getValue_double(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_DOUBLE) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            while (this->csManager->getProcessArray<double>(this->namePV)->readNonBlocking()) {
            }
        }
        if (this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0).size() == 1) {
            double dval = this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0).at(0);
            rv = UA_Variant_setScalarCopy(v, &dval, &UA_TYPES[UA_TYPES_DOUBLE]);
        }
        else {
            // Array
            std::vector<double> darr = this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0);
            UA_Variant_setArrayCopy(v, darr.data(), darr.size(), &UA_TYPES[UA_TYPES_DOUBLE]);
            UA_NumericRange arrayRange;
            arrayRange.dimensionsSize = 1;
            UA_NumericRangeDimension
                scalarThisDimension = (UA_NumericRangeDimension)
            {
                .min = 0, .max = (unsigned)darr.size()
            };
            arrayRange.dimensions = &scalarThisDimension;
            rv = UA_Variant_setRangeCopy(v, (UA_Double *)darr.data(), darr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::getValue_string(UA_Variant* v) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;

    if (this->type == UA_PV_STRING) {
        if (this->csManager->getProcessVariable(this->namePV)->isReadable()) {
            this->csManager->getProcessArray<string>(this->namePV)->readLatest();
        }
        if (this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).size() == 1) {
            string sval = this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).at(0);
            UA_String ua_val;
            CPPSTRING_TO_UASTRING(ua_val, sval);
            rv = UA_Variant_setScalarCopy(v, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
            UA_String_deleteMembers(&ua_val);
        }
        else {
            // Array
            // currently not implemented
            rv = UA_STATUSCODE_BADNOTIMPLEMENTED;
            //            std::vector<string> darr = this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0);
            //            UA_Variant_setArrayCopy(v, darr.data(), darr.size(), &UA_TYPES[UA_TYPES_STRING]);
            //            UA_NumericRange arrayRange;
            //            arrayRange.dimensionsSize = 1;
            //            UA_NumericRangeDimension
            //                scalarThisDimension = (UA_NumericRangeDimension)
            //            {
            //                .min = 0, .max = (unsigned)darr.size()
            //            };
            //            arrayRange.dimensions = &scalarThisDimension;
            //            rv = UA_Variant_setRangeCopy(v, (UA_String *)darr.data(), darr.size(), arrayRange);
        }
    }

    return rv;
}

UA_StatusCode ua_processvariable::ua_writeproxy_ua_processvariable_setValue(void *handle, const UA_NodeId nodeid, const UA_Variant *data, const UA_NumericRange *range) {
    UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;
    ua_processvariable *theClass = static_cast<ua_processvariable *> (handle);
    switch (theClass->type) {
    case UA_PV_INT8:
        retval = theClass->setValue_int8(data);
        break;

    case UA_PV_UINT8:
        retval = theClass->setValue_uint8(data);
        break;

    case UA_PV_INT16:
        retval = theClass->setValue_int16(data);
        break;

    case UA_PV_UINT16:
        retval = theClass->setValue_uint16(data);
        break;

    case UA_PV_INT32:
        retval = theClass->setValue_int32(data);
        break;

    case UA_PV_UINT32:
        retval = theClass->setValue_uint32(data);
        break;

    case UA_PV_INT64:
        retval = theClass->setValue_int64(data);
        break;

    case UA_PV_UINT64:
        retval = theClass->setValue_uint64(data);
        break;

    case UA_PV_FLOAT:
        retval = theClass->setValue_float(data);
        break;

    case UA_PV_DOUBLE:
        retval = theClass->setValue_double(data);
        break;

    case UA_PV_STRING:
        retval = theClass->setValue_string(data);
        break;

    default:
        std::cout << "Unknown type: " << theClass->type;
        break;
    }

    return retval;
}

UA_StatusCode ua_processvariable::setValue_int8(const UA_Variant* data) {
    UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

    if (type == UA_PV_INT8) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<int8_t> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                int8_t value = *((int8_t *)data->data);
                valueArray.push_back(value);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                int8_t* v = (int8_t *)data->data;
                valueArray.resize(data->arrayLength);
                for (uint32_t i = 0; i < valueArray.size(); i++) {
                    valueArray.at(i) = v[i];
                }
            }
            this->csManager->getProcessArray <int8_t>(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray <int8_t>(this->namePV)->write();
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

    if (type == UA_PV_UINT8) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<uint8_t> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                uint8_t value = *((uint8_t *)data->data);
                valueArray.push_back(value);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                uint8_t* v = (uint8_t *)data->data;
                valueArray.resize(data->arrayLength);
                for (uint32_t i = 0; i < valueArray.size(); i++) {
                    valueArray.at(i) = v[i];
                }
            }
            this->csManager->getProcessArray <uint8_t>(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray <uint8_t>(this->namePV)->write();
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

    if (type == UA_PV_INT16) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<int16_t> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                int16_t value = *((int16_t *)data->data);
                valueArray.push_back(value);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                int16_t* v = (int16_t *)data->data;
                valueArray.resize(data->arrayLength);
                for (uint32_t i = 0; i < valueArray.size(); i++) {
                    valueArray.at(i) = v[i];
                }
            }
            this->csManager->getProcessArray <int16_t>(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray <int16_t>(this->namePV)->write();
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

    if (type == UA_PV_UINT16) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<uint16_t> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                uint16_t value = *((uint16_t *)data->data);
                valueArray.push_back(value);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                uint16_t* v = (uint16_t *)data->data;
                valueArray.resize(data->arrayLength);
                for (uint32_t i = 0; i < valueArray.size(); i++) {
                    valueArray.at(i) = v[i];
                }
            }
            this->csManager->getProcessArray <uint16_t>(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray <uint16_t>(this->namePV)->write();
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

    if (type == UA_PV_INT32) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<int32_t> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                int32_t value = *((int32_t *)data->data);
                valueArray.push_back(value);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                int32_t* v = (int32_t *)data->data;
                valueArray.resize(data->arrayLength);
                //std::vector<int32_t> vectorizedValue(data->arrayLength);
                for (uint32_t i = 0; i < valueArray.size(); i++) {
                    valueArray.at(i) = v[i];
                }
            }
            this->csManager->getProcessArray < int32_t >(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray < int32_t >(this->namePV)->write();
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

    if (type == UA_PV_UINT32) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<uint32_t> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                uint32_t value = *((uint32_t *)data->data);
                valueArray.push_back(value);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                uint32_t* v = (uint32_t *)data->data;
                valueArray.resize(data->arrayLength);
                for (uint32_t i = 0; i < valueArray.size(); i++) {
                    valueArray.at(i) = v[i];
                }
            }
            this->csManager->getProcessArray <uint32_t>(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray <uint32_t>(this->namePV)->write();
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

    if (type == UA_PV_INT64) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<int64_t> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                int64_t value = *((int64_t *)data->data);
                valueArray.push_back(value);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                int64_t* v = (int64_t *)data->data;
                valueArray.resize(data->arrayLength);
                //std::vector<int64_t> vectorizedValue(data->arrayLength);
                for (uint32_t i = 0; i < valueArray.size(); i++) {
                    valueArray.at(i) = v[i];
                }
            }
            this->csManager->getProcessArray < int64_t >(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray < int64_t >(this->namePV)->write();
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

    if (type == UA_PV_UINT64) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<uint64_t> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                uint64_t value = *((uint64_t *)data->data);
                valueArray.push_back(value);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                uint64_t* v = (uint64_t *)data->data;
                valueArray.resize(data->arrayLength);
                for (uint32_t i = 0; i < valueArray.size(); i++) {
                    valueArray.at(i) = v[i];
                }
            }
            this->csManager->getProcessArray <uint64_t>(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray <uint64_t>(this->namePV)->write();
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

    if (type == UA_PV_FLOAT) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<float> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                float value = *((float *)data->data);
                valueArray.push_back(value);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                float* v = (float *)data->data;
                valueArray.resize(data->arrayLength);
                for (uint32_t i = 0; i < valueArray.size(); i++) {
                    valueArray.at(i) = v[i];
                }
            }
            this->csManager->getProcessArray <float>(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray <float>(this->namePV)->write();
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

    if (type == UA_PV_DOUBLE) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<double> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                double value = *((double *)data->data);
                valueArray.push_back(value);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                double* v = (double *)data->data;
                valueArray.resize(data->arrayLength);
                for (uint32_t i = 0; i < valueArray.size(); i++) {
                    valueArray.at(i) = v[i];
                }
            }
            this->csManager->getProcessArray <double>(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray <double>(this->namePV)->write();
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

    if (type == UA_PV_STRING) {
        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
            vector<string> valueArray;
            if (UA_Variant_isScalar(data) && (!array)) {
                string cpps;
                UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *)data->data)), cpps);
                //string value = *((string *)data->data);
                valueArray.push_back(cpps);
            }
            else if ((!UA_Variant_isScalar(data)) && array) {
                // currently not implemented
                retval = UA_STATUSCODE_BADNOTIMPLEMENTED;
                //                string* v = (string *)data->data;
                //                valueArray.resize(data->arrayLength);
                //                for (uint32_t i = 0; i < valueArray.size(); i++) {
                //                    valueArray.at(i) = v[i];
                //                }
            }
            this->csManager->getProcessArray <string>(this->namePV)->accessChannel(0) = valueArray;
            this->csManager->getProcessArray <string>(this->namePV)->write();
            retval = UA_STATUSCODE_GOOD;
        }
        else {
            retval = UA_STATUSCODE_BADNOTWRITABLE;
        }
    }

    return retval;
}

//UA_RDPROXY_INT8(ua_processvariable, getValue_int8_t);
//CREATE_READ_FUNCTION(int8_t)
//UA_RDPROXY_UINT8(ua_processvariable, getValue_uint8_t);
//CREATE_READ_FUNCTION(uint8_t)
//UA_RDPROXY_INT16(ua_processvariable, getValue_int16_t);
//CREATE_READ_FUNCTION(int16_t)
//UA_RDPROXY_UINT16(ua_processvariable, getValue_uint16_t);
//CREATE_READ_FUNCTION(uint16_t)
//UA_RDPROXY_INT32(ua_processvariable, getValue_int32_t);
//CREATE_READ_FUNCTION(int32_t)
//UA_RDPROXY_UINT32(ua_processvariable, getValue_uint32_t);
//CREATE_READ_FUNCTION(uint32_t)
//UA_RDPROXY_INT64(ua_processvariable, getValue_int64_t);
//CREATE_READ_FUNCTION(int64_t)
//UA_RDPROXY_UINT64(ua_processvariable, getValue_uint64_t);
//CREATE_READ_FUNCTION(uint64_t)
//UA_RDPROXY_FLOAT(ua_processvariable, getValue_float);
//CREATE_READ_FUNCTION(float)
//UA_RDPROXY_DOUBLE(ua_processvariable, getValue_double);
//CREATE_READ_FUNCTION(double)
//UA_RDPROXY_STRING(ua_processvariable, getValue_string);
//CREATE_READ_FUNCTION(string)

//UA_WRPROXY_INT8(ua_processvariable, setValue_int8_t);
//CREATE_WRITE_FUNCTION(int8_t)
//UA_WRPROXY_UINT8(ua_processvariable, setValue_uint8_t);
//CREATE_WRITE_FUNCTION(uint8_t)
//UA_WRPROXY_INT16(ua_processvariable, setValue_int16_t);
//CREATE_WRITE_FUNCTION(int16_t)
//UA_WRPROXY_UINT16(ua_processvariable, setValue_uint16_t);
//CREATE_WRITE_FUNCTION(uint16_t)
//UA_WRPROXY_INT32(ua_processvariable, setValue_int32_t);
//CREATE_WRITE_FUNCTION(int32_t)
//UA_WRPROXY_UINT32(ua_processvariable, setValue_uint32_t);
//CREATE_WRITE_FUNCTION(uint32_t)
//UA_WRPROXY_INT64(ua_processvariable, setValue_int64_t);
//CREATE_WRITE_FUNCTION(int64_t)
//UA_WRPROXY_UINT64(ua_processvariable, setValue_uint64_t);
//CREATE_WRITE_FUNCTION(uint64_t)
//UA_WRPROXY_FLOAT(ua_processvariable, setValue_float);
//CREATE_WRITE_FUNCTION(float)
//UA_WRPROXY_DOUBLE(ua_processvariable, setValue_double);
//CREATE_WRITE_FUNCTION(double)
//UA_WRPROXY_STRING(ua_processvariable, setValue_string);
//CREATE_WRITE_FUNCTION(string)

// Array
//UA_RDPROXY_ARRAY_INT8(ua_processvariable, getValue_Array_int8_t);
//CREATE_READ_FUNCTION_ARRAY(int8_t)
//UA_RDPROXY_ARRAY_UINT8(ua_processvariable, getValue_Array_uint8_t);
//CREATE_READ_FUNCTION_ARRAY(uint8_t)
//UA_RDPROXY_ARRAY_INT16(ua_processvariable, getValue_Array_int16_t);
//CREATE_READ_FUNCTION_ARRAY(int16_t)
//UA_RDPROXY_ARRAY_UINT16(ua_processvariable, getValue_Array_uint16_t);
//CREATE_READ_FUNCTION_ARRAY(uint16_t)
//UA_RDPROXY_ARRAY_INT32(ua_processvariable, getValue_Array_int32_t);
//CREATE_READ_FUNCTION_ARRAY(int32_t)
//UA_RDPROXY_ARRAY_UINT32(ua_processvariable, getValue_Array_uint32_t);
//CREATE_READ_FUNCTION_ARRAY(uint32_t)
//UA_RDPROXY_ARRAY_INT64(ua_processvariable, getValue_Array_int64_t);
//CREATE_READ_FUNCTION_ARRAY(int64_t)
//UA_RDPROXY_ARRAY_UINT64(ua_processvariable, getValue_Array_uint64_t);
//CREATE_READ_FUNCTION_ARRAY(uint64_t)
//UA_RDPROXY_ARRAY_FLOAT(ua_processvariable, getValue_Array_float);
//CREATE_READ_FUNCTION_ARRAY(float)
//UA_RDPROXY_ARRAY_DOUBLE(ua_processvariable, getValue_Array_double);
//CREATE_READ_FUNCTION_ARRAY(double)
//UA_RDPROXY_ARRAY_STRING(ua_processvariable, getValue_Array_string);
//CREATE_READ_FUNCTION_ARRAY(string)

//UA_WRPROXY_ARRAY_INT8(ua_processvariable, setValue_Array_int8_t);
//CREATE_WRITE_FUNCTION_ARRAY(int8_t)
//UA_WRPROXY_ARRAY_UINT8(ua_processvariable, setValue_Array_uint8_t);
//CREATE_WRITE_FUNCTION_ARRAY(uint8_t)
//UA_WRPROXY_ARRAY_INT16(ua_processvariable, setValue_Array_int16_t);
//CREATE_WRITE_FUNCTION_ARRAY(int16_t)
//UA_WRPROXY_ARRAY_UINT16(ua_processvariable, setValue_Array_uint16_t);
//CREATE_WRITE_FUNCTION_ARRAY(uint16_t)
//UA_WRPROXY_ARRAY_INT32(ua_processvariable, setValue_Array_int32_t);
//CREATE_WRITE_FUNCTION_ARRAY(int32_t)
//UA_WRPROXY_ARRAY_UINT32(ua_processvariable, setValue_Array_uint32_t);
//CREATE_WRITE_FUNCTION_ARRAY(uint32_t)
//UA_WRPROXY_ARRAY_INT64(ua_processvariable, setValue_Array_int64_t);
//CREATE_WRITE_FUNCTION_ARRAY(int64_t)
//UA_WRPROXY_ARRAY_UINT64(ua_processvariable, setValue_Array_uint64_t);
//CREATE_WRITE_FUNCTION_ARRAY(uint64_t)
//UA_WRPROXY_ARRAY_FLOAT(ua_processvariable, setValue_Array_float);
//CREATE_WRITE_FUNCTION_ARRAY(float)
//UA_WRPROXY_ARRAY_DOUBLE(ua_processvariable, setValue_Array_double);
//CREATE_WRITE_FUNCTION_ARRAY(double)
//UA_WRPROXY_ARRAY_STRING(ua_processvariable, setValue_Array_string);
//CREATE_WRITE_FUNCTION_ARRAY(string)

// Just a macro to easy pushing different types of dataSources
// ... and make sure we lock down writing to receivers in this stage already
// #define PUSH_RDVALUE_TYPE(_p_typeName) { \
// if(this->csManager->getProcessVariable(this->namePV)->isWriteable())  { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .description = description, .read=UA_RDPROXY_NAME(ua_processvariable, getValue_##_p_typeName), .write=UA_WRPROXY_NAME(ua_processvariable, setValue_##_p_typeName) }); } \
//     else                                                                    { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .description = description, .read=UA_RDPROXY_NAME(ua_processvariable, getValue_##_p_typeName), .write=NULL}); }\
// }

//#define PUSH_RDVALUE_ARRAY_TYPE(_p_typeName) { \
//if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .description = description, .read=UA_RDPROXY_NAME(ua_processvariable, getValue_Array_##_p_typeName), .write=UA_WRPROXY_NAME(ua_processvariable, setValue_Array_##_p_typeName) }); } \
//    else                                                                  { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .description = description, .read=UA_RDPROXY_NAME(ua_processvariable, getValue_Array_##_p_typeName), .write=NULL}); } \
//}

UA_StatusCode ua_processvariable::mapSelfToNamespace() {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_NodeId createdNodeId = UA_NODEID_NULL;

    if (UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE)
        return 0; // Something went UA_WRING (initializer should have set this!)

    UA_LocalizedText description;
    description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)this->csManager->getProcessVariable(this->namePV)->getDescription().c_str());

    // Create our toplevel instance
    UA_ObjectAttributes oAttr;
    UA_ObjectAttributes_init(&oAttr);

    oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", this->nameNew.c_str());
    oAttr.description = description;

    if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
        oAttr.userWriteMask = UA_ACCESSLEVELMASK_WRITE;
        oAttr.writeMask = UA_ACCESSLEVELMASK_WRITE;
    }

//    UA_INSTATIATIONCALLBACK(icb);
    UA_InstantiationCallback icb;
    icb.handle = (void *) &this->ownedNodes;
    icb.method = ua_mapInstantiatedNodes;
    UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1, 0),
        this->baseNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME_ALLOC(1, this->nameNew.c_str()), UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_CTKPROCESSVARIABLE), oAttr, &icb, &createdNodeId);

    // know your own nodeId
    this->ownNodeId = createdNodeId;

    // Delete old "Value" with numeric NodeId
    UA_BrowseDescription bDesc;
    UA_BrowseDescription_init(&bDesc);
    bDesc.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    bDesc.includeSubtypes = false;
    bDesc.nodeClassMask = UA_NODECLASS_VARIABLE;
    bDesc.nodeId = createdNodeId;
    bDesc.resultMask = UA_BROWSERESULTMASK_ALL;

    UA_BrowseResult bRes;
    UA_BrowseResult_init(&bRes);
    bRes = UA_Server_browse(this->mappedServer, 10, &bDesc);
    UA_NodeId toDeleteNodeId = UA_NODEID_NULL;
    for (uint32_t i = 0; i < bRes.referencesSize; i++) {
        UA_String varName = UA_String_fromChars("Value");
        if (UA_String_equal(&bRes.references[i].browseName.name, &varName)) {
            UA_Server_deleteNode(this->mappedServer, bRes.references[i].nodeId.nodeId, UA_TRUE);
            UA_NodeId_copy(&bRes.references[i].nodeId.nodeId, &toDeleteNodeId);
        }
    }

    for (nodePairList::reverse_iterator i = this->ownedNodes.rbegin(); i != this->ownedNodes.rend(); ++i) {
        UA_NodeId_pair *p = *(i);
        if (UA_NodeId_equal(&toDeleteNodeId, &p->targetNodeId)) {
            this->ownedNodes.remove(*(i));
        }
    }

    UA_BrowseDescription_deleteMembers(&bDesc);
    UA_BrowseResult_deleteMembers(&bRes);

    UA_NodeId valueNodeId = UA_NODEID_NULL;
    UA_VariableAttributes vAttr;
    UA_VariableAttributes_init(&vAttr);
    vAttr.description = UA_LOCALIZEDTEXT((char*) "en_US", (char*) "Value");
    vAttr.displayName = UA_LOCALIZEDTEXT((char*) "en_US", (char*) "Value");
    vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE);


    /* Use a datasource map to map any local getter/setter functions to opcua variables nodes */
    UA_DataSource_Map mapDs;
    // FIXME: We should not be using std::cout here... Where's our logger?
    std::type_info const & valueType = this->csManager->getProcessVariable(this->namePV)->getValueType();
    if (valueType == typeid(int8_t)) {
        //        if (this->csManager->getProcessArray < int8_t
        //                > (this->namePV)->accessChannel(0).size() == 1)
        //            PUSH_RDVALUE_TYPE(int8_t)
        //        else
        //            PUSH_RDVALUE_ARRAY_TYPE(int8_t)
        type = UA_PV_INT8;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <int8_t>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write = UA_WRPROXY_NAME(ua_processvariable, setValue_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write=UA_WRPROXY_NAME(ua_processvariable, setValue_Array_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else if (valueType == typeid(uint8_t)) {
        //        if (this->csManager->getProcessArray < uint8_t
        //                > (this->namePV)->accessChannel(0).size() == 1)
        //            PUSH_RDVALUE_TYPE(uint8_t)
        //        else
        //            PUSH_RDVALUE_ARRAY_TYPE(uint8_t)
        type = UA_PV_UINT8;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <uint8_t>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else if (valueType == typeid(int16_t)) {
        //        if (this->csManager->getProcessArray < int16_t
        //                > (this->namePV)->accessChannel(0).size() == 1)
        //            PUSH_RDVALUE_TYPE(int16_t)
        //        else
        //            PUSH_RDVALUE_ARRAY_TYPE(int16_t)
        type = UA_PV_INT16;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <int16_t>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write = UA_WRPROXY_NAME(ua_processvariable, setValue_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write=UA_WRPROXY_NAME(ua_processvariable, setValue_Array_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else if (valueType == typeid(uint16_t)) {
        //        if (this->csManager->getProcessArray < uint16_t
        //                > (this->namePV)->accessChannel(0).size() == 1)
        //            PUSH_RDVALUE_TYPE(uint16_t)
        //        else
        //            PUSH_RDVALUE_ARRAY_TYPE(uint16_t)
        type = UA_PV_UINT16;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <uint16_t>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write = UA_WRPROXY_NAME(ua_processvariable, setValue_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write=UA_WRPROXY_NAME(ua_processvariable, setValue_Array_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else if (valueType == typeid(int32_t)) {
        type = UA_PV_INT32;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID,
            CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <int32_t>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write = UA_WRPROXY_NAME(ua_processvariable, setValue_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write=UA_WRPROXY_NAME(ua_processvariable, setValue_Array_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else if (valueType == typeid(uint32_t)) {
        //        if (this->csManager->getProcessArray < uint32_t
        //                > (this->namePV)->accessChannel(0).size() == 1)
        //            PUSH_RDVALUE_TYPE(uint32_t)
        //        else
        //            PUSH_RDVALUE_ARRAY_TYPE(uint32_t)
        type = UA_PV_UINT32;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <uint32_t>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write = UA_WRPROXY_NAME(ua_processvariable, setValue_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write=UA_WRPROXY_NAME(ua_processvariable, setValue_Array_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else if (valueType == typeid(int64_t)) {
        //if (this->csManager->getProcessArray<int64_t>(this->namePV)->accessChannel(0).size() == 1) 
        //	PUSH_RDVALUE_TYPE(int64_t)
        //else 
        //	PUSH_RDVALUE_ARRAY_TYPE(int64_t)
        type = UA_PV_INT64;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID,
            CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <int64_t>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write = UA_WRPROXY_NAME(ua_processvariable, setValue_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                //mapElem.write=UA_WRPROXY_NAME(ua_processvariable, setValue_Array_int32_t);
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else if (valueType == typeid(uint64_t)) {
        //if (this->csManager->getProcessArray<uint64_t>(this->namePV)->accessChannel(0).size() == 1) 
        //    PUSH_RDVALUE_TYPE(uint64_t)
        //else 
        //    PUSH_RDVALUE_ARRAY_TYPE(uint64_t)
        type = UA_PV_UINT64;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID,
            CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <uint64_t>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else if (valueType == typeid(float)) {
        //        if (this->csManager->getProcessArray<float>(this->namePV)->accessChannel(
        //                0).size() == 1)
        //            PUSH_RDVALUE_TYPE(float)
        //        else
        //            PUSH_RDVALUE_ARRAY_TYPE(float)
        type = UA_PV_FLOAT;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <float>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else if (valueType == typeid(double)) {
        //        if (this->csManager->getProcessArray<double>(this->namePV)->accessChannel(
        //                0).size() == 1)
        //            PUSH_RDVALUE_TYPE(double)
        //        else
        //            PUSH_RDVALUE_ARRAY_TYPE(double)
        type = UA_PV_DOUBLE;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <double>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else if (valueType == typeid(string)) {
        //        if (this->csManager->getProcessArray < string
        //                > (this->namePV)->accessChannel(0).size() == 1)
        //            PUSH_RDVALUE_TYPE(string)
        //        else
        //            PUSH_RDVALUE_ARRAY_TYPE(string)
        type = UA_PV_STRING;
        UA_DataSource_Map_Element mapElem;
        mapElem.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE);
        mapElem.description = description;
        mapElem.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue;
        if (this->csManager->getProcessArray <string>(this->namePV)->accessChannel(0).size() == 1)
        {
            this->array = false;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        else
        {
            this->array = true;
            if (this->csManager->getProcessVariable(this->namePV)->isWriteable())
            {
                mapElem.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue;
            }
            else
            {
                mapElem.write = NULL;
            }
        }
        mapDs.push_back(mapElem);
    }
    else std::cout << "Cannot proxy unknown type " << typeid(valueType).name() << std::endl;

    UA_Server_addVariableNode(this->mappedServer, UA_NODEID_STRING(1, (char*)this->getName().c_str()), createdNodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(1, (char*) "Value"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), vAttr, &icb, &valueNodeId);

    UA_NodeId nodeIdVariableType = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE);
    NODE_PAIR_PUSH(this->ownedNodes, nodeIdVariableType, valueNodeId)

    //mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_NAME), UA_LOCALIZEDTEXT((char*)"", (char*)""), .read = UA_RDPROXY_NAME(ua_processvariable, getName), .write = NULL });
    UA_DataSource_Map_Element mapElemName;
    mapElemName.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_NAME);
    mapElemName.description = UA_LOCALIZEDTEXT((char*)"", (char*)"");
    mapElemName.read = ua_readproxy_ua_processvariable_getName;
    mapElemName.write = NULL;
    mapDs.push_back(mapElemName);

    //mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_DESC), UA_LOCALIZEDTEXT((char*)"", (char*)""), .read = UA_RDPROXY_NAME(ua_processvariable, getDescription), .write = UA_WRPROXY_NAME(ua_processvariable, setDescription) });
    UA_DataSource_Map_Element mapElemDesc;
    mapElemDesc.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_DESC);
    mapElemDesc.description = UA_LOCALIZEDTEXT((char*)"", (char*)"");
    mapElemDesc.read = ua_readproxy_ua_processvariable_getDescription;
    mapElemDesc.write = ua_writeproxy_ua_processvariable_setDescription;
    mapDs.push_back(mapElemDesc);
    //mapDs.push_back((UA_DataSource_Map_Element) { , , .read = UA_RDPROXY_NAME(ua_processvariable, getDescription), ;

    UA_DataSource_Map_Element mapElemEU;
    mapElemEU.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_UNIT);
    mapElemEU.description = UA_LOCALIZEDTEXT((char*)"", (char*)"");
    mapElemEU.read = ua_readproxy_ua_processvariable_getEngineeringUnit;
    mapElemEU.write = ua_writeproxy_ua_processvariable_setEngineeringUnit;
    mapDs.push_back(mapElemEU);
    //mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_UNIT), UA_LOCALIZEDTEXT((char*)"", (char*)""), .read = UA_RDPROXY_NAME(ua_processvariable, getEngineeringUnit), .write = UA_WRPROXY_NAME(ua_processvariable, setEngineeringUnit) });

    UA_DataSource_Map_Element mapElemType;
    mapElemType.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TYPE);
    mapElemType.description = UA_LOCALIZEDTEXT((char*)"", (char*)"");
    mapElemType.read = ua_readproxy_ua_processvariable_getType;
    mapElemType.write = NULL;
    mapDs.push_back(mapElemType);
    //mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TYPE), UA_LOCALIZEDTEXT((char*)"", (char*)""), .read = UA_RDPROXY_NAME(ua_processvariable, getType), .write = NULL });

    this->ua_mapDataSources((void *) this, &mapDs);

    return UA_STATUSCODE_GOOD;
}

/** @brief Reimplement the SourceTimeStamp to timestamp of csa_config
 *
 */
UA_DateTime ua_processvariable::getSourceTimeStamp() {
    auto t = this->csManager->getProcessVariable(this->namePV)->getVersionNumber().getTime();
    auto microseconds = std::chrono::time_point_cast<std::chrono::microseconds>(t).time_since_epoch().count();
    return (microseconds * UA_USEC_TO_DATETIME) + UA_DATETIME_UNIX_EPOCH;
}

UA_NodeId ua_processvariable::getOwnNodeId() {
        return this->ownNodeId;
}
