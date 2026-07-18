// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <map>
#include <string>

struct InfluxConfig {
  std::string url;
  std::string token;
  std::string org;
  std::string bucket;
  std::string measurement;
  std::string precision;
  std::map<std::string, std::string> extraTags;
};

class ConfigLoader {
 public:
  static InfluxConfig loadFromXmlFile(const std::string& path);
};
