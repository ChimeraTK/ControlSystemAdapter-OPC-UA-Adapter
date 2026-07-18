// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "history_backend/Config.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

struct InfluxRecord {
  std::string time;
  long long timestampNanoseconds = 0;
  std::string measurement;
  std::string field;
  std::string value;
  uint16_t port = 0;
};

class InfluxClient {
 public:
  explicit InfluxClient(InfluxConfig config);
  ~InfluxClient();

  bool writePoint(const std::string& fieldKey, double fieldValue, const std::map<std::string, std::string>& tags = {},
      std::optional<long long> timestampNanoseconds = std::nullopt, std::string* error = nullptr);

  std::vector<InfluxRecord> readRange(const std::string& start, const std::string& stop, std::string* error = nullptr);

  std::vector<InfluxRecord> readRangeUnixNanoseconds(long long startNanoseconds, long long stopNanoseconds,
      const std::string& fieldKey = "", const std::map<std::string, std::string>& tags = {},
      std::string* error = nullptr);

 private:
  InfluxConfig config_;

  std::vector<InfluxRecord> executeFluxReadQuery(const std::string& fluxQuery, std::string* error);

  bool sendRequest(const std::string& endpoint, const std::string& queryParameters, const std::string& method,
      const std::string& body, const std::string& contentType, const std::string& accept, long* httpStatus,
      std::string* responseBody, std::string* error);
};
