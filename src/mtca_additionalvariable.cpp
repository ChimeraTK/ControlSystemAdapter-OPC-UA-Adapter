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

#include "mtca_additionalvariable.h"
#include "csa_config.h"

extern "C" {
#include "mtca_namespaceinit_generated.h"
}

#include "ua_proxies.h"
#include "ua_proxies_callback.h"

#include <iostream>

using namespace std;

mtca_additionalvariable::mtca_additionalvariable(UA_Server* server, UA_NodeId basenodeid, string name, string value, string description) : ua_mapped_class(server, basenodeid) {
	
	this->name = name;
  this->value = value;
	this->description = description;
	
	this->mapSelfToNamespace();
}

mtca_additionalvariable::~mtca_additionalvariable()
{
  // Our ua_mapped_class destructor will take care of deleting our opcua footprint as long as all variables are mapped in this->ownedNodes
}

// Value
UA_RDPROXY_STRING(mtca_additionalvariable, getValue)
string mtca_additionalvariable::getValue() {
  return this->value;
}

UA_StatusCode mtca_additionalvariable::mapSelfToNamespace() {
 
	  UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_NodeId createdNodeId = UA_NODEID_NULL;
		
    if (UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE) 
        return 0; // Something went UA_WRING (initializer should have set this!)
		
    // Create our toplevel instance
    UA_ObjectAttributes oAttr; 
		UA_ObjectAttributes_init(&oAttr);
		
    oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", (char*)this->name.c_str());
    oAttr.description = UA_LOCALIZEDTEXT_ALLOC("en_US", (char*)this->description.c_str());
		    
    UA_INSTATIATIONCALLBACK(icb);  
    UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1,0),
                            this->baseNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME_ALLOC(1, this->name.c_str()), UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_MTCAADDITIONALVARIABLE), oAttr, &icb, &createdNodeId);
    
	/* Use a function map to map any local functioncalls to opcua methods (we do not own any methods) */
	UA_FunctionCall_Map mapThis;
	this->ua_mapFunctions(this, &mapThis, createdNodeId);
	
	// know your own nodeId
	this->ownNodeId = createdNodeId;
    
	/* Use a datasource map to map any local getter/setter functions to opcua variables nodes */
	UA_DataSource_Map mapDs;
	mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_ADDITIONAL_VARIABLE_VALUE), UA_LOCALIZEDTEXT((char*)"", (char*)""), .read=UA_RDPROXY_NAME(mtca_additionalvariable, getValue)});
			

	this->ua_mapDataSources((void *) this, &mapDs);
	
	return UA_STATUSCODE_GOOD;
}
	
UA_DateTime mtca_additionalvariable::getSourceTimeStamp() {
	return UA_DateTime_now();
}