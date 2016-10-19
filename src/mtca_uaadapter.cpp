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
#include "xml_file_handler.h"

#include "ChimeraTK/ControlSystemAdapter/ProcessScalar.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/TimeStamp.h"

using namespace ChimeraTK;
using namespace std;

void mtca_uaadapter::mtca_uaadapter_constructserver(uint16_t opcuaPort) {

    this->server_config = UA_ServerConfig_standard;
    this->server_nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, opcuaPort);
    this->server_config.logger = UA_Log_Stdout;
    this->server_config.networkLayers = &this->server_nl;
    this->server_config.networkLayersSize = 1;
		this->server_config.applicationDescription.applicationName =  UA_LOCALIZEDTEXT((char*)"en_US", (char*)"HZDR OPCUA Server");
		this->server_config.applicationDescription.gatewayServerUri = UA_STRING((char*)"GatewayURI");
		this->server_config.applicationDescription.applicationUri = UA_STRING((char*)"opc.tcp://localhost");
		this->server_config.applicationDescription.applicationType = UA_APPLICATIONTYPE_SERVER;
		this->server_config.buildInfo.productName = UA_STRING((char*)"ControlSystemAdapterOPCUA");
		this->server_config.buildInfo.productUri = UA_STRING((char*)"HZDR OPCUA Server");
		this->server_config.buildInfo.manufacturerName = UA_STRING((char*)"TU Dresden");
		
    this->mappedServer = UA_Server_new(this->server_config);
    this->baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  
    mtca_namespaceinit_generated(this->mappedServer);
}

mtca_uaadapter::mtca_uaadapter(uint16_t opcuaPort, std::string configFile) : ua_mapped_class() {
	this->mtca_uaadapter_constructserver(opcuaPort);
	
	// XML file handling for variable mapping, just an example...
	this->fileHandler = new xml_file_handler(configFile);
	
	this->mapSelfToNamespace();
}

mtca_uaadapter::~mtca_uaadapter() {
	if (this->isRunning()) {
		this->doStop();
	}
	
	this->~ipc_managed_object();
	
	//UA_Server_delete(this->mappedServer);
	this->fileHandler->~xml_file_handler();
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
	
	xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//map");
	xmlNodeSetPtr nodeset;
	string srcVarName = "";
	string applicName = "";
	
	// TODO. What happen if application name are not unique?
	if(result) {
		nodeset = result->nodesetval;
		for (int32_t i=0; i < nodeset->nodeNr; i++) {
			srcVarName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "sourceVariableName");
			//keyword = xmlNodeListGetString(this->fileHandler->getDoc(), nodeset->nodeTab[i]->xmlChildrenNode, 1);
			if(varName.compare(srcVarName) == 0) {
				// get name attribute from <application>-tag
				applicName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i]->parent, "name");
				// Check if "rename" is not empty
				string renameVar = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "rename");
			
				// Application Name have to be unique!!!
				UA_NodeId appliFolderNodeId = this->existFolder(this->ownNodeId, applicName);
				folderInfo newFolder;
				if(UA_NodeId_isNull(&appliFolderNodeId)) {
					newFolder.folderName = applicName;
					newFolder.folderNodeId = this->createFolder(this->ownNodeId, applicName);
					this->folderVector.push_back(newFolder);
					appliFolderNodeId = newFolder.folderNodeId;
				}
				
					vector<string> varPathVector;
					vector<xmlNodePtr> nodeVectorUnrollPath = this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "unrollPath");
					string seperator = "";
					bool unrollPathIs = false;
					for(auto nodeUnrollPath: nodeVectorUnrollPath) {
						string shouldUnrollPath = this->fileHandler->getContentFromNode(nodeUnrollPath);
						if(shouldUnrollPath.compare("True") == 0) {
							seperator = seperator + this->fileHandler->getAttributeValueFromNode(nodeUnrollPath, "pathSep");
							unrollPathIs = true;
						}	
					}
					
					if(!seperator.empty()) {
						vector<string> newPathVector = this->fileHandler->praseVariablePath(srcVarName, seperator);
						varPathVector.insert(varPathVector.end(), newPathVector.begin(), newPathVector.end());
					}
						
			
				// assumption last element is name of variable, hence no folder for name is needed
				if(renameVar.compare("") == 0 && !unrollPathIs) {
					renameVar = srcVarName;
				}
				else {
					if(unrollPathIs && renameVar.compare("") == 0) {
						renameVar = varPathVector.at(varPathVector.size()-1);
						varPathVector.pop_back();
					}	
					else {
						if(varPathVector.size() > 0) {
							varPathVector.pop_back();
						}
					}
				}
				std::cout << "Variable: '" << srcVarName << "' wird unter '" << renameVar << "' in Application Name: '" << applicName << "' eingetragen." << std::endl;
				
				vector<xmlNodePtr> nodeVectorFolderPath = this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "folder");
				vector<string> folderPathVector;
				bool createdVar = false;
				for(auto nodeFolderPath: nodeVectorFolderPath) {
						
						string folderPath = this->fileHandler->getContentFromNode(nodeFolderPath);
						if(folderPath.empty() && unrollPathIs) {
							break;
						}
			
						folderPathVector = this->fileHandler->praseVariablePath(folderPath);
						// Create folders
						UA_NodeId newFolderNodeId = appliFolderNodeId;
						if(folderPathVector.size() > 0) {
							newFolderNodeId = this->createFolderPath(newFolderNodeId, folderPathVector);
						}
						
						if(varPathVector.size() > 0) {
							newFolderNodeId = this->createFolderPath(newFolderNodeId, varPathVector);
						}
						new mtca_processvariable(this->mappedServer, newFolderNodeId, srcVarName, renameVar, mgr);
						createdVar = true;
				}	
				
				// in case no <folder> or <unrollpath> is set
				if(!createdVar) {
					UA_NodeId newFolderNodeId = appliFolderNodeId;
					if(varPathVector.size() > 0) {
						newFolderNodeId = this->createFolderPath(newFolderNodeId, varPathVector);
					}
					
					new mtca_processvariable(this->mappedServer, newFolderNodeId, srcVarName, renameVar, mgr);
				}
 			}
		}
	}
	xmlXPathFreeObject (result);	
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

UA_NodeId mtca_uaadapter::createUAFolder(UA_NodeId basenodeid, std::string folderName) {
	// FIXME: Check if folder name a possible name or should it be escaped (?!"§%-:, etc)
	UA_StatusCode retval = UA_STATUSCODE_GOOD;
	UA_NodeId createdNodeId = UA_NODEID_NULL;
  
	if(UA_NodeId_equal(&baseNodeId, &createdNodeId) == UA_TRUE) {
		return createdNodeId; // Something went UA_WRING (initializer should have set this!)
	}
    
    // Create our toplevel instance
    UA_ObjectAttributes oAttr; 
		UA_ObjectAttributes_init(&oAttr);
    // Classcast to prevent Warnings
    oAttr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)folderName.c_str());
    oAttr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)folderName.c_str());    
	
    UA_INSTATIATIONCALLBACK(icb);
    UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1,0),
                             basenodeid, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                             UA_QUALIFIEDNAME(1, (char*)folderName.c_str()), UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), oAttr, &icb, &createdNodeId);
    
	return createdNodeId;
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
    
	
	this->ownNodeId = createdNodeId;
	// Nodes "Variables" and "Constants" where created on object instantiation, we need these IDs to add new process variables to them...
	UA_NodeId_copy(nodePairList_getTargetIdBySourceId(this->ownedNodes, UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_CONSTANTS)), &this->constantsListId);
	UA_NodeId_copy(nodePairList_getTargetIdBySourceId(this->ownedNodes, UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLES)), &this->variablesListId);
		
    return UA_STATUSCODE_GOOD;
}

UA_NodeId mtca_uaadapter::getOwnNodeId() {
	return this->ownNodeId;
}

UA_NodeId mtca_uaadapter::existFolderPath(UA_NodeId basenodeid, std::vector<string> folderPath) {
	UA_NodeId lastNodeId = basenodeid;
	for(std::string t : folderPath) {
		lastNodeId = this->existFolder(lastNodeId, t);
		if(UA_NodeId_isNull(&lastNodeId)) {
			return UA_NODEID_NULL;
		}
	}
	return lastNodeId;
}

UA_NodeId mtca_uaadapter::existFolder(UA_NodeId basenodeid, string folder) {
	UA_NodeId lastNodeId = UA_NODEID_NULL;
	for(int32_t i=0; i < this->folderVector.size(); i++) {
		if((this->folderVector.at(i).folderName.compare(folder) == 0) && (UA_NodeId_equal(&this->folderVector.at(i).prevFolderNodeId, &basenodeid))) {
			return this->folderVector.at(i).folderNodeId;
		}
	}
	return UA_NODEID_NULL;
}

/*
 * 
 */
UA_NodeId mtca_uaadapter::createFolderPath(UA_NodeId basenodeid, std::vector<string> folderPath) {
		
 	if(UA_NodeId_isNull(&basenodeid)){
 		return UA_NODEID_NULL;
 	}
	
	// Check if path exist
	UA_NodeId toCheckNodeId = existFolderPath(basenodeid, folderPath);
	int32_t starter4Folder = 0;
	UA_NodeId nextNodeId = basenodeid;
	UA_NodeId startNodeId = basenodeid;
	bool setted = false;
 	if(UA_NodeId_isNull(&toCheckNodeId)) {
		// Check if path exist partly 
		for(int32_t m=0; m < folderPath.size(); m++) {
			for(int32_t i=0; i < this->folderVector.size(); i++) {
				// get correct folder NodeId from first folderPath element
				if(!setted && (folderPath.at(m).compare(this->folderVector.at(i).folderName) == 0) && (UA_NodeId_equal(&this->folderVector.at(i).prevFolderNodeId, &nextNodeId)) && ((m+1) < folderPath.size())) {
					// remeber on witch position the folder still exist
					setted = true;
					starter4Folder = m+1;
					nextNodeId = this->folderVector.at(i).folderNodeId;
				}
				if(setted) {
					break;
				}
			}
			setted = false;
		}
	}
	else {
		// Path exist nothing to do except
		return toCheckNodeId;
	}

	UA_NodeId prevNodeId = nextNodeId;
	// use the remembered position to start the loop
	for(int32_t m=starter4Folder; m < folderPath.size(); m++) {
		prevNodeId = this->createFolder(prevNodeId, folderPath.at(m));		
	}
	// return last created folder UA_NodeId
	return prevNodeId;
}

UA_NodeId mtca_uaadapter::createFolder(UA_NodeId basenodeid, string folderName) {
	
	if(UA_NodeId_isNull(&basenodeid)){
		return UA_NODEID_NULL;
	}
	
	// Check if path exist
	UA_NodeId toCheckNodeId = existFolder(basenodeid, folderName);
	folderInfo newFolder;
 	if(UA_NodeId_isNull(&toCheckNodeId)) {
		newFolder.folderName = folderName;
		newFolder.folderNodeId = this->createUAFolder(basenodeid, folderName);	
		newFolder.prevFolderNodeId = basenodeid;
		this->folderVector.push_back(newFolder);
	}

	return newFolder.folderNodeId;
}
