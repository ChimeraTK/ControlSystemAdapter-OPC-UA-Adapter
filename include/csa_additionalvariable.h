// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
// SPDX-FileCopyrightText: 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
// SPDX-FileCopyrightText: 2019-2020 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ua_mapped_class.h"

using namespace std;

namespace ChimeraTK {
  /** @class ua_additionalvariable
   *	@brief This class represent a additional variable from <variableMap.xml> in the information model of a OPC UA Server
   *
   *  @author Chris Iatrou, Julian Rahm
   *  @date 22.11.2016
   *
   */
  class ua_additionalvariable : ua_mapped_class {
   private:
    string value;
    string name;
    string description;

    /** @brief  Get node id of this processvariable instance
     *
     * @return <UA_NodeId> of this processvariable
     */
    UA_NodeId ownNodeId{};

    /** @brief  This method mapped all own nodes into the opcua server
     *
     * @return <UA_StatusCode>
     */
    UA_StatusCode mapSelfToNamespace();

   public:
    /** @brief Constructor from "<ua_additionalvariable>" for generic creation
     *
     * @param server A UA_Server type, with all server specific information from the used server
     * @param basenodeid Parent NodeId from OPC UA information model to add a new UA_ObjectNode
     * @param name Name of the additional node
     * @param value Value of the additional node
     * @param description Description of the additional node
     */
    ua_additionalvariable(UA_Server* server, UA_NodeId basenodeid, string name, string value, string description);

    ~ua_additionalvariable();

    UA_DateTime getSourceTimeStamp();

    static UA_StatusCode ua_readproxy_ua_additionalvariable_getValue(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range, UA_DataValue* value);

    string getValue();
  };
} // namespace ChimeraTK
