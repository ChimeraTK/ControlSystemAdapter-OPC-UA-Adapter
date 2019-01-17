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
 * Copyright (c) 2018 Andreas Ebner <Andreas.Ebner@iosb-extern.fraunhofer.de>
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
                //this->readAdditionalNodes();
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
    //TODO DEL_NEW_BLOCK
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
        vector<xmlNodePtr> mappingExceptionsVector = this->fileHandler->getNodesByName(
                nodeset->nodeTab[0]->children, "mapping_exceptions");
        placeHolder = this->fileHandler->getContentFromNode(mappingExceptionsVector[0]);
        transform(placeHolder.begin(), placeHolder.end(), placeHolder.begin(), ::toupper);
        if(placeHolder.compare("TRUE") != 0){
            this->mappingExceptions = UA_TRUE;
        } else {
            this->mappingExceptions = UA_FALSE;
        }
    }
    result = this->fileHandler->getNodeSet(xpath + "//login");
    if(result) {
        xmlNodeSetPtr nodeset = result->nodesetval;
        // There should be only one <login>-Tag in config file
        if(nodeset->nodeNr > 1) {
            throw std::runtime_error ("To many <login>-Tags in config file");
        }
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
    result = this->fileHandler->getNodeSet(xpath + "//server");
    if(result) {
        xmlNodeSetPtr nodeset = result->nodesetval;
        // There should be only one <server>-Tag in config file
        if(nodeset->nodeNr > 1) {
            throw std::runtime_error ("To many <server>-Tags in config file");
        }
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
        //if no root folder name is set, use application name
        if (this->serverConfig.rootFolder.empty()) {
            this->serverConfig.rootFolder = this->serverConfig.applicationName;
        }
    }
    else {
        cout << "No <server>-Tag in config file. Use default port 16664 and application name configuration." << endl;
    }
    result = this->fileHandler->getNodeSet(xpath+"//process_variable_hierarchy");
    if(result){
        //TODO runtime error if more than one?
        xmlNodeSetPtr nodeset = result->nodesetval;
        vector<xmlNodePtr> nodeVectorUnrollPathPV = this->fileHandler->getNodesByName(
                nodeset->nodeTab[0]->children, "unroll");
        for (auto nodeUnrollPath: nodeVectorUnrollPathPV) {
            string unrollSepEnabled = this->fileHandler->getContentFromNode(nodeUnrollPath);
            transform(unrollSepEnabled.begin(), unrollSepEnabled.end(), unrollSepEnabled.begin(), ::toupper);
            if(unrollSepEnabled.compare("TRUE") == 0){
                this->pvSeperator += this->fileHandler->getAttributeValueFromNode(nodeUnrollPath, "pathSep");
            }
        }
    } else {
        cout << "No <process_variable_hierarchy>-Tag in config file. Use default hierarchical mapping with '/'."
             << endl;
        this->pvSeperator = "/";
    }
    //TODO DEL_END_BLOCK
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

vector<int32_t> ua_uaadapter::findMappingIndex(std::string varName) {
        xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//map");
        xmlNodeSetPtr nodeset;
        string srcVarName = "";
        vector<int32_t> pvIndexes;

        if(result) {
                nodeset = result->nodesetval;
                for (int32_t i = 0; i < nodeset->nodeNr; i++) {
                        srcVarName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i],
                                                                                  "sourceVariableName");
                        if (varName.compare(srcVarName) == 0) {
                            pvIndexes.push_back(i);
                        }
                }
        }
        xmlXPathFreeObject (result);
        return pvIndexes;
}

UA_NodeId ua_uaadapter::enrollFolderPathFromString(string path, string seperator){
    //TODO use this function in add_variable inside the folder attribute code block
    vector<string> varPathVector;
    if (!seperator.empty()) {
        vector<string> newPathVector = this->fileHandler->praseVariablePath(path, seperator);
        varPathVector.insert(varPathVector.end(), newPathVector.begin(), newPathVector.end());
    }
    if (varPathVector.size() > 0) { //last element is the variable name itself
        varPathVector.pop_back();
    }
    if (varPathVector.size() > 0) {
        return this->createFolderPath(this->ownNodeId, varPathVector);
    }
    return UA_NODEID_NULL;
}

void ua_uaadapter::implicitVarMapping(std::string varName, boost::shared_ptr<ControlSystemPVManager> csManager){
    UA_NodeId folderPathNodeId = enrollFolderPathFromString(varName, this->pvSeperator);
    ua_processvariable *processvariable;
    if (!UA_NodeId_isNull(&folderPathNodeId)) {
        processvariable = new ua_processvariable(this->mappedServer, folderPathNodeId,
                                                 varName.substr(1, varName.size() - 1), csManager,
                                                 this->fileHandler->praseVariablePath(varName,
                                                         this->pvSeperator).back());
    } else {
        processvariable = new ua_processvariable(this->mappedServer, this->ownNodeId,
                                                 varName.substr(1, varName.size() - 1), csManager);
    }
    UA_Server_writeDisplayName(this->mappedServer, processvariable->getOwnNodeId(),
                               UA_LOCALIZEDTEXT((char *) "en_US",
                                                (char *) this->fileHandler->praseVariablePath(varName,
                                                                                              this->pvSeperator).back().c_str()));
    this->variables.push_back(processvariable);

}

void ua_uaadapter::deepCopyHierarchicalLayer(boost::shared_ptr<ControlSystemPVManager> csManager, UA_NodeId layer, UA_NodeId target){
    cout << "deep copy hierarchy layer " << (char *) layer.identifier.string.data << " and target node id " << (char *) target.identifier.string.data << endl;
    //copy pv's of current layer
    UA_BrowseDescription bd;
    bd.includeSubtypes = false;
    bd.nodeId = layer;
    bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    bd.resultMask = UA_BROWSERESULTMASK_NONE;
    bd.nodeClassMask = UA_NODECLASS_VARIABLE;
    bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    UA_BrowseResult br = UA_Server_browse(this->mappedServer, UA_UINT32_MAX, &bd);
    for (size_t j = 0; j < br.referencesSize; ++j) {
        UA_ReferenceDescription rd = br.references[j];
        //get unit, desc, ..
        UA_LocalizedText foundPVName;
        string foundPVNameCPP;
        UA_Server_readDisplayName(this->mappedServer, rd.nodeId.nodeId, &foundPVName);
        UASTRING_TO_CPPSTRING(foundPVName.text, foundPVNameCPP);

        string pvSourceNameid;
        UA_String foundPVSourceName;
        string foundPVSourceNameCPP;
        UA_Variant value;
        UA_STRING_TO_CPPSTRING_COPY(&rd.nodeId.nodeId.identifier.string, &pvSourceNameid);
        pvSourceNameid = pvSourceNameid.substr(0, pvSourceNameid.length()-5);
        UA_Server_readValue(this->mappedServer, UA_NODEID_STRING(1, (char *) (pvSourceNameid+"/name").c_str()), &value);
        foundPVSourceName = *((UA_String *) value.data);
        UASTRING_TO_CPPSTRING(foundPVSourceName, foundPVSourceNameCPP);

        cout << "asume following pv name " << foundPVSourceNameCPP << endl;
        string varName = this->fileHandler->praseVariablePath(foundPVNameCPP, "/").back();
        ua_processvariable *processvariable = new ua_processvariable(this->mappedServer, target, foundPVSourceNameCPP, csManager, foundPVNameCPP);

        this->variables.push_back(processvariable);
        //TODO clean sear results
    }


        //copy folders of current layer
    bd.includeSubtypes = false;
    bd.nodeId = layer;
    bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    bd.resultMask = UA_BROWSERESULTMASK_NONE;
    bd.nodeClassMask = UA_NODECLASS_OBJECT;
    bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    br = UA_Server_browse(this->mappedServer, UA_UINT32_MAX, &bd);
    for (size_t j = 0; j < br.referencesSize; ++j) {
        UA_ReferenceDescription rd = br.references[j];
        //all 'organizes' references are folders, maybe add the foldertype check for more code safety
        //UA_NodeId folderType = UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE);
        //if(UA_NodeId_equal(&rd., &folderType)){
        cout << "found folder type and starting copy hierarchy" << endl;
        UA_String folderName, description;
        UA_Variant value;
        UA_StatusCode result = UA_Server_readValue(this->mappedServer, rd.nodeId.nodeId, &value);
        if(result != UA_STATUSCODE_GOOD){
            //error handling, todo check als type of variant (string expected)
        }
        UA_LocalizedText foundFolderName;
        string foundFolderNameCPP;
        result = UA_Server_readDisplayName(this->mappedServer, rd.nodeId.nodeId, &foundFolderName);
        UASTRING_TO_CPPSTRING(foundFolderName.text, foundFolderNameCPP);
        if(result != UA_STATUSCODE_GOOD){
            //error handling
        }
        cout << "create new folder with name " << foundFolderNameCPP << " and node id " << (char *) target.identifier.string.data << endl;
        UA_NodeId newRootFolder = createFolder(target, foundFolderNameCPP);
        deepCopyHierarchicalLayer(csManager, rd.nodeId.nodeId, newRootFolder);
    //}
    }
    UA_BrowseResult_deleteMembers(&br);

}

void ua_uaadapter::buildFolderStructure(boost::shared_ptr<ControlSystemPVManager> csManager){
    xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//folder");
    xmlNodeSetPtr nodeset;
    if (result) {
        nodeset = result->nodesetval;
        for (int32_t i=0; i < nodeset->nodeNr; i++) {
            UA_NodeId folderPathNodeId;
            string folderPath, folder;
            vector<xmlNodePtr> nodeFolderPath = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "destination");
            vector<xmlNodePtr> nodeFolder = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "name");
            vector<xmlNodePtr> nodeDescription = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "description");
            string sourceName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "sourceName");
            string copy = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "copy");
            string destination;
            if(!nodeFolderPath.empty()){
                destination = this->fileHandler->getContentFromNode(nodeFolderPath[0]);
            }

            //check if source name is set -> map complete hierarchical structure to the destination
            if(!sourceName.empty()){
                //check if the src is a folder
                string sourceFolder = this->serverConfig.rootFolder+"/"+sourceName;
                bool isFolderType = false;
                //string parentSourceFolder = this->serverConfig.rootFolder+"/"+(sourceName.substr(0, sourceName.length()-this->fileHandler->praseVariablePath(sourceName, "/").back().length()-1));
                UA_NodeId sourceFolderId = UA_NODEID_STRING(1, (char *) sourceFolder.c_str());

                UA_BrowseDescription bd;
                bd.includeSubtypes = false;
                bd.nodeId = sourceFolderId;
                bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
                bd.resultMask = UA_BROWSERESULTMASK_ALL;
                bd.nodeClassMask = UA_NODECLASS_OBJECTTYPE;
                bd.browseDirection = UA_BROWSEDIRECTION_BOTH;
                UA_BrowseResult br = UA_Server_browse(this->mappedServer, 20, &bd);
                for (size_t j = 0; j < br.referencesSize; ++j) {
                    UA_ReferenceDescription rd = br.references[j];
                    UA_NodeId folderType = UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE);
                    if(UA_NodeId_equal(&rd.nodeId.nodeId, &folderType)){
                        isFolderType = true;
                    }
                }
                UA_BrowseResult_deleteMembers(&br);
                if(!isFolderType){
                    cout << "Error! Root of hierarchical map must be a folder. Skipping.";
                    continue;
                }
                UA_NodeId destinationFolderNodeId = enrollFolderPathFromString(destination+"/replacePart", "/");
                //enroll path destination -> copy / link the complete tree to this place
                transform(copy.begin(), copy.end(), copy.begin(), ::toupper);
                if(copy.compare("TRUE") == 0) {
                    // folder structure must be copied, pv nodes must be added
                    cout << "starting creating complete folder copy" << endl;
                    //add the source folder self to the new destinatoin -> correct?
                    UA_LocalizedText foundFolderName;
                    string foundFolderNameCPP;
                    UA_Server_readDisplayName(this->mappedServer, sourceFolderId, &foundFolderName);
                    UASTRING_TO_CPPSTRING(foundFolderName.text, foundFolderNameCPP);
                    UA_NodeId copyRoot = createFolder(destinationFolderNodeId, foundFolderNameCPP);
                    deepCopyHierarchicalLayer(csManager ,sourceFolderId, copyRoot);
                } else {
                    UA_ExpandedNodeId enid;
                    enid.serverIndex = 1;
                    enid.namespaceUri = UA_STRING_NULL;
                    enid.nodeId = sourceFolderId;
                    UA_StatusCode addRef = UA_Server_addReference(this->mappedServer, destinationFolderNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), enid, UA_TRUE);
                    const UA_StatusCodeDescription *st = UA_StatusCode_description(addRef);
                }
            }

            if(!nodeFolderPath.empty()) {
                folderPath = this->fileHandler->getContentFromNode(nodeFolderPath[0]);
            }
            if(!nodeFolder.empty()) {
                folder = this->fileHandler->getContentFromNode(nodeFolder[0]);
                if(folder.compare("") == 0){
                    if(this->mappingExceptions){
                        throw std::runtime_error ("Folder name is missing.");
                    }
                    cerr << "Folder name is missing. Skipping folder!" << endl;
                    continue;
                }
            }
            //TODO add else case if the name is not set in the xml and check the above exception
            if((folderPath+folder).compare("") != 0){
                //create the requested folder hierarchy
                folderPathNodeId = enrollFolderPathFromString((folderPath+"/"+folder)+"/removedPart", "/");
            }
            //set folder description
            if(nodeDescription.size() >= 1) {
                string description = this->fileHandler->getContentFromNode(nodeDescription[0]);
                if (description.compare("") != 0) {
                    UA_Server_writeDescription(this->mappedServer, folderPathNodeId, UA_LOCALIZEDTEXT((char *) "en_US", (char *) description.c_str()));
                }
            }
        }
    }
}

UA_NodeId ua_uaadapter::findSingleChildNode(UA_QualifiedName targetName, UA_NodeId referenceTypeId, UA_NodeId startingNode){
    UA_NodeId resultNodeId;
    UA_RelativePathElement rpe;
    UA_RelativePathElement_init(&rpe);
    rpe.referenceTypeId = referenceTypeId;
    rpe.isInverse = false;
    rpe.includeSubtypes = false;
    rpe.targetName = targetName;
    UA_BrowsePath bp;
    UA_BrowsePath_init(&bp);
    bp.startingNode = startingNode;
    bp.relativePath.elementsSize = 1;
    bp.relativePath.elements = &rpe;
    UA_BrowsePathResult bpr =
    UA_Server_translateBrowsePathToNodeIds(this->mappedServer, &bp);
    if(bpr.statusCode != UA_STATUSCODE_GOOD || bpr.targetsSize < 1){
        return UA_NODEID_NULL;}
    if(UA_NodeId_copy(&bpr.targets[0].targetId.nodeId, &resultNodeId) != UA_STATUSCODE_GOOD){
        UA_BrowsePathResult_deleteMembers(&bpr);
        return UA_NODEID_NULL;
    }
    UA_BrowsePathResult_deleteMembers(&bpr);
    return resultNodeId;
}


void ua_uaadapter::explicitVarMapping(boost::shared_ptr<ControlSystemPVManager> csManager) {
    xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//process_variable");
    xmlNodeSetPtr nodeset;
    if (result) {
        nodeset = result->nodesetval;
        for (int32_t i = 0; i < nodeset->nodeNr; i++) {
            string sourceName, copy, destination, name, unit, description;
            vector<xmlNodePtr> nodeDestination = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "destination");
            vector<xmlNodePtr> nodeName = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "name");
            vector<xmlNodePtr> nodeUnit = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "unit");
            vector<xmlNodePtr> nodeDescription = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "description");
            sourceName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "sourceName");
            copy = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "copy");

            if(!nodeDestination.empty()) {
                destination = this->fileHandler->getContentFromNode(nodeDestination[0]);
            }
            if(!nodeName.empty()) {
                name = this->fileHandler->getContentFromNode(nodeName[0]);
            }
            if(!nodeUnit.empty()) {
                unit = this->fileHandler->getContentFromNode(nodeUnit[0]);
            }
            if(!nodeDescription.empty()) {
                description = this->fileHandler->getContentFromNode(nodeDescription[0]);
            }

            cout << "current process variable " << sourceName << " " << copy << " " << destination << " " << name << " " << unit << " " << description << endl;
            if(sourceName.empty()){
                throw std::runtime_error ("Mapping Error. SourceName missing.");
            }
            //check if the pv still exists -> update requested
            if((destination+"/"+name).compare(sourceName) == 0){
                cout << "pv exists, value update requested" << endl;
                //check if the source var exists
                string parentSourceFolder = this->serverConfig.rootFolder+"/"+(sourceName.substr(0, sourceName.length()-this->fileHandler->praseVariablePath(sourceName, "/").back().length()-1));
                cout << "Folder of the var " << parentSourceFolder << endl;
                UA_NodeId parentSourceFolderId = UA_NODEID_STRING(1, (char *) parentSourceFolder.c_str());
                UA_NodeId pvNodeId = UA_NODEID_NULL;
                UA_BrowseDescription bd;
                bd.includeSubtypes = false;
                bd.nodeId = parentSourceFolderId;
                bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
                bd.resultMask = UA_BROWSERESULTMASK_ALL;
                bd.nodeClassMask = UA_NODECLASS_VARIABLE;
                bd.browseDirection = UA_BROWSEDIRECTION_BOTH;
                UA_BrowseResult br = UA_Server_browse(this->mappedServer, 20, &bd);
                for (size_t j = 0; j < br.referencesSize; ++j) {
                    UA_ReferenceDescription rd = br.references[j];
                    string name;
                    UASTRING_TO_CPPSTRING(rd.displayName.text, name)
                    if(name.compare(this->fileHandler->praseVariablePath(sourceName, "/").back()) == 0){
                         UA_NodeId_copy(&br.references[j].nodeId.nodeId, &pvNodeId);
                     }
                }
                UA_BrowseResult_deleteMembers(&br);
                if(UA_NodeId_isNull(&pvNodeId)) {
                    //TODO handle error case
                    cout << "nodeid was null" << endl;
                    return;
                }
                if(!unit.empty()){
                    //update unit
                    string unitNodeId;
                    UASTRING_TO_CPPSTRING(pvNodeId.identifier.string, unitNodeId);
                    unitNodeId = unitNodeId.substr(0, unitNodeId.length()-5);
                    unitNodeId += "/engineeringunit";
                    cout << "node id from pv to change unit" << unitNodeId << endl;
                    UA_String ua_unit = UA_STRING((char *) unit.c_str());
                    UA_Variant value;
                    UA_Variant_setScalar(&value, &ua_unit, &UA_TYPES[UA_TYPES_STRING]);
                    /*
                    UA_Server_writeAccessLevel(this->mappedServer, UA_NODEID_STRING(1, (char *) unitNodeId.c_str()), UA_ACCESSLEVELMASK_READ^UA_ACCESSLEVELMASK_WRITE);
                    UA_NodeId accessLevel = UA_NODEID_STRING(1, (char *) unitNodeId.c_str());
                    UA_Byte accessLevelMask = UA_ACCESSLEVELMASK_WRITE^UA_ACCESSLEVELMASK_READ;
                    __UA_Server_write(this->mappedServer, &accessLevel, UA_ATTRIBUTEID_USERACCESSLEVEL, &UA_TYPES[UA_TYPES_BYTE], &accessLevelMask);
                    */
                    //TODO remove develop result output
                    UA_StatusCode addrstl = UA_Server_writeValue(this->mappedServer, UA_NODEID_STRING(1, (char *) unitNodeId.c_str()), value);
                    const UA_StatusCodeDescription *st = UA_StatusCode_description(addrstl);
                    cout << "tried to update unit "<< st->name << endl;
                }
                if(!description.empty()){
                    //update description
                    string descriptionNodeId;
                    UASTRING_TO_CPPSTRING(pvNodeId.identifier.string, descriptionNodeId);
                    descriptionNodeId = descriptionNodeId.substr(0, descriptionNodeId.length()-5);
                    descriptionNodeId += "/description";
                    UA_String ua_description = UA_STRING((char *) description.c_str());
                    UA_Variant value;
                    UA_Variant_setScalar(&value, &ua_description, &UA_TYPES[UA_TYPES_STRING]);
                    //TODO remove develop result output
                    UA_StatusCode addrstl = UA_Server_writeValue(this->mappedServer, UA_NODEID_STRING(1, (char *) descriptionNodeId.c_str()), value);
                    const UA_StatusCodeDescription *st = UA_StatusCode_description(addrstl);
                    cout << "tried to update description "<< st->name << endl;
                }
                continue;
            }
            /*
            UA_NodeId requestedPV = this->findSingleChildNode(UA_QUALIFIEDNAME(1, ), UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT));
            UA_NodeId result;
            if(UA_Server_readDataType(this->mappedServer, UA_NODEID_STRING(1, (char *) (baseNodeIdName+"/"+this->nameNew+"Value").c_str()), &result) == UA_STATUSCODE_GOOD){
                std::cout << "WARNING: Same mapping name used for different mappings! Skipping." << std::endl;
                return UA_STATUSCODE_BADNODEIDEXISTS;
            }
            */

            //check the pv copy attribute -> copy of pv requested; false -> reference to original pv requested
            transform(copy.begin(), copy.end(), copy.begin(), ::toupper);
            if(copy.compare("TRUE") == 0){
                UA_NodeId destinationFolderNodeId = enrollFolderPathFromString(destination+"/removedPart", "/");
                cout << "destination "<< destination <<" string folder " << (char *) destinationFolderNodeId.identifier.string.data << endl;
                ua_processvariable *processvariable;
                if (!UA_NodeId_isNull(&destinationFolderNodeId)) {
                    processvariable = new ua_processvariable(this->mappedServer, destinationFolderNodeId,
                                                             sourceName,  csManager, name);
                    this->variables.push_back(processvariable);
                } else {
                    cout << "Error during PV mapping. Folder creation failed! Skipping." << endl;
                    continue;
                }
                if(!unit.empty()){
                    string unitNodeId;
                    UASTRING_TO_CPPSTRING(processvariable->getOwnNodeId().identifier.string, unitNodeId);
                    unitNodeId = unitNodeId.substr(0, unitNodeId.length()-5);
                    unitNodeId += "/engineeringunit";
                    cout << "node id from pv to change unit" << unitNodeId << endl;
                    UA_String ua_unit = UA_STRING((char *) unit.c_str());
                    UA_Variant value;
                    UA_Variant_setScalar(&value, &ua_unit, &UA_TYPES[UA_TYPES_STRING]);
                    //TODO remove develop result output
                    UA_StatusCode addrstl = UA_Server_writeValue(this->mappedServer, UA_NODEID_STRING(1, (char *) unitNodeId.c_str()), value);
                    const UA_StatusCodeDescription *st = UA_StatusCode_description(addrstl);
                    cout << "tried to update unit (copy true)"<< st->name << endl;
                }
                if(!description.empty()){
                    string descriptionNodeId;
                    UASTRING_TO_CPPSTRING(processvariable->getOwnNodeId().identifier.string, descriptionNodeId);
                    descriptionNodeId = descriptionNodeId.substr(0, descriptionNodeId.length()-5);
                    descriptionNodeId += "/description";
                    UA_String ua_description = UA_STRING((char *) description.c_str());
                    UA_Variant value;
                    UA_Variant_setScalar(&value, &ua_description, &UA_TYPES[UA_TYPES_STRING]);
                    //TODO remove develop result output
                    UA_StatusCode addrstl = UA_Server_writeValue(this->mappedServer, UA_NODEID_STRING(1, (char *) descriptionNodeId.c_str()), value);
                    const UA_StatusCodeDescription *st = UA_StatusCode_description(addrstl);
                    cout << "tried to update description (copy true)"<< st->name << endl;
                }

                UA_Server_writeDisplayName(this->mappedServer, processvariable->getOwnNodeId(),
                                           UA_LOCALIZEDTEXT((char *) "en_US", (char *) name.c_str()));
            } else {

                //get node id of the source node
                UA_NodeId destinationFolder = enrollFolderPathFromString(destination+"/removedpart", "/");

                //TODO remove this, source parent folder link should not be created on the fly, just get the nodeid of the parent
                UA_NodeId sourceParentFolder = enrollFolderPathFromString(sourceName, "/");
                string sourceVarName = this->fileHandler->praseVariablePath(sourceName, "/").back();

                cout << "srcnmae " << sourceVarName << "rename name " << name << endl;
                if(sourceVarName.compare(name) != 0){
                    cout << "Error! Skipping PV. The pv name can't changed if copy is false." << endl;
                    continue;
                }

                /* Find the NodeId of the status child variable */
                UA_RelativePathElement rpe;
                UA_RelativePathElement_init(&rpe);
                rpe.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
                rpe.isInverse = false;
                rpe.includeSubtypes = false;
                cout << "src var name " << sourceVarName.c_str() << endl;
                rpe.targetName = UA_QUALIFIEDNAME(1, (char *) sourceVarName.c_str());

                UA_BrowsePath bp;
                UA_BrowsePath_init(&bp);
                bp.startingNode = sourceParentFolder;
                bp.relativePath.elementsSize = 1;
                bp.relativePath.elements = &rpe;

                UA_BrowsePathResult bpr =
                        UA_Server_translateBrowsePathToNodeIds(this->mappedServer, &bp);
                if(bpr.statusCode != UA_STATUSCODE_GOOD || bpr.targetsSize < 1){
                    //TODO handle error case
                    continue;
                }
                UA_NodeId sourceNodeId;
                if(UA_NodeId_copy(&bpr.targets[0].targetId.nodeId, &sourceNodeId) != UA_STATUSCODE_GOOD){
                    UA_BrowsePathResult_deleteMembers(&bpr);
                    sourceNodeId =  UA_NODEID_NULL;
                }
                UA_BrowsePathResult_deleteMembers(&bpr);

                UA_ExpandedNodeId enid;
                enid.serverIndex = 1;
                enid.namespaceUri = UA_STRING_NULL;
                enid.nodeId = sourceNodeId;
                //add reference to the source node
                UA_StatusCode  addRef = UA_Server_addReference(this->mappedServer, destinationFolder, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), enid, UA_TRUE);
                const UA_StatusCodeDescription *st = UA_StatusCode_description(addRef);
                cout << "tried to link "<< st->name << endl;
                cout << "nodeid "<< (char *) sourceParentFolder.identifier.string.data << endl;
                cout << "nodeid "<< (char *) sourceNodeId.identifier.string.data << endl;
            }
        }
    }
}

void ua_uaadapter::addAdditionalVariables() {
    xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//additional_variable");
    xmlNodeSetPtr nodeset;
    if (result) {
        nodeset = result->nodesetval;
        for (int32_t i = 0; i < nodeset->nodeNr; i++) {
            string destination, name, description, value;
            vector<xmlNodePtr> nodeDestination = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "destination");
            vector<xmlNodePtr> nodeName = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "name");
            vector<xmlNodePtr> nodeDescription = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "description");
            vector<xmlNodePtr> nodeValue = this->fileHandler->getNodesByName(
                    nodeset->nodeTab[i]->children, "value");

            if(!nodeDestination.empty()) {
                destination = this->fileHandler->getContentFromNode(nodeDestination[0]);
            }
            if(!nodeName.empty()) {
                name = this->fileHandler->getContentFromNode(nodeName[0]);
            }
            if(!nodeDescription.empty()) {
                description = this->fileHandler->getContentFromNode(nodeDescription[0]);
            }
            if(!nodeValue.empty()) {
                value = this->fileHandler->getContentFromNode(nodeValue[0]);
            }
            cout << "current additional " << destination << " " << name << " " << description << " " << value << endl;
            UA_NodeId additionalVarFolderPath = UA_NODEID_NULL;
            if(!destination.empty()){
                additionalVarFolderPath = enrollFolderPathFromString(destination+"/removePart", "/");
            } else {
                additionalVarFolderPath = UA_NODEID_STRING(1, (char *) this->serverConfig.rootFolder.c_str());
            }
            if(UA_NodeId_isNull(&additionalVarFolderPath)){
                cout << "created folder node id was null " << endl;
                //TODO error handling
            }
            cout << "add new additional var "<< value << endl;
            ua_additionalvariable *additionalvariable = new ua_additionalvariable(this->mappedServer,
                    additionalVarFolderPath, name, value, description);
            this->additionalVariables.push_back(additionalvariable);

        }
    }
}

void ua_uaadapter::addVariable(std::string varName, boost::shared_ptr<ControlSystemPVManager> csManager) {
    vector<int32_t> mappedVariableIndex = findMappingIndex(varName);
    //there is no mapping entry contained in the xml file
    if (this->pvSeperator.compare("") != 0) {//implicit var mapping is enabled
        UA_NodeId folderPathNodeId = enrollFolderPathFromString(varName, this->pvSeperator);
        ua_processvariable *processvariable;
        if (!UA_NodeId_isNull(&folderPathNodeId)) {
            processvariable = new ua_processvariable(this->mappedServer, folderPathNodeId,
                                                     varName.substr(1, varName.size() - 1), csManager,
                                                     this->fileHandler->praseVariablePath(varName,
                                                                                          this->pvSeperator).back());
        } else {
            processvariable = new ua_processvariable(this->mappedServer, this->ownNodeId,
                                                     varName.substr(1, varName.size() - 1), csManager);
        }
        UA_Server_writeDisplayName(this->mappedServer, processvariable->getOwnNodeId(),
                                   UA_LOCALIZEDTEXT((char *) "en_US",
                                                    (char *) this->fileHandler->praseVariablePath(varName,
                                                                                                  this->pvSeperator).back().c_str()));
        this->variables.push_back(processvariable);
    }
    if(!mappedVariableIndex.empty()) { //mapping node exists in xml file
        xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//map");
        xmlNodeSetPtr nodeset;
        if (result) {
            nodeset = result->nodesetval;
            for (size_t mappingPosition = 0; mappingPosition < mappedVariableIndex.size(); mappingPosition++) {
                string srcVarName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[mappedVariableIndex.at(mappingPosition)],
                                                                                 "sourceVariableName");
                string applicName = this->fileHandler->getAttributeValueFromNode(
                        nodeset->nodeTab[mappedVariableIndex.at(mappingPosition)]->parent, "name");
                // Check if "rename" is not empty
                string renameVar = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[mappedVariableIndex.at(mappingPosition)],
                                                                                "rename");
                string engineeringUnit = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[mappedVariableIndex.at(mappingPosition)],
                                                                                      "engineeringUnit");
                string description = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[mappedVariableIndex.at(mappingPosition)], "description");
                // TODO. What happen if application name are not unique?
                // Application Name have to be unique!!!
                UA_NodeId appliFolderNodeId = this->existFolder(this->ownNodeId, applicName);
                FolderInfo newFolder;
                if (UA_NodeId_isNull(&appliFolderNodeId)) {
                    newFolder.folderName = applicName;
                    newFolder.folderNodeId = this->createFolder(this->ownNodeId, applicName);
                    this->folderVector.push_back(newFolder);
                    appliFolderNodeId = newFolder.folderNodeId;
                }

                vector<string> varPathVector;
                vector<xmlNodePtr> nodeVectorUnrollPath = this->fileHandler->getNodesByName(
                        nodeset->nodeTab[mappedVariableIndex.at(mappingPosition)]->children, "unrollPath");
                string seperator = "";
                bool unrollPathIs = false;
                for (auto nodeUnrollPath: nodeVectorUnrollPath) {
                    string shouldUnrollPath = this->fileHandler->getContentFromNode(nodeUnrollPath);
                    transform(shouldUnrollPath.begin(), shouldUnrollPath.end(), shouldUnrollPath.begin(), ::toupper);
                    if (shouldUnrollPath.compare("TRUE") == 0) {
                        seperator = seperator + this->fileHandler->getAttributeValueFromNode(nodeUnrollPath, "pathSep");
                        unrollPathIs = true;
                    }
                }

                if (!seperator.empty()) {
                    vector<string> newPathVector = this->fileHandler->praseVariablePath(srcVarName, seperator);
                    varPathVector.insert(varPathVector.end(), newPathVector.begin(), newPathVector.end());
                }

                // assumption last element is name of variable, hence no folder for name is needed
                if (renameVar.compare("") == 0 && !unrollPathIs) {
                    renameVar = srcVarName;
                    // std::cout << "Variable '" << srcVarName << "' renamed in '" << renameVar << "' and listed in folder '" << applicName << "'." << std::endl;
                } else {
                    if (unrollPathIs && renameVar.compare("") == 0) {
                        renameVar = varPathVector.at(varPathVector.size() - 1);
                        varPathVector.pop_back();
                    } else {
                        if (varPathVector.size() > 0) {
                            varPathVector.pop_back();
                        }
                    }
                    // std::cout << "Variable '" << srcVarName << "' listed in folder '" << applicName << "'." << std::endl;
                }

                vector<xmlNodePtr> nodeVectorFolderPath = this->fileHandler->getNodesByName(
                        nodeset->nodeTab[mappedVariableIndex.at(mappingPosition)]->children, "folder");
                vector<string> folderPathVector;
                bool createdVar = false;
                UA_NodeId newFolderNodeId = UA_NODEID_NULL;
                vector<UA_NodeId> mappedVariables;
                for (auto nodeFolderPath: nodeVectorFolderPath) {

                    string folderPath = this->fileHandler->getContentFromNode(nodeFolderPath);
                    if (folderPath.empty() && unrollPathIs) {
                        break;
                    }

                    folderPathVector = this->fileHandler->praseVariablePath(folderPath);
                    // Create folders
                    newFolderNodeId = appliFolderNodeId;
                    if (folderPathVector.size() > 0) {
                        newFolderNodeId = this->createFolderPath(newFolderNodeId, folderPathVector);
                    }

                    if (varPathVector.size() > 0) {
                        newFolderNodeId = this->createFolderPath(newFolderNodeId, varPathVector);
                    }
                    mappedVariables.push_back(newFolderNodeId);
                    createdVar = true;
                }

                // in case no <folder> or <unrollpath> is set
                if (!createdVar) {
                    newFolderNodeId = appliFolderNodeId;

                    if (varPathVector.size() > 0) {
                        mappedVariables.push_back(this->createFolderPath(newFolderNodeId, varPathVector));
                    } else {
                        // No <folder>
                        mappedVariables.push_back(appliFolderNodeId);
                    }
                }

                for (auto objectNodeId:mappedVariables) {
                    UA_NodeId createdNodeId = UA_NODEID_NULL;

                    ua_processvariable *processvariable;
                    if (renameVar.compare(srcVarName) == 0)
                        //default short name based on implicit var mapping param, alternativ a hard coded delimiter '/' can be used(this->pvSeperator -> instead "/")
                        processvariable = new ua_processvariable(this->mappedServer, objectNodeId, varName, csManager,
                                                                 this->fileHandler->praseVariablePath(varName,
                                                                                                      this->pvSeperator).back());
                    else
                        processvariable = new ua_processvariable(this->mappedServer, objectNodeId, varName, csManager,
                                                                 renameVar);
                    this->variables.push_back(processvariable);

                    //checks if a mapped variable has no rename option or unrollPathIds option
                    //in this case the name will be shorten using the default path delimiters, (string node id is still the full name)
                    if (renameVar.compare(srcVarName) == 0 && !unrollPathIs)
                        UA_Server_writeDisplayName(this->mappedServer, processvariable->getOwnNodeId(),
                                                   UA_LOCALIZEDTEXT((char *) "en_US",
                                                                          (char *) this->fileHandler->praseVariablePath(
                                                                                  varName, this->pvSeperator).back().c_str()));
                    else
                        UA_Server_writeDisplayName(this->mappedServer, processvariable->getOwnNodeId(),
                                                   UA_LOCALIZEDTEXT((char *) "en_US", (char *) renameVar.c_str()));

                    createdNodeId = objectNodeId;

                    UA_Variant newValue;
                    string childNodeId;
                    //get the nodeId string from the created pv node
                    UA_String parentNodeString = processvariable->getOwnNodeId().identifier.string;
                    UA_STRING_TO_CPPSTRING_COPY(&parentNodeString, &childNodeId);
                    if (!description.empty()) {
                        UA_Server_writeDescription(this->mappedServer, processvariable->getOwnNodeId(),
                                                   UA_LOCALIZEDTEXT((char *)"en_US", (char *) description.c_str()));
                        processvariable->setDescription(description);
                    }
                    if (!engineeringUnit.empty())
                        processvariable->setEngineeringUnit(engineeringUnit);

                    UA_Server_writeDescription(this->mappedServer, UA_NODEID_STRING(1, (char *) (childNodeId+"/description").c_str()), UA_LOCALIZEDTEXT((char *) "en_US", (char *) "Description"));
                    UA_Server_writeDescription(this->mappedServer, UA_NODEID_STRING(1, (char *) (childNodeId+"/engineeringunit").c_str()), UA_LOCALIZEDTEXT((char *) "en_US", (char *) "EngineeringUnit"));
                    UA_Server_writeDescription(this->mappedServer, UA_NODEID_STRING(1, (char *) (childNodeId+"/name").c_str()), UA_LOCALIZEDTEXT((char *) "en_US", (char *) "Name"));
                    UA_Server_writeDescription(this->mappedServer, UA_NODEID_STRING(1, (char *) (childNodeId+"/type").c_str()), UA_LOCALIZEDTEXT((char *) "en_US", (char *) "Type"));
                }
            }
            xmlXPathFreeObject(result);
        }
    }
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

        string parentNodeIdString;
        if(basenodeid.identifierType == UA_NODEIDTYPE_STRING){
            UASTRING_TO_CPPSTRING(basenodeid.identifier.string, parentNodeIdString);
            parentNodeIdString += '/' + folderName;
        } else if(basenodeid.identifierType == UA_NODEIDTYPE_NUMERIC){
            parentNodeIdString += '/' + to_string(basenodeid.identifier.numeric);
        }

        UA_INSTATIATIONCALLBACK(icb);
        UA_Server_addObjectNode(this->mappedServer, UA_NODEID_STRING(1, (char *) parentNodeIdString.c_str()), //UA_NODEID_NUMERIC(1,0)
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
        UA_Server_addObjectNode(this->mappedServer, UA_NODEID_STRING(1, (char*) this->serverConfig.rootFolder.c_str()),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, (char*)this->serverConfig.rootFolder.c_str()), UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_CTKMODULE), oAttr, &icb, &createdNodeId);

/*        UA_ObjectAttributes attr;
        UA_ObjectAttributes_init(&attr);
        attr.displayName = UA_LOCALIZEDTEXT((char *) "en_US", (char *) "Variables");
        attr.description = UA_LOCALIZEDTEXT((char *) "", (char *) "");
        UA_Server_addObjectNode(this->mappedServer, UA_NODEID_STRING(1, (char*) (this->serverConfig.rootFolder+"/Variables").c_str()),
                                createdNodeId, UA_NODEID_NUMERIC(0, 47), UA_QUALIFIEDNAME(1, (char *) "Variables"),
                                UA_NODEID_NUMERIC(0, 61), attr, &icb, &this->variablesListId); */
        this->ownNodeId = createdNodeId;
        // Nodes "Variables" where created on object instantiation, we need these IDs to add new process variables to them...
        //UA_NodeId_copy(nodePairList_getTargetIdBySourceId(this->ownedNodes, UA_NODEID_NUMERIC(CSA_NSID, CSA_NSID_VARIABLES)), &this->variablesListId);

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
                if((this->folderVector.at(i).folderName.compare(folder) == 0) &&
                   (UA_NodeId_equal(&this->folderVector.at(i).prevFolderNodeId, &basenodeid))) {
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
                                if(!setted && (folderPath.at(m).compare(this->folderVector.at(i).folderName) == 0) &&
                                    (UA_NodeId_equal(&this->folderVector.at(i).prevFolderNodeId, &nextNodeId)) &&
                                    ((m+1) < folderPath.size())) {
                                        // remember on witch position the folder still exist
                                        setted = true;
                                        starter4Folder = m+1;
                                        nextNodeId = this->folderVector.at(i).folderNodeId;
                                }
                                if(setted) {
                                        break;
                                }
                        }
                        if(!setted)
                            break;
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
