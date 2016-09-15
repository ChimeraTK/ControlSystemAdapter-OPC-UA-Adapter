/*
 * Copyright (c) 2016 <copyright holder> <email>
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 */

#include "xml_file_handler.h"


#include <iostream>

#include <boost/tokenizer.hpp>

#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>
#include <libxml2/libxml/tree.h>

xml_file_handler::xml_file_handler(std::string filePath) {
	
	this->doc = this->createDoc(filePath);
}

std::vector<xmlNodePtr> xml_file_handler::getNodesByName(xmlNodePtr startNode, std::string nodeName) {
	std::vector<xmlNodePtr> nodeVector;
	while (startNode != NULL) {
		if ((!xmlStrcmp(startNode->name, (const xmlChar *)nodeName.c_str()))) {
			nodeVector.push_back(startNode);
		}
		startNode = startNode->next;
	}
	
	return nodeVector;
}

xmlXPathObjectPtr xml_file_handler::getNodeSet(std::string xPathString) {
	xmlChar *xpath = (xmlChar*) xPathString.c_str();
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;
	
	context = xmlXPathNewContext(doc);
	if (context == NULL) {
		std::printf("Error in xmlXPathNewContext\n");
		return NULL;
	}
	result = xmlXPathEvalExpression(xpath, context);
	xmlXPathFreeContext(context);
	if (result == NULL) {
		std::printf("Error in xmlXPathEvalExpression\n");
		return NULL;
	}
	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
		xmlXPathFreeObject(result);
        std::printf("No result\n");
		return NULL;
	}
	return result;

}

xmlDocPtr xml_file_handler::createDoc(std::string filePath) {
	xmlDocPtr doc;
	doc = xmlParseFile(filePath.c_str());
	
	if (doc == NULL ) {
		std::fprintf(stderr,"Document not parsed successfully. \n");
		return NULL;
	}

	return doc;
}

// seperator can be a string of diffent seperators, like "_/:" and so on 
std::vector<std::string> xml_file_handler::praseVariablePath(std::string variablePath, std::string seperator) {

	std::vector<std::string> pathList;
    boost::char_separator<char> sep(seperator.c_str());
    boost::tokenizer<boost::char_separator<char>> tokens(variablePath, sep);
 	for (const auto& t : tokens) {
		pathList.push_back(t);
 	}
	return pathList;
}

std::string xml_file_handler::getAttributeValueFromNode(xmlNode* node, std::string attributeName) {
	
	xmlAttrPtr attr = xmlHasProp(node, (xmlChar*)attributeName.c_str());
	if(!(attr == NULL)) {
		std::string merker = (std::string)((char*)attr->children->content);
		//xmlFree(attr);
		return merker;
	}
	return "";
}

std::string xml_file_handler::getContentFromNode(xmlNode* node) {
	xmlChar* content = xmlNodeGetContent(node->xmlChildrenNode);
	if(content != NULL) {
		std::string merker = (std::string)((char*)content);
		xmlFree(content);
		return merker;
	}
	return "";
}


xml_file_handler::~xml_file_handler() {
	xmlFreeDoc(this->doc);
	xmlCleanupParser();
}
