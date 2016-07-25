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

#include <list>

#include "mtca_uaadapter.h"
#include "mtca_processvariable.h"
#include "mtca_processscalar.h"
#include "mtca_processarray.h"

#include "ProcessScalar.h"
#include "ControlSystemPVManager.h"
#include "ControlSystemSynchronizationUtility.h"
#include <ControlSystemSynchronizationUtility.h>
#include "PVManager.h"
//#include "IndependentControlCore.h"
#include "TimeStamp.h"

using namespace mtca4u;

void mtca_uaadapter::mtca_uaadapter_constructserver(uint16_t opcuaPort) {
	
	this->server_config = UA_ServerConfig_standard;
	this->server_nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, opcuaPort);
	this->server_config.logger = UA_Log_Stdout;
	this->server_config.networkLayers = &this->server_nl;
	this->server_config.networkLayersSize = 1;
  
	this->mappedServer = UA_Server_new(this->server_config);
	this->baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  
	mtca_namespaceinit_generated(this->mappedServer);
	this->mapSelfToNamespace();
  
}

mtca_uaadapter::mtca_uaadapter(uint16_t opcuaPort) : ua_mapped_class() {
	this->mtca_uaadapter_constructserver(opcuaPort);
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


// Feste Node_ID sind wahrscheinlich bescheiden
void mtca_uaadapter::addVariable(std::string varName, std::string varValue, std::type_info const &valueType, mtca4u::TimeStamp varTimeStamp) {
	this->variables.scalar.push_back(new mtca_processscalar(this->mappedServer, UA_NODEID_NUMERIC(2, 5003), varName, varValue, valueType, varTimeStamp));
}

void mtca_uaadapter::addVariable(std::string varName, std::vector<int32_t> varValue, std::type_info const &valueType,  mtca4u::TimeStamp varTimeStamp) {
	this->variables.array.push_back(new mtca_processarray(this->mappedServer, UA_NODEID_NUMERIC(2, 5004), varName, varValue, valueType, varTimeStamp));
}

void mtca_uaadapter::addConstant(std::string varName, std::string varValue,  std::type_info const &type, mtca4u::TimeStamp varTimeStamp) {
  //this->constants.push_back(new mtca_processvariable(this->mappedServer, UA_NODEID_NUMERIC(2, 5002), varName, varValue, type, varTimeStamp));
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
							UA_QUALIFIEDNAME(1, (char*)"MTCAUAAdapter"), UA_NODEID_NUMERIC(2, UA_NS2ID_MTCAMODULE), oAttr, &icb, &createdNodeId);
	
	// Nodes "Variables" and "Constants" where created on object instantiation, we need these IDs to add new process variables to them...
	//UA_NodeId_copy(nodePairList_getTargetIdBySourceId(this->ownedNodes, UA_NODEID_NUMERIC(2, 5003)), &this->constantsListId);
	
	UA_Server_addReference(this->mappedServer, createdNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_EXPANDEDNODEID_NUMERIC(2, 5001), true);
	UA_Server_addReference(this->mappedServer, createdNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_EXPANDEDNODEID_NUMERIC(2, 5002), true);
		
	return UA_STATUSCODE_GOOD;
}