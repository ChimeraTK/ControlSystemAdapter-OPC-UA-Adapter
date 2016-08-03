/*
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "mtca_processvariable.h"
#include "csa_config.h"

extern "C" {
#include "mtca_namespaceinit_generated.h"
}

#include "ua_proxies.h"
#include "ua_proxies_callback.h"
#include <iostream>


mtca_processvariable::mtca_processvariable(UA_Server* server, UA_NodeId basenodeid, std::string name, shCSysPVManager csManager) : ua_mapped_class(server, basenodeid)
{
  this->name = name;
  this->csManager = csManager;
  
  this->mapSelfToNamespace();
}

mtca_processvariable::~mtca_processvariable()
{
  // Our ua_mapped_class destructor will take care of deleting our opcua footprint as long as all variables are mapped in this->ownedNodes
}

/* UA Mapped class members */
// FIXME: We would need to tie all our model values into getter/setter functions... for now, only engineeringunit is passed through

// Name
UA_RDPROXY_STRING(mtca_processvariable, getName)
std::string mtca_processvariable::getName() {
  return this->name;
}
UA_WRPROXY_STRING(mtca_processvariable, setName)
void mtca_processvariable::setName(std::string name) {
  this->name = name;
}

// Type
UA_RDPROXY_STRING(mtca_processvariable, getType)
std::string mtca_processvariable::getType() {
    // Note: typeid().name() may return the name; may as well return the symbol's name from the binary though...
    std::type_info const & valueType = this->csManager->getProcessVariable(this->name)->getValueType();
    if (valueType == typeid(int8_t))        return "int8_t";
    else if (valueType == typeid(uint8_t))  return "uint8_t";
    else if (valueType == typeid(int16_t))  return "int16_t";
    else if (valueType == typeid(uint16_t)) return "uint16_t";
    else if (valueType == typeid(int32_t))  return "int32_t";
    else if (valueType == typeid(uint32_t)) return "uint32_t";
    else if (valueType == typeid(float))    return "float";
    else if (valueType == typeid(double))   return "double";
    else                                    return "Unsupported type";
}

UA_WRPROXY_STRING(mtca_processvariable, setType)
void mtca_processvariable::setType(std::string type) {
    return; // Change of Type is not possible
}

// TimeStamp
UA_RDPROXY_UINT32(mtca_processvariable, getTimeStamp)
uint32_t mtca_processvariable::getTimeStamp() {
    //FIXME: Disabled
    return 0;
}

UA_WRPROXY_UINT32(mtca_processvariable, setTimeStamp)
void mtca_processvariable::setTimeStamp(uint32_t timeStamp) {
    //FIXME: Disabled
    return;
}

/* Multivariant Read Functions for Value (without template-Foo) */
#define CREATE_READ_FUNCTION(_p_type) \
_p_type    mtca_processvariable::getValue_##_p_type() { \
    if (this->csManager->getProcessVariable(this->name)->getValueType() != typeid(_p_type)) return 0; \
    if (this->csManager->getProcessVariable(this->name)->isArray()) return 0; \
    _p_type v = this->csManager->getProcessScalar<_p_type>(this->name)->get(); \
    return v; \
} \

UA_RDPROXY_INT8(mtca_processvariable, getValue_int8_t);
CREATE_READ_FUNCTION(int8_t)
UA_RDPROXY_UINT8(mtca_processvariable, getValue_uint8_t);
CREATE_READ_FUNCTION(uint8_t)
UA_RDPROXY_INT16(mtca_processvariable, getValue_int16_t);
CREATE_READ_FUNCTION(int16_t)
UA_RDPROXY_UINT16(mtca_processvariable, getValue_uint16_t);
CREATE_READ_FUNCTION(uint16_t)
UA_RDPROXY_INT32(mtca_processvariable, getValue_int32_t);
CREATE_READ_FUNCTION(int32_t)
UA_RDPROXY_UINT32(mtca_processvariable, getValue_uint32_t);
CREATE_READ_FUNCTION(uint32_t)
UA_RDPROXY_FLOAT(mtca_processvariable, getValue_float);
CREATE_READ_FUNCTION(float)
UA_RDPROXY_DOUBLE(mtca_processvariable, getValue_double);
CREATE_READ_FUNCTION(double)


#define CREATE_WRITE_FUNCTION(_p_type) \
void mtca_processvariable::setValue_##_p_type(_p_type value) { \
    if (this->csManager->getProcessVariable(this->name)->getValueType() != typeid(_p_type)) return; \
    if (this->csManager->getProcessVariable(this->name)->isArray()) return;   \
    if (this->csManager->getProcessVariable(this->name)->isSender()) { \
        this->csManager->getProcessScalar<_p_type>(this->name)->set(value);   \
        this->csManager->getProcessScalar<_p_type>(this->name)->send();       \
    }\
    return; \
}

UA_WRPROXY_INT8(mtca_processvariable, setValue_int8_t);
CREATE_WRITE_FUNCTION(int8_t)
UA_WRPROXY_UINT8(mtca_processvariable, setValue_uint8_t);
CREATE_WRITE_FUNCTION(uint8_t)
UA_WRPROXY_INT16(mtca_processvariable, setValue_int16_t);
CREATE_WRITE_FUNCTION(int16_t)
UA_WRPROXY_UINT16(mtca_processvariable, setValue_uint16_t);
CREATE_WRITE_FUNCTION(uint16_t)
UA_WRPROXY_INT32(mtca_processvariable, setValue_int32_t);
CREATE_WRITE_FUNCTION(int32_t)
UA_WRPROXY_UINT32(mtca_processvariable, setValue_uint32_t);
CREATE_WRITE_FUNCTION(uint32_t)
UA_WRPROXY_FLOAT(mtca_processvariable, setValue_float);
CREATE_WRITE_FUNCTION(float)
UA_WRPROXY_DOUBLE(mtca_processvariable, setValue_double);
CREATE_WRITE_FUNCTION(double)

/**/

// Just a macro to easy pushing different types of dataSources
// ... and make sure we lock down writing to receivers in this stage already
#define PUSH_RDVALUE_TYPE(_p_typeName) \
if ( this->csManager->getProcessScalar<int32_t>(this->name)->isSender() ) { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .read=UA_RDPROXY_NAME(mtca_processvariable, getValue_##_p_typeName), .write=UA_WRPROXY_NAME(mtca_processvariable, setValue_##_p_typeName) }); } \
else                                                                      { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .read=UA_RDPROXY_NAME(mtca_processvariable, getValue_##_p_typeName) }); }\
    

UA_StatusCode mtca_processvariable::mapSelfToNamespace() {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_NodeId createdNodeId = UA_NODEID_NULL;
    
    if (UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE) 
        return 0; // Something went UA_WRING (initializer should have set this!)
    
    // Create our toplevel instance
    UA_ObjectAttributes oAttr; 
    oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", this->name.c_str());
    oAttr.description = UA_LOCALIZEDTEXT_ALLOC("en_US", "A process variable");
    
    UA_INSTATIATIONCALLBACK(icb);  
    UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1,0),
                            this->baseNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME_ALLOC(1, this->name.c_str()), UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_MTCAPROCESSVARIABLE), oAttr, &icb, &createdNodeId);
    
    /* Use a function map to map any local functioncalls to opcua methods (we do not own any methods) */
    UA_FunctionCall_Map mapThis;
    this->ua_mapFunctions(this, &mapThis, createdNodeId);
    
    /* Use a datasource map to map any local getter/setter functions to opcua variables nodes */
    UA_DataSource_Map mapDs;
    //     mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .read=UA_RDPROXY_NAME(mtca_processvariable, getValue), .write=UA_WRPROXY_NAME(mtca_processvariable, setValue)});
    if (! this->csManager->getProcessVariable(this->name)->isArray()) {
        std::type_info const & valueType = this->csManager->getProcessVariable(this->name)->getValueType();
        if (valueType == typeid(int8_t))          PUSH_RDVALUE_TYPE(int8_t)
        else if (valueType == typeid(uint8_t))    PUSH_RDVALUE_TYPE(uint8_t)
        else if (valueType == typeid(int16_t))    PUSH_RDVALUE_TYPE(int16_t)
        else if (valueType == typeid(uint16_t))   PUSH_RDVALUE_TYPE(uint16_t)
        else if (valueType == typeid(int32_t))    PUSH_RDVALUE_TYPE(int32_t)
        else if (valueType == typeid(uint32_t))   PUSH_RDVALUE_TYPE(uint32_t)
        else if (valueType == typeid(float))      PUSH_RDVALUE_TYPE(float)
        else if (valueType == typeid(double))     PUSH_RDVALUE_TYPE(double)
    }
    else {
        // FIXME: If Array, this is also readable, but we are currently missing some Proxies for arrays
    }

    mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_NAME), .read=UA_RDPROXY_NAME(mtca_processvariable, getName), .write=UA_WRPROXY_NAME(mtca_processvariable, setName)});
    mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TYPE), .read=UA_RDPROXY_NAME(mtca_processvariable, getType), .write=UA_WRPROXY_NAME(mtca_processvariable, setType)});
    mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, 6013), .read=UA_RDPROXY_NAME(mtca_processvariable, getTimeStamp), .write=UA_WRPROXY_NAME(mtca_processvariable, setTimeStamp)});
    
    ua_callProxy_mapDataSources(this->mappedServer, this->ownedNodes, &mapDs, (void *) this);
    
    // FIXME: Need to add a timestamp here!
    
    return UA_STATUSCODE_GOOD;
}