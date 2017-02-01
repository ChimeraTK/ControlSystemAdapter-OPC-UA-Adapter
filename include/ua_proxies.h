/** @class ua_proxies
 * 	@brief Helper class to interact with open62541 in a easy way.
 * 	This class is a kind of a proxy to interact with the open62541 stack. For this, the class mapped all variables to the nodestore of the open62541.
 * 	
 *	@author Chris Iatrou
 *	@author Julian Rahm
 *  @date 22.11.2016
 * 
 */

#ifndef HAVE_UA_PROXIES_H
#define HAVE_UA_PROXIES_H

#include "open62541.h"
#include <list>
#include <iostream>

#include "ua_proxies_typeconversion.h"
#include "ua_proxies_callback.h"

using namespace std;

/** 
 * @struct UA_NodeId_pair_t
 * @brief This struct contains a NodeId generated from the open62541-stack and the NodeId of the model side
 * 
 */
typedef struct UA_NodeId_pair_t {
  UA_NodeId sourceNodeId;	// Model NodeId
  UA_NodeId targetNodeId;	// Stack NodeId
} UA_NodeId_pair;
typedef std::list<UA_NodeId_pair*> nodePairList; 

/** 
 * @struct UA_FunctionCall_InstanceLookupTable_Element_t
 * @brief Lookup Table: Multiple instances of the object may exist, a lookup for their respecive Id must be performed
 * 
 */
typedef struct UA_FunctionCall_InstanceLookupTable_Element_t {
    UA_Server         *server;            // Server instance the node lives in (might have multiple servers with the same object/methodId)
    void              *classInstance;     // Object instance Id
    UA_NodeId         classObjectId;      // Method Id
} UA_FunctionCall_InstanceLookupTable_Element;
typedef std::list<UA_FunctionCall_InstanceLookupTable_Element*> UA_FunctionCall_InstanceLookUpTable; 

/** 
 * @struct UA_DataSource_Map_Element_t
 * @brief For generic callback use, this sturct contains the methode pointer and a NodeId of the model. Additinaly a description from controlSystemAdapter can be setted here.
 * 
 */
typedef struct UA_DataSource_Map_Element_t {
  UA_NodeId     typeTemplateId;
	UA_LocalizedText description; // individuell description for every variable
  UA_StatusCode (*read)(void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp,const UA_NumericRange *range, UA_DataValue *value);
  UA_StatusCode (*write)(void *handle, const UA_NodeId nodeid, const UA_Variant *data, const UA_NumericRange *range);
} UA_DataSource_Map_Element;
typedef std::list<UA_DataSource_Map_Element> UA_DataSource_Map;


#define NODE_PAIR_PUSH(_p_listname, _p_srcId, _p_targetId) do {\
UA_NodeId_pair *tmp = new UA_NodeId_pair;\
UA_NodeId_copy(&_p_srcId, &tmp->sourceNodeId);\
UA_NodeId_copy(&_p_targetId, &tmp->targetNodeId);\
_p_listname.push_back(tmp); } while (0);

#define PUSH_OWNED_NODEID(_p_nodeid) do {\
UA_NodeId_pair * _p_nodeid##_tmp = new UA_NodeId_pair;\
UA_NodeId_copy(&UA_NODEID_NULL, & _p_nodeid##_tmp->sourceNodeId);\
UA_NodeId_copy(&_p_nodeid, & _p_nodeid##_tmp->targetNodeId);\
this->ownedNodes.push_back(_p_nodeid##_tmp); } while(0);

/**
 * @brief Searching for NodeId's in <pairList> with the same NodeId from <remoteId>
 * 
 * @param pairList
 * @param remoteId
 *
 * @return UA_NodeId from the found node
 * 
 */
UA_NodeId *nodePairList_getTargetIdBySourceId(nodePairList pairList, UA_NodeId remoteId);

/**
 * @brief Node function and proxy mapping for new nodes
 * 
 * @param objectId targetNodeId
 * @param defintionId sourceNodeId
 * @param handle read and write handel for the node
 *
 * @return UA_StatusCode 
 */
UA_StatusCode ua_mapInstantiatedNodes(UA_NodeId objectId, UA_NodeId definitionId, void *handle);

/**
 * @brief This methode map all variables in sort of a <UA_DataSourceMap> from the called class to the open62541
 * 
 * @param server	This param provides the OPC UA server
 * @param instantiatedNodesList Contains all instantiated nodes
 * @param map	Contains all Node from the class the should instantiated into the server 
 * @param scrClass Pointer to our class instance 
 * 
 * @return UA_StatusCode 
 */
UA_StatusCode ua_callProxy_mapDataSources(UA_Server* server, nodePairList instantiatedNodesList, UA_DataSource_Map *map, void *srcClass);

/* Instatiation NodeId gatherer Macro (because it's always the same...) */
#define UA_INSTATIATIONCALLBACK(_p_lstName) \
UA_InstantiationCallback _p_lstName; \
_p_lstName.handle = (void *) &this->ownedNodes; \
_p_lstName.method = ua_mapInstantiatedNodes;


#endif // Header include

