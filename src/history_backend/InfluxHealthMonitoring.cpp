// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "history_backend/InfluxHealthMonitoring.h"

#include "history_backend/InfluxClient.h"
namespace HealthMonitoring {

  void writeUInt64Node(UA_Server* server, const UA_NodeId& nodeId, UA_UInt64 value) {
    UA_Variant variant;
    UA_Variant_init(&variant);
    UA_Variant_setScalar(&variant, &value, &UA_TYPES[UA_TYPES_UINT64]);
    (void)UA_Server_writeValue(server, nodeId, variant);
  }

  void writeBooleanNode(UA_Server* server, const UA_NodeId& nodeId, UA_Boolean value) {
    UA_Variant variant;
    UA_Variant_init(&variant);
    UA_Variant_setScalar(&variant, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
    (void)UA_Server_writeValue(server, nodeId, variant);
  }

  void writeStringNode(UA_Server* server, const UA_NodeId& nodeId, const std::string& value) {
    UA_String uaText = UA_STRING_ALLOC(value.c_str());
    UA_Variant variant;
    UA_Variant_init(&variant);
    UA_Variant_setScalar(&variant, &uaText, &UA_TYPES[UA_TYPES_STRING]);
    (void)UA_Server_writeValue(server, nodeId, variant);
    UA_String_clear(&uaText);
  }

  void updateInfluxHealth(UA_Server* server, void* data) {
    InfluxHealthContext* ctx = static_cast<InfluxHealthContext*>(data);
    const InfluxWriteStats stats = ctx->client->getWriteStats();

    writeUInt64Node(server, ctx->queuedPointsNodeId, static_cast<UA_UInt64>(stats.queuedPoints));
    writeUInt64Node(server, ctx->queuedPointsDroppedNodeId, static_cast<UA_UInt64>(stats.queuedPointsDropped));
    writeUInt64Node(server, ctx->pointsWrittenNodeId, static_cast<UA_UInt64>(stats.pointsWritten));
    writeUInt64Node(server, ctx->pointsDroppedNodeId, static_cast<UA_UInt64>(stats.pointsDropped));
    writeUInt64Node(server, ctx->batchesWrittenNodeId, static_cast<UA_UInt64>(stats.batchesWritten));
    writeUInt64Node(server, ctx->batchFailuresNodeId, static_cast<UA_UInt64>(stats.batchWriteFailures));
    writeUInt64Node(server, ctx->retryAttemptsNodeId, static_cast<UA_UInt64>(stats.retryAttempts));

    const bool hasError = ctx->client->hasAsyncWriteError();
    writeBooleanNode(server, ctx->asyncErrorActiveNodeId, hasError ? UA_TRUE : UA_FALSE);
    writeStringNode(server, ctx->asyncErrorNodeId, hasError ? ctx->client->getLastAsyncWriteError() : "");
  }

  bool addReadOnlyNodeUInt64(UA_Server* server, const UA_NodeId& parentNodeId, const char* nodeIdText,
      const char* browseName, const char* description, UA_NodeId* outNodeId) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    const UA_UInt64 initialValue = 0;
    UA_Variant_setScalarCopy(&attr.value, &initialValue, &UA_TYPES[UA_TYPES_UINT64]);
    attr.description = UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>(description));
    attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>(browseName));
    attr.dataType = UA_TYPES[UA_TYPES_UINT64].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    attr.userAccessLevel = UA_ACCESSLEVELMASK_READ;

    const UA_NodeId requestedNodeId = UA_NODEID_STRING_ALLOC(1, const_cast<char*>(nodeIdText));
    const UA_StatusCode rc = UA_Server_addVariableNode(server, requestedNodeId, parentNodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(1, const_cast<char*>(browseName)),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, nullptr, outNodeId);
    UA_NodeId_clear(const_cast<UA_NodeId*>(&requestedNodeId));
    UA_Variant_clear(&attr.value);
    return rc == UA_STATUSCODE_GOOD;
  }

  bool addReadOnlyNodeBoolean(UA_Server* server, const UA_NodeId& parentNodeId, const char* nodeIdText,
      const char* browseName, const char* description, UA_NodeId* outNodeId) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    const UA_Boolean initialValue = UA_FALSE;
    UA_Variant_setScalarCopy(&attr.value, &initialValue, &UA_TYPES[UA_TYPES_BOOLEAN]);
    attr.description = UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>(description));
    attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>(browseName));
    attr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    attr.userAccessLevel = UA_ACCESSLEVELMASK_READ;

    const UA_NodeId requestedNodeId = UA_NODEID_STRING_ALLOC(1, const_cast<char*>(nodeIdText));
    const UA_StatusCode rc = UA_Server_addVariableNode(server, requestedNodeId, parentNodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(1, const_cast<char*>(browseName)),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, nullptr, outNodeId);
    UA_NodeId_clear(const_cast<UA_NodeId*>(&requestedNodeId));
    UA_Variant_clear(&attr.value);
    return rc == UA_STATUSCODE_GOOD;
  }

  bool addReadOnlyNodeString(UA_Server* server, const UA_NodeId& parentNodeId, const char* nodeIdText,
      const char* browseName, const char* description, UA_NodeId* outNodeId) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_String initialValue = UA_STRING_ALLOC("");
    UA_Variant_setScalarCopy(&attr.value, &initialValue, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&initialValue);
    attr.description = UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>(description));
    attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>(browseName));
    attr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    attr.userAccessLevel = UA_ACCESSLEVELMASK_READ;

    const UA_NodeId requestedNodeId = UA_NODEID_STRING_ALLOC(1, const_cast<char*>(nodeIdText));
    const UA_StatusCode rc = UA_Server_addVariableNode(server, requestedNodeId, parentNodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(1, const_cast<char*>(browseName)),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, nullptr, outNodeId);
    UA_NodeId_clear(const_cast<UA_NodeId*>(&requestedNodeId));
    UA_Variant_clear(&attr.value);
    return rc == UA_STATUSCODE_GOOD;
  }

} // namespace HealthMonitoring