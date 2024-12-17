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

#pragma once

#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>

#include <boost/tokenizer.hpp>

#include <iostream>
#include <vector>

namespace ChimeraTK {
  /** @class xml_file_handler
   *	@brief This class support any file interaction with a xml file.
   *
   * 	To use all config/mapping xml-files in a confortable way this class provides some useful methodes to read specal
   *nodes, get attributes and so one
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
    xml_file_handler(const std::string& filePath);

    /** @brief This methode set a document pointer to the file it ist given by the file path
     *
     * @param filePath Path to a xml file which you want to read
     *
     * @return Returns true if a document is setted, if not it returns false
     */
    bool createDoc(const std::string& filePath);

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
    xmlXPathObjectPtr getNodeSet(const std::string& xPathString);

    /** @brief This methode return a list of all nodes with the given name nodeName starting by the given startNode
     *
     * @param startNode Start node, all child nodes will be searched by the given nodeName
     * @param nodeName Is the name of the xml tag, we use to search for
     *
     * @return Returns a list of all pointer from founded child nodes with nodeName
     */
    static std::vector<xmlNodePtr> getNodesByName(xmlNodePtr startNode, const std::string& nodeName);
    /** @brief This methode splitt a given string bey the given seperators. You can use the seperator as string of
     * seperators like "_/&" all characters will be used as single seperator
     *
     * @param variablePath String which you want to seperate by the seperator
     * @param seperator String of, if you want different, seperators, whicht this methode use to split the string
     *
     * @return Returns a vector of every single splitet word
     */
    static std::vector<std::string> parseVariablePath(
        const std::string& variablePath, const std::string& seperator = "/");

    /** @brief This methode returns a value of the given attribute from the given node you want to know
     *
     * @param node Node with the attribute of interest
     * @param attributeName Name of the wanted attribute
     *
     * @return Returns a string of the attribute
     */
    static std::string getAttributeValueFromNode(xmlNode* node, const std::string& attributeName);

    /** @brief This methode returns the value between a xml tag. For example \<tagelement>content\</tagelement> the
     * returned value ist "content".
     *
     * @param node Node with the content of interest
     *
     * @return Returns a string of the inner content
     */
    static std::string getContentFromNode(xmlNode* node);

    /** @brief Destructor of the class, frees the document and clean the parser.
     *
     */
    ~xml_file_handler();

   private:
    xmlDocPtr doc{};
    xmlNodePtr cur{};
  };
} // namespace ChimeraTK
