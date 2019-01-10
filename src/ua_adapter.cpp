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
 */

extern "C" {
#include "unistd.h"
#include "csa_namespaceinit_generated.h" // Output des pyUANamespacecompilers
#include <stdio.h>
#include <stdlib.h>
}

#include <thread>
#include <future>
#include <functional>     // std::ref

#include "csa_config.h"

#include "ua_adapter.h"
#include "ua_processvariable.h"
#include "ua_additionalvariable.h"
#include "xml_file_handler.h"
#include "ipc_manager.h"
#include "ua_proxies.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"

using namespace ChimeraTK;
using namespace std;

ua_uaadapter::ua_uaadapter(string configFile) : ua_mapped_class() {
        this->fileHandler = new xml_file_handler(configFile);
        this->readConfig();

        this->constructServer();

        this->mapSelfToNamespace();

        if(!configFile.empty()) {
                this->readAdditionalNodes();
        }
}

ua_uaadapter::~ua_uaadapter() {

        if (this->isRunning()) {
                this->doStop();
        }
        //UA_Server_delete(this->mappedServer);
        this->fileHandler->~xml_file_handler();
        for(auto ptr : variables) delete ptr;
        for(auto ptr : additionalVariables) delete ptr;
        for(auto ptr : mappedVariables) delete ptr;

}

void ua_uaadapter::constructServer() {

    this->server_config = UA_ServerConfig_standard;
    this->server_nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, this->serverConfig.opcuaPort);
    this->server_config.logger = UA_Log_Stdout;
    this->server_config.networkLayers = &this->server_nl;
    this->server_config.networkLayersSize = 1;

                this->server_config.enableUsernamePasswordLogin = this->serverConfig.UsernamePasswordLogin;
                this->server_config.enableAnonymousLogin = !this->serverConfig.UsernamePasswordLogin;

    UA_UsernamePasswordLogin* usernamePasswordLogins = new UA_UsernamePasswordLogin; //!< Brief description after the member
                usernamePasswordLogins->password = UA_STRING((char*)this->serverConfig.password.c_str());
                usernamePasswordLogins->username = UA_STRING((char*)this->serverConfig.username.c_str());
                this->server_config.usernamePasswordLogins = usernamePasswordLogins;
                this->server_config.usernamePasswordLoginsSize = (size_t)(usernamePasswordLogins->password.length + usernamePasswordLogins->username.length);
                this->server_config.applicationDescription.applicationName =  UA_LOCALIZEDTEXT((char*)"en_US", (char*)this->serverConfig.applicationName.c_str());
                this->server_config.applicationDescription.gatewayServerUri = UA_STRING((char*)"GatewayURI");
                this->server_config.applicationDescription.applicationUri = UA_STRING((char*)"opc.tcp://localhost");
                this->server_config.applicationDescription.applicationType = UA_APPLICATIONTYPE_SERVER;
                this->server_config.buildInfo.productName = UA_STRING((char*)"csa_opcua_adapter");
                this->server_config.buildInfo.productUri = UA_STRING((char*)"HZDR OPCUA Server");
                this->server_config.buildInfo.manufacturerName = UA_STRING((char*)"TU Dresden - Professur für Prozessleittechnik");

    this->mappedServer = UA_Server_new(this->server_config);
                this->baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);

                csa_namespaceinit_generated(this->mappedServer);
}

void ua_uaadapter::readConfig() {

        string xpath = "//config";
        xmlXPathObjectPtr result = this->fileHandler->getNodeSet(xpath);
        string placeHolder = "";
        if(result) {
                xmlNodeSetPtr nodeset = result->nodesetval;
                // There should be only one <config>-Tag in config file
                if(nodeset->nodeNr > 1) {
                        //cout << "More than one <config>-Tag was found in config file. Please provide only one Tag." << endl;
                        throw std::runtime_error ("To many <config>-Tags in config file");
                }

                placeHolder = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "rootFolder");
                if(!placeHolder.empty()) {
                        this->serverConfig.rootFolder = placeHolder;
                }

                placeHolder = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "description");
                if(!placeHolder.empty()) {
                        this->serverConfig.descriptionFolder = placeHolder;
                }
        }

        result = this->fileHandler->getNodeSet(xpath + "//login");
        if(result) {
                xmlNodeSetPtr nodeset = result->nodesetval;
                this->serverConfig.UsernamePasswordLogin = UA_TRUE;
                placeHolder = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "password");
                if(!placeHolder.empty()) {
                        this->serverConfig.password = placeHolder;
                }

                placeHolder = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "username");
                if(!placeHolder.empty()) {
                        this->serverConfig.username = placeHolder;
                }
        }
        else {
                        this->serverConfig.UsernamePasswordLogin = UA_FALSE;
        }

        result = this->fileHandler->getNodeSet(xpath +"//serverConfig");
        if(result) {
                xmlNodeSetPtr nodeset = result->nodesetval;
                string opcuaPort = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "port");
                if(opcuaPort.compare("") != 0) {
                        this->serverConfig.opcuaPort = std::stoi(opcuaPort);
                }
                else {
                        cout << "No 'port'-Attribute in config file is set. Use default Port: 16664" << endl;
                }

                placeHolder = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "applicationName");
                if(placeHolder.compare("") != 0) {
                        this->serverConfig.applicationName = placeHolder;
                }
                else {
                        cout << "No 'applicationName'-Attribute is set in config file. Use default Applicationname." << endl;
                }
        }
        else {
                cout << "No <serverConfig>-Tag in config file. Use default port 16664 and application name configuration." << endl;
        }
}

void ua_uaadapter::readAdditionalNodes() {
        xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//additionalNodes");
        if(result) {
                xmlNodeSetPtr nodeset = result->nodesetval;
                for (int32_t i=0; i < nodeset->nodeNr; i++) {

                        string folderName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "folderName");
                        string folderDescription = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "description");
                        if(folderName.empty()) {
                                cout << "There is no folder name specified, ignore <additionalNode>-Element. Please set a name" << endl;
                        }
                        else {
                                UA_NodeId folderNodeId = this->createFolder(this->ownNodeId, folderName, folderDescription);

                                vector<xmlNodePtr> variableNodeList = this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "variable");

                                for(auto variableNode: variableNodeList) {
                                        string name = this->fileHandler->getAttributeValueFromNode(variableNode, "name").c_str();
                                        string value = this->fileHandler->getAttributeValueFromNode(variableNode, "value");
                                        string description = this->fileHandler->getAttributeValueFromNode(variableNode, "description");
                                        this->additionalVariables.push_back(new ua_additionalvariable(this->mappedServer, folderNodeId, name, value, description));
                                }
                        }
                }
        }
}

void ua_uaadapter::workerThread() {
        if (this->mappedServer == nullptr) {
                return;
        }

        cout << "Starting the server worker thread" << endl;

        UA_Server_run_startup(this->mappedServer);

        while(this->isRunning()) {
            UA_Server_run_iterate(this->mappedServer, true);
        }

        UA_Server_run_shutdown(this->mappedServer);

        cout << "Stopped the server worker thread" << endl;
}

void ua_uaadapter::addVariable(std::string varName, boost::shared_ptr<ControlSystemPVManager> csManager) {

        ua_processvariable *processvariable = new ua_processvariable(this->mappedServer, this->variablesListId, varName, csManager);
        this->variables.push_back(processvariable);

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
                                string engineeringUnit = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "engineeringUnit");
                                string description = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "description");

                                // Application Name have to be unique!!!
                                UA_NodeId appliFolderNodeId = this->existFolder(this->ownNodeId, applicName);
                                FolderInfo newFolder;
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
                                        // std::cout << "Variable '" << srcVarName << "' renamed in '" << renameVar << "' and listed in folder '" << applicName << "'." << std::endl;
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
                                        // std::cout << "Variable '" << srcVarName << "' listed in folder '" << applicName << "'." << std::endl;
                                }



                                vector<xmlNodePtr> nodeVectorFolderPath = this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "folder");
                                vector<string> folderPathVector;
                                bool createdVar = false;
                                UA_NodeId newFolderNodeId = UA_NODEID_NULL;
                                vector<UA_NodeId> mappedVariables;
                                for(auto nodeFolderPath: nodeVectorFolderPath) {

                                                string folderPath = this->fileHandler->getContentFromNode(nodeFolderPath);
                                                if(folderPath.empty() && unrollPathIs) {
                                                        break;
                                                }

                                                folderPathVector = this->fileHandler->praseVariablePath(folderPath);
                                                // Create folders
                                                newFolderNodeId = appliFolderNodeId;
                                                if(folderPathVector.size() > 0) {
                                                        newFolderNodeId = this->createFolderPath(newFolderNodeId, folderPathVector);
                                                }

                                                if(varPathVector.size() > 0) {
                                                        newFolderNodeId = this->createFolderPath(newFolderNodeId, varPathVector);
                                                }
                                                mappedVariables.push_back(newFolderNodeId);
                                                createdVar = true;
                                }

                                // in case no <folder> or <unrollpath> is set
                                if(!createdVar) {
                                        newFolderNodeId = appliFolderNodeId;

                                        if(varPathVector.size() > 0) {
                                                mappedVariables.push_back(this->createFolderPath(newFolderNodeId, varPathVector));
                                        }
                                        else {
                                                // No <folder>
                                                mappedVariables.push_back(appliFolderNodeId);
                                        }
                                }

                                // Create all nessesary mapped ObjectVaraibles with inner variables (reference or attribute, depending attributes are set (engineeringUnit, dexcription)
                                for(auto objectNodeId:mappedVariables) {
                                        UA_NodeId createdNodeId = UA_NODEID_NULL;

                                        // Create our new "Value" Variable
                                        UA_ObjectAttributes oAttr;
                                        UA_ObjectAttributes_init(&oAttr);
                                        oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", renameVar.c_str());
                                        oAttr.description = UA_LOCALIZEDTEXT_ALLOC("en_US", description.c_str());

                                        UA_INSTATIATIONCALLBACK(icb);
                                        UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1, 0),
                                                                                                        objectNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                                                                                        UA_QUALIFIEDNAME_ALLOC(1, renameVar.c_str()), UA_NODEID_NULL, oAttr, &icb, &createdNodeId);

                                        UA_ExpandedNodeId *targetNodeId = UA_ExpandedNodeId_new();
                                        targetNodeId->nodeId = createdNodeId;

                                        UA_BrowseDescription bDesc;
                                        UA_BrowseDescription_init(&bDesc);
                                        bDesc.browseDirection = UA_BROWSEDIRECTION_FORWARD;
                                        bDesc.includeSubtypes = false;
                                        bDesc.nodeClassMask = UA_NODECLASS_VARIABLE;
                                        bDesc.nodeId = processvariable->getOwnNodeId();
                                        bDesc.resultMask = UA_BROWSERESULTMASK_ALL;

                                        UA_BrowseResult bRes;
                                        UA_BrowseResult_init(&bRes);

                                        UA_VariableAttributes vAttr;
                                        UA_VariableAttributes_init(&vAttr);
                                        vAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);
                                        vAttr.accessLevel = UA_ACCESSLEVELMASK_WRITE^UA_ACCESSLEVELMASK_READ;
                                        vAttr.userAccessLevel = UA_ACCESSLEVELMASK_WRITE^UA_ACCESSLEVELMASK_READ;
                                        vAttr.valueRank = -1;


                                        bRes = UA_Server_browse(this->mappedServer, 10, &bDesc);

                                        for(uint32_t i=0; i < bRes.referencesSize; i++) {
                                                UA_NodeId newNodeId = UA_NODEID_NULL;

                                                UA_String varName = UA_String_fromChars("EngineeringUnit");
                                                if(UA_String_equal(&bRes.references[i].browseName.name, &varName) && !engineeringUnit.empty()) {
                                                        vAttr.description = UA_LOCALIZEDTEXT((char*)"en_US",(char*) "EngineeringUnit");
                                                        vAttr.displayName = UA_LOCALIZEDTEXT((char*)"en_US",(char*) "EngineeringUnit");

                                                        UA_String engineringUnit = UA_String_fromChars(engineeringUnit.c_str());
                                                        UA_Variant_setScalar(&vAttr.value, &engineringUnit, &UA_TYPES[UA_TYPES_STRING]);
                                                        UA_Server_addVariableNode(this->mappedServer, UA_NODEID_NUMERIC(1, 0), createdNodeId,
                                                                                                                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(1, (char*) "EngineeringUnit"),
                                                                                                                                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), vAttr, &icb, &newNodeId);
                                                }

                                                varName = UA_String_fromChars("Description");
                                                if(UA_String_equal(&bRes.references[i].browseName.name, &varName) && !description.empty()) {
                                                        vAttr.description = UA_LOCALIZEDTEXT((char*)"en_US",(char*) "Description");
                                                        vAttr.displayName = UA_LOCALIZEDTEXT((char*)"en_US",(char*) "Description");

                                                        UA_String engineringUnit = UA_String_fromChars(description.c_str());
                                                        UA_Variant_setScalar(&vAttr.value, &engineringUnit, &UA_TYPES[UA_TYPES_STRING]);
                                                        UA_Server_addVariableNode(this->mappedServer, UA_NODEID_NUMERIC(1, 0), createdNodeId,
                                                                                                                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(1, (char*) "Description"),
                                                                                                                                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), vAttr, &icb, &newNodeId);
                                                }

                                                if(UA_NodeId_isNull(&newNodeId)) {
                                                        UA_Server_addReference(this->mappedServer, bRes.references[i].nodeId.nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), *targetNodeId, false);
                                                }
                                        }

                                        UA_BrowseDescription_deleteMembers(&bDesc);
                                        UA_BrowseResult_deleteMembers(&bRes);

                                }
                        }
                }
        }

        xmlXPathFreeObject (result);
}

vector<ua_processvariable *> ua_uaadapter::getVariables() {
        return this->variables;
}

UA_NodeId ua_uaadapter::createUAFolder(UA_NodeId basenodeid, std::string folderName, std::string description) {
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
        oAttr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)description.c_str());

        UA_INSTATIATIONCALLBACK(icb);
        UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1,0),
                          basenodeid, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                          UA_QUALIFIEDNAME(1, (char*)folderName.c_str()), UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), oAttr, &icb, &createdNodeId);

        return createdNodeId;
}

UA_StatusCode ua_uaadapter::mapSelfToNamespace() {
        UA_StatusCode retval = UA_STATUSCODE_GOOD;
        UA_NodeId createdNodeId = UA_NODEID_NULL;

        if (UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE) {
                return 0; // Something went UA_WRING (initializer should have set this!)
        }

        // Create our toplevel instance
        UA_ObjectAttributes oAttr;
        UA_ObjectAttributes_init(&oAttr);
        // Classcast to prevent Warnings
        oAttr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)this->serverConfig.rootFolder.c_str());
        oAttr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)this->serverConfig.descriptionFolder.c_str());

        UA_INSTATIATIONCALLBACK(icb);
        UA_Server_addObjectNode(this->mappedServer, UA_NODEID_NUMERIC(1,0),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, (char*)this->serverConfig.rootFolder.c_str()), UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_CTKMODULE), oAttr, &icb, &createdNodeId);

        this->ownNodeId = createdNodeId;
        // Nodes "Variables" where created on object instantiation, we need these IDs to add new process variables to them...
        UA_NodeId_copy(nodePairList_getTargetIdBySourceId(this->ownedNodes, UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLES)), &this->variablesListId);

    return UA_STATUSCODE_GOOD;
}

UA_NodeId ua_uaadapter::getOwnNodeId() {
        return this->ownNodeId;
}

UA_NodeId ua_uaadapter::existFolderPath(UA_NodeId basenodeid, std::vector<string> folderPath) {
        UA_NodeId lastNodeId = basenodeid;
        for(std::string t : folderPath) {
                lastNodeId = this->existFolder(lastNodeId, t);
                if(UA_NodeId_isNull(&lastNodeId)) {
                        return UA_NODEID_NULL;
                }
        }
        return lastNodeId;
}

UA_NodeId ua_uaadapter::existFolder(UA_NodeId basenodeid, string folder) {
        UA_NodeId lastNodeId = UA_NODEID_NULL;
        for(uint32_t i=0; i < this->folderVector.size(); i++) {
                if((this->folderVector.at(i).folderName.compare(folder) == 0) && (UA_NodeId_equal(&this->folderVector.at(i).prevFolderNodeId, &basenodeid))) {
                        return this->folderVector.at(i).folderNodeId;
                }
        }
        return UA_NODEID_NULL;
}

UA_NodeId ua_uaadapter::createFolderPath(UA_NodeId basenodeid, std::vector<string> folderPath) {

        if(UA_NodeId_isNull(&basenodeid)){
                return UA_NODEID_NULL;
        }

        // Check if path exist
        UA_NodeId toCheckNodeId = existFolderPath(basenodeid, folderPath);
        int32_t starter4Folder = 0;
        UA_NodeId nextNodeId = basenodeid;
        UA_NodeId startNodeId = basenodeid;
        if(UA_NodeId_isNull(&toCheckNodeId)) {
                bool setted = false;
                // Check if path exist partly
                for(uint32_t m=0; m < folderPath.size(); m++) {
                        for(uint32_t i=0; i < this->folderVector.size(); i++) {
                                // get correct folder NodeId from first folderPath element
                                if(!setted && (folderPath.at(m).compare(this->folderVector.at(i).folderName) == 0) && (UA_NodeId_equal(&this->folderVector.at(i).prevFolderNodeId, &nextNodeId)) && ((m+1) < folderPath.size())) {
                                        // remember on witch position the folder still exist
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
                // Path exist nothing to do
                return toCheckNodeId;
        }

        UA_NodeId prevNodeId = nextNodeId;
        // use the remembered position to start the loop
        for(uint32_t m=starter4Folder; m < folderPath.size(); m++) {
                prevNodeId = this->createFolder(prevNodeId, folderPath.at(m));
        }
        // return last created folder UA_NodeId
        return prevNodeId;
}

UA_NodeId ua_uaadapter::createFolder(UA_NodeId basenodeid, string folderName, string description) {

        if(UA_NodeId_isNull(&basenodeid)){
                return UA_NODEID_NULL;
        }

        // Check if path exist
        UA_NodeId toCheckNodeId = existFolder(basenodeid, folderName);
        FolderInfo newFolder;
        if(UA_NodeId_isNull(&toCheckNodeId)) {
                newFolder.folderName = folderName;
                newFolder.folderNodeId = this->createUAFolder(basenodeid, folderName, description);
                newFolder.prevFolderNodeId = basenodeid;
                this->folderVector.push_back(newFolder);
        }

        return newFolder.folderNodeId;
}

vector<string> ua_uaadapter::getAllNotMappableVariablesNames() {

        vector<string> notMappableVariablesNames;
        xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//map");

        if(result) {
                xmlNodeSetPtr nodeset = result->nodesetval;
                for (int32_t i=0; i < nodeset->nodeNr; i++) {
                        //for(auto var:this->variables) {
                        bool mapped = false;
                        string mappedVar = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "sourceVariableName");
                        for(auto var:this->getVariables()) {
                                if(var->getName().compare(mappedVar) == 0) {
                                        mapped = true;
                                }
                        }
                        if(!mapped) {
                                notMappableVariablesNames.push_back(mappedVar);
                        }
                }
        }

        return notMappableVariablesNames;
}

UA_DateTime ua_uaadapter::getSourceTimeStamp() {
        return UA_DateTime_now();
}
