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

#ifndef XMLFILEHANDLER_H
#define XMLFILEHANDLER_H

#include <iostream>
#include <vector>

#include <boost/tokenizer.hpp>

#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>
#include <libxml2/libxml/tree.h>

class xml_file_handler {
	
public:
	
	
	xml_file_handler(std::string filePath);
	
	xmlDocPtr createDoc(std::string filePath);
	xmlXPathObjectPtr getNodeSet(std::string xPathString);
	std::vector<xmlNodePtr> getNodesByName(xmlNodePtr startNode, std::string nodeName);
	
	std::vector<std::string> praseVariablePath(std::string variablePath, std::string seperator = "/");
	std::string getAttributeValueFromNode(xmlNode* node, std::string attributeName);
	std::string getContentFromNode(xmlNode* node);

	~xml_file_handler();
	
private:
	FILE * stderr;
	
	xmlDocPtr doc;
	xmlNodePtr cur;    
	
	bool isDocSetted();

};

#endif // XMLFILEHANDLER_H
