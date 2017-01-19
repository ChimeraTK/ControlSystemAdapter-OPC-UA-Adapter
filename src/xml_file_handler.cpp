#include "xml_file_handler.h"

#include <iostream>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>
#include <libxml2/libxml/tree.h>


xml_file_handler::xml_file_handler(std::string filePath) {
	// FIXME: Add some check routine if file realy exist
	this->createDoc(filePath);
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
	
	if(!isDocSetted()) {
		return NULL;
	}
	context = xmlXPathNewContext(this->doc);
	if (context == NULL) {
		//std::cout("Error in xmlXPathNewContext\n");
		return NULL;
	}
	result = xmlXPathEvalExpression(xpath, context);
	xmlXPathFreeContext(context);
	if (result == NULL) {
		//std::cout("Error in xmlXPathEvalExpression\n");
		return NULL;
	}
	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
		xmlXPathFreeObject(result);
		//std::cout("No result\n");
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
	
 	if(this->doc == NULL ) {
		//std::cout << "Document not parsed successfully." << std::endl;
		return false;
 	}

	return true;
}

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
