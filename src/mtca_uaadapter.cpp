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

void mtca_uaadapter::mtca_uaadapter_constructserver(uint16_t opcuaPort) {
	
    this->server_config = UA_ServerConfig_standard;
    this->server_nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, opcuaPort);
    this->server_config.logger = UA_Log_Stdout;
    this->server_config.networkLayers = &this->server_nl;
    this->server_config.networkLayersSize = 1;
		this->server_config.applicationDescription.applicationName =  UA_LOCALIZEDTEXT((char*)"en_US", (char*)"HZDR OPCUA Server");
		this->server_config.applicationDescription.gatewayServerUri = UA_STRING("GatewayURI");
		this->server_config.applicationDescription.applicationUri = UA_STRING("opc.tcp://localhost");
		this->server_config.applicationDescription.applicationType = UA_APPLICATIONTYPE_SERVER;
		this->server_config.buildInfo.productName = UA_STRING("ControlSystemAdapterOPCUA");
		this->server_config.buildInfo.productUri = UA_STRING("HZDR OPCUA Server");
		this->server_config.buildInfo.manufacturerName = UA_STRING("TU Dresden");
		
    this->mappedServer = UA_Server_new(this->server_config);
    this->baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  
    mtca_namespaceinit_generated(this->mappedServer);
}

mtca_uaadapter::mtca_uaadapter(uint16_t opcuaPort, std::string configFile) : ua_mapped_class() {
    this->mtca_uaadapter_constructserver(opcuaPort);
	
	// XML file handling for variable mapping, just an example...
	this->fileHandler = new XMLFileHandler(configFile);
	
    this->mapSelfToNamespace();
}

mtca_uaadapter::~mtca_uaadapter() {
	if (this->isRunning()) {
		this->doStop();
	}
	
	//UA_Server_delete(this->mappedServer);
	this->fileHandler->~XMLFileHandler();
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
	
	xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//name");
	xmlNodeSetPtr nodeset;
	xmlChar *keyword;
	std::string applicName = "";
	
	if(result) {
		nodeset = result->nodesetval;
		for (int32_t i=0; i < nodeset->nodeNr; i++) {
			keyword = xmlNodeListGetString(this->fileHandler->getDoc(), nodeset->nodeTab[i]->xmlChildrenNode, 1);
			if(varName.compare((std::string)((char*)keyword)) == 0) {
				// get name attribute from <application>-tag
				applicName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i]->parent->parent, "name");
				//std::cout << "Variabe: '" << keyword << "' wird in Application Name: '" << applicName << "' eingetragen." << std::endl;
				// Application Name have to be unique!!!
				UA_NodeId merk = this->existFolderPath(this->ownNodeId, applicName);
				
				std::string ignoreSourceVarPath = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i]->parent, "ignoreSourceVariablePath");
				std::string srcVarName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i]->parent, "sourceVarName");
				// if application folder did not exist -> create one
				folderInfo newFolder;
				if(UA_NodeId_isNull(&merk)) {
					newFolder.folderName = applicName;
					newFolder.folderNodeId = this->createFolderPath(this->ownNodeId, applicName);
					this->folderVector.push_back(newFolder);
					merk = newFolder.folderNodeId;
				}
				
				if(ignoreSourceVarPath.compare("False") == 0) {
					merk = this->createFolderPath(merk, srcVarName);
				}
				
				// FIXME: Maybe tag element position can be different, than "preceding-sibling" should be used or added 
 				std::string xpath = "//application[@name='" + applicName + "']//map//name[text() ='" + (char*)keyword + "']//following-sibling::unrollPaths";
  				xmlXPathObjectPtr searchResult = this->fileHandler->getNodeSet(xpath);
 				// Only one tag unrolePaths is allowed
  				if(searchResult) {
  					xmlNodePtr nodeSetFromResult = searchResult->nodesetval->nodeTab[0];
  					std::string unrolepath = this->fileHandler->getContentFromNode(nodeSetFromResult);
  					if(unrolepath.compare("True") == 0) {
  						// Onöly the first path will be prased and generated
  						xpath = "//application[@name='" + applicName + "']//map//name[text() ='" + (char*)keyword + "']//following-sibling::folderPath";
  						searchResult = this->fileHandler->getNodeSet(xpath);
 						
  						if(searchResult) {
  							nodeSetFromResult = searchResult->nodesetval->nodeTab[0];
  							std::string folderPath = this->fileHandler->getContentFromNode(nodeSetFromResult);
							//std::cout << "Pfad: " << folderPath << std::endl;
  							new mtca_processvariable(this->mappedServer, this->createFolderPath(merk, folderPath), varName, mgr);	
						}
  					}
 					else {
  						new mtca_processvariable(this->mappedServer, merk, varName, mgr);
  					}  					
 				}
				xmlFree(keyword);
			}
		}
		xmlXPathFreeObject (result);
	}
	
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
  
    if (UA_NodeId_equal(&baseNodeId, &createdNodeId) == UA_TRUE) {
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

UA_NodeId mtca_uaadapter::existFolderPath(UA_NodeId basenodeid, std::string folderPath) {
	std::vector<std::string> prasedName = this->fileHandler->praseVariablePath(folderPath);
	UA_NodeId lastNodeId = UA_NODEID_NULL;
	for(int32_t i=0; i < this->folderVector.size(); i++) {
		for(std::string t : prasedName) {
// 			printf("%-16d \n", this->folderVector.at(i).prevFolderNodeId.identifier.numeric);
// 			printf("%-16d \n", basenodeid.identifier.numeric);
			if((this->folderVector.at(i).folderName.compare(t) == 0) && (UA_NodeId_equal(&this->folderVector.at(i).prevFolderNodeId, &basenodeid))) {
				basenodeid = this->folderVector.at(i).folderNodeId;
				//return this->folderVector.at(i).folderNodeId;		
				lastNodeId = basenodeid;
			}
		}
	}
	return lastNodeId;
}

/*
 * 
 */
UA_NodeId mtca_uaadapter::createFolderPath(UA_NodeId basenodeid, std::string folderPath) {
	
	std::vector<std::string>  prasedName = this->fileHandler->praseVariablePath(folderPath);
	
	// Check if path exist
	UA_NodeId toCheckNodeId = existFolderPath(basenodeid, folderPath);
	int32_t starter4Folder = 0;
	UA_NodeId parrentFolderNodeId = UA_NODEID_NULL;
 	if(!UA_NodeId_isNull(&toCheckNodeId)) {
 		basenodeid = toCheckNodeId;
		for(const auto& oneFolderInfo : this->folderVector) {
			if(UA_NodeId_equal(&oneFolderInfo.folderNodeId, &basenodeid)) {
				// remeber on witch position the folder still exist
				for(int32_t m=0; m < prasedName.size(); m++) {
					if(prasedName.at(m).compare(oneFolderInfo.folderName) == 0) {
						// start with the next folder, because the current folder on position m still exist
						if((m+1) < prasedName.size()) {
							starter4Folder = m+1;
						}
						else {
							// if the last folder is reached, there is nothing to do, hence we are done and can return this UA_NodeId
							return oneFolderInfo.folderNodeId;
						}
					}
				}
			}
		}
 	}

	UA_NodeId prevNodeId = basenodeid;
	UA_NodeId nextNodeId = parrentFolderNodeId;
	folderInfo newFolder;
	// use the remembered position to start the loop
	for(int32_t m=starter4Folder; m < prasedName.size(); m++) {
		newFolder.folderName = prasedName.at(m);
		
		newFolder.folderNodeId = this->createUAFolder(prevNodeId, prasedName.at(m));	
		newFolder.prevFolderNodeId = prevNodeId;
		this->folderVector.push_back(newFolder);
 		prevNodeId = newFolder.folderNodeId;
		
	}
	// return last created folder UA_NodeId
	return newFolder.folderNodeId;
}
