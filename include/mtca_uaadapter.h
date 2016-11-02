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
#include "xml_file_handler.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"

using namespace ChimeraTK;
using namespace std;
  
typedef boost::shared_ptr<ControlSystemPVManager> shCSysPVManager;

struct FolderInfo {
	string folderName;
	UA_NodeId folderNodeId = UA_NODEID_NULL;
	UA_NodeId nextFolderNodeId = UA_NODEID_NULL;
	UA_NodeId prevFolderNodeId = UA_NODEID_NULL;
};

struct ServerConfig {
	string rootFolder = "DefaultRootFolder";
	string descriptionFolder = "No description is set";
	UA_Boolean	UsernamePasswordLogin = UA_FALSE;
	string password	= "DefaultPassword";
	string username = "DefaultUser";
	string applicationName = "DefaultApplication";
	uint16_t opcuaPort = 16660;
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
	
	list<mtca_processvariable *> variables;
	list<mtca_processvariable *> constants;
	
	xml_file_handler *fileHandler;
	
	void mtca_uaadapter_constructserver(uint16_t opcuaPort);
	UA_StatusCode mapSelfToNamespace();
	UA_NodeId createUAFolder(UA_NodeId basenodeid, string folderName, string description = "");
	
public:
	mtca_uaadapter(uint16_t opcuaPort, string configPath);
	~mtca_uaadapter();
	
	UA_NodeId createFolderPath(UA_NodeId basenodeid, vector<string> folderPathVector);
	UA_NodeId createFolder(UA_NodeId basenodeid, string folder, string description = "");
	UA_NodeId existFolderPath(UA_NodeId basenodeid, vector<string> folderPath);
	UA_NodeId existFolder(UA_NodeId basenodeid, string folder);
	
    
	void addVariable(string name, shCSysPVManager mgr);
	void addConstant(string name, shCSysPVManager mgr);
	
	UA_NodeId getOwnNodeId();
	list<mtca_processvariable *> getVariables();
	list<mtca_processvariable *> getConstants();
	
	bool connectionState();
	void workerThread();
	
	void readConfig();
	void readAdditionalNodes();
};

#endif // MTCA_UAADAPTER_H
