// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "history_backend/Config.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <algorithm>
#include <stdexcept>

namespace {
  std::string requireChildContent(xmlNodePtr root, const char* elementName) {
    for(xmlNodePtr child = root->children; child != nullptr; child = child->next) {
      if(child->type != XML_ELEMENT_NODE) {
        continue;
      }

      if(xmlStrcmp(child->name, BAD_CAST elementName) == 0) {
        xmlChar* content = xmlNodeGetContent(child);
        if(content == nullptr) {
          throw std::runtime_error(std::string("Missing text content for XML element: ") + elementName);
        }

        const std::string value(reinterpret_cast<const char*>(content));
        xmlFree(content);

        if(value.empty()) {
          throw std::runtime_error(std::string("Empty text content for XML element: ") + elementName);
        }

        return value;
      }
    }

    throw std::runtime_error(std::string("Missing required XML element: ") + elementName);
  }

  std::string optionalChildContent(xmlNodePtr root, const char* elementName) {
    for(xmlNodePtr child = root->children; child != nullptr; child = child->next) {
      if(child->type != XML_ELEMENT_NODE) {
        continue;
      }

      if(xmlStrcmp(child->name, BAD_CAST elementName) == 0) {
        xmlChar* content = xmlNodeGetContent(child);
        if(content == nullptr) {
          return "";
        }

        const std::string value(reinterpret_cast<const char*>(content));
        xmlFree(content);
        return value;
      }
    }

    return "";
  }

  std::map<std::string, std::string> parseExtraTags(xmlNodePtr root) {
    for(xmlNodePtr child = root->children; child != nullptr; child = child->next) {
      if(child->type != XML_ELEMENT_NODE) {
        continue;
      }

      if(xmlStrcmp(child->name, BAD_CAST "extra_tags") != 0) {
        continue;
      }

      std::map<std::string, std::string> tags;
      for(xmlNodePtr tagNode = child->children; tagNode != nullptr; tagNode = tagNode->next) {
        if(tagNode->type != XML_ELEMENT_NODE || xmlStrcmp(tagNode->name, BAD_CAST "tag") != 0) {
          continue;
        }

        xmlChar* nameAttr = xmlGetProp(tagNode, BAD_CAST "name");
        if(nameAttr == nullptr) {
          continue;
        }

        const std::string name(reinterpret_cast<const char*>(nameAttr));
        xmlFree(nameAttr);
        if(name.empty()) {
          continue;
        }

        xmlChar* content = xmlNodeGetContent(tagNode);
        if(content == nullptr) {
          continue;
        }

        const std::string tag(reinterpret_cast<const char*>(content));
        xmlFree(content);

        if(!tag.empty()) {
          tags[name] = tag;
        }
      }

      return tags;
    }

    return {};
  }

  std::size_t parseSizeValue(const std::string& value, std::size_t defaultValue, std::size_t minValue) {
    if(value.empty()) {
      return defaultValue;
    }
    char* end = nullptr;
    const unsigned long parsed = std::strtoul(value.c_str(), &end, 10);
    if(end == value.c_str() || *end != '\0') {
      return defaultValue;
    }
    const std::size_t casted = static_cast<std::size_t>(parsed);
    return casted < minValue ? minValue : casted;
  }

  int parseIntValue(const std::string& value, int defaultValue, int minValue) {
    if(value.empty()) {
      return defaultValue;
    }
    char* end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if(end == value.c_str() || *end != '\0') {
      return defaultValue;
    }
    const int casted = static_cast<int>(parsed);
    return casted < minValue ? minValue : casted;
  }

  bool parseBooleanValue(const std::string& value, bool defaultValue) {
    if(value.empty()) {
      return defaultValue;
    }

    std::string normalized = value;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if(normalized == "true" || normalized == "1") {
      return true;
    }
    if(normalized == "false" || normalized == "0") {
      return false;
    }
    return defaultValue;
  }

  InfluxWriteBatchingConfig parseWriteBatching(xmlNodePtr root) {
    InfluxWriteBatchingConfig batching;

    for(xmlNodePtr child = root->children; child != nullptr; child = child->next) {
      if(child->type != XML_ELEMENT_NODE) {
        continue;
      }

      if(xmlStrcmp(child->name, BAD_CAST "write_batching") != 0) {
        continue;
      }

      const std::string enabled = optionalChildContent(child, "enabled");
      const std::string maxBatch = optionalChildContent(child, "max_batch_points");
      const std::string maxQueue = optionalChildContent(child, "max_queue_points");
      const std::string flushMs = optionalChildContent(child, "flush_interval_ms");
      const std::string maxRetries = optionalChildContent(child, "max_retries");
      const std::string retryBackoffMs = optionalChildContent(child, "retry_backoff_ms");
      const std::string failFast = optionalChildContent(child, "fail_fast_on_async_error");

      batching.enabled = parseBooleanValue(enabled, batching.enabled);
      batching.maxBatchPoints = parseSizeValue(maxBatch, batching.maxBatchPoints, 1);
      batching.maxQueuePoints = parseSizeValue(maxQueue, batching.maxQueuePoints, 1);
      batching.flushIntervalMs = parseIntValue(flushMs, batching.flushIntervalMs, 1);
      batching.maxRetries = parseSizeValue(maxRetries, batching.maxRetries, 0);
      batching.retryBackoffMs = parseIntValue(retryBackoffMs, batching.retryBackoffMs, 1);
      batching.failFastOnAsyncError = parseBooleanValue(failFast, batching.failFastOnAsyncError);

      return batching;
    }

    return batching;
  }
} // namespace

InfluxConfig ConfigLoader::loadFromXmlFile(const std::string& path) {
  xmlDocPtr document = xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOBLANKS);
  if(document == nullptr) {
    throw std::runtime_error("Failed to parse XML config file: " + path);
  }

  xmlNodePtr root = xmlDocGetRootElement(document);
  if(root == nullptr || xmlStrcmp(root->name, BAD_CAST "influxdb") != 0) {
    xmlFreeDoc(document);
    throw std::runtime_error("Invalid XML config: missing root element <influxdb>");
  }

  InfluxConfig config;
  config.url = requireChildContent(root, "url");
  config.token = requireChildContent(root, "token");
  config.org = requireChildContent(root, "org");
  config.bucket = requireChildContent(root, "bucket");
  config.measurement = requireChildContent(root, "measurement");
  config.precision = requireChildContent(root, "precision");
  config.extraTags = parseExtraTags(root);
  config.writeBatching = parseWriteBatching(root);

  xmlFreeDoc(document);

  return config;
}
