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
 * Copyright (c) 2019-2020 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
 */

#include "ua_proxies.h"
#include "csa_config.h"

#include <iostream>

using namespace std;

UA_NodeId* nodePairList_getTargetIdBySourceId(nodePairList pairList, UA_NodeId remoteId) {
  UA_NodeId* local = nullptr;
  // cppcheck-suppress postfixOperator                  REASON: List iterator cannot be prefixed
  for(nodePairList::iterator j = pairList.begin(); j != pairList.end(); j++)
    if(UA_NodeId_equal(&((*j)->sourceNodeId), &remoteId) == UA_TRUE) {
      local = &((*j)->targetNodeId);
      break;
    }
  return local;
}

UA_StatusCode ua_mapInstantiatedNodes(UA_NodeId objectId, UA_NodeId definitionId, void* handle) {
  nodePairList* lst = static_cast<nodePairList*>(handle);

  UA_NodeId_pair* thisNode = new UA_NodeId_pair;
  UA_NodeId_copy(&definitionId, &thisNode->sourceNodeId);
  UA_NodeId_copy(&objectId, &thisNode->targetNodeId);
  lst->push_back(thisNode);

  return UA_STATUSCODE_GOOD;
}

UA_StatusCode ua_callProxy_mapDataSources(
    UA_Server* server, nodePairList instantiatedNodesList, UA_DataSource_Map* map, void* srcClass) {
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  if(map == nullptr || server == nullptr) return retval;

  // Functions are not instantiated... they are just linked to the node.
  for(nodePairList::iterator l = instantiatedNodesList.begin(); l != instantiatedNodesList.end(); ++l) {
    UA_NodeId typeTemplateId = (*l)->sourceNodeId;
    UA_NodeId instantiatedId = (*l)->targetNodeId;

    // Check if we have this node in our map
    UA_DataSource_Map_Element* ele = nullptr;
    // cppcheck-suppress postfixOperator                REASON: List iterator cannot be prefixed
    for(UA_DataSource_Map::iterator j = map->begin(); j != map->end(); j++) {
      UA_DataSource_Map_Element thisEle = *(j);
      if(UA_NodeId_equal(&typeTemplateId, (const UA_NodeId*)&thisEle.typeTemplateId) == UA_TRUE) {
        ele = new UA_DataSource_Map_Element;
        *ele = thisEle;
        break;
      }
    }
    if(ele == nullptr) continue;

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
      accessLevel = UA_ACCESSLEVELMASK_WRITE ^ UA_ACCESSLEVELMASK_READ;
    }
    if(ele->write == NULL && ele->read != NULL) {
      accessLevel = UA_ACCESSLEVELMASK_READ;
    }

    retval = UA_Server_writeAccessLevel(server, instantiatedId, accessLevel);
    if(retval != UA_STATUSCODE_GOOD){
      return retval;
    }
    // There is currently no high- level function to do this. (02.12.2016)
    // -> Update to Stack 1.X https://open62541.org/doc/current/server.html?highlight=useraccesslevel
    // The following attributes cannot be written from the server, as they are specific to the different users and set by the access control callback://
    //    UserWriteMask
    //    UserAccessLevel
    //    UserExecutable
    //__UA_Server_write(server, &instantiatedId, UA_ATTRIBUTEID_USERACCESSLEVEL, &UA_TYPES[UA_TYPES_BYTE], &accessLevel);

    retval = UA_Server_setVariableNode_dataSource(server, instantiatedId, ds);
    if(retval != UA_STATUSCODE_GOOD){
      return retval;
    }

    retval = UA_Server_writeDescription(server, instantiatedId, ele->description);
    if(retval != UA_STATUSCODE_GOOD){
      return retval;
    }
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
      if(retval != UA_STATUSCODE_GOOD){
        return retval;
      }
      if(UA_NodeId_equal(&datatTypeNodeId, &basedatatype) && retval == UA_STATUSCODE_GOOD) {
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
        retval = UA_Server_writeValueRank(server, instantiatedId, valueRank);
        if(retval != UA_STATUSCODE_GOOD){
          return retval;
        }
      }
      retval = UA_Server_writeDataType(server, instantiatedId, variantVal.type->typeId);

      UA_Variant_clear(&variantVal);
    }
  }

  return retval;
}
