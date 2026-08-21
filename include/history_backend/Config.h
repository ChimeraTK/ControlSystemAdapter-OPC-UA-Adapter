// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct InfluxWriteBatchingConfig {
  bool enabled = true;
  std::size_t maxBatchPoints = 250;
  std::size_t maxQueuePoints = 10000;
  int flushIntervalMs = 100;
  std::size_t maxRetries = 3;
  int retryBackoffMs = 200;
  bool failFastOnAsyncError = false;
};

/**
 * @brief Structure to hold information about tags to be added to InfluxDB measurements.
 */
struct TagInformation {
  std::string tagName;    ///< Name of the tag.
  std::string tagValue;   ///< Value of the tag.
  std::string sourceName; ///< Source name of the PV which should get the tag. If empty, the tag is added to all PVs.
  bool operator==(const TagInformation& other) const { return tagName == other.tagName && tagValue == other.tagValue; }
};
struct InfluxConfig {
  std::string url;
  std::string token;
  std::string org;
  std::string bucket;
  std::string measurement;
  std::string precision;
  std::vector<TagInformation> extraTags;
  InfluxWriteBatchingConfig writeBatching;
};

class ConfigLoader {
 public:
  static InfluxConfig loadFromXmlFile(const std::string& path);
};
