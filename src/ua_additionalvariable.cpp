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

#include "ua_additionalvariable.h"
#include "csa_config.h"

extern "C" {
#include "csa_namespaceinit_generated.h"
}

#include "ua_proxies.h"
#include "ua_proxies_callback.h"

#include <iostream>

using namespace std;

ua_additionalvariable::ua_additionalvariable(UA_Server* server, UA_NodeId basenodeid, string name, string value, string description) : ua_mapped_class(server, basenodeid) {
	
	this->name = name;
  this->value = value;
	this->description = description;
	
	this->mapSelfToNamespace();
}

ua_additionalvariable::~ua_additionalvariable()
{
  // Our ua_mapped_class destructor will take care of deleting our opcua footprint as long as all variables are mapped in this->ownedNodes
}

// Value
UA_StatusCode ua_additionalvariable::ua_readproxy_ua_additionalvariable_getValue(void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value) {
    ua_additionalvariable *thisObj = static_cast<ua_additionalvariable *>(handle);
    UA_String ua_val;
    {
        char *s = (char *) malloc(thisObj->getValue().length() + 1);
        strncpy(s, (char*) thisObj->getValue().c_str(), thisObj->getValue().length());
        ua_val.length = thisObj->getValue().length();
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

//UA_RDPROXY_STRING(ua_additionalvariable, getValue)
string ua_additionalvariable::getValue() {
  return this->value;
}

UA_StatusCode ua_additionalvariable::mapSelfToNamespace() {
 
	  UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_NodeId createdNodeId = UA_NODEID_NULL;
		
    if (UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE) 
        return 0; // Something went UA_WRING (initializer should have set this!)
		
    // Create our toplevel instance
    UA_ObjectAttributes oAttr; 
		UA_ObjectAttributes_init(&oAttr);
		
    oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC((char*) "en_US", (char*)this->name.c_str());
    oAttr.description = UA_LOCALIZEDTEXT_ALLOC((char*) "en_US", (char*)this->description.c_str());
		    		
//    UA_INSTATIATIONCALLBACK(icb);
    UA_InstantiationCallback icb;
    icb.handle = (void *) &this->ownedNodes;
    icb.method = ua_mapInstantiatedNodes;
    UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1, 0), this->baseNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME_ALLOC(1, this->name.c_str()), UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_CTKADDITIONALVARIABLE), oAttr, &icb, &createdNodeId);
    
	// know your own nodeId
	this->ownNodeId = createdNodeId;
		
	/* Use a datasource map to map any local getter/setter functions to opcua variables nodes */
	UA_DataSource_Map mapDs;
	UA_DataSource_Map_Element mapElemValue;
	mapElemValue.typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_ADDITIONAL_VARIABLE_VALUE);
	mapElemValue.description = oAttr.description;
	mapElemValue.read = ua_readproxy_ua_additionalvariable_getValue;
	mapElemValue.write = NULL;
	mapDs.push_back(mapElemValue);
	//mapDs.push_back((UA_DataSource_Map_Element) { .typeTemplateId = UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_ADDITIONAL_VARIABLE_VALUE), oAttr.description, .read=UA_RDPROXY_NAME(ua_additionalvariable, getValue), .write=NULL});
	
	this->ua_mapDataSources((void *) this, &mapDs);
	
	return UA_STATUSCODE_GOOD;
}
	

UA_DateTime ua_additionalvariable::getSourceTimeStamp() {
	return UA_DateTime_now();
}
