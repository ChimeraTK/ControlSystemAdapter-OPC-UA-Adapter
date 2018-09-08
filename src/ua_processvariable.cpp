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
#include "ua_proxies_callback.h"

#include <iostream>

ua_processvariable::ua_processvariable(UA_Server* server, UA_NodeId basenodeid, string namePV, boost::shared_ptr<ControlSystemPVManager> csManager) : ua_mapped_class(server, basenodeid) {

        // FIXME Check if name member of a csManager Parameter
        this->namePV = namePV;
        this->nameNew = namePV;
        this->csManager = csManager;

        this->mapSelfToNamespace();
}

ua_processvariable::~ua_processvariable()
{
  //* Our ua_mapped_class destructor will take care of deleting our opcua footprint as long as all variables are mapped in this->ownedNodes
}

// Name
UA_RDPROXY_STRING(ua_processvariable, getName)
string ua_processvariable::getName() {
  return this->namePV;
}

// EngineeringUnit
UA_WRPROXY_STRING(ua_processvariable, setEngineeringUnit)
void ua_processvariable::setEngineeringUnit(string engineeringUnit) {
        this->engineeringUnit = engineeringUnit;
}

UA_RDPROXY_STRING(ua_processvariable, getEngineeringUnit)
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
UA_WRPROXY_STRING(ua_processvariable, setDescription)
void ua_processvariable::setDescription(string description) {
        this->description = description;
}

UA_RDPROXY_STRING(ua_processvariable, getDescription)
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
UA_RDPROXY_STRING(ua_processvariable, getType)
string ua_processvariable::getType() {
    // Note: typeid().name() may return the name; may as well return the symbol's name from the binary though...
    std::type_info const & valueType = this->csManager->getProcessVariable(this->namePV)->getValueType();
    if (valueType == typeid(int8_t))        return "int8_t";
    else if (valueType == typeid(uint8_t))  return "uint8_t";
    else if (valueType == typeid(int16_t))  return "int16_t";
    else if (valueType == typeid(uint16_t)) return "uint16_t";
    else if (valueType == typeid(int32_t))  return "int32_t";
    else if (valueType == typeid(uint32_t)) return "uint32_t";
    else if (valueType == typeid(float))    return "float";
    else if (valueType == typeid(double))   return "double";
    else if (valueType == typeid(string))   return "string";
    else                                    return "Unsupported type";
}

/* Multivariant Read Functions for Value (without template-Foo) */
#define CREATE_READ_FUNCTION(_p_type) \
_p_type    ua_processvariable::getValue_##_p_type() { \
    _p_type v = _p_type(); \
    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return 0; \
    if (this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).size() == 1) { \
                        if(this->csManager->getProcessVariable(this->namePV)->isReadable()) { \
                                while(this->csManager->getProcessArray<_p_type>(this->namePV)->readNonBlocking()) {} \
                        } \
                        v = this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).at(0); \
                } \
    return v; \
} \


#define CREATE_READ_FUNCTION_ARRAY(_p_type) \
std::vector<_p_type>    ua_processvariable::getValue_Array_##_p_type() { \
    std::vector<_p_type> v; \
    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return v; \
    if (this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).size() > 1) { \
                        if(this->csManager->getProcessVariable(this->namePV)->isReadable()) { \
                                while(this->csManager->getProcessArray<_p_type>(this->namePV)->readNonBlocking()) {} \
                        } \
                        v = this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0); \
                } \
    return v; \
} \


#define CREATE_WRITE_FUNCTION(_p_type) \
void ua_processvariable::setValue_##_p_type(_p_type value) { \
    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return; \
    if (this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).size() == 1) {   \
                        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) { \
                                vector<_p_type> valueArray; \
                                valueArray.push_back(value); \
                                this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0) = valueArray; \
                                this->csManager->getProcessArray<_p_type>(this->namePV)->write(); \
                        } \
                } \
    return; \
}


#define CREATE_WRITE_FUNCTION_ARRAY(_p_type) \
void ua_processvariable::setValue_Array_##_p_type(std::vector<_p_type> value) { \
    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return; \
    if (this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).size() <= 1) return; \
                        if (this->csManager->getProcessVariable(this->namePV)->isWriteable()) { \
                                int32_t valueSize = this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0).size(); \
                                value.resize(valueSize); \
                                this->csManager->getProcessArray<_p_type>(this->namePV)->accessChannel(0) = value; \
                                this->csManager->getProcessArray<_p_type>(this->namePV)->write(); \
                } \
        return; \
}

 UA_RDPROXY_INT8(ua_processvariable, getValue_int8_t);
 CREATE_READ_FUNCTION(int8_t)
 UA_RDPROXY_UINT8(ua_processvariable, getValue_uint8_t);
 CREATE_READ_FUNCTION(uint8_t)
 UA_RDPROXY_INT16(ua_processvariable, getValue_int16_t);
 CREATE_READ_FUNCTION(int16_t)
 UA_RDPROXY_UINT16(ua_processvariable, getValue_uint16_t);
 CREATE_READ_FUNCTION(uint16_t)
 UA_RDPROXY_INT32(ua_processvariable, getValue_int32_t);
 CREATE_READ_FUNCTION(int32_t)
 UA_RDPROXY_UINT32(ua_processvariable, getValue_uint32_t);
 CREATE_READ_FUNCTION(uint32_t)
 UA_RDPROXY_FLOAT(ua_processvariable, getValue_float);
 CREATE_READ_FUNCTION(float)
 UA_RDPROXY_DOUBLE(ua_processvariable, getValue_double);
 CREATE_READ_FUNCTION(double)
 UA_RDPROXY_STRING(ua_processvariable, getValue_string);
 CREATE_READ_FUNCTION(string)

 UA_WRPROXY_INT8(ua_processvariable, setValue_int8_t);
 CREATE_WRITE_FUNCTION(int8_t)
 UA_WRPROXY_UINT8(ua_processvariable, setValue_uint8_t);
 CREATE_WRITE_FUNCTION(uint8_t)
 UA_WRPROXY_INT16(ua_processvariable, setValue_int16_t);
 CREATE_WRITE_FUNCTION(int16_t)
 UA_WRPROXY_UINT16(ua_processvariable, setValue_uint16_t);
 CREATE_WRITE_FUNCTION(uint16_t)
 UA_WRPROXY_INT32(ua_processvariable, setValue_int32_t);
 CREATE_WRITE_FUNCTION(int32_t)
 UA_WRPROXY_UINT32(ua_processvariable, setValue_uint32_t);
 CREATE_WRITE_FUNCTION(uint32_t)
 UA_WRPROXY_FLOAT(ua_processvariable, setValue_float);
 CREATE_WRITE_FUNCTION(float)
 UA_WRPROXY_DOUBLE(ua_processvariable, setValue_double);
 CREATE_WRITE_FUNCTION(double)
 UA_WRPROXY_STRING(ua_processvariable, setValue_string);
 CREATE_WRITE_FUNCTION(string)

// Array
UA_RDPROXY_ARRAY_INT8(ua_processvariable, getValue_Array_int8_t);
CREATE_READ_FUNCTION_ARRAY(int8_t)
UA_RDPROXY_ARRAY_UINT8(ua_processvariable, getValue_Array_uint8_t);
CREATE_READ_FUNCTION_ARRAY(uint8_t)
UA_RDPROXY_ARRAY_INT16(ua_processvariable, getValue_Array_int16_t);
CREATE_READ_FUNCTION_ARRAY(int16_t)
UA_RDPROXY_ARRAY_UINT16(ua_processvariable, getValue_Array_uint16_t);
CREATE_READ_FUNCTION_ARRAY(uint16_t)
UA_RDPROXY_ARRAY_INT32(ua_processvariable, getValue_Array_int32_t);
CREATE_READ_FUNCTION_ARRAY(int32_t)
UA_RDPROXY_ARRAY_UINT32(ua_processvariable, getValue_Array_uint32_t);
CREATE_READ_FUNCTION_ARRAY(uint32_t)
UA_RDPROXY_ARRAY_FLOAT(ua_processvariable, getValue_Array_float);
CREATE_READ_FUNCTION_ARRAY(float)
UA_RDPROXY_ARRAY_DOUBLE(ua_processvariable, getValue_Array_double);
CREATE_READ_FUNCTION_ARRAY(double)
UA_RDPROXY_ARRAY_STRING(ua_processvariable, getValue_Array_string);
CREATE_READ_FUNCTION_ARRAY(string)

UA_WRPROXY_ARRAY_INT8(ua_processvariable, setValue_Array_int8_t);
CREATE_WRITE_FUNCTION_ARRAY(int8_t)
UA_WRPROXY_ARRAY_UINT8(ua_processvariable, setValue_Array_uint8_t);
CREATE_WRITE_FUNCTION_ARRAY(uint8_t)
UA_WRPROXY_ARRAY_INT16(ua_processvariable, setValue_Array_int16_t);
CREATE_WRITE_FUNCTION_ARRAY(int16_t)
UA_WRPROXY_ARRAY_UINT16(ua_processvariable, setValue_Array_uint16_t);
CREATE_WRITE_FUNCTION_ARRAY(uint16_t)
UA_WRPROXY_ARRAY_INT32(ua_processvariable, setValue_Array_int32_t);
CREATE_WRITE_FUNCTION_ARRAY(int32_t)
UA_WRPROXY_ARRAY_UINT32(ua_processvariable, setValue_Array_uint32_t);
CREATE_WRITE_FUNCTION_ARRAY(uint32_t)
UA_WRPROXY_ARRAY_FLOAT(ua_processvariable, setValue_Array_float);
CREATE_WRITE_FUNCTION_ARRAY(float)
UA_WRPROXY_ARRAY_DOUBLE(ua_processvariable, setValue_Array_double);
CREATE_WRITE_FUNCTION_ARRAY(double)
UA_WRPROXY_ARRAY_STRING(ua_processvariable, setValue_Array_string);
CREATE_WRITE_FUNCTION_ARRAY(string)

// Just a macro to easy pushing different types of dataSources
// ... and make sure we lock down writing to receivers in this stage already
 #define PUSH_RDVALUE_TYPE(_p_typeName) { \
 if(this->csManager->getProcessVariable(this->namePV)->isWriteable())  { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .description = description, .read=UA_RDPROXY_NAME(ua_processvariable, getValue_##_p_typeName), .write=UA_WRPROXY_NAME(ua_processvariable, setValue_##_p_typeName) }); } \
     else                                                                    { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .description = description, .read=UA_RDPROXY_NAME(ua_processvariable, getValue_##_p_typeName), .write=NULL}); }\
 }

#define PUSH_RDVALUE_ARRAY_TYPE(_p_typeName) { \
if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .description = description, .read=UA_RDPROXY_NAME(ua_processvariable, getValue_Array_##_p_typeName), .write=UA_WRPROXY_NAME(ua_processvariable, setValue_Array_##_p_typeName) }); } \
    else                                                                  { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .description = description, .read=UA_RDPROXY_NAME(ua_processvariable, getValue_Array_##_p_typeName), .write=NULL}); } \
}

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

    UA_INSTATIATIONCALLBACK(icb);
    UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1, 0),//UA_NODEID_STRING(1, (char *) ("Variables/"+this->nameNew).c_str()), //
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
        for(uint32_t i=0; i < bRes.referencesSize; i++) {
                UA_String varName = UA_String_fromChars("Value");
                if(UA_String_equal(&bRes.references[i].browseName.name, &varName)) {
                        UA_Server_deleteNode(this->mappedServer, bRes.references[i].nodeId.nodeId, UA_TRUE);
                        UA_NodeId_copy(&bRes.references[i].nodeId.nodeId, &toDeleteNodeId);
                }
        }

        for (nodePairList::reverse_iterator i = this->ownedNodes.rbegin(); i != this->ownedNodes.rend(); ++i) {
                UA_NodeId_pair *p = *(i);
                if(UA_NodeId_equal(&toDeleteNodeId, &p->targetNodeId)) {
                        this->ownedNodes.remove(*(i));
                }
        }

        UA_BrowseDescription_deleteMembers(&bDesc);
        UA_BrowseResult_deleteMembers(&bRes);

        UA_NodeId valueNodeId = UA_NODEID_NULL;
        UA_VariableAttributes vAttr;
        UA_VariableAttributes_init(&vAttr);
        vAttr.description = UA_LOCALIZEDTEXT((char*) "en_US",(char*) "Value");
        vAttr.displayName = UA_LOCALIZEDTEXT((char*) "en_US",(char*) "Value");
        vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE);


        /* Use a datasource map to map any local getter/setter functions to opcua variables nodes */
        UA_DataSource_Map mapDs;
        // FIXME: We should not be using std::cout here... Where's our logger?
        std::type_info const & valueType = this->csManager->getProcessVariable(this->namePV)->getValueType();
        if (valueType == typeid(int8_t)) {
//  		vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_SBYTE);
                if(this->csManager->getProcessArray<int8_t>(this->namePV)->accessChannel(0).size() == 1) PUSH_RDVALUE_TYPE(int8_t)
                        else PUSH_RDVALUE_ARRAY_TYPE(int8_t)
                }
                else if (valueType == typeid(uint8_t)) {
//  			vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_BYTE);
                        if(this->csManager->getProcessArray<uint8_t>(this->namePV)->accessChannel(0).size() == 1) PUSH_RDVALUE_TYPE(uint8_t)
                        else PUSH_RDVALUE_ARRAY_TYPE(uint8_t)
                }
                else if (valueType == typeid(int16_t)) {
// 			vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_INT16);
                        if(this->csManager->getProcessArray<int16_t>(this->namePV)->accessChannel(0).size() == 1) PUSH_RDVALUE_TYPE(int16_t)
                        else PUSH_RDVALUE_ARRAY_TYPE(int16_t)
                }
                else if (valueType == typeid(uint16_t)) {
// 			vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_UINT16);
                        if(this->csManager->getProcessArray<uint16_t>(this->namePV)->accessChannel(0).size() == 1) PUSH_RDVALUE_TYPE(uint16_t)
                        else PUSH_RDVALUE_ARRAY_TYPE(uint16_t)
                }
                else if (valueType == typeid(int32_t)) {
// 			vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_INT32);
                        if(this->csManager->getProcessArray<int32_t>(this->namePV)->accessChannel(0).size() == 1) PUSH_RDVALUE_TYPE(int32_t)
                        else PUSH_RDVALUE_ARRAY_TYPE(int32_t)
                }
                else if (valueType == typeid(uint32_t)) {
// 			vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_UINT32);
                        if(this->csManager->getProcessArray<uint32_t>(this->namePV)->accessChannel(0).size() == 1) PUSH_RDVALUE_TYPE(uint32_t)
                        else PUSH_RDVALUE_ARRAY_TYPE(uint32_t)
                }
                else if (valueType == typeid(float)) {
// 			vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_FLOAT);
                        if(this->csManager->getProcessArray<float>(this->namePV)->accessChannel(0).size() == 1) PUSH_RDVALUE_TYPE(float)
                        else PUSH_RDVALUE_ARRAY_TYPE(float)
                }
                else if (valueType == typeid(double)) {
// 			vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE);
                        if(this->csManager->getProcessArray<double>(this->namePV)->accessChannel(0).size() == 1) PUSH_RDVALUE_TYPE(double)
                        else PUSH_RDVALUE_ARRAY_TYPE(double)
                }
                else if (valueType == typeid(string)) {
// 			vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE);
                        if(this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).size() == 1) PUSH_RDVALUE_TYPE(string)
                        else PUSH_RDVALUE_ARRAY_TYPE(string)
                }
        else std::cout << "Cannot proxy unknown type " << typeid(valueType).name()  << std::endl;

        UA_Server_addVariableNode(this->mappedServer, UA_NODEID_STRING(1, (char*)this->getName().c_str()), createdNodeId,
                                                                                                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(1, (char*) "Value"),
                                                                                                                UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), vAttr, &icb, &valueNodeId);

        UA_NodeId nodeIdVariableType = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE);
        NODE_PAIR_PUSH(this->ownedNodes, nodeIdVariableType, valueNodeId)

        mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_NAME), UA_LOCALIZEDTEXT((char*)"", (char*)""), .read=UA_RDPROXY_NAME(ua_processvariable, getName), .write=NULL});
        mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_DESC), UA_LOCALIZEDTEXT((char*)"", (char*)""), .read=UA_RDPROXY_NAME(ua_processvariable, getDescription), .write=UA_WRPROXY_NAME(ua_processvariable, setDescription)});
        mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_UNIT), UA_LOCALIZEDTEXT((char*)"", (char*)""), .read=UA_RDPROXY_NAME(ua_processvariable, getEngineeringUnit), .write=UA_WRPROXY_NAME(ua_processvariable, setEngineeringUnit)});
        mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TYPE), UA_LOCALIZEDTEXT((char*)"", (char*)""), .read=UA_RDPROXY_NAME(ua_processvariable, getType), .write=NULL});

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
