/*
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 * Chair for Process Systems Engineering
 * Technical University of Dresden
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

/** @class ua_proxies
 * 	@brief Helper class to interact with open62541
 * 	This class is a kind of a proxy to interact with the open62541 stack. For this the class mapped all variables to the nodestore of the open62541
 * 	
 *	@author Chris Iatrou
 *	@author Julian Rahm
 *  @date 22.11.2016
 * 
 */

extern "C" 
{
  #include "open62541.h"
}

#include "ua_proxies.h"

#include <iostream>

using namespace std;

UA_NodeId *nodePairList_getTargetIdBySourceId(nodePairList pairList, UA_NodeId remoteId) {
  UA_NodeId *local = nullptr;
  // cppcheck-suppress postfixOperator                  REASON: List iterator cannot be prefixed
  for(nodePairList::iterator j = pairList.begin(); j != pairList.end(); j++)
    if (UA_NodeId_equal(&((*j)->sourceNodeId), &remoteId ) == UA_TRUE) {
      local = &((*j)->targetNodeId);
      break;
    }
  return local;
}

UA_StatusCode ua_mapInstantiatedNodes(UA_NodeId objectId, UA_NodeId definitionId, void *handle) {
  nodePairList *lst = static_cast<nodePairList*>(handle);
  
  UA_NodeId_pair *thisNode = new UA_NodeId_pair;
  UA_NodeId_copy(&definitionId, &thisNode->sourceNodeId);
  UA_NodeId_copy(&objectId, &thisNode->targetNodeId);
  lst->push_back(thisNode);
  
  return UA_STATUSCODE_GOOD;
}

/**
 * @brief This methode map all variables in sort of a <UA_DataSourceMap> from the called class to the open62541
 * 
 */
UA_StatusCode ua_callProxy_mapDataSources(UA_Server* server, nodePairList instantiatedNodesList, UA_DataSource_Map *map, void *srcClass) 
{
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  if (map == nullptr || server == nullptr)
    return retval;
  
  // Functions are not instantiated... they are just linked to the node. 
  for (nodePairList::iterator l = instantiatedNodesList.begin(); l != instantiatedNodesList.end(); ++l) {
    UA_NodeId typeTemplateId = (*l)->sourceNodeId;
    UA_NodeId instantiatedId = (*l)->targetNodeId;
    
    // Check if we have this node in our map
    UA_DataSource_Map_Element *ele = nullptr;
    // cppcheck-suppress postfixOperator                REASON: List iterator cannot be prefixed
    for (UA_DataSource_Map::iterator j=map->begin(); j != map->end(); j++) {
      UA_DataSource_Map_Element thisEle = *(j);
      if(UA_NodeId_equal(&typeTemplateId, (const UA_NodeId *) &thisEle.typeTemplateId) == UA_TRUE) {
        ele = new UA_DataSource_Map_Element;
        *ele = thisEle;
        break;
      }
    }
    if (ele == nullptr)
      continue;
    
    // Found a mapping, now register the dataSource
    UA_DataSource ds;
    ds.read = ele->read;		
    ds.write = ele->write;
		// Set accesslevel depending on callback functions
		UA_Byte accessLevel;
		if(ele->write == NULL && ele->read == NULL) {
			accessLevel = 0;
		}
		if(ele->write != NULL && ele->read == NULL) {
			accessLevel = UA_ACCESSLEVELMASK_WRITE;
		}
		if(ele->write != NULL && ele->read != NULL) {
			accessLevel = UA_ACCESSLEVELMASK_WRITE^UA_ACCESSLEVELMASK_READ;
		}
		if(ele->write == NULL && ele->read != NULL) {
			accessLevel = UA_ACCESSLEVELMASK_READ;
		}
		retval = UA_Server_writeAccessLevel(server, instantiatedId, accessLevel);
		// There is currently no high- level function to do this. (02.12.2016)
		retval = __UA_Server_write(server, &instantiatedId, UA_ATTRIBUTEID_USERACCESSLEVEL, &UA_TYPES[UA_TYPES_BYTE], &accessLevel);
		
		ds.handle = srcClass;
       
    retval |= UA_Server_setVariableNode_dataSource(server, instantiatedId, ds);
		
		UA_Server_writeDescription(server, instantiatedId, ele->description);
		delete ele; // inhibit memleak warning during static analysis
		
	/* Set the right Value Datatype and ValueRank
	 * -> This is a quickfix for subjective data handling by open62541
	 * (02.12.2016)
	 */
	UA_NodeId datatTypeNodeId = UA_NODEID_NULL;
    retval = UA_Server_readDataType(server, instantiatedId, &datatTypeNodeId);
		if(retval == UA_STATUSCODE_GOOD) {
			const UA_NodeId basedatatype = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE);
			UA_Variant variantVal;
			UA_Variant_init(&variantVal);
			retval = UA_Server_readValue(server, instantiatedId, &variantVal);
			if(UA_NodeId_equal(&datatTypeNodeId, &basedatatype) && retval == UA_STATUSCODE_GOOD) {
				retval = UA_Server_writeDataType(server, instantiatedId, variantVal.type->typeId);
				
				// See IEC 62541-3: OPC Unified Architecture - Part 3: Address space model -> Page 75
				UA_Int32 valueRank = -2;	
				if(variantVal.arrayDimensionsSize == 0 && UA_Variant_isScalar(&variantVal)) {
					// Scalar
					valueRank = -1;
				}
				else {
					if(variantVal.arrayDimensionsSize > 1) {
							// OneOrMoreDimensions
							valueRank = 0;
					}
					else {
						if(variantVal.arrayDimensionsSize == 1 || variantVal.arrayLength) {
							// OneDimension
							valueRank = 1;
						}
					}
				}
				UA_Server_writeValueRank(server, instantiatedId, valueRank);
			}
		}
	}
  
  return retval;
}
