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
 * Copyright (c) 2019-2023 Andreas Ebner  <Andreas.Ebner@iosb.fraunhofer.de>
 */

#pragma once

#include "ua_typeconversion.h"
#include <open62541/server.h>

#include <iostream>
#include <list>

using namespace std;

namespace ChimeraTK {
  /**
   * @struct UA_NodeId_pair_t
   * @brief This struct contains a NodeId generated from the open62541-stack and the NodeId of the model side
   *
   */
  typedef struct UA_NodeId_pair_t {
    UA_NodeId sourceNodeId; // Model NodeId
    UA_NodeId targetNodeId; // Stack NodeId
  } UA_NodeId_pair;
  typedef std::list<UA_NodeId_pair*> nodePairList;

  /**
   * @struct UA_DataSource_Map_Element_t
   * @brief For generic callback use, this sturct contains the methode pointer and a NodeId of the model. Additinaly a
   * description from controlSystemAdapter can be setted here.
   *
   */
  typedef struct UA_DataSource_Map_Element_t {
    UA_NodeId typeTemplateId;
    UA_NodeId concreteNodeId;
    UA_LocalizedText description; // individual description for every variable
    UA_DataSource dataSource;
  } UA_DataSource_Map_Element;
  typedef std::list<UA_DataSource_Map_Element> UA_DataSource_Map;

#define NODE_PAIR_PUSH(_p_listname, _p_srcId, _p_targetId)                                                             \
  do {                                                                                                                 \
    UA_NodeId_pair* tmp = new UA_NodeId_pair;                                                                          \
    UA_NodeId_copy(&_p_srcId, &tmp->sourceNodeId);                                                                     \
    UA_NodeId_copy(&_p_targetId, &tmp->targetNodeId);                                                                  \
    _p_listname.push_back(tmp);                                                                                        \
  } while(0);

  /**
   * @brief Node function and proxy mapping for new nodes
   *
   * @param objectId targetNodeId
   * @param defintionId sourceNodeId
   * @param handle read and write handel for the node
   *
   * @return UA_StatusCode
   */
  UA_StatusCode ua_mapInstantiatedNodes(UA_NodeId objectId, UA_NodeId definitionId, void* handle);
} // namespace ChimeraTK
