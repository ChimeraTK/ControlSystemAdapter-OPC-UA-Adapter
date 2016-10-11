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

UA_NodeId *nodePairList_getSourceIdByTargetId(nodePairList pairList, UA_NodeId targetId) {
  UA_NodeId *local = nullptr;
  // cppcheck-suppress postfixOperator                  REASON: List iterator cannot be prefixed
  for(nodePairList::iterator j = pairList.begin(); j != pairList.end(); j++)
    if (UA_NodeId_equal(&((*j)->targetNodeId), &targetId ) == UA_TRUE) {
      local = &((*j)->sourceNodeId);
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

UA_StatusCode ua_callProxy_mapFunctions(UA_Server *server, UA_NodeId objectId, UA_FunctionCall_Map *map, void *srcClass, int recursive) {
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  if (map == nullptr || server == nullptr)
    return retval;
    
  // Functions are not instantiated... they are just linked to the node. 
  UA_BrowseDescription bDesc;
  UA_BrowseDescription_init(&bDesc);
  bDesc.nodeId = objectId;
  bDesc.browseDirection = UA_BROWSEDIRECTION_FORWARD;
  bDesc.includeSubtypes = UA_FALSE;
  bDesc.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
  bDesc.nodeClassMask = UA_NODECLASS_METHOD;
  bDesc.resultMask = UA_BROWSERESULTMASK_ALL;
  
  UA_BrowseResult bResp = UA_Server_browse(server, 65536, &bDesc);
  for (int i = 0; i<bResp.referencesSize; i++) {
    UA_ReferenceDescription ref =  bResp.references[i];
    
    // Recursive on objects
    if (ref.nodeClass == UA_NODECLASS_OBJECT && recursive != 0) {
        ua_callProxy_mapFunctions(server, ref.nodeId.nodeId, map, srcClass, recursive < 0 ? recursive : recursive-1); // Recursive==-1 is infinite, else adjust recursion count
        continue;
    }
    
    if (! (ref.nodeClass == UA_NODECLASS_METHOD))
      continue;
    
    // Check if we have this node in our map
    UA_FunctionCall_Map_Element *ele = nullptr;
    // cppcheck-suppress postfixOperator                REASON: List iterator cannot be prefixed
    for (std::list<UA_FunctionCall_Map_Element>::iterator j=map->begin(); j != map->end(); j++) {
      UA_FunctionCall_Map_Element thisEle = *(j);
      if(UA_NodeId_equal(&ref.nodeId.nodeId, (const UA_NodeId *) &thisEle.typeTemplateId) == UA_TRUE) {
        ele = &thisEle;
	break;
      }
    }
    if (ele == nullptr)
      continue;
    
    // Found a mapping: templateId <-> objectId /w functionpointer...
    UA_FunctionCall_InstanceLookUpTable *tbl =  ele->lookupTable;
    if (tbl == NULL)
      continue; // Only possible on wrong call, since the tables are allocated during compilation!
    
    // Register this objects Id with the callbacks lookup table
    UA_FunctionCall_InstanceLookupTable_Element *ntel = new UA_FunctionCall_InstanceLookupTable_Element;
    ntel->classInstance = srcClass;
    UA_NodeId_copy( &objectId, &ntel->classObjectId);
    ntel->server = server;
    ele->lookupTable->push_back(ntel);
    
    // I forgot what this was supposed to be doing - but it clearly was meant to ... eeee...?
    //     for (UA_FunctionCall_InstanceLookUpTable::iterator j=tbl->begin(); j != tbl->end(); j++) 
    //       if(UA_NodeId_equal(&(*(j))->classObjectId, &objectId)) {
    //         void *a = &(*(j))->classInstance;
    //         break;
    //       }
    
    // Found a mapping, now register the callback; pass the server along (instance and object are identified in the table)
    retval |= UA_Server_setMethodNode_callback(server, (const UA_NodeId) ele->typeTemplateId, ele->callback, (void *) server);
  }
  
  return retval;
}

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
    ds.handle = srcClass;
    delete ele; // inhibit memleak warning during static analysis
    
    retval |= UA_Server_setVariableNode_dataSource(server, (const UA_NodeId) instantiatedId, ds);

	/* Ste the right Value Datatype */
	UA_NodeId datatTypeNodeId = UA_NODEID_NULL;
    retval = UA_Server_readDataType(server, instantiatedId, &datatTypeNodeId);
		if(retval == UA_STATUSCODE_GOOD) {
			const UA_NodeId basedatatype = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE);
			UA_Variant variantVal;
			UA_Variant_init(&variantVal);
			retval = UA_Server_readValue(server, instantiatedId, &variantVal);
			if(UA_NodeId_equal(&datatTypeNodeId, &basedatatype) && retval == UA_STATUSCODE_GOOD) {
				retval = UA_Server_writeDataType(server, instantiatedId, variantVal.type->typeId);
				
				UA_Int32 valueRank = -2;				
				if(variantVal.arrayDimensionsSize == 0 && UA_Variant_isScalar(&variantVal)) {
					// Scalar
					valueRank = -1;
				}
				else {
					if(variantVal.arrayDimensionsSize == 1) {
						// OneDimension
						valueRank = 1;
					}
					else {
						if(variantVal.arrayDimensionsSize > 1) {
							// OneOrMoreDimensions
							valueRank = 0;
						}
					}
				}
				UA_Server_writeValueRank(server, instantiatedId, valueRank);
			}
		}
	}
  
  return retval;
}

template <class C, typename T, uint32_t UA_T>
UA_StatusCode ua_valuesource_rdproxy(void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, 
                                     const UA_NumericRange *range, UA_DataValue *value) 
{
  C *thisObj = static_cast<C *> (handle); 
  if (includeSourceTimeStamp) { 
    value->serverTimestamp = UA_DateTime_now(); 
    value->hasServerTimestamp = UA_TRUE; 
  } 
  
  T ua_val = thisObj->M(); 
  UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_T]); 
  
  value->hasValue = UA_TRUE; 
  if (includeSourceTimeStamp) { 
    value->sourceTimestamp = UA_DateTime_now(); 
    value->hasSourceTimestamp = UA_TRUE; 
  } 
  
  return UA_STATUSCODE_GOOD;
}
