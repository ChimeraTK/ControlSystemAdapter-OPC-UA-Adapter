// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <cstddef>
#include <map>
#include <string>

struct InfluxWriteBatchingConfig {
  bool enabled = true;
  std::size_t maxBatchPoints = 250;
  std::size_t maxQueuePoints = 10000;
  int flushIntervalMs = 100;
  std::size_t maxRetries = 3;
  int retryBackoffMs = 200;
  bool failFastOnAsyncError = false;
};

struct InfluxConfig {
  std::string url;
  std::string token;
  std::string org;
  std::string bucket;
  std::string measurement;
  std::string precision;
  std::map<std::string, std::string> extraTags;
  InfluxWriteBatchingConfig writeBatching;
};

class ConfigLoader {
 public:
  static InfluxConfig loadFromXmlFile(const std::string& path);
};
