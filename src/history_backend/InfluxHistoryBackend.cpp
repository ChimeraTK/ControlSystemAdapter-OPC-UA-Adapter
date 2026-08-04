// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "history_backend/InfluxHistoryBackend.h"

#include <open62541/plugin/log_stdout.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <map>
#include <string>
#include <vector>

namespace {
  struct InfluxHistoryBackendContext {
    InfluxClient* client;
    std::string fieldKey;
    std::string nodeIdTagName;
    std::string host;
    std::string applicationName;
    uint16_t port;
  };

  size_t parseContinuationPointOffset(const UA_ByteString* continuationPoint, UA_StatusCode* status) {
    if(status != nullptr) {
      *status = UA_STATUSCODE_GOOD;
    }

    if(continuationPoint == nullptr || continuationPoint->length == 0) {
      return 0;
    }

    if(continuationPoint->length != sizeof(size_t) || continuationPoint->data == nullptr) {
      if(status != nullptr) {
        *status = UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
      }
      return 0;
    }

    size_t offset = 0;
    std::memcpy(&offset, continuationPoint->data, sizeof(size_t));
    return offset;
  }

  UA_StatusCode writeContinuationPointOffset(size_t offset, UA_ByteString* outContinuationPoint) {
    if(outContinuationPoint == nullptr) {
      return UA_STATUSCODE_GOOD;
    }

    const UA_StatusCode rc = UA_ByteString_allocBuffer(outContinuationPoint, sizeof(size_t));
    if(rc != UA_STATUSCODE_GOOD) {
      return rc;
    }

    std::memcpy(outContinuationPoint->data, &offset, sizeof(size_t));
    return UA_STATUSCODE_GOOD;
  }

  long long uaDateTimeToUnixNanoseconds(UA_DateTime value) {
    if(value <= UA_DATETIME_UNIX_EPOCH) {
      return 0;
    }

    const UA_DateTime delta100ns = value - UA_DATETIME_UNIX_EPOCH;
    const UA_DateTime maxSafeDelta100ns = static_cast<UA_DateTime>(std::numeric_limits<long long>::max() / 100);
    if(delta100ns >= maxSafeDelta100ns) {
      return std::numeric_limits<long long>::max();
    }

    return static_cast<long long>(delta100ns * 100);
  }

  UA_DateTime unixNanosecondsToUaDateTime(long long value) {
    if(value <= 0) {
      return UA_DATETIME_UNIX_EPOCH;
    }
    return UA_DATETIME_UNIX_EPOCH + static_cast<UA_DateTime>(value / 100);
  }

  std::string nodeIdToString(const UA_NodeId* nodeId) {
    UA_String encoded = UA_STRING_NULL;
    UA_StatusCode rc = UA_NodeId_print(nodeId, &encoded);
    if(rc != UA_STATUSCODE_GOOD || encoded.length == 0 || encoded.data == nullptr) {
      return "unknown-node";
    }

    const std::string result(reinterpret_cast<const char*>(encoded.data), encoded.length);
    UA_String_clear(&encoded);
    return result;
  }

  bool variantToScalarDouble(const UA_Variant* variant, double* outValue) {
    if(variant == nullptr || outValue == nullptr || !UA_Variant_isScalar(variant)) {
      return false;
    }

    if(variant->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
      *outValue = *static_cast<const UA_Double*>(variant->data);
      return true;
    }
    if(variant->type == &UA_TYPES[UA_TYPES_FLOAT]) {
      *outValue = static_cast<double>(*static_cast<const UA_Float*>(variant->data));
      return true;
    }
    if(variant->type == &UA_TYPES[UA_TYPES_INT64]) {
      *outValue = static_cast<double>(*static_cast<const UA_Int64*>(variant->data));
      return true;
    }
    if(variant->type == &UA_TYPES[UA_TYPES_UINT64]) {
      *outValue = static_cast<double>(*static_cast<const UA_UInt64*>(variant->data));
      return true;
    }
    if(variant->type == &UA_TYPES[UA_TYPES_INT32]) {
      *outValue = static_cast<double>(*static_cast<const UA_Int32*>(variant->data));
      return true;
    }
    if(variant->type == &UA_TYPES[UA_TYPES_UINT32]) {
      *outValue = static_cast<double>(*static_cast<const UA_UInt32*>(variant->data));
      return true;
    }

    return false;
  }

  bool variantToArrayDouble(const UA_Variant* variant, std::vector<double>& outValue) {
    if(variant == nullptr || variant->arrayLength == 0) {
      return false;
    }
    outValue.resize(variant->arrayLength);
    for(size_t i = 0; i < variant->arrayLength; ++i) {
      if(variant->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
        const auto* tmp = static_cast<const UA_Double*>(variant->data);
        outValue[i] = tmp[i];
        continue;
      }
      if(variant->type == &UA_TYPES[UA_TYPES_FLOAT]) {
        const auto* tmp = static_cast<const UA_Float*>(variant->data);
        outValue[i] = static_cast<double>(tmp[i]);
        continue;
      }
      if(variant->type == &UA_TYPES[UA_TYPES_INT64]) {
        const auto* tmp = static_cast<const UA_Int64*>(variant->data);
        outValue[i] = static_cast<double>(tmp[i]);
        continue;
      }
      if(variant->type == &UA_TYPES[UA_TYPES_UINT64]) {
        const auto* tmp = static_cast<const UA_UInt64*>(variant->data);
        outValue[i] = static_cast<double>(tmp[i]);
        continue;
      }
      if(variant->type == &UA_TYPES[UA_TYPES_INT32]) {
        const auto* tmp = static_cast<const UA_Int32*>(variant->data);
        outValue[i] = static_cast<double>(tmp[i]);
        continue;
      }
      if(variant->type == &UA_TYPES[UA_TYPES_UINT32]) {
        const auto* tmp = static_cast<const UA_UInt32*>(variant->data);
        outValue[i] = static_cast<double>(tmp[i]);
        continue;
      }
      return false;
    }
    return true;
  }

  UA_Boolean boundSupportedInflux(UA_Server* /*server*/, void* /*hdbContext*/, const UA_NodeId* /*sessionId*/,
      void* /*sessionContext*/, const UA_NodeId* /*nodeId*/) {
    /* Accept returnBounds requests so clients like UAExpert can still read data.
     */
    return UA_TRUE;
  }

  UA_Boolean timestampsToReturnSupportedInflux(UA_Server* /*server*/, void* /*hdbContext*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/,
      const UA_TimestampsToReturn timestampsToReturn) {
    return timestampsToReturn == UA_TIMESTAMPSTORETURN_SOURCE || timestampsToReturn == UA_TIMESTAMPSTORETURN_SERVER ||
        timestampsToReturn == UA_TIMESTAMPSTORETURN_BOTH || timestampsToReturn == UA_TIMESTAMPSTORETURN_NEITHER;
  }

  void deleteMembersInflux(UA_HistoryDataBackend* backend) {
    if(backend == nullptr || backend->context == nullptr) {
      return;
    }

    delete static_cast<InfluxHistoryBackendContext*>(backend->context);
    backend->context = nullptr;
  }

  UA_StatusCode serverSetHistoryDataInflux(UA_Server* /*server*/, void* hdbContext, const UA_NodeId* /*sessionId*/,
      void* /*sessionContext*/, const UA_NodeId* nodeId, UA_Boolean historizing, const UA_DataValue* value) {
    if(!historizing || hdbContext == nullptr || nodeId == nullptr || value == nullptr || !value->hasValue) {
      return UA_STATUSCODE_GOOD;
    }

    auto* ctx = static_cast<InfluxHistoryBackendContext*>(hdbContext);

    UA_DateTime ts = UA_DateTime_now();
    if(value->hasSourceTimestamp) {
      ts = value->sourceTimestamp;
    }
    else if(value->hasServerTimestamp) {
      ts = value->serverTimestamp;
    }

    const long long timestampNanoseconds = uaDateTimeToUnixNanoseconds(ts);

    std::map<std::string, std::string> tags;
    tags[ctx->nodeIdTagName] = nodeIdToString(nodeId);
    tags["host"] = ctx->host;
    tags["application"] = ctx->applicationName;
    tags["port"] = std::to_string(ctx->port);

    double numericValue = 0.0;
    if(variantToScalarDouble(&value->value, &numericValue)) {
      std::string writeError;
      const bool ok = ctx->client->writePoint(ctx->fieldKey, numericValue, tags, timestampNanoseconds, &writeError);
      if(!ok) {
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Influx history write failed: %s", writeError.c_str());
        return UA_STATUSCODE_BADINTERNALERROR;
      }
    }
    else {
      std::vector<double> numericArray;
      if(variantToArrayDouble(&value->value, numericArray)) {
        for(size_t i = 0; i < numericArray.size(); ++i) {
          std::string writeError;
          tags["index"] = std::to_string(i);
          const bool ok =
              ctx->client->writePoint(ctx->fieldKey, numericArray[i], tags, timestampNanoseconds, &writeError);
          if(!ok) {
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Influx history write failed: %s", writeError.c_str());
            return UA_STATUSCODE_BADINTERNALERROR;
          }
        }
      }
      else {
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
            "Influx history write failed: Unsupported data type for node %s", nodeIdToString(nodeId).c_str());
        return UA_STATUSCODE_BADTYPEMISMATCH;
      }
    }

    return UA_STATUSCODE_GOOD;
  }

  UA_StatusCode getHistoryDataInflux(UA_Server* /*server*/, const UA_NodeId* /*sessionId*/, void* /*sessionContext*/,
      const UA_HistoryDataBackend* backend, const UA_DateTime start, const UA_DateTime end, const UA_NodeId* nodeId,
      size_t maxSizePerResponse, UA_UInt32 numValuesPerNode, UA_Boolean /*returnBounds*/,
      UA_TimestampsToReturn /*timestampsToReturn*/, UA_NumericRange /*range*/, UA_Boolean releaseContinuationPoints,
      const UA_ByteString* continuationPoint, UA_ByteString* outContinuationPoint, UA_HistoryData* result) {
    if(backend == nullptr || backend->context == nullptr || nodeId == nullptr || result == nullptr) {
      return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    if(outContinuationPoint != nullptr) {
      UA_ByteString_init(outContinuationPoint);
    }

    if(releaseContinuationPoints) {
      result->dataValues = nullptr;
      result->dataValuesSize = 0;
      return UA_STATUSCODE_GOOD;
    }

    UA_StatusCode continuationStatus = UA_STATUSCODE_GOOD;
    const size_t skip = parseContinuationPointOffset(continuationPoint, &continuationStatus);
    if(continuationStatus != UA_STATUSCODE_GOOD) {
      return continuationStatus;
    }

    auto* ctx = static_cast<InfluxHistoryBackendContext*>(backend->context);
    std::map<std::string, std::string> tags;
    tags[ctx->nodeIdTagName] = nodeIdToString(nodeId);
    tags["host"] = ctx->host;
    tags["application"] = ctx->applicationName;
    tags["port"] = std::to_string(ctx->port);

    const bool reverse = (end != LLONG_MIN && start != LLONG_MIN && end < start);

    long long startNanoseconds = 0;
    long long endNanoseconds = uaDateTimeToUnixNanoseconds(UA_DateTime_now());

    if(start == LLONG_MIN && end == LLONG_MIN) {
      startNanoseconds = 0;
    }
    else if(start == LLONG_MIN) {
      startNanoseconds = 0;
      endNanoseconds = uaDateTimeToUnixNanoseconds(end);
    }
    else if(end == LLONG_MIN) {
      startNanoseconds = uaDateTimeToUnixNanoseconds(start);
    }
    else {
      startNanoseconds = uaDateTimeToUnixNanoseconds(reverse ? end : start);
      endNanoseconds = uaDateTimeToUnixNanoseconds(reverse ? start : end);
    }

    if(endNanoseconds <= startNanoseconds) {
      endNanoseconds = std::max(startNanoseconds + 1, uaDateTimeToUnixNanoseconds(UA_DateTime_now()));
    }

    std::string readError;
    std::vector<InfluxRecord> records =
        ctx->client->readRangeUnixNanoseconds(startNanoseconds, endNanoseconds, ctx->fieldKey, tags, &readError);
    if(!readError.empty()) {
      UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Influx history read failed: %s", readError.c_str());
      return UA_STATUSCODE_BADINTERNALERROR;
    }

    if(reverse) {
      std::reverse(records.begin(), records.end());
    }

    if(skip >= records.size()) {
      result->dataValues = nullptr;
      result->dataValuesSize = 0;
      return UA_STATUSCODE_GOOD;
    }

    size_t limit = records.size() - skip;
    if(numValuesPerNode > 0) {
      limit = std::min(limit, static_cast<size_t>(numValuesPerNode));
    }
    if(maxSizePerResponse > 0) {
      limit = std::min(limit, maxSizePerResponse);
    }

    result->dataValuesSize = limit;
    if(limit == 0) {
      result->dataValues = nullptr;
      return UA_STATUSCODE_GOOD;
    }

    result->dataValues = static_cast<UA_DataValue*>(UA_Array_new(limit, &UA_TYPES[UA_TYPES_DATAVALUE]));
    if(result->dataValues == nullptr) {
      result->dataValuesSize = 0;
      return UA_STATUSCODE_BADOUTOFMEMORY;
    }

    for(size_t i = 0; i < limit; ++i) {
      UA_DataValue_init(&result->dataValues[i]);

      const InfluxRecord& record = records[skip + i];
      const double value = std::strtod(record.value.c_str(), nullptr);

      result->dataValues[i].hasValue = true;
      UA_Variant_setScalarCopy(&result->dataValues[i].value, &value, &UA_TYPES[UA_TYPES_DOUBLE]);

      const UA_DateTime sourceTime = unixNanosecondsToUaDateTime(record.timestampNanoseconds);
      result->dataValues[i].hasSourceTimestamp = true;
      result->dataValues[i].sourceTimestamp = sourceTime;
      result->dataValues[i].hasServerTimestamp = true;
      result->dataValues[i].serverTimestamp = sourceTime;
    }

    const size_t nextOffset = skip + limit;
    if(nextOffset < records.size()) {
      const UA_StatusCode rc = writeContinuationPointOffset(nextOffset, outContinuationPoint);
      if(rc != UA_STATUSCODE_GOOD) {
        UA_Array_delete(result->dataValues, result->dataValuesSize, &UA_TYPES[UA_TYPES_DATAVALUE]);
        result->dataValues = nullptr;
        result->dataValuesSize = 0;
        return rc;
      }
    }

    return UA_STATUSCODE_GOOD;
  }
} // namespace

UA_HistoryDataBackend UA_HistoryDataBackend_Influx(InfluxClient* client, const std::string& fieldKey,
    const std::string& nodeIdTagName, const std::string& hostname, const std::string& applicationName, uint16_t port) {
  UA_HistoryDataBackend backend;
  std::memset(&backend, 0, sizeof(UA_HistoryDataBackend));

  auto* ctx = new InfluxHistoryBackendContext();
  ctx->client = client;
  ctx->fieldKey = fieldKey;
  ctx->nodeIdTagName = nodeIdTagName;
  ctx->host = hostname;
  ctx->applicationName = applicationName;
  ctx->port = port;
  backend.context = ctx;
  backend.deleteMembers = deleteMembersInflux;
  backend.serverSetHistoryData = serverSetHistoryDataInflux;
  backend.getHistoryData = getHistoryDataInflux;
  backend.boundSupported = boundSupportedInflux;
  backend.timestampsToReturnSupported = timestampsToReturnSupportedInflux;

  return backend;
}
