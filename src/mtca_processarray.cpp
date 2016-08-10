/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  <copyright holder> <email>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "mtca_processarray.h"

extern "C" {
#include "mtca_namespaceinit_generated.h"
}

#include "ua_proxies.h"
#include "ua_proxies_callback.h"
#include "mtca_processvariable.h"
#include <mtca_timestamp.h>

#include <iostream>
#include <vector>


mtca_processarray::mtca_processarray(UA_Server *server, UA_NodeId baseNodeId, std::string name, std::vector<std::string> value, std::type_info const &valueType, mtca4u::TimeStamp timeStamp):ua_mapped_class(server, baseNodeId)
{
  // maybe use templating to create a more generalized class?
  this->mtca_processvariable_globalinit(name, value,  valueType, timeStamp);
}

mtca_processarray::~mtca_processarray()
{
  // Our ua_mapped_class destructor will take care of deleting our opcua footprint as long as all variables are mapped in this->ownedNodes
}

void mtca_processarray::mtca_processvariable_globalinit(std::string name, std::vector<std::string> value, std::type_info const &valueType, mtca4u::TimeStamp timeStamp)
{
    this->name = name;
  
  // Mapping the right type
    if (valueType  == typeid(int8_t)) {
        this->valueType = "int8_t";
    } else if (valueType == typeid(uint8_t)) {
        this->valueType = "uint8_t";
    } else if (valueType == typeid(int16_t)) {
        this->valueType = "int16_t";
    } else if (valueType == typeid(uint16_t)) {
        this->valueType = "uint16_t";
    } else if (valueType == typeid(int32_t)) {
        this->valueType = "int32_t";
    } else if (valueType == typeid(uint32_t)) {
        this->valueType = "uint16_t";
    } else if (valueType == typeid(float)) {
        this->valueType = "float";
    } else if (valueType == typeid(double)) {
        this->valueType = "double";
    } else {
        throw std::invalid_argument("unsupported value type");    
    }
    
    
    
    this->value = value;
    this->timeStamp = timeStamp;
    this->mapSelfToNamespace();
}

/* UA Mapped class members */
// FIXME: We would need to tie all our model values into getter/setter functions... for now, only engineeringunit is passed through

// Setter/Getter
// Name
UA_RDPROXY_STRING(mtca_processarray, getName)
std::string mtca_processarray::getName() 
{
  return this->name;
}

UA_WRPROXY_STRING(mtca_processarray, setName)
void mtca_processarray::setName(std::string name) 
{
  this->name = name;
}

// Type
UA_RDPROXY_STRING(mtca_processarray, getType)
std::string mtca_processarray::getType() 
{
  return this->valueType;
}

UA_WRPROXY_STRING(mtca_processarray, setType)
void mtca_processarray::setType(std::string valueType) 
{
  this->valueType = valueType;
}

// Value
//UA_RDPROXY_STRING_ARRAY(mtca_processarray, getValue)
std::vector<std::string> mtca_processarray::getValue() 
{
	return this->value;
}

//UA_WRPROXY_STRING_ARRAY(mtca_processarray, setValue)
void mtca_processarray::setValue(std::vector<std::string> varValue) 
{
	//void * UA_Array_new(size_t size, const UA_DataType *type);
	//UA_Array_copy(const void *src, size_t size, void **dst,
           //   const UA_DataType *type);
	this->value = varValue;
}

// TimeStamp
//UA_RDPROXY_UINT32(mtca_processarray, getTimeStamp)
mtca4u::TimeStamp mtca_processarray::getTimeStamp() 
{
  return this->timeStamp;
}

//UA_WRPROXY_UINT32(mtca_processarray, setTimeStamp)
void mtca_processarray::setTimeStamp(mtca4u::TimeStamp timeStamp) 
{
  this->timeStamp = timeStamp;
}

UA_StatusCode mtca_processarray::mapSelfToNamespace() 
{
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  UA_NodeId createdNodeId = UA_NODEID_NULL;
  
    if (UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE) {
        return 0; // Something went UA_WRING (initializer should have set this!)
    }
    
    // Create our toplevel instance
    UA_ObjectAttributes oAttr; 
    oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", this->name.c_str());
    oAttr.description = UA_LOCALIZEDTEXT_ALLOC("en_US", "A process array");
  
    UA_INSTATIATIONCALLBACK(icb);
    UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1,0),
                            UA_NODEID_NUMERIC(2,5004), UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME_ALLOC(1, this->name.c_str()), UA_NODEID_NUMERIC(2, UA_NS2ID_MTCAPROCESSARRAY), oAttr, &icb, &createdNodeId);
    
    // FIXME Irgendwas mit Array....
    //UA_DataSource dataSource;
    //UA_Server_setVariableNode_dataSource(this->mappedServer, UA_NODEID_NUMERIC(2, 6005), dataSource);

  
    // Create TimeStamp Object
    new mtca_timestamp(this->mappedServer, createdNodeId, this->timeStamp.seconds, this->timeStamp.nanoSeconds, this->timeStamp.index0, this->timeStamp.index1);
  
    /* Use a function map to map any local functioncalls to opcua methods (we do not own any methods) */
    UA_FunctionCall_Map mapThis;
    this->ua_mapFunctions(this, &mapThis, createdNodeId);

    /* Use a datasource map to map any local getter/setter functions to opcua variables nodes */
    UA_DataSource_Map mapDs;
    mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6001), .read=UA_RDPROXY_NAME(mtca_processarray, getName), .write=UA_WRPROXY_NAME(mtca_processarray, setName)});
    mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6002), .read=UA_RDPROXY_NAME(mtca_processarray, getType), .write=UA_WRPROXY_NAME(mtca_processarray, setType)});
    //mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6005), .read=UA_RDPROXY_NAME(mtca_processarray, getValue), .write=UA_WRPROXY_NAME(mtca_processarray, setValue)});
    //mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6007), .read=UA_RDPROXY_NAME(mtca_processarray, getTimeStamp), .write=UA_WRPROXY_NAME(mtca_processarray, setTimeStamp)});

    ua_callProxy_mapDataSources(this->mappedServer, this->ownedNodes, &mapDs, (void *) this);

    return UA_STATUSCODE_GOOD;

}
