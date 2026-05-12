// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
// SPDX-FileCopyrightText: 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
// SPDX-FileCopyrightText: 2023 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "xml_file_handler.h"

#include "open62541/plugin/log_stdout.h"

#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <iostream>

using namespace std;

namespace ChimeraTK {
  xml_file_handler::xml_file_handler(const std::string& filePath) {
    if(!this->createDoc(filePath)) {
      throw logic_error(
          std::string("Failed to parse ") + filePath + (". Check if mapping file exsists and is well formated."));
    }
  }

  std::vector<xmlNodePtr> xml_file_handler::getNodesByName(xmlNodePtr startNode, const std::string& nodeName) {
    std::vector<xmlNodePtr> nodeVector;
    while(startNode != nullptr) {
      if((!xmlStrcmp(startNode->name, (const xmlChar*)nodeName.c_str()))) {
        nodeVector.push_back(startNode);
      }
      startNode = startNode->next;
    }

    return nodeVector;
  }

  xmlXPathObjectPtr xml_file_handler::getNodeSet(const std::string& xPathString) {
    auto* xpath = (xmlChar*)xPathString.c_str();
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    if(!isDocSetted()) {
      return nullptr;
    }
    context = xmlXPathNewContext(this->doc);
    if(context == nullptr) {
      return nullptr;
    }
    if(xmlXPathRegisterNs(context, (xmlChar*)"csa",
           (xmlChar*)"https://github.com/ChimeraTK/ControlSystemAdapter-OPC-UA-Adapter") != 0) {
      UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
          "Failed to register xml namespace: https://github.com/ChimeraTK/ControlSystemAdapter-OPC-UA-Adapter");
      return nullptr;
    }

    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    if(result == nullptr) {
      return nullptr;
    }
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
      xmlXPathFreeObject(result);
      return nullptr;
    }
    return result;
  }

  bool xml_file_handler::isDocSetted() {
    return this->doc != nullptr;
  }

  bool xml_file_handler::createDoc(const std::string& filePath) {
    if(filePath.empty()) {
      this->doc = nullptr;
      return false;
    }

    this->doc = xmlParseFile(filePath.c_str());

    if(this->doc == nullptr) {
      UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Document not parsed successfully.");
      return false;
    }

    return true;
  }

  std::vector<std::string> xml_file_handler::parseVariablePath(
      const std::string& variablePath, const std::string& seperator) {
    std::vector<std::string> pathList;
    boost::char_separator<char> sep(seperator.c_str());
    boost::tokenizer<boost::char_separator<char>> tokens(variablePath, sep);

    for(const auto& t : tokens) {
      pathList.push_back(t);
    }
    return pathList;
  }

  std::string xml_file_handler::getAttributeValueFromNode(xmlNode* node, const std::string& attributeName) {
    xmlAttrPtr attr = xmlHasProp(node, (xmlChar*)attributeName.c_str());
    if(attr != nullptr) {
      std::string merker = (std::string)((char*)attr->children->content);
      return merker;
    }
    return "";
  }

  std::string xml_file_handler::getContentFromNode(xmlNode* node) {
    if(node != nullptr) {
      xmlChar* content = xmlNodeGetContent(node->xmlChildrenNode);
      if(content != nullptr) {
        std::string maker = (std::string)((char*)content);
        xmlFree(content);
        boost::trim(maker);
        return maker;
      }
    }
    return "";
  }

  xml_file_handler::~xml_file_handler() {
    xmlFreeDoc(this->doc);
    xmlCleanupParser();
  }
} // namespace ChimeraTK
