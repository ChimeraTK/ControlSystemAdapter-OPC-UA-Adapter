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

/** @class xml_file_handler
 *	@brief This class support any file interaction with a xml file.
 * 
 * 	To use all config/mapping xml-files in a confortable way this class provides some useful methodes to read specal nodes, get attributes and so one
 *   
 *  @author Chris Iatrou
 *	@author Julian Rahm
 *  @date 22.11.2016
 * 
 */

#include "xml_file_handler.h"

#include <iostream>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>
#include <libxml2/libxml/tree.h>

/** @brief The constructor of the class creates a doc pointer depending on the file path
 */
xml_file_handler::xml_file_handler(std::string filePath) {
	// FIXME: Add some check routine if file realy exist
	this->createDoc(filePath);
}

/** @brief This methode return a list of all nodes with the given name nodeName starting by the given startNode
 * 
 * @param startNode Start node, all child nodes will be searched by the given nodeName
 * @param nodeName Is the name of the xml tag, we use to search for
 * 
 * @return Returns a list of all pointer from founded child nodes with nodeName
 */
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

/** @brief This methode return a pointer of a xPath element depending of the given xPathString
 * 
 * @param xPathString A xPath string we seaching for
 * 
 * @return Returns a element pointer if some was found, in other cases it will return NULL
 */
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

/** @brief This Methode check if a document is currently setted
 * 
 * @return True or false depending if a document is set or not
 */
bool xml_file_handler::isDocSetted() {
		if(this->doc != NULL) {
			return true;
		}
		return false;
}

/** @brief This methode set a document pointer to the file it ist given by the file path
 * 
 * @param filePath Path to a xml file which you want to read
 * 
 * @return Returns true if a document is setted, if not it returns false
 */
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


/** @brief This methode splitt a given string bey the given seperators. You can use the seperator as string of seperators like "_/&" all characters will be used as single seperator
 * 
 * @param variablePath String which you want to seperate by the seperator
 * @param seperator String of, if you want different, seperators, whicht this methode use to split the string
 * 
 * @return Returns a vector of every single splitet word
 */
std::vector<std::string> xml_file_handler::praseVariablePath(std::string variablePath, std::string seperator) {

	std::vector<std::string> pathList;
    boost::char_separator<char> sep(seperator.c_str());
    boost::tokenizer<boost::char_separator<char>> tokens(variablePath, sep);
 	for (const auto& t : tokens) {
		pathList.push_back(t);
 	}
	return pathList;
}

/** @brief This methode returns a value of the given attribute from the given node you want to know
 * 
 * @param node Node with the attribute of interest
 * @param attributeName Name of the wanted attribute
 * 
 * @return Returns a string of the attribute
 */
std::string xml_file_handler::getAttributeValueFromNode(xmlNode* node, std::string attributeName) {
	
	xmlAttrPtr attr = xmlHasProp(node, (xmlChar*)attributeName.c_str());
	if(!(attr == NULL)) {
		std::string merker = (std::string)((char*)attr->children->content);
		return merker;
	}
	return "";
}

/** @brief This methode returns the value between a xml tag. For example \<tagelement>content\</tagelement> the returned value ist "content".
 * 
 * @param node Node with the content of interest
 * 
 * @return Returns a string of the inner content
 */
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

/** @brief Destructor of the class, frees the document and clean the parser.
 * 
 */
xml_file_handler::~xml_file_handler() {
	xmlFreeDoc(this->doc);
	xmlCleanupParser();
}
