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

#ifndef UA_MAPPED_CLASS_H
#define UA_MAPPED_CLASS_H

extern "C" {
#include <open62541/server.h>
}

#include "ua_map_types.h"

/** @class ua_mapped_class
 *	@brief This class mapped all inforamtion into the opca server
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
  *
  * @return Returns a element pointer if some was found, in other cases it will return NULL
  */
  ua_mapped_class(UA_Server* server, UA_NodeId baseNodeId);

  /** @brief Constructor of the class with parameter */
  ua_mapped_class();

  /** @brief Destructor of the class */
  ~ua_mapped_class();

  /** @brief This methode mapped all own nodes into the opcua server
  *
  * @return UA_StatusCode
  */
  UA_StatusCode ua_mapSelfToNamespace();

  /** @brief This methode unmapped all nodes
  *
  * @return UA_StatusCode
  */
  UA_StatusCode ua_unmapSelfFromNamespace();

  /** @brief This methode return a pointer of a xPath element depending of the given xPathString
  *
  * @param map	Contains all Node from the class the should instantiated into the server
  * @param scrClass Pointer to our class instance
  *
  * @return UA_StatusCode
  */
  UA_StatusCode ua_mapDataSources(void* srcClass, UA_DataSource_Map* map);

  /** @brief Get the SourceTimeStamp from node in the OPC UA server
   * Virtual methode which returned Timestamp is setted into the node with the help of the proxy_callback.h
   *
   * @return Returns a UA_DateTime
   */
  virtual UA_DateTime getSourceTimeStamp() = 0;
};

#endif // UA_MAPPED_CLASS_H
