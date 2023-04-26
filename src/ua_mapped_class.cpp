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
 * Copyright (c) 2021-2023 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
 */

#include "ua_mapped_class.h"
#include <iostream>

using namespace std;

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
  this->mappedClient = nullptr;
  UA_NodeId_clear(&this->baseNodeId);
}

UA_StatusCode ua_mapped_class::ua_mapSelfToNamespace() {
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  UA_NodeId nullId = UA_NODEID_NULL;
  if(UA_NodeId_equal(&this->baseNodeId, &UA_NODEID_NULL)) return -1;
  if(this->mappedServer == nullptr) return -2;

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
