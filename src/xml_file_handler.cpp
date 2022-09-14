/*
 * This file is part of ChimeraTKs ControlSystem-OPC-UA-Adapter.
 *
 * ChimeraTKs ControlSystem-OPC-UA-Adapter is free software: you can
 * redistribute it and/or modify it under the terms of the Lesser GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ChimeraTKs ControlSystem-OPC-UA-Adapter is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty ofMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see https://www.gnu.org/licenses/lgpl.html
 *
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 * Copyright (c) 2016 Julian Rahm  <Julian.Rahm@tu-dresden.de>
 */

#include "xml_file_handler.h"

#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <iostream>

using namespace std;

xml_file_handler::xml_file_handler(std::string filePath) {
  // FIXME: Add some check routine if file realy exist
  this->createDoc(filePath);
}

std::vector<xmlNodePtr> xml_file_handler::getNodesByName(xmlNodePtr startNode, std::string nodeName) {
  std::vector<xmlNodePtr> nodeVector;
  while(startNode != NULL) {
    if((!xmlStrcmp(startNode->name, (const xmlChar*)nodeName.c_str()))) {
      nodeVector.push_back(startNode);
    }
    startNode = startNode->next;
  }

  return nodeVector;
}

xmlXPathObjectPtr xml_file_handler::getNodeSet(std::string xPathString) {
  xmlChar* xpath = (xmlChar*)xPathString.c_str();
  xmlXPathContextPtr context;
  xmlXPathObjectPtr result;

  if(!isDocSetted()) {
    return NULL;
  }
  context = xmlXPathNewContext(this->doc);
  if(context == NULL) {
    // std::cout("Error in xmlXPathNewContext\n");
    return NULL;
  }
  if(xmlXPathRegisterNs(
         context, (xmlChar*)"csa", (xmlChar*)"https://github.com/ChimeraTK/ControlSystemAdapter-OPC-UA-Adapter") != 0) {
    std::cerr << "Failed to register xml namespace: https://github.com/ChimeraTK/ControlSystemAdapter-OPC-UA-Adapter"
              << std::endl;
    return NULL;
  }

  result = xmlXPathEvalExpression(xpath, context);
  xmlXPathFreeContext(context);
  if(result == NULL) {
    // std::cout("Error in xmlXPathEvalExpression\n");
    return NULL;
  }
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    // std::cout("No result\n");
    return NULL;
  }
  return result;
}

bool xml_file_handler::isDocSetted() {
  if(this->doc != NULL) {
    return true;
  }
  return false;
}

bool xml_file_handler::createDoc(std::string filePath) {
  if(filePath.empty()) {
    this->doc = NULL;
    return false;
  }

  this->doc = xmlParseFile(filePath.c_str());

  if(this->doc == NULL) {
    std::cout << "Document not parsed successfully." << std::endl;
    return false;
  }

  return true;
}

std::vector<std::string> xml_file_handler::praseVariablePath(std::string variablePath, std::string seperator) {
  std::vector<std::string> pathList;
  boost::char_separator<char> sep(seperator.c_str());
  boost::tokenizer<boost::char_separator<char>> tokens(variablePath, sep);

  for(const auto& t : tokens) {
    pathList.push_back(t);
  }
  return pathList;
}

std::string xml_file_handler::getAttributeValueFromNode(xmlNode* node, std::string attributeName) {
  xmlAttrPtr attr = xmlHasProp(node, (xmlChar*)attributeName.c_str());
  if(!(attr == NULL)) {
    std::string merker = (std::string)((char*)attr->children->content);
    return merker;
  }
  return "";
}

std::string xml_file_handler::getContentFromNode(xmlNode* node) {
  if(node != NULL) {
    xmlChar* content = xmlNodeGetContent(node->xmlChildrenNode);
    if(content != NULL) {
      std::string merker = (std::string)((char*)content);
      xmlFree(content);
      boost::trim(merker);
      return merker;
    }
  }
  return "";
}

xml_file_handler::~xml_file_handler() {
  xmlFreeDoc(this->doc);
  xmlCleanupParser();
}
