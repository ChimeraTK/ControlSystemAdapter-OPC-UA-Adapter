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

extern "C" {
#include "mtca_namespaceinit_generated.h"
}

#include "ua_proxies.h"
#include "ua_proxies_callback.h"
#include <iostream>


mtca_processvariable::mtca_processvariable(UA_Server *server, UA_NodeId basenodeid, std::string name, std::string type, std::string value, uint32_t timeStamp) : ua_mapped_class(server, basenodeid)
{
  // maybe use templating to create a more generalized class?
  this->mtca_processvariable_globalinit(name, type, value, timeStamp);
}

mtca_processvariable::~mtca_processvariable()
{
  // Our ua_mapped_class destructor will take care of deleting our opcua footprint as long as all variables are mapped in this->ownedNodes
}

void mtca_processvariable::mtca_processvariable_globalinit(std::string name, std::string type, std::string value, uint32_t timeStamp)
{
  this->name = name;
  this->type = type;
  this->value = value;
  this->timeStamp = timeStamp;
  this->mapSelfToNamespace();
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
  return this->type;
}
UA_WRPROXY_STRING(mtca_processvariable, setType)
void mtca_processvariable::setType(std::string type) {
  this->value = type;
}
// Value
 UA_RDPROXY_STRING(mtca_processvariable, getValue)
std::string mtca_processvariable::getValue() {
  return this->value;
}
UA_WRPROXY_STRING(mtca_processvariable, setValue)
void mtca_processvariable::setValue(std::string varValue) {
  this->value = varValue;
}
// TimeStamp
UA_RDPROXY_UINT32(mtca_processvariable, getTimeStamp)
uint32_t mtca_processvariable::getTimeStamp() {
  return this->timeStamp;
}
UA_WRPROXY_UINT32(mtca_processvariable, setTimeStamp)
void mtca_processvariable::setTimeStamp(uint32_t timeStamp) {
  this->timeStamp = timeStamp;
}

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
                          UA_NODEID_NUMERIC(2,5002), UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                          UA_QUALIFIEDNAME_ALLOC(1, this->name.c_str()), UA_NODEID_NUMERIC(2, UA_NS2ID_MTCAPROCESSVARIABLE), oAttr, &icb, &createdNodeId);
  
  /* Use a function map to map any local functioncalls to opcua methods (we do not own any methods) */
  UA_FunctionCall_Map mapThis;
  this->ua_mapFunctions(this, &mapThis, createdNodeId);
  
  /* Use a datasource map to map any local getter/setter functions to opcua variables nodes */
  UA_DataSource_Map mapDs;
  mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6004), .read=UA_RDPROXY_NAME(mtca_processvariable, getName), .write=UA_WRPROXY_NAME(mtca_processvariable, setName)});
  mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6012), .read=UA_RDPROXY_NAME(mtca_processvariable, getType), .write=UA_WRPROXY_NAME(mtca_processvariable, setType)});
  mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6014), .read=UA_RDPROXY_NAME(mtca_processvariable, getValue), .write=UA_WRPROXY_NAME(mtca_processvariable, setValue)});
  mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(2, 6013), .read=UA_RDPROXY_NAME(mtca_processvariable, getTimeStamp), .write=UA_WRPROXY_NAME(mtca_processvariable, setTimeStamp)});
  
  ua_callProxy_mapDataSources(this->mappedServer, this->ownedNodes, &mapDs, (void *) this);
  
  return UA_STATUSCODE_GOOD;
}