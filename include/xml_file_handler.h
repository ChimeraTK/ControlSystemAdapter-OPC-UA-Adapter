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

class xml_file_handler {
	
public:
	
	/** @brief The constructor of the class creates a doc pointer depending on the file path
	 * 
	 * @param filePath  Path to a xml file which you want to read
	*/
	xml_file_handler(std::string filePath);
	
	/** @brief This methode set a document pointer to the file it ist given by the file path
	* 
	* @param filePath Path to a xml file which you want to read
	* 
	* @return Returns true if a document is setted, if not it returns false
	*/
	bool createDoc(std::string filePath);
	
	/** @brief This Methode check if a document is currently setted
	* 
	* @return True or false depending if a document is set or not
	*/
	bool isDocSetted();
	
	/** @brief This methode return a pointer of a xPath element depending of the given xPathString
	* 
	* @param xPathString A xPath string we seaching for
	* 
	* @return Returns a element pointer if some was found, in other cases it will return NULL
	*/
	xmlXPathObjectPtr getNodeSet(std::string xPathString);
	
	/** @brief This methode return a list of all nodes with the given name nodeName starting by the given startNode
	* 
	* @param startNode Start node, all child nodes will be searched by the given nodeName
	* @param nodeName Is the name of the xml tag, we use to search for
	* 
	* @return Returns a list of all pointer from founded child nodes with nodeName
	*/
	std::vector<xmlNodePtr> getNodesByName(xmlNodePtr startNode, std::string nodeName);
	/** @brief This methode splitt a given string bey the given seperators. You can use the seperator as string of seperators like "_/&" all characters will be used as single seperator
	* 
	* @param variablePath String which you want to seperate by the seperator
	* @param seperator String of, if you want different, seperators, whicht this methode use to split the string
	* 
	* @return Returns a vector of every single splitet word
	*/
	std::vector<std::string> praseVariablePath(std::string variablePath, std::string seperator = "/");
	
	/** @brief This methode returns a value of the given attribute from the given node you want to know
	* 
	* @param node Node with the attribute of interest
	* @param attributeName Name of the wanted attribute
	* 
	* @return Returns a string of the attribute
	*/
	std::string getAttributeValueFromNode(xmlNode* node, std::string attributeName);
	
	/** @brief This methode returns the value between a xml tag. For example \<tagelement>content\</tagelement> the returned value ist "content".
	* 
	* @param node Node with the content of interest
	* 
	* @return Returns a string of the inner content
	*/
	std::string getContentFromNode(xmlNode* node);

	/** @brief Destructor of the class, frees the document and clean the parser.
	* 
	*/
	~xml_file_handler();
	
private:	
	xmlDocPtr doc;
	xmlNodePtr cur;    
};

#endif // XMLFILEHANDLER_H
