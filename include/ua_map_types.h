// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
// SPDX-FileCopyrightText: 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
// SPDX-FileCopyrightText: 2019-2023 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <open62541/server.h>

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
   * @brief For generic callback use, this struct contains the method pointer and a NodeId of the model. Additionally a
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
   * @param definitionId sourceNodeId
   * @param handle read and write handel for the node
   *
   * @return UA_StatusCode
   */
  UA_StatusCode ua_mapInstantiatedNodes(UA_NodeId objectId, UA_NodeId definitionId, void* handle);
} // namespace ChimeraTK
