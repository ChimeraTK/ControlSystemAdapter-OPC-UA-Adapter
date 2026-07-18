// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "history_backend/Config.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

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

  xmlFreeDoc(document);

  return config;
}
