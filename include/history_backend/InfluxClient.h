// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "history_backend/Config.h"
#include "history_backend/InfluxHealthMonitoring.h"
#include "open62541/server.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

struct InfluxRecord {
  std::string time;
  long long timestampNanoseconds = 0;
  std::string measurement;
  std::string field;
  std::string value;
  uint16_t port = 0;
};

// batch write support related struct
struct InfluxWriteStats {
  std::size_t queuedPoints = 0;
  std::size_t queuedPointsDropped = 0;
  std::size_t pointsWritten = 0;
  std::size_t pointsDropped = 0;
  std::size_t batchesWritten = 0;
  std::size_t batchWriteFailures = 0;
  std::size_t retryAttempts = 0;
};

class InfluxClient {
 public:
  explicit InfluxClient(InfluxConfig config);
  ~InfluxClient();

  bool writePoint(const std::string& fieldKey, double fieldValue, const std::map<std::string, std::string>& tags = {},
      std::optional<int64_t> timestampNanoseconds = std::nullopt, std::string* error = nullptr);

  std::vector<InfluxRecord> readRange(const std::string& start, const std::string& stop, std::string* error = nullptr);

  std::vector<InfluxRecord> readRangeUnixNanoseconds(int64_t startNanoseconds, int64_t stopNanoseconds,
      const std::string& fieldKey = "", const std::map<std::string, std::string>& tags = {},
      std::string* error = nullptr);

  // batch write support related publicmethods
  [[nodiscard]] InfluxWriteStats getWriteStats() const;
  [[nodiscard]] std::string getLastAsyncWriteError() const;
  [[nodiscard]] bool hasAsyncWriteError() const;
  void clearAsyncWriteError();
  /**
   * @brief Adds health monitoring nodes to the OPC UA server if batch writing is enabled. If successful
   * healthNodesAdded_ will be set to true.
   * @param server The OPC UA server to add nodes to.
   */
  void addHealthMonitoringNodes(UA_Server* server);
  void removeHealthNodesCallback(UA_Server* server);

 private:
  InfluxConfig config_;

  std::vector<InfluxRecord> executeFluxReadQuery(const std::string& fluxQuery, std::string* error);

  bool sendRequest(const std::string& endpoint, const std::string& queryParameters, const std::string& method,
      const std::string& body, const std::string& contentType, const std::string& accept, long* httpStatus,
      std::string* responseBody, std::string* error);

  // batch write support related struct
  struct PendingWritePoint {
    std::string fieldKey;
    double fieldValue = 0.0;
    std::map<std::string, std::string> tags;
    std::optional<int64_t> timestampNanoseconds;
  };
  // batch write support related members
  std::unique_ptr<HealthMonitoring::InfluxHealthContext> healthContext_{
      std::make_unique<HealthMonitoring::InfluxHealthContext>()};
  UA_UInt64 healthCallbackId{0};
  bool healthNodesAdded_ = false;
  mutable std::mutex writeQueueMutex_;
  std::condition_variable writeQueueCv_;
  std::deque<PendingWritePoint> writeQueue_;
  std::thread writeWorkerThread_;
  bool stopWriteWorker_ = false;
  std::atomic<std::size_t> queuedPointsDropped_{0};
  std::atomic<std::size_t> pointsWritten_{0};
  std::atomic<std::size_t> pointsDropped_{0};
  std::atomic<std::size_t> batchesWritten_{0};
  std::atomic<std::size_t> batchWriteFailures_{0};
  std::atomic<std::size_t> retryAttempts_{0};
  mutable std::mutex asyncErrorMutex_;
  std::string lastAsyncWriteError_;

  // batch write support related methods
  /**
   * @brief Sends a write payload to InfluxDB.
   * @param payload The payload in line protocol format to send.
   * @param error Pointer to a string to store any error message.
   * @return True if the payload was sent successfully, false otherwise.
   */
  bool sendWritePayload(const std::string& payload, std::string* error);

  /**
   * @brief Builds a line protocol string for a single point.
   * @param fieldKey The key of the field to write.
   * @param fieldValue The value of the field to write.
   * @param tags The tags for the point.
   * @param timestampNanoseconds The timestamp of the point in nanoseconds.
   * @return The line protocol string for the point.
   */
  std::string buildLineProtocol(const std::string& fieldKey, double fieldValue,
      const std::map<std::string, std::string>& tags, std::optional<int64_t> timestampNanoseconds) const;
  void writeWorkerLoop();
  /**
   * @brief Attempts to send a batch of points to InfluxDB with retries.
   * @param payload The payload in line protocol format to send.
   * @param error Pointer to a string to store any error message.
   * @return True if the batch was sent successfully, false otherwise.
   */
  bool attemptSendBatch(const std::string& payload, std::string* error);

  /**
   * @brief Sets the last asynchronous write error message.
   * @param error The error message to set.
   */
  void setLastAsyncWriteError(const std::string& error);
};
