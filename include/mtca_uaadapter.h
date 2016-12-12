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

#ifndef MTCA_UAADAPTER_H
#define MTCA_UAADAPTER_H

#include <vector>

#include "ua_mapped_class.h"
#include "ipc_managed_object.h"
#include "mtca_processvariable.h"
#include "mtca_additionalvariable.h"
#include "xml_file_handler.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"

using namespace ChimeraTK;
using namespace std;
  
/** @class FolderInfo
 *	@brief This struct represents a folder in OPCUA with its own node id and with his parent and child node id. For this FolderInfo are a singly-linked list 
 *   
 *  @author Chris Iatrou, Julian Rahm
 *  @date 03.12.2016
 * 
 */
struct FolderInfo {
	string folderName;
	UA_NodeId folderNodeId = UA_NODEID_NULL;
	UA_NodeId nextFolderNodeId = UA_NODEID_NULL;
	UA_NodeId prevFolderNodeId = UA_NODEID_NULL;
};

/** @class ServerConfig
 *	@brief This struct represents a server config. If the hole config file is prased, all information will be stored in die struct. 
 *   
 *  @author Chris Iatrou, Julian Rahm
 *  @date 03.12.2016
 * 
 */
struct ServerConfig {
	string rootFolder = "DefaultRootFolder";
	string descriptionFolder = "No description is set";
	UA_Boolean	UsernamePasswordLogin = UA_FALSE;
	string password;
	string username;
	string applicationName;
	uint16_t opcuaPort;
};

class mtca_uaadapter : ua_mapped_class, public ipc_managed_object {
private:	 
	UA_ServerConfig          	server_config;
	UA_ServerNetworkLayer    	server_nl;
	UA_Logger                	logger;
	
	UA_NodeId                	variablesListId;
	UA_NodeId                	constantsListId;
	
	vector<FolderInfo> 				folderVector;	
	
	UA_NodeId									ownNodeId;
	
	ServerConfig 							serverConfig;
	
	vector<mtca_processvariable *> variables;
	vector<mtca_processvariable *> constants;
	vector<mtca_additionalvariable *> additionalVariables;
	vector<mtca_processvariable *> mappedVariables;
	
	xml_file_handler *fileHandler;
	
	void mtca_uaadapter_constructserver();
	UA_StatusCode mapSelfToNamespace();
	UA_NodeId createUAFolder(UA_NodeId basenodeid, string folderName, string description = "");
	
public:
	mtca_uaadapter(string configPath);
	~mtca_uaadapter();
	
	UA_DateTime getSourceTimeStamp();
	
	UA_NodeId createFolderPath(UA_NodeId basenodeid, vector<string> folderPathVector);
	UA_NodeId createFolder(UA_NodeId basenodeid, string folder, string description = "");
	UA_NodeId existFolderPath(UA_NodeId basenodeid, vector<string> folderPath);
	UA_NodeId existFolder(UA_NodeId basenodeid, string folder);
	
    
	void addVariable(string name, boost::shared_ptr<ControlSystemPVManager> csManager);
	void addConstant(string name, boost::shared_ptr<ControlSystemPVManager> csManager);
	
	UA_NodeId getOwnNodeId();
	vector<mtca_processvariable *> getVariables();
	vector<mtca_processvariable *> getConstants();
	
	void workerThread();
	
	void readConfig();
	void readAdditionalNodes();
	
	vector<string> getAllNotMappableVariablesNames();
};

#endif // MTCA_UAADAPTER_H
