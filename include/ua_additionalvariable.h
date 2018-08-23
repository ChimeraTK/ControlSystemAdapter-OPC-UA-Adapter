/*
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
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

#ifndef MTCA_ADDITIONALVARIABLE_H
#define MTCA_ADDITIONALVARIABLE_H

#include "ua_mapped_class.h"

using namespace std;

/** @class ua_additionalvariable
 *	@brief This class represent a additional variable from <variableMap.xml> in the information model of a OPC UA Server
 *
 *  @author Chris Iatrou, Julian Rahm
 *  @date 22.11.2016
 *
 */
class ua_additionalvariable : ua_mapped_class {
private:
        /// Kurzbeschreibung Variable a
  string value;
        string name;
        string description;

        /** @brief  Get node id of this processvariable instance
        *
        * @return <UA_NodeId> of this processvariable
        */
        UA_NodeId ownNodeId;

        /** @brief  This methode mapped all own nodes into the opcua server
        *
        * @return <UA_StatusCode>
        */
        UA_StatusCode mapSelfToNamespace();

public:

        /** @brief Constructor from <ua_additionalvariable> for generic creation
        *
        * @param server A UA_Server type, with all server specific information from the used server
        * @param basenodeid Parent NodeId from OPC UA information model to add a new UA_ObjectNode
        * @param name Name of the additinal node
        * @param value Value of the additinal node
        * @param description Descripption of the additinal node
        */
        ua_additionalvariable(UA_Server *server, UA_NodeId basenodeid, string name, string value, string description);

        ~ua_additionalvariable();

        UA_DateTime getSourceTimeStamp();

        void setValue(string value);
        string getValue();

};

#endif // MTCA_ADDITIONALVARIABLE_H
