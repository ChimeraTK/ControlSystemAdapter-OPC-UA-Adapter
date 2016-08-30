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

extern "C" {
#include "unistd.h"
#include "mtca_namespaceinit_generated.h" // Output des pyUANamespacecompilers
}
#include "csa_config.h"

#include <list>

#include "mtca_uaadapter.h"
#include "mtca_processvariable.h"

#include "ChimeraTK/ControlSystemAdapter/ProcessScalar.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/TimeStamp.h"

using namespace ChimeraTK;

void mtca_uaadapter::mtca_uaadapter_constructserver(uint16_t opcuaPort) {
    
    this->server_config = UA_ServerConfig_standard;
    this->server_nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, opcuaPort);
    this->server_config.logger = UA_Log_Stdout;
    this->server_config.networkLayers = &this->server_nl;
    this->server_config.networkLayersSize = 1;
  
    this->mappedServer = UA_Server_new(this->server_config);
    this->baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  
    mtca_namespaceinit_generated(this->mappedServer);
}

mtca_uaadapter::mtca_uaadapter(uint16_t opcuaPort) : ua_mapped_class() {
    this->mtca_uaadapter_constructserver(opcuaPort);
    this->mapSelfToNamespace();
}

mtca_uaadapter::~mtca_uaadapter() {
    if (this->isRunning()) {
        this->doStop();
    }
    UA_Server_delete(this->mappedServer);
}

void mtca_uaadapter::workerThread() {
    if (this->mappedServer == nullptr) {
        return;
    }
    
    UA_Boolean runUAServer = UA_TRUE;
    thread *serverThread = new std::thread(UA_Server_run, this->mappedServer, &runUAServer);
  
    while (runUAServer == UA_TRUE) {
        if (! this->isRunning()) {
            runUAServer = UA_FALSE;
        }
        sleep(1);
    }
    if (serverThread->joinable()) {
        serverThread->join();
    }
  
    delete serverThread;
}

void mtca_uaadapter::addVariable(std::string varName, shCSysPVManager mgr) {
    this->variables.push_back(new mtca_processvariable(this->mappedServer, this->variablesListId, varName, mgr));
}

void mtca_uaadapter::addConstant(std::string varName, shCSysPVManager mgr) {
    this->constants.push_back(new mtca_processvariable(this->mappedServer, this->constantsListId, varName, mgr));
}

std::list<mtca_processvariable *> mtca_uaadapter::getVariables() {
	return this->variables;
}

std::list<mtca_processvariable *> mtca_uaadapter::getConstants() {
	return this->constants;
}

UA_StatusCode mtca_uaadapter::mapSelfToNamespace() {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_NodeId createdNodeId = UA_NODEID_NULL;
  
    if (UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE) {
        return 0; // Something went UA_WRING (initializer should have set this!)
    }
    
    // Create our toplevel instance
    UA_ObjectAttributes oAttr; 
    // Classcast to prevent Warnings
    oAttr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"MTCA4U Process Adapter");
    oAttr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)"MTCA4U Process Adapter depicting current process variables");
    
    UA_INSTATIATIONCALLBACK(icb);
    UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1,0),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, (char*)"MTCAUAAdapter"), UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_MTCAMODULE), oAttr, &icb, &createdNodeId);
    
    // Nodes "Variables" and "Constants" where created on object instantiation, we need these IDs to add new process variables to them...
    UA_NodeId_copy(nodePairList_getTargetIdBySourceId(this->ownedNodes, UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_CONSTANTS)), &this->constantsListId);
    UA_NodeId_copy(nodePairList_getTargetIdBySourceId(this->ownedNodes, UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLES)), &this->variablesListId);
    
    return UA_STATUSCODE_GOOD;
}