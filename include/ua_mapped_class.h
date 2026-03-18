// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
// SPDX-FileCopyrightText: 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ua_map_types.h"

#include <open62541/server.h>

namespace ChimeraTK {
  /** @class ua_mapped_class
   *	@brief This class mapped all information into the opca server
   */
  class ua_mapped_class {
   protected:
    nodePairList ownedNodes;
    UA_NodeId baseNodeId{};

   public:
    UA_Server* mappedServer;

    /** @brief Constructor of the class with parameter
     *
     * @param server opcua server
     * @param baseNodeId Node id from the parent node
     */
    ua_mapped_class(UA_Server* server, UA_NodeId baseNodeId);

    /** @brief Constructor of the class with parameter */
    ua_mapped_class();

    /** @brief Destructor of the class */
    virtual ~ua_mapped_class();

    /** @brief This method mapped all own nodes into the opcua server
     *
     * @return UA_StatusCode
     */
    UA_StatusCode ua_mapSelfToNamespace();

    /** @brief This method unmapped all nodes
     *
     * @return UA_StatusCode
     */
    UA_StatusCode ua_unmapSelfFromNamespace();

    /** @brief Get the SourceTimeStamp from node in the OPC UA server
     * Virtual method which returned Timestamp is setted into the node with the help of the proxy_callback.h
     *
     * @return Returns a UA_DateTime
     */
    virtual UA_DateTime getSourceTimeStamp() = 0;
  };
} // namespace ChimeraTK
