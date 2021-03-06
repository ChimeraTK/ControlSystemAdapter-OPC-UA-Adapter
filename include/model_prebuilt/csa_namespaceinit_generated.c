/* WARNING: This is a generated file.
 * Any manual changes will be overwritten.

 */
#include "csa_namespaceinit_generated.h"
UA_INLINE UA_StatusCode csa_namespaceinit_generated(UA_Server *server) {
UA_StatusCode retval = UA_STATUSCODE_GOOD; 
if(retval == UA_STATUSCODE_GOOD){retval = UA_STATUSCODE_GOOD;} //ensure that retval is used
if (UA_Server_addNamespace(server, "http://adapterIM/") != 2)
    return UA_STATUSCODE_BADUNEXPECTEDERROR;

do {
// Referencing node found and declared as parent: i=58/BaseObjectType using i=45/HasSubtype
// Node: opcua_node_objectType_t(ns=2;i=1005), 1:ctkAdditionalVariable
UA_VariableTypeAttributes attr;
UA_VariableTypeAttributes_init(&attr);
attr.displayName = UA_LOCALIZEDTEXT("", "ctkAdditionalVariable");
attr.description = UA_LOCALIZEDTEXT("", "");
attr.valueRank = -2;
UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 1005);
UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, 62);
UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE);
UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE);
UA_QualifiedName nodeName = UA_QUALIFIEDNAME(1, "ctkAdditionalVariable");
UA_Server_addVariableTypeNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName, typeDefinition, attr, NULL, NULL);
//UA_Server_deleteReference(server, nodeId, UA_NODEID_NUMERIC(0, 40), true, UA_EXPANDEDNODEID_NUMERIC(0, 62), true); //remove HasTypeDefinition refs generated by addVariableNode
//UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 1001), UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_EXPANDEDNODEID_NUMERIC(0, 63), true);
} while(0);

/*
do {
// Referencing node found and declared as parent: i=58/BaseObjectType using i=45/HasSubtype
// Node: opcua_node_objectType_t(ns=2;i=1005), 1:ctkAdditionalVariable
UA_ObjectTypeAttributes attr;
UA_ObjectTypeAttributes_init(&attr);
attr.displayName = UA_LOCALIZEDTEXT("", "ctkAdditionalVariable");
attr.description = UA_LOCALIZEDTEXT("", "");
UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 1005);
UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, 58);
UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, 45);
UA_QualifiedName nodeName = UA_QUALIFIEDNAME(1, "ctkAdditionalVariable");
UA_Server_addObjectTypeNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName
       , attr, NULL, NULL);
} while(0);
*/

do {
// Referencing node found and declared as parent: i=58/BaseObjectType using i=45/HasSubtype
// Node: opcua_node_objectType_t(ns=2;i=1002), 1:ctkModule
UA_ObjectTypeAttributes attr;
UA_ObjectTypeAttributes_init(&attr);
attr.displayName = UA_LOCALIZEDTEXT("", "ctkModule");
attr.description = UA_LOCALIZEDTEXT("", "");
UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 1002);
UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, 58);
UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, 45);
UA_QualifiedName nodeName = UA_QUALIFIEDNAME(1, "ctkModule");
UA_Server_addObjectTypeNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName
       , attr, NULL, NULL);
} while(0);

do {
// Referencing node found and declared as parent: i=62/BaseVariableType using i=45/HasSubtype
// Node: opcua_node_variableType_t(ns=2;i=1001), 1:ctkProcessVariable
UA_VariableTypeAttributes attr;
UA_VariableTypeAttributes_init(&attr);
attr.displayName = UA_LOCALIZEDTEXT("", "ctkProcessVariable");
attr.description = UA_LOCALIZEDTEXT("", "");
attr.valueRank = -2;
UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 1001);
UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, 62);
UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE);
UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE);
UA_QualifiedName nodeName = UA_QUALIFIEDNAME(1, "ctkProcessVariable");
UA_Server_addVariableTypeNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName, typeDefinition, attr, NULL, NULL);
//UA_Server_addObjectTypeNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName, attr, NULL, NULL);
UA_Server_deleteReference(server, nodeId, UA_NODEID_NUMERIC(0, 40), true, UA_EXPANDEDNODEID_NUMERIC(0, 62), true); //remove HasTypeDefinition refs generated by addVariableNode
UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 1001), UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_EXPANDEDNODEID_NUMERIC(0, 63), true);
} while(0);

/*
do {
// Referencing node found and declared as parent: ns=2;i=1005/1:ctkAdditionalVariable using i=47/HasComponent
// Node: opcua_node_variable_t(ns=2;i=6002), 1:Value
UA_VariableAttributes attr;
UA_VariableAttributes_init(&attr);
attr.displayName = UA_LOCALIZEDTEXT("", "Value");
attr.description = UA_LOCALIZEDTEXT("", "");
attr.accessLevel = 3;
attr.userAccessLevel = 3;
attr.valueRank = -1;
UA_String *opcua_node_variable_t_ns_2_i_6002_variant_DataContents =  UA_String_new();
*opcua_node_variable_t_ns_2_i_6002_variant_DataContents = UA_STRING_ALLOC("");
UA_Variant_setScalar( &attr.value, opcua_node_variable_t_ns_2_i_6002_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 6002);
UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, 63);
UA_NodeId parentNodeId = UA_NODEID_NUMERIC(2, 1005);
UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, 47);
UA_QualifiedName nodeName = UA_QUALIFIEDNAME(1, "Value");
UA_Server_addVariableNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName
       , typeDefinition
       , attr, NULL, NULL);
UA_Server_deleteReference(server, nodeId, UA_NODEID_NUMERIC(0, 40), true, UA_EXPANDEDNODEID_NUMERIC(0, 62), true); //remove HasTypeDefinition refs generated by addVariableNode
// This node has the following references that can be created:
UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 6002), UA_NODEID_NUMERIC(0, 40), UA_EXPANDEDNODEID_NUMERIC(0, 63), true);
UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 6002), UA_NODEID_NUMERIC(0, 37), UA_EXPANDEDNODEID_NUMERIC(0, 78), true);
} while(0);
*/

do {
// Referencing node found and declared as parent: ns=2;i=1002/1:ctkModule using i=47/HasComponent
// Node: opcua_node_object_t(ns=2;i=5001), 1:Variables
UA_ObjectAttributes attr;
UA_ObjectAttributes_init(&attr);
attr.displayName = UA_LOCALIZEDTEXT("", "Variables");
attr.description = UA_LOCALIZEDTEXT("", "");
UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 5001);
UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, 61);
UA_NodeId parentNodeId = UA_NODEID_NUMERIC(2, 1002);
UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, 47);
UA_QualifiedName nodeName = UA_QUALIFIEDNAME(1, "Variables");
UA_Server_addObjectNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName
       , typeDefinition
       , attr, NULL, NULL);
UA_Server_deleteReference(server, nodeId, UA_NODEID_NUMERIC(0, 40), true, UA_EXPANDEDNODEID_NUMERIC(0, 58), true); //remove HasTypeDefinition refs generated by addObjectNode
// This node has the following references that can be created:
UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 5001), UA_NODEID_NUMERIC(0, 40), UA_EXPANDEDNODEID_NUMERIC(0, 61), true);
} while(0);

do {
// Referencing node found and declared as parent: ns=2;i=1001/1:ctkProcessVariable using i=47/HasComponent
// Node: opcua_node_variable_t(ns=2;i=6001), 1:Description
UA_VariableAttributes attr;
UA_VariableAttributes_init(&attr);
attr.displayName = UA_LOCALIZEDTEXT("", "Description");
attr.description = UA_LOCALIZEDTEXT("", "");
attr.accessLevel = 3;
attr.userAccessLevel = 3;
attr.valueRank = -1;
//UA_String *opcua_node_variable_t_ns_2_i_6001_variant_DataContents =  UA_String_new();
//*opcua_node_variable_t_ns_2_i_6001_variant_DataContents = UA_STRING_ALLOC("");
UA_String opcua_node_variable_t_ns_2_i_6001_variant_DataContents = UA_STRING_ALLOC("");
UA_Variant_setScalar( &attr.value, &opcua_node_variable_t_ns_2_i_6001_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 6001);
UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, 63);
UA_NodeId parentNodeId = UA_NODEID_NUMERIC(2, 1001);
UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, 47);
UA_QualifiedName nodeName = UA_QUALIFIEDNAME(1, "Description");
UA_Server_addVariableNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName
       , typeDefinition
       , attr, NULL, NULL);
UA_Server_deleteReference(server, nodeId, UA_NODEID_NUMERIC(0, 40), true, UA_EXPANDEDNODEID_NUMERIC(0, 62), true); //remove HasTypeDefinition refs generated by addVariableNode
// This node has the following references that can be created:
UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 6001), UA_NODEID_NUMERIC(0, 40), UA_EXPANDEDNODEID_NUMERIC(0, 63), true);
} while(0);

do {
// Referencing node found and declared as parent: ns=2;i=1001/1:ctkProcessVariable using i=47/HasComponent
// Node: opcua_node_variable_t(ns=2;i=6006), 1:EngineeringUnit
UA_VariableAttributes attr;
UA_VariableAttributes_init(&attr);
attr.displayName = UA_LOCALIZEDTEXT("", "EngineeringUnit");
attr.description = UA_LOCALIZEDTEXT("", "");
attr.accessLevel = 3;
attr.userAccessLevel = 3;
attr.valueRank = -1;
UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 6006);
UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, 63);
UA_NodeId parentNodeId = UA_NODEID_NUMERIC(2, 1001);
UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, 47);
UA_QualifiedName nodeName = UA_QUALIFIEDNAME(1, "EngineeringUnit");
UA_Server_addVariableNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName
       , typeDefinition
       , attr, NULL, NULL);
UA_Server_deleteReference(server, nodeId, UA_NODEID_NUMERIC(0, 40), true, UA_EXPANDEDNODEID_NUMERIC(0, 62), true); //remove HasTypeDefinition refs generated by addVariableNode
// This node has the following references that can be created:
UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 6006), UA_NODEID_NUMERIC(0, 40), UA_EXPANDEDNODEID_NUMERIC(0, 63), true);
} while(0);

do {
// Referencing node found and declared as parent: ns=2;i=1001/1:ctkProcessVariable using i=47/HasComponent
// Node: opcua_node_variable_t(ns=2;i=6004), 1:Name
UA_VariableAttributes attr;
UA_VariableAttributes_init(&attr);
attr.displayName = UA_LOCALIZEDTEXT("", "Name");
attr.description = UA_LOCALIZEDTEXT("", "");
attr.accessLevel = 3;
attr.userAccessLevel = 3;
attr.valueRank = -1;
//UA_String *opcua_node_variable_t_ns_2_i_6004_variant_DataContents =  UA_String_new();
//*opcua_node_variable_t_ns_2_i_6004_variant_DataContents = UA_STRING_ALLOC("");
UA_String opcua_node_variable_t_ns_2_i_6004_variant_DataContents = UA_STRING_ALLOC("");
UA_Variant_setScalar( &attr.value, &opcua_node_variable_t_ns_2_i_6004_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 6004);
UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, 63);
UA_NodeId parentNodeId = UA_NODEID_NUMERIC(2, 1001);
UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, 47);
UA_QualifiedName nodeName = UA_QUALIFIEDNAME(1, "Name");
UA_Server_addVariableNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName
       , typeDefinition
       , attr, NULL, NULL);
UA_Server_deleteReference(server, nodeId, UA_NODEID_NUMERIC(0, 40), true, UA_EXPANDEDNODEID_NUMERIC(0, 62), true); //remove HasTypeDefinition refs generated by addVariableNode
// This node has the following references that can be created:
UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 6004), UA_NODEID_NUMERIC(0, 40), UA_EXPANDEDNODEID_NUMERIC(0, 63), true);
} while(0);

do {
// Referencing node found and declared as parent: ns=2;i=1001/1:ctkProcessVariable using i=47/HasComponent
// Node: opcua_node_variable_t(ns=2;i=6012), 1:Type
UA_VariableAttributes attr;
UA_VariableAttributes_init(&attr);
attr.displayName = UA_LOCALIZEDTEXT("", "Type");
attr.description = UA_LOCALIZEDTEXT("", "");
attr.accessLevel = 3;
attr.userAccessLevel = 3;
attr.valueRank = -1;
//UA_String *opcua_node_variable_t_ns_2_i_6012_variant_DataContents =  UA_String_new();
//*opcua_node_variable_t_ns_2_i_6012_variant_DataContents = UA_STRING_ALLOC("");
UA_String opcua_node_variable_t_ns_2_i_6012_variant_DataContents = UA_STRING_ALLOC("");
UA_Variant_setScalar( &attr.value, &opcua_node_variable_t_ns_2_i_6012_variant_DataContents, &UA_TYPES[UA_TYPES_STRING]);
UA_NodeId nodeId = UA_NODEID_NUMERIC(2, 6012);
UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, 63);
UA_NodeId parentNodeId = UA_NODEID_NUMERIC(2, 1001);
UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, 47);
UA_QualifiedName nodeName = UA_QUALIFIEDNAME(1, "Type");
UA_Server_addVariableNode(server, nodeId, parentNodeId, parentReferenceNodeId, nodeName
       , typeDefinition
       , attr, NULL, NULL);
UA_Server_deleteReference(server, nodeId, UA_NODEID_NUMERIC(0, 40), true, UA_EXPANDEDNODEID_NUMERIC(0, 62), true); //remove HasTypeDefinition refs generated by addVariableNode
// This node has the following references that can be created:
UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 6012), UA_NODEID_NUMERIC(0, 40), UA_EXPANDEDNODEID_NUMERIC(0, 63), true);
} while(0);

return UA_STATUSCODE_GOOD;
}

