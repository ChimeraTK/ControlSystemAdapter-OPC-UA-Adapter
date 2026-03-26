// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
// SPDX-FileCopyrightText: 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
// SPDX-FileCopyrightText: 2019-2023 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ua_map_types.h"

#include <iostream>

using namespace std;

namespace ChimeraTK {
  // ToDo move function to remove proxies file
  UA_StatusCode ua_mapInstantiatedNodes(UA_NodeId objectId, UA_NodeId definitionId, void* handle) {
    auto* lst = static_cast<nodePairList*>(handle);

    auto* thisNode = new UA_NodeId_pair;

    UA_NodeId_copy(&definitionId, &thisNode->sourceNodeId);
    UA_NodeId_copy(&objectId, &thisNode->targetNodeId);
    lst->push_back(std::move(thisNode));

    return UA_STATUSCODE_GOOD;
  }
} // namespace ChimeraTK
