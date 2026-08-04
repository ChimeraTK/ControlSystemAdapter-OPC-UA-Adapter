// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <open62541/server.h>

#include <string>

class InfluxClient;
namespace HealthMonitoring {

  struct InfluxHealthContext {
    InfluxClient* client = nullptr;
    UA_NodeId queuedPointsNodeId{};
    UA_NodeId queuedPointsDroppedNodeId{};
    UA_NodeId pointsWrittenNodeId{};
    UA_NodeId pointsDroppedNodeId{};
    UA_NodeId batchesWrittenNodeId{};
    UA_NodeId batchFailuresNodeId{};
    UA_NodeId retryAttemptsNodeId{};
    UA_NodeId asyncErrorNodeId{};
    UA_NodeId asyncErrorActiveNodeId{};
  };

  void writeUInt64Node(UA_Server* server, const UA_NodeId& nodeId, UA_UInt64 value);

  void writeBooleanNode(UA_Server* server, const UA_NodeId& nodeId, UA_Boolean value);

  void writeStringNode(UA_Server* server, const UA_NodeId& nodeId, const std::string& value);

  void updateInfluxHealth(UA_Server* server, void* data);

  bool addReadOnlyNodeUInt64(UA_Server* server, const UA_NodeId& parentNodeId, const char* nodeIdText,
      const char* browseName, const char* description, UA_NodeId* outNodeId);

  bool addReadOnlyNodeBoolean(UA_Server* server, const UA_NodeId& parentNodeId, const char* nodeIdText,
      const char* browseName, const char* description, UA_NodeId* outNodeId);

  bool addReadOnlyNodeString(UA_Server* server, const UA_NodeId& parentNodeId, const char* nodeIdText,
      const char* browseName, const char* description, UA_NodeId* outNodeId);

} // namespace HealthMonitoring