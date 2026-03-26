// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
// SPDX-FileCopyrightText: 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
// SPDX-FileCopyrightText: 2021-2023 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ua_mapped_class.h"

#include <iostream>

using namespace std;

namespace ChimeraTK {
  ua_mapped_class::ua_mapped_class() {
    this->baseNodeId = UA_NODEID_NUMERIC(0, 0);
    this->mappedServer = nullptr;

    ua_mapSelfToNamespace();
  }

  ua_mapped_class::ua_mapped_class(UA_Server* server, UA_NodeId baseNodeId) {
    UA_NodeId_copy(&baseNodeId, &this->baseNodeId);
    this->mappedServer = server;

    ua_mapSelfToNamespace();
  }

  ua_mapped_class::~ua_mapped_class() {
    this->ua_unmapSelfFromNamespace();
    this->mappedServer = nullptr;
    UA_NodeId_clear(&this->baseNodeId);
  }

  UA_StatusCode ua_mapped_class::ua_mapSelfToNamespace() {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_NodeId nullId = UA_NODEID_NULL;
    if(UA_NodeId_equal(&this->baseNodeId, &nullId)) {
      return -1;
    }
    if(this->mappedServer == nullptr) {
      return -2;
    }

    return retval;
  }

  UA_StatusCode ua_mapped_class::ua_unmapSelfFromNamespace() {
    while(!this->ownedNodes.empty()) {
      UA_NodeId_pair* p = *(this->ownedNodes.begin());
      // Node is deleted by UA_Server_delete
      UA_NodeId_clear(&p->sourceNodeId);
      UA_NodeId_clear(&p->targetNodeId);
      this->ownedNodes.remove(p);
      delete p;
    }
    return UA_STATUSCODE_GOOD;
  }
} // namespace ChimeraTK
