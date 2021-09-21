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
 * Copyright (c) 2019-2020 Andreas Ebner  <Andreas.Ebner@iosb.fraunhofer.de>
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
  ua_additionalvariable(UA_Server* server, UA_NodeId basenodeid, string name, string value, string description);

  ~ua_additionalvariable();

  UA_DateTime getSourceTimeStamp();

  static UA_StatusCode ua_readproxy_ua_additionalvariable_getValue(UA_Server* server, const UA_NodeId* sessionId,
                                                                   void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
                                                                   const UA_NumericRange* range, UA_DataValue* value);

  string getValue();
};

#endif // MTCA_ADDITIONALVARIABLE_H
