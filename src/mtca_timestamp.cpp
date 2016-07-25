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

#include "mtca_timestamp.h"

extern "C" {
#include "mtca_namespaceinit_generated.h"
}

#include "ua_proxies.h"
#include "ua_proxies_callback.h"

#include <iostream>

mtca_timestamp::mtca_timestamp(UA_Server *server, UA_NodeId baseNodeId, uint32_t seconds, uint32_t nanoseconds, uint32_t index0, uint32_t index1):ua_mapped_class(server, baseNodeId)
{
  // maybe use templating to create a more generalized class?
  this->mtca_timestamp_globalinit(seconds, nanoseconds, index0, index1);

}

mtca_timestamp::mtca_timestamp() {
	
}

mtca_timestamp::~mtca_timestamp() {

}

void mtca_timestamp::mtca_timestamp_globalinit(uint32_t seconds, uint32_t nanoseconds, uint32_t index0, uint32_t index1)
{
  this->seconds = seconds;
  this->nanoseconds = nanoseconds;
  this->index0 = index0;
  this->index1 = index1;
  
  this->mapSelfToNamespace();
}


// Setter/Getter
// Seconds
UA_RDPROXY_UINT32(mtca_timestamp, getSeconds)
uint32_t mtca_timestamp::getSeconds() {
  return this->seconds;
}
UA_WRPROXY_UINT32(mtca_timestamp, setSeconds)
void mtca_timestamp::setSeconds(uint32_t seconds) {
  this->seconds = seconds;
}

// Nanoseconds
UA_RDPROXY_UINT32(mtca_timestamp, getNanoseconds)
uint32_t mtca_timestamp::getNanoseconds() {
  return this->nanoseconds;
}
UA_WRPROXY_UINT32(mtca_timestamp, setNanoseconds)
void mtca_timestamp::setNanoseconds(uint32_t nanoseconds) {
  this->nanoseconds = nanoseconds;
}

// Index0
UA_RDPROXY_UINT32(mtca_timestamp, getIndex0)
uint32_t mtca_timestamp::getIndex0() {
  return this->index0;
}
UA_WRPROXY_UINT32(mtca_timestamp, setIndex0)
void mtca_timestamp::setIndex0(uint32_t index0) {
  this->index0 = index0;
}
// Index1
UA_RDPROXY_UINT32(mtca_timestamp, getIndex1)
uint32_t mtca_timestamp::getIndex1() {
  return this->index1;
}
UA_WRPROXY_UINT32(mtca_timestamp, setIndex1)
void mtca_timestamp::setIndex1(uint32_t index1) {
  this->index1 = index1;
}

UA_StatusCode mtca_timestamp::mapSelfToNamespace() {
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  UA_NodeId createdNodeId = UA_NODEID_NULL;
  
  if (UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE) 
    return 0; // Something went UA_WRING (initializer should have set this!)
    
  // Create our toplevel instance
  UA_ObjectAttributes oAttr; 
  oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", "TimeStamp");
  oAttr.description = UA_LOCALIZEDTEXT_ALLOC("en_US", "TimeStamp");
  
  UA_INSTATIATIONCALLBACK(icb);
  UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1,0),
                          this->baseNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                          UA_QUALIFIEDNAME_ALLOC(1, "TimeStamp"), UA_NODEID_NUMERIC(2, UA_NS2ID_MTCATIMESTAMP), oAttr, &icb, &createdNodeId);
  
  /* Use a function map to map any local functioncalls to opcua methods (we do not own any methods) */
  UA_FunctionCall_Map mapThis;
  this->ua_mapFunctions(this, &mapThis, createdNodeId);
  
  /* Use a datasource map to map any local getter/setter functions to opcua variables nodes */
  UA_DataSource_Map mapDs;
  mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6013), .read=UA_RDPROXY_NAME(mtca_timestamp, getSeconds), .write=UA_WRPROXY_NAME(mtca_timestamp, setSeconds)});
  mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6016), .read=UA_RDPROXY_NAME(mtca_timestamp, getNanoseconds), .write=UA_WRPROXY_NAME(mtca_timestamp, setNanoseconds)});
  mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6020), .read=UA_RDPROXY_NAME(mtca_timestamp, getIndex0), .write=UA_WRPROXY_NAME(mtca_timestamp, setIndex0)});
  mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6021), .read=UA_RDPROXY_NAME(mtca_timestamp, getIndex1), .write=UA_WRPROXY_NAME(mtca_timestamp, setIndex1)});
  
  ua_callProxy_mapDataSources(this->mappedServer, this->ownedNodes, &mapDs, (void *) this);
  
  return UA_STATUSCODE_GOOD;
}