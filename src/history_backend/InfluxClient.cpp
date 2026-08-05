// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "history_backend/InfluxClient.h"

#include <curl/curl.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace {
  size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    const size_t realSize = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), realSize);
    return realSize;
  }

  std::string urlEncode(CURL* curl, const std::string& value) {
    char* encoded = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.size()));
    if(encoded == nullptr) {
      return "";
    }

    std::string result(encoded);
    curl_free(encoded);
    return result;
  }

  std::string escapeJson(const std::string& input) {
    std::ostringstream out;
    for(char c : input) {
      switch(c) {
        case '\\':
          out << "\\\\";
          break;
        case '"':
          out << "\\\"";
          break;
        case '\n':
          out << "\\n";
          break;
        case '\r':
          out << "\\r";
          break;
        case '\t':
          out << "\\t";
          break;
        default:
          out << c;
          break;
      }
    }
    return out.str();
  }

  std::string escapeLineProtocolIdentifier(const std::string& input) {
    std::string out;
    out.reserve(input.size() * 2);
    for(char c : input) {
      if(c == ',' || c == ' ' || c == '=') {
        out.push_back('\\');
      }
      out.push_back(c);
    }
    return out;
  }

  std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> cells;
    std::string current;
    bool inQuotes = false;

    for(size_t i = 0; i < line.size(); ++i) {
      const char c = line[i];
      if(c == '"') {
        if(inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
          current.push_back('"');
          ++i;
        }
        else {
          inQuotes = !inQuotes;
        }
      }
      else if(c == ',' && !inQuotes) {
        cells.push_back(current);
        current.clear();
      }
      else {
        current.push_back(c);
      }
    }
    cells.push_back(current);

    return cells;
  }

  std::string trim(std::string value) {
    value.erase(
        value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
        value.end());
    return value;
  }

  std::string stripTrailingSlash(const std::string& url) {
    if(!url.empty() && url.back() == '/') {
      return url.substr(0, url.size() - 1);
    }
    return url;
  }

  long long parseTimestampToNanoseconds(const std::string& timestamp) {
    std::tm tm{};
    std::istringstream timeStream(timestamp.substr(0, 19));
    timeStream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if(timeStream.fail()) {
      return 0;
    }

    long long fractionalNanoseconds = 0;
    const size_t dotPos = timestamp.find('.');
    const size_t zPos = timestamp.find('Z');
    if(dotPos != std::string::npos && zPos != std::string::npos && zPos > dotPos + 1) {
      std::string fraction = timestamp.substr(dotPos + 1, zPos - dotPos - 1);
      if(fraction.size() > 9) {
        fraction = fraction.substr(0, 9);
      }
      while(fraction.size() < 9) {
        fraction.push_back('0');
      }
      fractionalNanoseconds = std::stoll(fraction);
    }

#ifdef _WIN32
    const long long secondsSinceEpoch = static_cast<long long>(_mkgmtime(&tm));
#else
    const long long secondsSinceEpoch = static_cast<long long>(timegm(&tm));
#endif
    if(secondsSinceEpoch < 0) {
      return 0;
    }

    return secondsSinceEpoch * 1000000000LL + fractionalNanoseconds;
  }

  std::string buildFluxReadQuery(const InfluxConfig& config, const std::string& startExpression,
      const std::string& stopExpression, const std::string& fieldKey, const std::map<std::string, std::string>& tags) {
    std::ostringstream flux;
    flux << "from(bucket: \"" << escapeJson(config.bucket) << "\")"
         << " |> range(start: " << startExpression << ", stop: " << stopExpression << ")"
         << " |> filter(fn: (r) => r._measurement == \"" << escapeJson(config.measurement) << "\")";

    if(!fieldKey.empty()) {
      flux << " |> filter(fn: (r) => r._field == \"" << escapeJson(fieldKey) << "\")";
    }

    for(const auto& tag : tags) {
      flux << " |> filter(fn: (r) => r[\"" << escapeJson(tag.first) << "\"] == \"" << escapeJson(tag.second) << "\")";
    }

    return flux.str();
  }

  std::string formatRfc3339FromNanoseconds(long long epochNanoseconds) {
    if(epochNanoseconds < 0) {
      epochNanoseconds = 0;
    }

    const long long epochSeconds = epochNanoseconds / 1000000000LL;
    long long nanosRemainder = epochNanoseconds % 1000000000LL;
    if(nanosRemainder < 0) {
      nanosRemainder += 1000000000LL;
    }

    std::time_t timeValue = static_cast<std::time_t>(epochSeconds);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &timeValue);
#else
    gmtime_r(&timeValue, &tm);
#endif

    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << "." << std::setw(9) << std::setfill('0') << nanosRemainder << "Z";
    return out.str();
  }
} // namespace

InfluxClient::InfluxClient(InfluxConfig config) : config_(std::move(config)) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  // Start the write worker thread if batching is enabled
  if(config_.writeBatching.enabled) {
    writeWorkerThread_ = std::thread([this]() { writeWorkerLoop(); });
  }
}

InfluxClient::~InfluxClient() {
  if(config_.writeBatching.enabled) {
    {
      std::lock_guard<std::mutex> lock(writeQueueMutex_);
      stopWriteWorker_ = true;
    }
    writeQueueCv_.notify_one();
    if(writeWorkerThread_.joinable()) {
      writeWorkerThread_.join();
    }
  }
  curl_global_cleanup();
}

std::string InfluxClient::buildLineProtocol(const std::string& fieldKey, double fieldValue,
    const std::map<std::string, std::string>& tags, std::optional<int64_t> timestampNanoseconds) const {
  std::ostringstream lineProtocol;
  lineProtocol << escapeLineProtocolIdentifier(config_.measurement);

  std::map<std::string, std::string> mergedTags = tags;
  for(const auto& [configTagKey, configTagValue] : config_.extraTags) {
    if(mergedTags.find(configTagKey) == mergedTags.end()) {
      mergedTags.emplace(configTagKey, configTagValue);
    }
  }

  for(const auto& [tagKey, tagValue] : mergedTags) {
    lineProtocol << "," << escapeLineProtocolIdentifier(tagKey) << "=" << escapeLineProtocolIdentifier(tagValue);
  }

  lineProtocol << " " << escapeLineProtocolIdentifier(fieldKey) << "=";
  lineProtocol << std::setprecision(10) << fieldValue;

  if(timestampNanoseconds.has_value()) {
    lineProtocol << " " << timestampNanoseconds.value();
  }

  return lineProtocol.str();
}

bool InfluxClient::sendWritePayload(const std::string& payload, std::string* error) {
  long status = 0;
  std::string response;
  std::string requestError;

  CURL* encodeCurl = curl_easy_init();
  if(encodeCurl == nullptr) {
    if(error != nullptr) {
      *error = "Failed to initialize CURL for URL encoding";
    }
    return false;
  }

  std::string query = "org=" + urlEncode(encodeCurl, config_.org) + "&bucket=" + urlEncode(encodeCurl, config_.bucket) +
      "&precision=" + urlEncode(encodeCurl, config_.precision);
  curl_easy_cleanup(encodeCurl);

  bool ok = sendRequest("/api/v2/write", query, "POST", payload, "text/plain; charset=utf-8", "application/json",
      &status, &response, &requestError);

  if(!ok) {
    if(error != nullptr) {
      *error = requestError;
    }
    return false;
  }

  if(status < 200 || status >= 300) {
    if(error != nullptr) {
      *error = "Write failed with HTTP " + std::to_string(status) + ": " + response;
    }
    return false;
  }

  return true;
}

std::vector<InfluxRecord> InfluxClient::readRange(
    const std::string& start, const std::string& stop, std::string* error) {
  const std::string fluxQuery = buildFluxReadQuery(config_, start, stop, "", {});

  return executeFluxReadQuery(fluxQuery, error);
}

std::vector<InfluxRecord> InfluxClient::executeFluxReadQuery(const std::string& fluxQuery, std::string* error) {
  std::ostringstream body;
  body << "{\"query\":\"" << escapeJson(fluxQuery) << "\",\"type\":\"flux\"}";

  long status = 0;
  std::string response;
  std::string requestError;

  CURL* encodeCurl = curl_easy_init();
  if(encodeCurl == nullptr) {
    if(error != nullptr) {
      *error = "Failed to initialize CURL for URL encoding";
    }
    return {};
  }

  const std::string query = "org=" + urlEncode(encodeCurl, config_.org);
  curl_easy_cleanup(encodeCurl);

  bool ok = sendRequest("/api/v2/query", query, "POST", body.str(), "application/json", "application/csv", &status,
      &response, &requestError);

  if(!ok) {
    if(error != nullptr) {
      *error = requestError;
    }
    return {};
  }

  if(status < 200 || status >= 300) {
    if(error != nullptr) {
      *error = "Read failed with HTTP " + std::to_string(status) + ": " + response;
    }
    return {};
  }

  std::vector<InfluxRecord> records;
  std::vector<std::string> headers;

  std::istringstream stream(response);
  std::string line;
  while(std::getline(stream, line)) {
    line = trim(line);
    if(line.empty() || line[0] == '#') {
      continue;
    }

    if(headers.empty()) {
      headers = parseCsvLine(line);
      continue;
    }

    const std::vector<std::string> row = parseCsvLine(line);
    if(row.size() < headers.size()) {
      continue;
    }

    InfluxRecord record;
    for(size_t i = 0; i < headers.size(); ++i) {
      const std::string& name = headers[i];
      const std::string& value = row[i];
      if(name == "_time") {
        record.time = value;
        record.timestampNanoseconds = parseTimestampToNanoseconds(value);
      }
      else if(name == "_measurement") {
        record.measurement = value;
      }
      else if(name == "_field") {
        record.field = value;
      }
      else if(name == "_value") {
        record.value = value;
      }
    }

    if(!record.time.empty()) {
      records.push_back(record);
    }
  }

  return records;
}

std::vector<InfluxRecord> InfluxClient::readRangeUnixNanoseconds(int64_t startNanoseconds, int64_t stopNanoseconds,
    const std::string& fieldKey, const std::map<std::string, std::string>& tags, std::string* error) {
  if(stopNanoseconds <= startNanoseconds) {
    if(error != nullptr) {
      *error = "Invalid range: stopNanoseconds must be greater than "
               "startNanoseconds";
    }
    return {};
  }

  const std::string startExpr = "time(v: \"" + formatRfc3339FromNanoseconds(startNanoseconds) + "\")";
  const std::string stopExpr = "time(v: \"" + formatRfc3339FromNanoseconds(stopNanoseconds) + "\")";
  const std::string fluxQuery = buildFluxReadQuery(config_, startExpr, stopExpr, fieldKey, tags);

  return executeFluxReadQuery(fluxQuery, error);
}

bool InfluxClient::sendRequest(const std::string& endpoint, const std::string& queryParameters,
    const std::string& method, const std::string& body, const std::string& contentType, const std::string& accept,
    long* httpStatus, std::string* responseBody, std::string* error) {
  CURL* curl = curl_easy_init();
  if(curl == nullptr) {
    if(error != nullptr) {
      *error = "Failed to initialize CURL handle";
    }
    return false;
  }

  const std::string baseUrl = stripTrailingSlash(config_.url);
  const std::string url = baseUrl + endpoint + (queryParameters.empty() ? "" : "?" + queryParameters);

  struct curl_slist* headers = nullptr;
  headers = curl_slist_append(headers, ("Authorization: Token " + config_.token).c_str());
  headers = curl_slist_append(headers, ("Content-Type: " + contentType).c_str());
  headers = curl_slist_append(headers, ("Accept: " + accept).c_str());

  std::string response;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode rc = curl_easy_perform(curl);
  if(rc != CURLE_OK) {
    if(error != nullptr) {
      *error = std::string("HTTP request failed: ") + curl_easy_strerror(rc);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return false;
  }

  long status = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

  if(httpStatus != nullptr) {
    *httpStatus = status;
  }
  if(responseBody != nullptr) {
    *responseBody = response;
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return true;
}

// batch write support related methods
InfluxWriteStats InfluxClient::getWriteStats() const {
  InfluxWriteStats stats;
  {
    std::lock_guard<std::mutex> lock(writeQueueMutex_);
    stats.queuedPoints = writeQueue_.size();
  }
  stats.queuedPointsDropped = queuedPointsDropped_.load();
  stats.pointsWritten = pointsWritten_.load();
  stats.pointsDropped = pointsDropped_.load();
  stats.batchesWritten = batchesWritten_.load();
  stats.batchWriteFailures = batchWriteFailures_.load();
  stats.retryAttempts = retryAttempts_.load();
  return stats;
}

std::string InfluxClient::getLastAsyncWriteError() const {
  std::lock_guard<std::mutex> lock(asyncErrorMutex_);
  return lastAsyncWriteError_;
}

bool InfluxClient::hasAsyncWriteError() const {
  std::lock_guard<std::mutex> lock(asyncErrorMutex_);
  return !lastAsyncWriteError_.empty();
}

void InfluxClient::clearAsyncWriteError() {
  std::lock_guard<std::mutex> lock(asyncErrorMutex_);
  lastAsyncWriteError_.clear();
}

void InfluxClient::setLastAsyncWriteError(const std::string& error) {
  std::lock_guard<std::mutex> lock(asyncErrorMutex_);
  lastAsyncWriteError_ = error;
}

bool InfluxClient::attemptSendBatch(const std::string& payload, std::string* error) {
  const std::size_t maxAttempts = config_.writeBatching.maxRetries + 1;
  for(std::size_t attempt = 1; attempt <= maxAttempts; ++attempt) {
    if(sendWritePayload(payload, error)) {
      return true;
    }

    if(attempt < maxAttempts) {
      retryAttempts_.fetch_add(1);
      const std::size_t shift = std::min<std::size_t>(attempt - 1, 6);
      const int backoffMultiplier = 1 << static_cast<int>(shift);
      const int waitMs = std::max(1, config_.writeBatching.retryBackoffMs) * backoffMultiplier;
      std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
    }
  }

  return false;
}

void InfluxClient::writeWorkerLoop() {
  while(true) {
    std::vector<PendingWritePoint> batch;

    {
      std::unique_lock<std::mutex> lock(writeQueueMutex_);
      // Wait until there are points to write or the worker is stopped
      writeQueueCv_.wait(lock, [this]() { return stopWriteWorker_ || !writeQueue_.empty(); });

      // Collect more points, respecting the maxBatchPoints limit
      if(!stopWriteWorker_ && writeQueue_.size() < config_.writeBatching.maxBatchPoints) {
        writeQueueCv_.wait_for(lock, std::chrono::milliseconds(config_.writeBatching.flushIntervalMs),
            [this]() { return stopWriteWorker_ || writeQueue_.size() >= config_.writeBatching.maxBatchPoints; });
      }

      // stop here if the worker is stopped and there are no points to write
      if(writeQueue_.empty() && stopWriteWorker_) {
        return;
      }

      // Move points from the queue to the batch
      const std::size_t batchSize = std::min(writeQueue_.size(), config_.writeBatching.maxBatchPoints);
      batch.reserve(batchSize);
      for(std::size_t i = 0; i < batchSize; ++i) {
        batch.emplace_back(std::move(writeQueue_.front()));
        writeQueue_.pop_front();
      }
    }

    // Build the payload in line protocol format
    std::ostringstream payload;
    for(std::size_t i = 0; i < batch.size(); ++i) {
      payload << buildLineProtocol(
          batch[i].fieldKey, batch[i].fieldValue, batch[i].tags, batch[i].timestampNanoseconds);
      if(i + 1 < batch.size()) {
        payload << "\n";
      }
    }

    // Attempt to send the batch with retries and update the write statistics accordingly
    std::string writeError;
    if(attemptSendBatch(payload.str(), &writeError)) {
      batchesWritten_.fetch_add(1);
      pointsWritten_.fetch_add(batch.size());
      clearAsyncWriteError();
      continue;
    }

    batchWriteFailures_.fetch_add(1);
    setLastAsyncWriteError(writeError.empty() ? "Async write failed" : writeError);

    // If the batch write failed, requeue the points if possible, otherwise drop them
    bool requeued = false;
    {
      std::lock_guard<std::mutex> lock(writeQueueMutex_);
      if(!stopWriteWorker_ && writeQueue_.size() + batch.size() <= config_.writeBatching.maxQueuePoints) {
        for(auto it = batch.rbegin(); it != batch.rend(); ++it) {
          writeQueue_.push_front(std::move(*it));
        }
        requeued = true;
      }
    }

    if(requeued) {
      writeQueueCv_.notify_one();
      continue;
    }

    pointsDropped_.fetch_add(batch.size());
    queuedPointsDropped_.fetch_add(batch.size());
  }
}

bool InfluxClient::writePoint(const std::string& fieldKey, double fieldValue,
    const std::map<std::string, std::string>& tags, std::optional<int64_t> timestampNanoseconds, std::string* error) {
  if(!config_.writeBatching.enabled) {
    const bool ok = attemptSendBatch(buildLineProtocol(fieldKey, fieldValue, tags, timestampNanoseconds), error);
    if(ok) {
      pointsWritten_.fetch_add(1);
      batchesWritten_.fetch_add(1);
    }
    else {
      batchWriteFailures_.fetch_add(1);
      pointsDropped_.fetch_add(1);
    }
    return ok;
  }

  if(config_.writeBatching.failFastOnAsyncError && hasAsyncWriteError()) {
    if(error != nullptr) {
      *error = getLastAsyncWriteError();
    }
    return false;
  }

  PendingWritePoint point;
  point.fieldKey = fieldKey;
  point.fieldValue = fieldValue;
  point.tags = tags;
  point.timestampNanoseconds = timestampNanoseconds;

  {
    std::lock_guard<std::mutex> lock(writeQueueMutex_);
    if(writeQueue_.size() >= config_.writeBatching.maxQueuePoints) {
      if(error != nullptr) {
        *error = "Write queue is full";
      }
      pointsDropped_.fetch_add(1);
      queuedPointsDropped_.fetch_add(1);
      return false;
    }
    writeQueue_.push_back(std::move(point));
  }

  writeQueueCv_.notify_one();

  return true;
}

void InfluxClient::addHealthMonitoringNodes(UA_Server* server) {
  if(!config_.writeBatching.enabled) {
    return;
  }
  auto config = UA_ServerConfig(server);
  UA_ObjectAttributes healthAttr = UA_ObjectAttributes_default;
  healthAttr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>("InfluxHealth"));
  const UA_NodeId healthObjectNodeId = UA_NODEID_STRING(1, const_cast<char*>("InfluxHealth"));
  auto rc = UA_Server_addObjectNode(server, healthObjectNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
      UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_QUALIFIEDNAME(1, const_cast<char*>("InfluxHealth")),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), healthAttr, nullptr, nullptr);
  if(rc != UA_STATUSCODE_GOOD) {
    UA_LOG_ERROR(
        config.logging, UA_LOGCATEGORY_USERLAND, "Failed to add InfluxHealth object: %s", UA_StatusCode_name(rc));
    return;
  }

  healthContext_ = std::make_unique<HealthMonitoring::InfluxHealthContext>();
  healthContext_->client = this;
  if(!HealthMonitoring::addReadOnlyNodeUInt64(server, healthObjectNodeId, "InfluxHealth.QueuedPoints", "QueuedPoints",
         "Current number of queued write points", &healthContext_->queuedPointsNodeId) ||
      !HealthMonitoring::addReadOnlyNodeUInt64(server, healthObjectNodeId, "InfluxHealth.QueuedPointsDropped",
          "QueuedPointsDropped", "Total points dropped due to queue pressure",
          &healthContext_->queuedPointsDroppedNodeId) ||
      !HealthMonitoring::addReadOnlyNodeUInt64(server, healthObjectNodeId, "InfluxHealth.PointsWritten",
          "PointsWritten", "Total points successfully written to InfluxDB", &healthContext_->pointsWrittenNodeId) ||
      !HealthMonitoring::addReadOnlyNodeUInt64(server, healthObjectNodeId, "InfluxHealth.PointsDropped",
          "PointsDropped", "Total points dropped after write failures", &healthContext_->pointsDroppedNodeId) ||
      !HealthMonitoring::addReadOnlyNodeUInt64(server, healthObjectNodeId, "InfluxHealth.BatchesWritten",
          "BatchesWritten", "Total batches successfully written", &healthContext_->batchesWrittenNodeId) ||
      !HealthMonitoring::addReadOnlyNodeUInt64(server, healthObjectNodeId, "InfluxHealth.BatchWriteFailures",
          "BatchWriteFailures", "Total failed batch write attempts", &healthContext_->batchFailuresNodeId) ||
      !HealthMonitoring::addReadOnlyNodeUInt64(server, healthObjectNodeId, "InfluxHealth.RetryAttempts",
          "RetryAttempts", "Total retry attempts after write failures", &healthContext_->retryAttemptsNodeId) ||
      !HealthMonitoring::addReadOnlyNodeBoolean(server, healthObjectNodeId, "InfluxHealth.AsyncErrorActive",
          "AsyncErrorActive", "Indicates whether an async write error is active",
          &healthContext_->asyncErrorActiveNodeId) ||
      !HealthMonitoring::addReadOnlyNodeString(server, healthObjectNodeId, "InfluxHealth.LastAsyncError",
          "LastAsyncError", "Last asynchronous write error text", &healthContext_->asyncErrorNodeId)) {
    UA_LOG_ERROR(config.logging, UA_LOGCATEGORY_USERLAND, "Failed to add Influx health variable nodes");
    return;
  }

  HealthMonitoring::updateInfluxHealth(server, healthContext_.get());
  rc = UA_Server_addRepeatedCallback(
      server, HealthMonitoring::updateInfluxHealth, healthContext_.get(), 1000.0, &healthCallbackId);
  if(rc != UA_STATUSCODE_GOOD) {
    UA_LOG_ERROR(config.logging, UA_LOGCATEGORY_USERLAND, "Failed to add health callback: %s", UA_StatusCode_name(rc));
    return;
  }
  healthNodesAdded_ = true;
}

void InfluxClient::removeHealthNodesCallback(UA_Server* server) {
  if(healthNodesAdded_) {
    UA_Server_removeRepeatedCallback(server, healthCallbackId);
    healthNodesAdded_ = false;
  }
}