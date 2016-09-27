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


mtca_processvariable::mtca_processvariable(UA_Server* server, UA_NodeId basenodeid, std::string namePV, boost::shared_ptr<ChimeraTK::ControlSystemPVManager> csManager) : ua_mapped_class(server, basenodeid) {
	
	// FIXME Check if name member of a csManager Parameter
	this->namePV = namePV;
	this->nameNew = namePV;
	this->csManager = csManager;
	
	this->mapSelfToNamespace();
}

mtca_processvariable::mtca_processvariable(UA_Server* server, UA_NodeId basenodeid, std::string namePV, std::string nameNew, boost::shared_ptr<ChimeraTK::ControlSystemPVManager> csManager) : ua_mapped_class(server, basenodeid) {
	
	// FIXME Check if name member of a csManager Parameter
	this->namePV = namePV;
	this->nameNew = nameNew;
	this->csManager = csManager;
	
	this->mapSelfToNamespace();
}

mtca_processvariable::~mtca_processvariable()
{
  // Our ua_mapped_class destructor will take care of deleting our opcua footprint as long as all variables are mapped in this->ownedNodes
}

// Name
UA_RDPROXY_STRING(mtca_processvariable, getName)
std::string mtca_processvariable::getName() {
  return this->namePV;
}

/*
UA_WRPROXY_STRING(mtca_processvariable, setName)
void mtca_processvariable::setName(std::string name) {
	return; // Change of name is not possible
}
*/

UA_WRPROXY_STRING(mtca_processvariable, setEngineeringUnit)
void mtca_processvariable::setEngineeringUnit(std::string engineeringUnit) {
	this->engineeringUnit = engineeringUnit;
}

UA_RDPROXY_STRING(mtca_processvariable, getEngineeringUnit)
std::string mtca_processvariable::getEngineeringUnit() {
	return this->engineeringUnit;
}

		
// Type
UA_RDPROXY_STRING(mtca_processvariable, getType)
std::string mtca_processvariable::getType() {		
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
    else                                    return "Unsupported type";
}


//UA_WRPROXY_STRING(mtca_processvariable, setType)
//void mtca_processvariable::setType(std::string type) {
  //return; // Change of Type is not possible
//}


// TimeStamp
//UA_RDPROXY_UINT32(mtca_processvariable, getTimeStamp)
ChimeraTK::TimeStamp mtca_processvariable::getTimeStamp() {
    return this->csManager->getProcessVariable(this->namePV)->getTimeStamp();
}

/*
//UA_WRPROXY_UINT32(mtca_processvariable, setTimeStamp)
void mtca_processvariable::setTimeStamp(ChimeraTK::TimeStamp timeStamp) {
    return; // Change of TimeStamp is not possible
}
*/

/*
UA_WRPROXY_UINT32(mtca_processvariable, setTimeStampSeconds)
void mtca_processvariable::setTimeStampSeconds(uint32_t seconds) {
	return;
}
*/

UA_RDPROXY_UINT32(mtca_processvariable, getTimeStampSeconds)
uint32_t mtca_processvariable::getTimeStampSeconds() {
	return this->csManager->getProcessVariable(this->namePV)->getTimeStamp().seconds;
}

UA_RDPROXY_UINT32(mtca_processvariable, getTimeStampNanoSeconds)
uint32_t mtca_processvariable::getTimeStampNanoSeconds() {
	return this->csManager->getProcessVariable(this->namePV)->getTimeStamp().nanoSeconds;
}

/*
UA_WRPROXY_UINT32(mtca_processvariable, setTimeStampNanoSeconds)
void mtca_processvariable::setTimeStampNanoSeconds(uint32_t indexNanoSeconds) {
	return;
}
*/

UA_RDPROXY_UINT32(mtca_processvariable, getTimeStampIndex0)
uint32_t mtca_processvariable::getTimeStampIndex0() {
	return this->csManager->getProcessVariable(this->namePV)->getTimeStamp().index0;
}

/*
UA_WRPROXY_UINT32(mtca_processvariable, setTimeStampIndex0)
void mtca_processvariable::setTimeStampIndex0(uint32_t index0) {
	return;
}
*/

UA_RDPROXY_UINT32(mtca_processvariable, getTimeStampIndex1)
uint32_t mtca_processvariable::getTimeStampIndex1() {
	return this->csManager->getProcessVariable(this->namePV)->getTimeStamp().index1;
}

/*
UA_WRPROXY_UINT32(mtca_processvariable, setTimeStampIndex1)
void mtca_processvariable::setTimeStampIndex1(uint32_t index1) {
	return;
}
*/

/* Multivariant Read Functions for Value (without template-Foo) */
#define CREATE_READ_FUNCTION(_p_type) \
_p_type    mtca_processvariable::getValue_##_p_type() { \
    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return 0; \
    if (this->csManager->getProcessVariable(this->namePV)->isArray()) return 0; \
    _p_type v = this->csManager->getProcessScalar<_p_type>(this->namePV)->get(); \
    return v; \
} \

#define CREATE_READ_FUNCTION_ARRAY(_p_type) \
std::vector<_p_type>    mtca_processvariable::getValue_Array_##_p_type() { \
    std::vector<_p_type> v; \
    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return v; \
    if (!this->csManager->getProcessVariable(this->namePV)->isArray()) return v; \
    v = this->csManager->getProcessArray<_p_type>(this->namePV)->get(); \
    return v; \
} \

#define CREATE_WRITE_FUNCTION(_p_type) \
void mtca_processvariable::setValue_##_p_type(_p_type value) { \
    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return; \
    if (this->csManager->getProcessVariable(this->namePV)->isArray()) return;   \
    if (this->csManager->getProcessVariable(this->namePV)->isSender()) { \
        this->csManager->getProcessScalar<_p_type>(this->namePV)->set(value);   \
        this->csManager->getProcessScalar<_p_type>(this->namePV)->send();       \
    }\
    return; \
}

#define CREATE_WRITE_FUNCTION_ARRAY(_p_type) \
void mtca_processvariable::setValue_Array_##_p_type(std::vector<_p_type> value) { \
    if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return; \
    if (!this->csManager->getProcessVariable(this->namePV)->isArray())    return; \
    if (this->csManager->getProcessVariable(this->namePV)->isSender()) { \
				for(auto merk: value) { \
					std::cout << merk; \
					std::cout << "test";\
				} \
        this->csManager->getProcessArray<_p_type>(this->namePV)->set(value);	\
    } \
    return; \
}

// #define CREATE_WRITE_FUNCTION_ARRAY(_p_type) \
// void mtca_processvariable::setValue_Array_##_p_type(std::vector<_p_type> value) { \
//     if (this->csManager->getProcessVariable(this->namePV)->getValueType() != typeid(_p_type)) return; \
//     if (!this->csManager->getProcessVariable(this->namePV)->isArray())    return; \
//     if (this->csManager->getProcessVariable(this->namePV)->isSender()) { \
// 				int32_t valueSize = this->csManager->getProcessArray<_p_type>(this->namePV)->get().size(); \
// 				value.resize(valueSize); \
//         this->csManager->getProcessArray<_p_type>(this->namePV)->set(value); \
//     } \
//     return; \
// }

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

// Array
UA_RDPROXY_ARRAY_INT8(mtca_processvariable, getValue_Array_int8_t);
CREATE_READ_FUNCTION_ARRAY(int8_t)
UA_RDPROXY_ARRAY_UINT8(mtca_processvariable, getValue_Array_uint8_t);
CREATE_READ_FUNCTION_ARRAY(uint8_t)
UA_RDPROXY_ARRAY_INT16(mtca_processvariable, getValue_Array_int16_t);
CREATE_READ_FUNCTION_ARRAY(int16_t)
UA_RDPROXY_ARRAY_UINT16(mtca_processvariable, getValue_Array_uint16_t);
CREATE_READ_FUNCTION_ARRAY(uint16_t)
UA_RDPROXY_ARRAY_INT32(mtca_processvariable, getValue_Array_int32_t);
CREATE_READ_FUNCTION_ARRAY(int32_t)
UA_RDPROXY_ARRAY_UINT32(mtca_processvariable, getValue_Array_uint32_t);
CREATE_READ_FUNCTION_ARRAY(uint32_t)
UA_RDPROXY_ARRAY_FLOAT(mtca_processvariable, getValue_Array_float);
CREATE_READ_FUNCTION_ARRAY(float)
UA_RDPROXY_ARRAY_DOUBLE(mtca_processvariable, getValue_Array_double);
CREATE_READ_FUNCTION_ARRAY(double)

UA_WRPROXY_ARRAY_INT8(mtca_processvariable, setValue_Array_int8_t);
CREATE_WRITE_FUNCTION_ARRAY(int8_t)
UA_WRPROXY_ARRAY_UINT8(mtca_processvariable, setValue_Array_uint8_t);
CREATE_WRITE_FUNCTION_ARRAY(uint8_t)
UA_WRPROXY_ARRAY_INT16(mtca_processvariable, setValue_Array_int16_t);
CREATE_WRITE_FUNCTION_ARRAY(int16_t)
UA_WRPROXY_ARRAY_UINT16(mtca_processvariable, setValue_Array_uint16_t);
CREATE_WRITE_FUNCTION_ARRAY(uint16_t)
UA_WRPROXY_ARRAY_INT32(mtca_processvariable, setValue_Array_int32_t);
CREATE_WRITE_FUNCTION_ARRAY(int32_t)
UA_WRPROXY_ARRAY_UINT32(mtca_processvariable, setValue_Array_uint32_t);
CREATE_WRITE_FUNCTION_ARRAY(uint32_t)
UA_WRPROXY_ARRAY_FLOAT(mtca_processvariable, setValue_Array_float);
CREATE_WRITE_FUNCTION_ARRAY(float)
UA_WRPROXY_ARRAY_DOUBLE(mtca_processvariable, setValue_Array_double);
CREATE_WRITE_FUNCTION_ARRAY(double)


// Just a macro to easy pushing different types of dataSources
// ... and make sure we lock down writing to receivers in this stage already
#define PUSH_RDVALUE_TYPE(_p_typeName) { \
if(this->csManager->getProcessScalar<_p_typeName>(this->namePV)->isSender())  { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .read=UA_RDPROXY_NAME(mtca_processvariable, getValue_##_p_typeName), .write=UA_WRPROXY_NAME(mtca_processvariable, setValue_##_p_typeName) }); } \
    else                                                                    { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .read=UA_RDPROXY_NAME(mtca_processvariable, getValue_##_p_typeName) }); }\
}
    
#define PUSH_RDVALUE_ARRAY_TYPE(_p_typeName) { \
if(this->csManager->getProcessArray<_p_typeName>(this->namePV)->isSender()) { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .read=UA_RDPROXY_NAME(mtca_processvariable, getValue_Array_##_p_typeName), .write=UA_WRPROXY_NAME(mtca_processvariable, setValue_Array_##_p_typeName) }); } \
    else                                                                  { mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_VALUE), .read=UA_RDPROXY_NAME(mtca_processvariable, getValue_Array_##_p_typeName) }); } \
}

UA_StatusCode mtca_processvariable::mapSelfToNamespace() {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_NodeId createdNodeId = UA_NODEID_NULL;
    
    if (UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE) 
        return 0; // Something went UA_WRING (initializer should have set this!)
    
    // Create our toplevel instance
    UA_ObjectAttributes oAttr; 
		UA_ObjectAttributes_init(&oAttr);
		
    oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", this->nameNew.c_str());
    oAttr.description = UA_LOCALIZEDTEXT_ALLOC("en_US", "A process variable");
		
		if (this->csManager->getProcessVariable(this->namePV)->isSender()) {
			oAttr.userWriteMask = UA_ACCESSLEVELMASK_WRITE;
			oAttr.writeMask = UA_ACCESSLEVELMASK_WRITE;
		}
    
    UA_INSTATIATIONCALLBACK(icb);  
    UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1,0),
                            this->baseNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME_ALLOC(1, this->nameNew.c_str()), UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_MTCAPROCESSVARIABLE), oAttr, &icb, &createdNodeId);
    
    /* Use a function map to map any local functioncalls to opcua methods (we do not own any methods) */
    UA_FunctionCall_Map mapThis;
    this->ua_mapFunctions(this, &mapThis, createdNodeId);
	
	// know your own nodeId
	this->ownNodeId = createdNodeId;
    
    /* Use a datasource map to map any local getter/setter functions to opcua variables nodes */
    UA_DataSource_Map mapDs;
    // FIXME: We should not be using std::cout here... Where's our logger?
    if (! this->csManager->getProcessVariable(this->namePV)->isArray()) {
        std::type_info const & valueType = this->csManager->getProcessVariable(this->namePV)->getValueType();
        if (valueType == typeid(int8_t))          PUSH_RDVALUE_TYPE(int8_t)
        else if (valueType == typeid(uint8_t))    PUSH_RDVALUE_TYPE(uint8_t)
        else if (valueType == typeid(int16_t))    PUSH_RDVALUE_TYPE(int16_t)
        else if (valueType == typeid(uint16_t))   PUSH_RDVALUE_TYPE(uint16_t)
        else if (valueType == typeid(int32_t))    PUSH_RDVALUE_TYPE(int32_t)
        else if (valueType == typeid(uint32_t))   PUSH_RDVALUE_TYPE(uint32_t)
        else if (valueType == typeid(float))      PUSH_RDVALUE_TYPE(float)
        else if (valueType == typeid(double))     PUSH_RDVALUE_TYPE(double)
        else std::cout << "Cannot proxy unknown type " << typeid(valueType).name()  << std::endl;
    }
    else {
        std::type_info const & valueType = this->csManager->getProcessVariable(this->namePV)->getValueType();
        if (valueType == typeid(int8_t))          PUSH_RDVALUE_ARRAY_TYPE(int8_t)
        else if (valueType == typeid(uint8_t))    PUSH_RDVALUE_ARRAY_TYPE(uint8_t)
        else if (valueType == typeid(int16_t))    PUSH_RDVALUE_ARRAY_TYPE(int16_t)
        else if (valueType == typeid(uint16_t))   PUSH_RDVALUE_ARRAY_TYPE(uint16_t)
        else if (valueType == typeid(int32_t))    PUSH_RDVALUE_ARRAY_TYPE(int32_t)
        else if (valueType == typeid(uint32_t))   PUSH_RDVALUE_ARRAY_TYPE(uint32_t)
        else if (valueType == typeid(float))      PUSH_RDVALUE_ARRAY_TYPE(float)
        else if (valueType == typeid(double))     PUSH_RDVALUE_ARRAY_TYPE(double)
        else std::cout << "Cannot proxy unknown array type " << typeid(valueType).name() << std::endl;
    }
    
    
		mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_NAME), .read=UA_RDPROXY_NAME(mtca_processvariable, getName)});
    mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_UNIT), .read=UA_RDPROXY_NAME(mtca_processvariable, getEngineeringUnit), .write=UA_WRPROXY_NAME(mtca_processvariable, setEngineeringUnit)});
    mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TYPE), .read=UA_RDPROXY_NAME(mtca_processvariable, getType)});
		mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TIMESTAMP_SECONDS), .read=UA_RDPROXY_NAME(mtca_processvariable, getTimeStampSeconds)});
    mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TIMESTAMP_NANOSECONDS), .read=UA_RDPROXY_NAME(mtca_processvariable, getTimeStampNanoSeconds)});
    mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TIMESTAMP_INDEX0), .read=UA_RDPROXY_NAME(mtca_processvariable, getTimeStampIndex0)});
    mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLE_TIMESTAMP_INDEX1), .read=UA_RDPROXY_NAME(mtca_processvariable, getTimeStampIndex1)});

	this->ua_mapDataSources((void *) this, &mapDs);
	
	return UA_STATUSCODE_GOOD;
}

UA_NodeId mtca_processvariable::getOwnNodeId() {
	return this->ownNodeId;
}