// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "history_backend/InfluxHistoryBackend.h"

#include <open62541/plugin/log_stdout.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace influxdb {
  struct InfluxHistoryBackendContext {
    InfluxClient* client{nullptr};
    std::string influxFieldName;
    std::string nodeIdTagName;
    std::string engineeringUnit;
    std::string host;
    std::string applicationName;
    uint16_t port{0};
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

  int64_t uaDateTimeToUnixNanoseconds(UA_DateTime value) {
    if(value <= UA_DATETIME_UNIX_EPOCH) {
      return 0;
    }

    const UA_DateTime delta100ns = value - UA_DATETIME_UNIX_EPOCH;
    const UA_DateTime maxSafeDelta100ns = static_cast<UA_DateTime>(std::numeric_limits<int64_t>::max() / 100);
    if(delta100ns >= maxSafeDelta100ns) {
      return std::numeric_limits<int64_t>::max();
    }

    return static_cast<int64_t>(delta100ns * 100);
  }

  UA_DateTime unixNanosecondsToUaDateTime(int64_t value) {
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

    std::string result;
    result.assign(encoded.data, encoded.data + encoded.length);
    UA_String_clear(&encoded);
    return result;
  }

  FieldValue uaValueToString(const void* value, const UA_DataType* type) {
    if(value == nullptr || type == nullptr) {
      return FieldValue({"", true});
    }

    if(type->typeKind == UA_DATATYPEKIND_STRING) {
      const auto* stringValue = static_cast<const UA_String*>(value);
      if(stringValue->data == nullptr || stringValue->length == 0) {
        return FieldValue({"", true});
      }
      std::string result;
      result.assign(stringValue->data, stringValue->data + stringValue->length);
      return FieldValue({result, true});
    }

    if(type->typeKind == UA_DATATYPEKIND_BOOLEAN) {
      return FieldValue({*static_cast<const UA_Boolean*>(value) ? "true" : "false"});
    }

    if(type->typeKind == UA_DATATYPEKIND_SBYTE) {
      return FieldValue({std::to_string(*static_cast<const UA_SByte*>(value))});
    }
    if(type->typeKind == UA_DATATYPEKIND_BYTE) {
      return FieldValue({std::to_string(*static_cast<const UA_Byte*>(value))});
    }
    if(type->typeKind == UA_DATATYPEKIND_INT16) {
      return FieldValue({std::to_string(*static_cast<const UA_Int16*>(value))});
    }
    if(type->typeKind == UA_DATATYPEKIND_UINT16) {
      return FieldValue({std::to_string(*static_cast<const UA_UInt16*>(value))});
    }
    if(type->typeKind == UA_DATATYPEKIND_INT32) {
      return FieldValue({std::to_string(*static_cast<const UA_Int32*>(value))});
    }
    if(type->typeKind == UA_DATATYPEKIND_UINT32) {
      return FieldValue({std::to_string(*static_cast<const UA_UInt32*>(value))});
    }
    if(type->typeKind == UA_DATATYPEKIND_INT64) {
      return FieldValue({std::to_string(*static_cast<const UA_Int64*>(value))});
    }
    if(type->typeKind == UA_DATATYPEKIND_UINT64) {
      return FieldValue({std::to_string(*static_cast<const UA_UInt64*>(value))});
    }
    if(type->typeKind == UA_DATATYPEKIND_FLOAT) {
      std::ostringstream out;
      out << std::setprecision(std::numeric_limits<double>::max_digits10)
          << static_cast<double>(*static_cast<const UA_Float*>(value));
      return FieldValue({out.str()});
    }
    if(type->typeKind == UA_DATATYPEKIND_DOUBLE) {
      std::ostringstream out;
      out << std::setprecision(std::numeric_limits<double>::max_digits10) << *static_cast<const UA_Double*>(value);
      return FieldValue({out.str()});
    }
    if(type->typeKind == UA_DATATYPEKIND_DATETIME) {
      return FieldValue({std::to_string(static_cast<std::int64_t>(*static_cast<const UA_DateTime*>(value)))});
    }

    UA_String encoded = UA_STRING_NULL;
    if(UA_print(value, type, &encoded) != UA_STATUSCODE_GOOD || encoded.data == nullptr) {
      return FieldValue({"", true});
    }

    std::string result;
    result.assign(encoded.data, encoded.data + encoded.length);
    UA_String_clear(&encoded);
    return FieldValue({result, true});
  }

  bool variantToStrings(const UA_Variant* variant, std::vector<FieldValue>& outValues) {
    if(variant == nullptr || variant->type == nullptr) {
      return false;
    }

    outValues.clear();

    if(UA_Variant_isScalar(variant)) {
      outValues.emplace_back(uaValueToString(variant->data, variant->type));
      return true;
    }

    if(variant->arrayLength == 0 || variant->data == nullptr) {
      return false;
    }

    outValues.reserve(variant->arrayLength);
    for(size_t i = 0; i < variant->arrayLength; ++i) {
      const auto* element = static_cast<const UA_Byte*>(variant->data) + (i * variant->type->memSize);
      outValues.emplace_back(uaValueToString(element, variant->type));
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

  UA_StatusCode serverSetHistoryDataInflux(UA_Server* server, void* hdbContext, const UA_NodeId* /*sessionId*/,
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

    const int64_t timestampNanoseconds = uaDateTimeToUnixNanoseconds(ts);

    std::vector<TagInformation> tags;
    tags.emplace_back(ctx->nodeIdTagName, nodeIdToString(nodeId));
    tags.emplace_back("host", ctx->host);
    tags.emplace_back("application", ctx->applicationName);
    tags.emplace_back("port", std::to_string(ctx->port));
    if(!ctx->engineeringUnit.empty()) {
      tags.emplace_back("unit", ctx->engineeringUnit);
    }
    if(ctx->client->addExtraTags()) {
      tags.emplace_back("name", ctx->influxFieldName);
    }

    std::vector<FieldValue> fieldValues;
    if(!variantToStrings(&value->value, fieldValues)) {
      auto* config = UA_Server_getConfig(server);
      UA_LOG_WARNING(config->logging, UA_LOGCATEGORY_SERVER,
          "Influx history write failed: Unsupported data type for node %s", nodeIdToString(nodeId).c_str());
      return UA_STATUSCODE_BADTYPEMISMATCH;
    }

    if(fieldValues.size() == 1) {
      std::string writeError;
      const bool ok =
          ctx->client->writePoint(ctx->influxFieldName, fieldValues.front(), tags, timestampNanoseconds, &writeError);
      if(!ok) {
        auto* config = UA_Server_getConfig(server);
        UA_LOG_WARNING(config->logging, UA_LOGCATEGORY_SERVER, "Influx history write failed: %s", writeError.c_str());
        return UA_STATUSCODE_BADINTERNALERROR;
      }
    }
    else {
      tags.emplace_back("index");
      for(size_t i = 0; i < fieldValues.size(); ++i) {
        std::string writeError;
        tags.back().value = std::to_string(i);
        const bool ok =
            ctx->client->writePoint(ctx->influxFieldName, fieldValues[i], tags, timestampNanoseconds, &writeError);
        if(!ok) {
          auto* config = UA_Server_getConfig(server);
          UA_LOG_WARNING(config->logging, UA_LOGCATEGORY_SERVER, "Influx history write failed: %s", writeError.c_str());
          return UA_STATUSCODE_BADINTERNALERROR;
        }
      }
    }

    return UA_STATUSCODE_GOOD;
  }
  /**
   * Parses a string into a numeric value.
   *
   * Parsing will succeed if the string represents a valid number and contains no other non-whitespace characters.
   * @param value The string to parse.
   * @param outValue The parsed numeric value.
   * @return True if the parsing was successful, false otherwise.
   */
  bool parseNumberValue(const std::string& value, double& outValue) {
    const std::string trimmed = trim(value);
    if(trimmed.empty()) {
      return false;
    }

    char* endPtr = nullptr;
    // try to convert the trimmed string to a double using std::strtod
    // E.g. the string "42.5 val" will be converted to 42.5 and endPtr will point to the space before " val"
    outValue = std::strtod(trimmed.c_str(), &endPtr);
    if(endPtr == trimmed.c_str()) {
      // no conversion was performed, the string is not a valid number
      return false;
    }

    // check if there are any non-whitespace characters after the number
    while(endPtr != nullptr && *endPtr != '\0' && std::isspace(static_cast<unsigned char>(*endPtr))) {
      ++endPtr;
    }
    // if endPtr points to the null terminator, the entire string was a valid number
    return endPtr != nullptr && *endPtr == '\0';
  }

  UA_StatusCode getHistoryDataInflux(UA_Server* server, const UA_NodeId* /*sessionId*/, void* /*sessionContext*/,
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
    std::vector<TagInformation> tags;
    tags.emplace_back(ctx->nodeIdTagName, nodeIdToString(nodeId));
    tags.emplace_back("host", ctx->host);
    tags.emplace_back("application", ctx->applicationName);
    tags.emplace_back("port", std::to_string(ctx->port));

    const bool reverse = (end != LLONG_MIN && start != LLONG_MIN && end < start);

    int64_t startNanoseconds = 0;
    int64_t endNanoseconds = uaDateTimeToUnixNanoseconds(UA_DateTime_now());

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
        ctx->client->readRangeUnixNanoseconds(startNanoseconds, endNanoseconds, ctx->influxFieldName, tags, &readError);
    if(!readError.empty()) {
      auto* config = UA_Server_getConfig(server);
      UA_LOG_WARNING(config->logging, UA_LOGCATEGORY_SERVER, "Influx history read failed: %s", readError.c_str());
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
      result->dataValues[i].hasValue = true;
      double numericValue = 0.0;
      if(parseNumberValue(record.value, numericValue)) {
        UA_Variant_setScalarCopy(&result->dataValues[i].value, &numericValue, &UA_TYPES[UA_TYPES_DOUBLE]);
      }
      else {
        UA_String stringValue = UA_String_fromChars(record.value.c_str());
        UA_Variant_setScalarCopy(&result->dataValues[i].value, &stringValue, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_clear(&stringValue);
      }

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

  UA_HistoryDataBackend UA_HistoryDataBackend_Influx(InfluxClient* client, const std::string& influxFieldName,
      const std::string& nodeIdTagName, const std::string& engineeringUnit, const std::string& hostname,
      const std::string& applicationName, uint16_t port) {
    UA_HistoryDataBackend backend;
    std::memset(&backend, 0, sizeof(UA_HistoryDataBackend));

    auto* ctx = new InfluxHistoryBackendContext();
    ctx->client = client;
    ctx->influxFieldName = influxFieldName;
    ctx->nodeIdTagName = nodeIdTagName;
    ctx->host = hostname;
    ctx->applicationName = applicationName;
    ctx->port = port;
    ctx->engineeringUnit = engineeringUnit;
    backend.context = ctx;
    backend.deleteMembers = deleteMembersInflux;
    backend.serverSetHistoryData = serverSetHistoryDataInflux;
    backend.getHistoryData = getHistoryDataInflux;
    backend.boundSupported = boundSupportedInflux;
    backend.timestampsToReturnSupported = timestampsToReturnSupportedInflux;

    return backend;
  }
} // namespace influxdb