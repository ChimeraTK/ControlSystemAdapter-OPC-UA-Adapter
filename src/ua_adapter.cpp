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
 * Copyright (c) 2018-2021 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
 */

extern "C" {
#include "unistd.h"
#include "csa_namespaceinit_generated.h"
#include <stdio.h>
#include <stdlib.h>
}

#include <functional>
#include <string>
#include <regex>

#include "csa_config.h"
#include "ua_adapter.h"
#include "ua_processvariable.h"
#include "ua_additionalvariable.h"
#include "xml_file_handler.h"
#include "ua_proxies.h"

//These defines allow the use of an define as a string
#define xstr(a) str(a)
#define str(a) #a

using namespace ChimeraTK;
using namespace std;

ua_uaadapter::ua_uaadapter(string configFile) : ua_mapped_class() {
  this->mappingExceptions = UA_FALSE;
  this->fileHandler = new xml_file_handler(configFile);
  this->readConfig();
  this->constructServer();
  this->mapSelfToNamespace();
}

ua_uaadapter::~ua_uaadapter() {
  UA_Server_delete(this->mappedServer);
  this->fileHandler->~xml_file_handler();
  for(auto ptr : variables) delete ptr;
  for(auto ptr : additionalVariables) delete ptr;
  for(auto ptr : mappedVariables) delete ptr;
}

static string cleanUri(string s) {
  //replace multiple with a single space
  std::regex r1("\\s+");
  s = std::regex_replace(s, r1, " ");
  s = std::regex_replace(s, r1, "_");
  //remove all not printable basic ASCII character
  std::regex r2("[^ -~]+");
  s = std::regex_replace(s, r2, "");
  return s;
}

void ua_uaadapter::constructServer() {
  auto config = (UA_ServerConfig*)UA_calloc(1, sizeof(UA_ServerConfig));
  UA_ServerConfig_setMinimal(config, this->serverConfig.opcuaPort, NULL);

  /*get hostname */
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);
  string hostname_uri = "opc.tcp://";
  hostname_uri.append(hostname);

  config->applicationDescription.applicationType = UA_APPLICATIONTYPE_SERVER;
  UA_String* hostName = UA_String_new();
  *hostName = UA_STRING_ALLOC(hostname);
  config->applicationDescription.discoveryUrls = hostName;
  config->applicationDescription.discoveryUrlsSize = 1;
  config->applicationDescription.applicationName =
      UA_LOCALIZEDTEXT_ALLOC((char*)"en_US", (char*)this->serverConfig.applicationName.c_str());
  config->applicationDescription.applicationUri = UA_STRING_ALLOC((char*)(cleanUri(hostname_uri)).c_str());
  config->buildInfo.manufacturerName = UA_STRING_ALLOC((char*)"ChimeraTK Team");
  std::string
      versionString =
      "open62541: " xstr(UA_OPEN62541_VER_MAJOR) "." xstr(UA_OPEN62541_VER_MINOR) "." xstr(UA_OPEN62541_VER_PATCH) ", ControlSystemAdapter-OPC-UA-Adapter: " xstr(
          PROJECT_VER_MAJOR) "." xstr(PROJECT_VER_MINOR) "." xstr(PTOJECT_VER_PATCH) "";
  config->buildInfo.softwareVersion = UA_STRING_ALLOC((char*)versionString.c_str());
  config->buildInfo.buildDate = UA_DateTime_now();
  config->buildInfo.buildNumber = UA_STRING_ALLOC((char*)"");

  config->buildInfo.productName = UA_STRING_ALLOC((char*)this->serverConfig.applicationName.c_str());
  string product_urn = "urn:ChimeraTK:" + this->serverConfig.applicationName;
  config->buildInfo.productUri = UA_STRING_ALLOC((char*)(cleanUri(product_urn)).c_str());

  for(size_t i = 0; i < config->endpointsSize; ++i){
    UA_ApplicationDescription_clear(&config->endpoints[i].server);
    UA_ApplicationDescription_copy(&config->applicationDescription, &config->endpoints[i].server);
  }

  this->mappedServer = UA_Server_newWithConfig(config);
  this->server_config = UA_Server_getConfig(this->mappedServer);

  //Username/Password handling
  auto* usernamePasswordLogins = new UA_UsernamePasswordLogin; //!< Brief description after the member
  usernamePasswordLogins->password = UA_STRING_ALLOC((char*)this->serverConfig.password.c_str());
  usernamePasswordLogins->username = UA_STRING_ALLOC((char*)this->serverConfig.username.c_str());
  UA_AccessControl_default(this->server_config, !this->serverConfig.UsernamePasswordLogin, NULL,
                           &this->server_config->securityPolicies[this->server_config->securityPoliciesSize - 1].policyUri, 1,
                           usernamePasswordLogins);

  this->baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  csa_namespaceinit_generated(this->mappedServer);

  UA_free(config);
  UA_String_clear(&usernamePasswordLogins->password);
  UA_String_clear(&usernamePasswordLogins->username);
  delete usernamePasswordLogins;
}

void ua_uaadapter::readConfig() {
  string xpath = "//config";
  xmlXPathObjectPtr result = this->fileHandler->getNodeSet(xpath);
  string placeHolder;
  if(result) {
    xmlNodeSetPtr nodeset = result->nodesetval;
    if(nodeset->nodeNr > 1) {
      throw std::runtime_error("To many <config>-Tags in config file");
    }

    placeHolder = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "rootFolder");
    if(!placeHolder.empty()) {
      this->serverConfig.rootFolder = placeHolder;
    }

    placeHolder = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "description");
    if(!placeHolder.empty()) {
      this->serverConfig.descriptionFolder = placeHolder;
    }
    vector<xmlNodePtr> mappingExceptionsVector =
        this->fileHandler->getNodesByName(nodeset->nodeTab[0]->children, "mapping_exceptions");
    if(mappingExceptionsVector.empty()) {
      this->mappingExceptions = UA_FALSE;
    }
    else {
      placeHolder = this->fileHandler->getContentFromNode(mappingExceptionsVector[0]);
      transform(placeHolder.begin(), placeHolder.end(), placeHolder.begin(), ::toupper);
      if(placeHolder.compare("TRUE") == 0) {
        this->mappingExceptions = UA_TRUE;
      }
      else {
        this->mappingExceptions = UA_FALSE;
      }
    }

    xmlXPathFreeObject(result);
  }
  result = this->fileHandler->getNodeSet(xpath + "//login");
  if(result) {
    xmlNodeSetPtr nodeset = result->nodesetval;
    if(nodeset->nodeNr > 1) {
      throw std::runtime_error("To many <login>-Tags in config file");
    }
    this->serverConfig.UsernamePasswordLogin = UA_TRUE;
    placeHolder = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "password");
    if(!placeHolder.empty()) {
      this->serverConfig.password = placeHolder;
    }
    else {
      throw std::runtime_error("<login>-Tag requires username");
    }

    placeHolder = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "username");
    if(!placeHolder.empty()) {
      this->serverConfig.username = placeHolder;
    }
    else {
      throw std::runtime_error("<login>-Tag requires password");
    }

    xmlXPathFreeObject(result);
  }
  else {
    this->serverConfig.UsernamePasswordLogin = UA_FALSE;
  }
  result = this->fileHandler->getNodeSet(xpath + "//server");
  if(result) {
    xmlNodeSetPtr nodeset = result->nodesetval;
    if(nodeset->nodeNr > 1) {
      throw std::runtime_error("To many <server>-Tags in config file");
    }
    string opcuaPort = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "port");
    if(!opcuaPort.empty()) {
      this->serverConfig.opcuaPort = std::stoi(opcuaPort);
    }
    else {
      cout << "No 'port'-Attribute in config file is set. Use default Port: 16664" << endl;
    }

    placeHolder = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "applicationName");
    if(!placeHolder.empty()) {
      this->serverConfig.applicationName = placeHolder;
      if(this->serverConfig.rootFolder.empty()) {
        this->serverConfig.rootFolder = placeHolder;
      }
    }
    else {
      string applicationName;
      try {
        string applicationName = ApplicationBase::getInstance().getName();
        this->serverConfig.applicationName = applicationName.c_str();
      }
      catch(ChimeraTK::logic_error) {
      }
      cout << "No 'applicationName'-Attribute is set in config file. Use default application-name." << endl;
    }
    //if no root folder name is set, use application name
    if(this->serverConfig.rootFolder.empty()) {
      this->serverConfig.rootFolder = this->serverConfig.applicationName;
    }
    xmlXPathFreeObject(result);
  }
  else {
    cout << "No <server>-Tag in config file. Use default port 16664 and application name configuration." << endl;
    this->serverConfig.rootFolder = this->serverConfig.applicationName;
  }
  result = this->fileHandler->getNodeSet(xpath + "//process_variable_hierarchy");
  if(result) {
    xmlNodeSetPtr nodeset = result->nodesetval;
    vector<xmlNodePtr> nodeVectorUnrollPathPV =
        this->fileHandler->getNodesByName(nodeset->nodeTab[0]->children, "unroll");
    for(auto nodeUnrollPath : nodeVectorUnrollPathPV) {
      string unrollSepEnabled = this->fileHandler->getContentFromNode(nodeUnrollPath);
      transform(unrollSepEnabled.begin(), unrollSepEnabled.end(), unrollSepEnabled.begin(), ::toupper);
      if(unrollSepEnabled.compare("TRUE") == 0) {
        this->pvSeperator += this->fileHandler->getAttributeValueFromNode(nodeUnrollPath, "pathSep");
      }
    }
    xmlXPathFreeObject(result);
  }
  else {
    cout << "No <process_variable_hierarchy>-Tag in config file. Use default hierarchical mapping with '/'."
         << endl;
    this->pvSeperator = "/";
  }
}

void ua_uaadapter::applyMapping(boost::shared_ptr<ControlSystemPVManager> csManager) {
  //build folder structure
  this->buildFolderStructure(csManager);
  //start explicit mapping
  this->explicitVarMapping(csManager);
  //add configured additional variables
  this->addAdditionalVariables();
}

void ua_uaadapter::workerThread() {
  if(this->mappedServer == nullptr) {
    cout << "No server mapped" << endl;
    return;
  }

  cout << "Starting the server worker thread" << endl;
  UA_Server_run_startup(this->mappedServer);

  this->running = true;
  while(this->running) {
    UA_Server_run_iterate(this->mappedServer, true);
  }

  UA_Server_run_shutdown(this->mappedServer);
  cout << "Stopped the server worker thread" << endl;
}

UA_NodeId ua_uaadapter::enrollFolderPathFromString(string path, string seperator) {
  vector<string> varPathVector;
  if(!seperator.empty()) {
    vector<string> newPathVector = this->fileHandler->praseVariablePath(path, seperator);
    varPathVector.insert(varPathVector.end(), newPathVector.begin(), newPathVector.end());
  }
  if(!varPathVector.empty()) { //last element is the variable name itself
    varPathVector.pop_back();
    return this->createFolderPath(this->ownNodeId, varPathVector);
  }
  return UA_NODEID_NULL;
}

void ua_uaadapter::implicitVarMapping(std::string varName, boost::shared_ptr<ControlSystemPVManager> csManager) {
  UA_NodeId folderPathNodeId = enrollFolderPathFromString(varName, this->pvSeperator);
  ua_processvariable* processvariable;
  if(!UA_NodeId_isNull(&folderPathNodeId)) {
    processvariable =
        new ua_processvariable(this->mappedServer, folderPathNodeId, varName.substr(1, varName.size() - 1),
                               csManager, this->fileHandler->praseVariablePath(varName, this->pvSeperator).back());
  }
  else {
    processvariable = new ua_processvariable(
        this->mappedServer, this->ownNodeId, varName.substr(1, varName.size() - 1), csManager);
  }
  this->variables.push_back(processvariable);
  UA_NodeId tmpNodeId = processvariable->getOwnNodeId();
  UA_Server_writeDisplayName(this->mappedServer, tmpNodeId,
                             UA_LOCALIZEDTEXT(
                                 (char*)"en_US", (char*)this->fileHandler->praseVariablePath(varName, this->pvSeperator).back().c_str()));
  UA_NodeId_clear(&tmpNodeId);
}

void ua_uaadapter::deepCopyHierarchicalLayer(
    boost::shared_ptr<ControlSystemPVManager> csManager, UA_NodeId layer, UA_NodeId target) {
  //copy pv's of current layer
  UA_BrowseDescription bd;
  bd.includeSubtypes = false;
  bd.nodeId = layer;
  bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
  bd.resultMask = UA_BROWSERESULTMASK_NONE;
  bd.nodeClassMask = UA_NODECLASS_VARIABLE;
  bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
  UA_BrowseResult br = UA_Server_browse(this->mappedServer, UA_UINT32_MAX, &bd);
  for(size_t j = 0; j < br.referencesSize; ++j) {
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
    UA_Server_readValue(this->mappedServer, UA_NODEID_STRING(1, (char*)(pvSourceNameid + "/Name").c_str()), &value);
    foundPVSourceName = *((UA_String*)value.data);
    UASTRING_TO_CPPSTRING(foundPVSourceName, foundPVSourceNameCPP);
    string varName = this->fileHandler->praseVariablePath(foundPVNameCPP, "/").back();
    auto* processvariable =
        new ua_processvariable(this->mappedServer, target, foundPVSourceNameCPP, csManager, foundPVNameCPP);
    this->variables.push_back(processvariable);
  }
  UA_BrowseResult_clear(&br);
  //copy folders of current layer
  bd.includeSubtypes = false;
  bd.nodeId = layer;
  bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  bd.resultMask = UA_BROWSERESULTMASK_NONE;
  bd.nodeClassMask = UA_NODECLASS_OBJECT;
  bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
  br = UA_Server_browse(this->mappedServer, UA_UINT32_MAX, &bd);
  for(size_t j = 0; j < br.referencesSize; ++j) {
    UA_ReferenceDescription rd = br.references[j];
    UA_String folderName, description;
    UA_LocalizedText foundFolderName, foundFolderDescription;
    UA_LocalizedText_init(&foundFolderDescription);
    string foundFolderNameCPP;
    UA_Server_readDescription(this->mappedServer, rd.nodeId.nodeId, &foundFolderDescription);
    UA_StatusCode result = UA_Server_readDisplayName(this->mappedServer, rd.nodeId.nodeId, &foundFolderName);
    UASTRING_TO_CPPSTRING(foundFolderName.text, foundFolderNameCPP);
    if(result != UA_STATUSCODE_GOOD) {
      //error handling
      continue;
    }
    UA_NodeId newRootFolder = createFolder(target, foundFolderNameCPP);
    if(foundFolderDescription.text.length > 0)
      UA_Server_writeDescription(this->mappedServer, newRootFolder, foundFolderDescription);
    deepCopyHierarchicalLayer(csManager, rd.nodeId.nodeId, newRootFolder);
  }
  UA_BrowseResult_clear(&br);
}

void ua_uaadapter::buildFolderStructure(boost::shared_ptr<ControlSystemPVManager> csManager) {
  xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//folder");
  xmlNodeSetPtr nodeset;
  if(result) {
    nodeset = result->nodesetval;
    for(int32_t i = 0; i < nodeset->nodeNr; i++) {
      UA_NodeId folderPathNodeId;
      string destination, description, folder;
      vector<xmlNodePtr> nodeFolderPath =
          this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "destination");
      vector<xmlNodePtr> nodeFolder = this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "name");
      vector<xmlNodePtr> nodeDescription =
          this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "description");
      string sourceName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "sourceName");
      string copy = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "copy");
      if(!nodeFolderPath.empty()) {
        destination = this->fileHandler->getContentFromNode(nodeFolderPath[0]);
      }
      if(!nodeDescription.empty()) {
        description = this->fileHandler->getContentFromNode(nodeDescription[0]);
      }
      if(!nodeFolder.empty()) {
        folder = this->fileHandler->getContentFromNode(nodeFolder[0]);
      }
      if(folder.empty()) {
        if(this->mappingExceptions) {
          throw std::runtime_error("Error! Folder creation failed. Name is missing. Mapping line number: " +
                                   to_string(nodeset->nodeTab[i]->line));
        }
        if(description.empty()) {
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping Folder. Folder creation failed. Name is missing. Mapping line number: %u",
                         nodeset->nodeTab[i]->line);
        }
        else {
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping Folder. Folder creation failed. Name is missing 'description: %s'. Mapping "
                         "line number: %u",
                         description.c_str(), nodeset->nodeTab[i]->line);
        }
        continue;
      }
      //check if a pv with the requested node id exists
      UA_NodeId tmpOutput = UA_NODEID_NULL;
      string pvNodeString;
      if(destination.empty()) {
        pvNodeString += this->serverConfig.rootFolder + "/" + folder + "Dir";
      }
      else {
        pvNodeString += this->serverConfig.rootFolder + "/" + destination + "/" + folder + "Dir";
      }
      UA_NodeId pvNode = UA_NODEID_STRING(1, (char*)pvNodeString.c_str());
      UA_Server_readNodeId(this->mappedServer, pvNode, &tmpOutput);
      if(!UA_NodeId_isNull(&tmpOutput)) {
        UA_NodeId_clear(&tmpOutput);
        //set folder description
        if(sourceName.empty() && !description.empty()) {
          UA_Server_writeDescription(
              this->mappedServer, pvNode, UA_LOCALIZEDTEXT((char*)"en_US", (char*)description.c_str()));
          continue;
        }
        if(this->mappingExceptions) {
          throw std::runtime_error("Error! Folder with source mapping failed. Target folder node id exists. "
                                   "Mapping line number: " +
                                   to_string(nodeset->nodeTab[i]->line));
        }
        UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                       "Warning! Skipping folder: %s. Folder with source mapping failed. Target folder node id exists. "
                       "Mapping line number: %u",
                       sourceName.c_str(), nodeset->nodeTab[i]->line);
        continue;
      }
      if(!copy.empty() && sourceName.empty()) {
        if(this->mappingExceptions) {
          throw std::runtime_error(
              "Error! Folder creation failed. Source folder missing. Mapping line number: " +
              to_string(nodeset->nodeTab[i]->line));
        }
        UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                       "Warning! Skipping Folder. Source 'name: %s' folder missing. Mapping line number: %u",
                       folder.c_str(), nodeset->nodeTab[i]->line);
        continue;
      }
      folderPathNodeId = enrollFolderPathFromString(destination + "/replacePart", "/");
      if(UA_NodeId_isNull(&folderPathNodeId)) {
        folderPathNodeId = UA_NODEID_STRING(1, (char*)(this->serverConfig.rootFolder + "Dir").c_str());
      }
      //check if source name is set -> map complete hierarchical structure to the destination
      if(!sourceName.empty()) {
        if((destination.empty() && sourceName.compare(folder) == 0) ||
           (!destination.empty() && sourceName.compare(destination + "/" + folder) == 0)) {
          if(this->mappingExceptions) {
            throw std::runtime_error(
                "Error! Folder creation failed. Source and Destination equal. Mapping line number: " +
                to_string(nodeset->nodeTab[i]->line));
          }
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping Folder. Source '%s' and Destination equal. Mapping line number: %u",
                         sourceName.c_str(), nodeset->nodeTab[i]->line);
          continue;
        }
        //check if the src is a folder
        string sourceFolder = this->serverConfig.rootFolder + "/" + sourceName + "Dir";
        bool isFolderType = false;
        UA_NodeId sourceFolderId = UA_NODEID_STRING(1, (char*)sourceFolder.c_str());

        UA_BrowseDescription bd;
        bd.includeSubtypes = false;
        bd.nodeId = sourceFolderId;
        bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
        bd.resultMask = UA_BROWSERESULTMASK_ALL;
        bd.nodeClassMask = UA_NODECLASS_OBJECTTYPE;
        bd.browseDirection = UA_BROWSEDIRECTION_BOTH;
        UA_BrowseResult br = UA_Server_browse(this->mappedServer, 1000, &bd);
        for(size_t j = 0; j < br.referencesSize; ++j) {
          UA_ReferenceDescription rd = br.references[j];
          UA_NodeId folderType = UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE);
          if(UA_NodeId_equal(&rd.nodeId.nodeId, &folderType)) {
            isFolderType = true;
          }
        }
        UA_BrowseResult_clear(&br);
        if(!isFolderType) {
          if(this->mappingExceptions) {
            throw std::runtime_error("Error! Folder creation failed. No corresponding source folder: " +
                                     sourceName + ". Mapping line number: " + to_string(nodeset->nodeTab[i]->line));
          }
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping Folder. No corresponding source '%s' folder. Mapping line number: %u",
                         sourceName.c_str(), nodeset->nodeTab[i]->line);
          continue;
        }
        //enroll path destination -> copy / link the complete tree to this place
        transform(copy.begin(), copy.end(), copy.begin(), ::toupper);
        if(copy.compare("TRUE") == 0) {
          bool sourceAndDestinationEqual = false;
          if(!destination.empty()) {
            if(sourceName.compare(destination + "/" + folder) == 0) sourceAndDestinationEqual = true;
          }
          else {
            if(sourceName.compare(folder) == 0) sourceAndDestinationEqual = true;
          }
          if(sourceAndDestinationEqual) {
            if(this->mappingExceptions) {
              throw std::runtime_error(
                  "Error! Folder creation failed. Source and destination must be different for '" +
                  sourceName + "' folder. Mapping line number: " + to_string(nodeset->nodeTab[i]->line));
            }
            UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                           "Warning! Skipping Folder. Folder creation failed. Source and destination must be "
                           "different for '%s' folder. Mapping line number: %u",
                           sourceName.c_str(), nodeset->nodeTab[i]->line);
            continue;
          }
          // folder structure must be copied, pv nodes must be added
          UA_LocalizedText foundFolderName;
          UA_NodeId copyRoot = createFolder(folderPathNodeId, folder);
          if(UA_NodeId_isNull(&copyRoot)) {
            string existingDestinationFolderString;
            if(destination.empty()) {
              existingDestinationFolderString += this->serverConfig.rootFolder + "/" + folder + "Dir";
            }
            else {
              existingDestinationFolderString =
              this->serverConfig.rootFolder += "/" + destination + "/" + folder + "Dir";
            }
            copyRoot = UA_NODEID_STRING(1, (char*)existingDestinationFolderString.c_str());
          }
          deepCopyHierarchicalLayer(csManager, sourceFolderId, copyRoot);
          if(!description.empty()) {
            UA_Server_writeDescription(
                this->mappedServer, copyRoot, UA_LOCALIZEDTEXT((char*)"en_US", (char*)description.c_str()));
          }
        }
        else {
          string existingDestinationFolderString;
          UA_NodeId copyRoot = createFolder(folderPathNodeId, folder);
          if(UA_NodeId_isNull(&copyRoot)) {
            if(destination.empty()) {
              existingDestinationFolderString = this->serverConfig.rootFolder + "/" + folder;
            }
            else {
              existingDestinationFolderString =
                  this->serverConfig.rootFolder + "/" + destination + "/" + folder;
            }
            copyRoot = UA_NODEID_STRING(1, (char*)existingDestinationFolderString.c_str());
          }
          else {
            if(!description.empty()) {
              UA_Server_writeDescription(this->mappedServer, copyRoot,
                                         UA_LOCALIZEDTEXT((char*)"en_US", (char*)description.c_str()));
            }
          }
          UA_BrowseDescription bd;
          bd.includeSubtypes = false;
          bd.nodeId = sourceFolderId;
          bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
          bd.resultMask = UA_BROWSERESULTMASK_ALL;
          bd.nodeClassMask = UA_NODECLASS_OBJECT;
          bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
          UA_BrowseResult br = UA_Server_browse(this->mappedServer, 1000, &bd);
          for(size_t j = 0; j < br.referencesSize; ++j) {
            UA_ExpandedNodeId enid;
            enid.serverIndex = 0;
            enid.namespaceUri = UA_STRING_NULL;
            enid.nodeId = br.references[j].nodeId.nodeId;
            UA_Server_addReference(
                this->mappedServer, copyRoot, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), enid, UA_TRUE);
          }
          UA_BrowseResult_clear(&br);
          bd.nodeId = sourceFolderId;
          bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
          bd.resultMask = UA_BROWSERESULTMASK_ALL;
          bd.nodeClassMask = UA_NODECLASS_VARIABLE;
          bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
          br = UA_Server_browse(this->mappedServer, 1000, &bd);
          for(size_t j = 0; j < br.referencesSize; ++j) {
            UA_ExpandedNodeId enid;
            enid.serverIndex = 0;
            enid.namespaceUri = UA_STRING_NULL;
            enid.nodeId = br.references[j].nodeId.nodeId;
            UA_Server_addReference(
                this->mappedServer, copyRoot, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), enid, UA_TRUE);
          }
          UA_BrowseResult_clear(&br);
        }
        continue;
      }
      //create the requested folder hierarchy
      UA_NodeId retnode = createFolder(folderPathNodeId, folder);
      //set folder description
      if(copy.empty() && sourceName.empty() && !description.empty()) {
        UA_Server_writeDescription(this->mappedServer, retnode,
                                   UA_LOCALIZEDTEXT((char*)"en_US", (char*)description.c_str()));
      }
    }

    xmlXPathFreeObject(result);
  }
}

void ua_uaadapter::explicitVarMapping(boost::shared_ptr<ControlSystemPVManager> csManager) {
  xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//process_variable");
  xmlNodeSetPtr nodeset;
  if(result) {
    nodeset = result->nodesetval;
    for(int32_t i = 0; i < nodeset->nodeNr; i++) {
      string sourceName, copy, destination, name, unit, description, unrollPath;
      vector<xmlNodePtr> nodeDestination =
          this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "destination");
      vector<xmlNodePtr> nodeName = this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "name");
      vector<xmlNodePtr> nodeUnit = this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "unit");
      vector<xmlNodePtr> nodeDescription =
          this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "description");
      sourceName = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "sourceName");
      copy = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "copy");
      transform(copy.begin(), copy.end(), copy.begin(), ::toupper);

      if(!nodeDestination.empty()) {
        destination = this->fileHandler->getContentFromNode(nodeDestination[0]);
        unrollPath = this->fileHandler->getAttributeValueFromNode(nodeDestination[0], "unrollPath");
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

      if(sourceName.empty()) {
        if(this->mappingExceptions) {
          throw std::runtime_error("Error! PV mapping failed. SourceName missing.");
        }
        if(!name.empty()) {
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping PV mapping. SourceName 'name: %s' is missing.", name.c_str());
        }
        else {
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping PV mapping. SourceName is missing.");
        }
        continue;
      }
      //check if the pv still exists -> update requested
      if((destination + "/" + name).compare(sourceName) == 0 && copy.compare("FALSE") == 0) {
        //check if the source var exists
        string parentSourceFolder = this->serverConfig.rootFolder + "/" +
                                    (sourceName.substr(0,
                                                       sourceName.length() - this->fileHandler->praseVariablePath(sourceName, "/").back().length() -
                                                       1));
        UA_NodeId parentSourceFolderId = UA_NODEID_STRING(1, (char*)parentSourceFolder.c_str());
        UA_NodeId pvNodeId = UA_NODEID_NULL;
        UA_BrowseDescription bd;
        bd.includeSubtypes = false;
        bd.nodeId = parentSourceFolderId;
        bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
        bd.resultMask = UA_BROWSERESULTMASK_ALL;
        bd.nodeClassMask = UA_NODECLASS_VARIABLE;
        bd.browseDirection = UA_BROWSEDIRECTION_BOTH;
        UA_BrowseResult br = UA_Server_browse(this->mappedServer, 20, &bd);
        for(size_t j = 0; j < br.referencesSize; ++j) {
          UA_ReferenceDescription rd = br.references[j];
          string name;
          UASTRING_TO_CPPSTRING(rd.displayName.text, name)
          if(name.compare(this->fileHandler->praseVariablePath(sourceName, "/").back()) == 0) {
            UA_NodeId_copy(&br.references[j].nodeId.nodeId, &pvNodeId);
          }
        }
        UA_BrowseResult_clear(&br);
        if(UA_NodeId_isNull(&pvNodeId)) {
          if(this->mappingExceptions) {
            throw std::runtime_error("Error! PV mapping failed. No corresponding source pv.");
          }
          if(!name.empty()) {
            UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                           "Warning! Skipping PV mapping. No corresponding source pv 'name: %s'.", name.c_str());
          }
          else {
            UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                           "Warning! Skipping PV mapping. No corresponding source pv.");
          }
          return;
        }
        if(!unit.empty()) {
          //update unit
          for(size_t n = 0; n < this->variables.size(); n++) {
            UA_NodeId tmpNodeId = this->variables[n]->getOwnNodeId();
            if(UA_NodeId_equal(&tmpNodeId, &pvNodeId)) {
              this->variables[n]->setEngineeringUnit(unit);
            }
            UA_NodeId_clear(&tmpNodeId);
          }
        }
        if(!description.empty()) {
          //update description
          for(size_t n = 0; n < this->variables.size(); n++) {
            UA_NodeId tmpNodeId = this->variables[n]->getOwnNodeId();
            if(UA_NodeId_equal(&tmpNodeId, &pvNodeId)) {
              this->variables[n]->setDescription(description);
            }
            UA_NodeId_clear(&tmpNodeId);
          }
        }

        if(!UA_NodeId_isNull(&pvNodeId)) {
          UA_NodeId_clear(&pvNodeId);
        }
        continue;
      }
      //check if the source pv exists
      string parentSourceString = this->serverConfig.rootFolder + "/" + sourceName;
      UA_NodeId parentSourceId = UA_NODEID_STRING(1, (char*)parentSourceString.c_str());
      UA_NodeId tmpOutput = UA_NODEID_NULL;
      UA_Server_readNodeId(this->mappedServer, parentSourceId, &tmpOutput);
      if(UA_NodeId_isNull(&tmpOutput)) {
        if(this->mappingExceptions) {
          if(!name.empty()) {
            throw std::runtime_error("Error! PV mapping failed. Source PV not found 'name' : '" + name +
                                     "'. Mapping line number: " + to_string(nodeset->nodeTab[i]->line));
          }
          else {
            throw std::runtime_error(
                "Error! PV mapping failed. Source PV not found. Mapping line number: " +
                to_string(nodeset->nodeTab[i]->line));
          }
        }
        if(!name.empty()) {
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping PV mapping. Source PV not found 'name: %s'. Mapping line number: %u",
                         name.c_str(), nodeset->nodeTab[i]->line);
        }
        else {
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping PV mapping. Source PV not found. Mapping line number: %u",
                         nodeset->nodeTab[i]->line);
        }
        continue;
      }
      else {
        UA_NodeId_clear(&tmpOutput);
        UA_NodeId_init(&tmpOutput);
      }
      //check the pv copy attribute -> copy of pv requested; false -> reference to original pv requested
      UA_NodeId createdNodeId = UA_NODEID_NULL;
      if(copy.compare("TRUE") == 0) {
        if(sourceName.compare(destination + "/" + name) == 0) {
          if(this->mappingExceptions) {
            throw std::runtime_error(
                "Error! PV mapping failed. Source and destination must be different if copy='true'.");
          }
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping PV mapping. Source and destination must be different if copy='true' 'name: "
                         "%s'.",
                         name.c_str());
          continue;
        }
        UA_NodeId destinationFolderNodeId = UA_NODEID_NULL;
        if(destination.empty()) {
          destinationFolderNodeId = this->ownNodeId;
        }
        else {
          if(unrollPath.empty()) {
            destinationFolderNodeId = enrollFolderPathFromString(destination + "/removedPart", "/");
          }
          else {
            //legacy code to make old mapping possible
            if(destination.find("Variables" + unrollPath) == 0) {
              string requestedBrowseName = destination;
              requestedBrowseName.erase(0, ("Variables" + unrollPath).length());
              //check if the requested path still exists in the Variables folder
              string requestedPVBrowseName =
                  this->serverConfig.rootFolder + "/Variables/" + requestedBrowseName;
              UA_NodeId requestedPVBrowseId = UA_NODEID_STRING(1, (char*)requestedPVBrowseName.c_str());
              UA_NodeId tmpOutput = UA_NODEID_NULL;
              UA_Server_readNodeId(this->mappedServer, requestedPVBrowseId, &tmpOutput);
              if(!UA_NodeId_isNull(&tmpOutput)) {
                UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                               "Warning! Using legacy code. Mapped PV is used as PV mapping target");
                UA_NodeId_copy(&requestedPVBrowseId, &destinationFolderNodeId);
              }
              else {
                destinationFolderNodeId =
                    enrollFolderPathFromString(destination + unrollPath + "removedPart", unrollPath);
              }
            }
            else {
              destinationFolderNodeId =
                  enrollFolderPathFromString(destination + unrollPath + "removedPart", unrollPath);
            }
          }
        }
        if(name.empty()) {
          name = sourceName;
        }
        ua_processvariable* processvariable;
        if(!UA_NodeId_isNull(&destinationFolderNodeId)) {
          processvariable = new ua_processvariable(
              this->mappedServer, destinationFolderNodeId, sourceName, csManager, name);
          UA_NodeId tmpProcessVariableNodeId = processvariable->getOwnNodeId();
          if(UA_NodeId_isNull(&tmpProcessVariableNodeId)) {
            if(this->mappingExceptions) {
              throw std::runtime_error(
                  "PV creation failed. PV with same name mapped. Mapping line number: " +
                  to_string(nodeset->nodeTab[i]->line));
            }
            UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                           "Warning! Skipping PV %s. PV with same name mapped. Mapping line number: %u",
                           sourceName.c_str(), nodeset->nodeTab[i]->line);
            continue;
          }
          else {
            UA_NodeId_clear(&tmpProcessVariableNodeId);
          }
          this->variables.push_back(processvariable);
        }
        else {
          if(this->mappingExceptions) {
            throw std::runtime_error("Folder creation failed.");
          }
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping PV. Folder creation failed.");
          continue;
        }
        UA_NodeId tmpPVNodeId = processvariable->getOwnNodeId();
        UA_NodeId_copy(&tmpPVNodeId, &createdNodeId);
        UA_NodeId_clear(&tmpPVNodeId);
      }
      else {
        //get node id of the source node
        string sourceVarName = this->fileHandler->praseVariablePath(sourceName, "/").back();
        if(name.empty()) {
          name = sourceVarName;
        }
        if(sourceVarName.compare(name) != 0) {
          if(this->mappingExceptions) {
            throw std::runtime_error(
                "Error! PV mapping failed. The pv name can't changed if copy is false.");
          }
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping PV mapping. The pv name can't changed if copy is false. 'name: %s'.",
                         name.c_str());
          continue;
        }
        //create destination folder
        UA_NodeId destinationFolder = enrollFolderPathFromString(destination + "/removedpart", "/");
        UA_ExpandedNodeId enid;
        enid.serverIndex = 1;
        enid.namespaceUri = UA_STRING_NULL;
        enid.nodeId = parentSourceId;
        //add reference to the source node
        UA_StatusCode addRef = UA_Server_addReference(
            this->mappedServer, destinationFolder, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), enid, UA_TRUE);
        if(sourceVarName.compare(name) != 0) {
          if(this->mappingExceptions) {
            throw std::runtime_error("PV mapping failed. Can't create reference to original pv.");
          }
          UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                         "Warning! Skipping PV. Can't create reference to original pv.");
          continue;
        }
        UA_NodeId_copy(&parentSourceId, &createdNodeId);
      }
      if(!unit.empty()) {
        for(size_t n = 0; n < this->variables.size(); n++) {
          UA_NodeId tmpNodeId = this->variables[n]->getOwnNodeId();
          if(UA_NodeId_equal(&tmpNodeId, &createdNodeId)) {
            this->variables[n]->setEngineeringUnit(unit);
          }
          UA_NodeId_clear(&tmpNodeId);
        }
      }
      if(!description.empty()) {
        for(size_t n = 0; n < this->variables.size(); n++) {
          UA_NodeId tmpNodeId = this->variables[n]->getOwnNodeId();
          if(UA_NodeId_equal(&tmpNodeId, &createdNodeId)) {
            this->variables[n]->setDescription(description);
          }
          UA_NodeId_clear(&tmpNodeId);
        }
      }
      UA_Server_writeDisplayName(
          this->mappedServer, createdNodeId, UA_LOCALIZEDTEXT((char*)"en_US", (char*)name.c_str()));
      UA_NodeId_clear(&createdNodeId);
    }

    xmlXPathFreeObject(result);
  }
}

void ua_uaadapter::addAdditionalVariables() {
  xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//additional_variable");
  xmlNodeSetPtr nodeset;
  if(result) {
    nodeset = result->nodesetval;
    for(int32_t i = 0; i < nodeset->nodeNr; i++) {
      string destination, name, description, value;
      vector<xmlNodePtr> nodeDestination =
          this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "destination");
      vector<xmlNodePtr> nodeName = this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "name");
      vector<xmlNodePtr> nodeDescription =
          this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "description");
      vector<xmlNodePtr> nodeValue = this->fileHandler->getNodesByName(nodeset->nodeTab[i]->children, "value");

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
      //check if name is empty
      if(name.empty()) {
        if(this->mappingExceptions) {
          throw std::runtime_error("Additional variable node creation failed. Additional variable name is "
                                   "mandatory. Mapping line number: " +
                                   to_string(nodeset->nodeTab[i]->line));
        }
        UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                       "Warning! Skipping additional variable. Additional variable name is mandatory. Mapping line "
                       "number: %u",
                       nodeset->nodeTab[i]->line);
        continue;
      }
      //check if the av node still exists
      UA_NodeId tmpOutput = UA_NODEID_NULL;
      string avNodeString;
      if(destination.empty()) {
        avNodeString = this->serverConfig.rootFolder + "/" + name + "AdditionalVariable";
      }
      else {
        avNodeString = this->serverConfig.rootFolder + "/" + destination + "/" + name + "AdditionalVariable";
      }
      UA_NodeId avNode = UA_NODEID_STRING(1, (char*)avNodeString.c_str());
      UA_Server_readNodeId(this->mappedServer, avNode, &tmpOutput);
      if(!UA_NodeId_isNull(&tmpOutput)) {
        UA_NodeId_clear(&tmpOutput);
        UA_NodeId_init(&tmpOutput);
        if(this->mappingExceptions) {
          throw std::runtime_error(
              "Additional variable node creation failed. Additional variable already exists.");
        }
        UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                       "Warning! Skipping additional variable %s. Node already exists. Mapping line number: %u",
                       name.c_str(), nodeset->nodeTab[i]->line);
        continue;
      }
      //check if pv with same name exists in the target folder
      UA_NodeId_clear(&tmpOutput);
      string pvNodeString;
      if(destination.empty()) {
        pvNodeString = this->serverConfig.rootFolder + "/" + name + "Value";
      }
      else {
        pvNodeString = this->serverConfig.rootFolder + "/" + destination + "/" + name + "Value";
      }
      UA_NodeId pvNode = UA_NODEID_STRING(1, (char*)pvNodeString.c_str());
      UA_Server_readNodeId(this->mappedServer, pvNode, &tmpOutput);
      if(!UA_NodeId_isNull(&tmpOutput)) {
        UA_NodeId_clear(&tmpOutput);
        UA_NodeId_init(&tmpOutput);
        if(this->mappingExceptions) {
          throw std::runtime_error(
              "Additional variable node creation failed. PV with same name already exists.");
        }
        UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                       "Warning! Skipping additional variable %s. PV with same name already exists. Mapping line number: "
                       "%u",
                       name.c_str(), nodeset->nodeTab[i]->line);
        continue;
      }

      UA_NodeId additionalVarFolderPath = UA_NODEID_NULL;
      string additionalVarFolderPathNodeId;
      if(!destination.empty()) {
        additionalVarFolderPath = enrollFolderPathFromString(destination + "/removePart", "/");
      }
      else {
        additionalVarFolderPathNodeId += this->serverConfig.rootFolder + "Dir";
        additionalVarFolderPath = UA_NODEID_STRING_ALLOC(1, (char*) additionalVarFolderPathNodeId.c_str());
      }
      if(UA_NodeId_isNull(&additionalVarFolderPath)) {
        if(this->mappingExceptions) {
          throw std::runtime_error("Error! Creation of additional variable folder failed.");
        }
        UA_LOG_WARNING(&this->server_config->logger, UA_LOGCATEGORY_USERLAND,
                       "Warning! Skipping additional variable. Creation of additional variable folder failed. Skipping.");
        continue;
      }
      ua_additionalvariable* additionalvariable =
          new ua_additionalvariable(this->mappedServer, additionalVarFolderPath, name, value, description);
      this->additionalVariables.push_back(additionalvariable);
    }

    xmlXPathFreeObject(result);
  }
}

vector<ua_processvariable*> ua_uaadapter::getVariables() {
  return this->variables;
}

UA_NodeId ua_uaadapter::createUAFolder(UA_NodeId basenodeid, std::string folderName, std::string description) {
  // FIXME: Check if folder name a possible name or should it be escaped (?!"§%-:, etc)
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  UA_NodeId createdNodeId = UA_NODEID_NULL;

  if(UA_NodeId_equal(&baseNodeId, &createdNodeId) == UA_TRUE) {
    return createdNodeId; // Something went wrong (initializer should set this!)
  }

  // Create our toplevel instance
  UA_ObjectAttributes oAttr;
  UA_ObjectAttributes_init(&oAttr);
  // Classcast to prevent Warnings
  oAttr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)folderName.c_str());
  oAttr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)description.c_str());

  string parentNodeIdString;
  if(basenodeid.identifierType == UA_NODEIDTYPE_STRING) {
    UASTRING_TO_CPPSTRING(basenodeid.identifier.string, parentNodeIdString);
    if (!parentNodeIdString.empty()) {
      parentNodeIdString.resize(parentNodeIdString.size() - 3);
    }
    parentNodeIdString += '/' + folderName + "Dir";
  }
  else if(basenodeid.identifierType == UA_NODEIDTYPE_NUMERIC) {
    parentNodeIdString += '/' + to_string(basenodeid.identifier.numeric) + "Dir";
  }

  UA_Server_addObjectNode(this->mappedServer,
                          UA_NODEID_STRING(1, (char*) parentNodeIdString.c_str()), //UA_NODEID_NUMERIC(1,0)
                          basenodeid, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_QUALIFIEDNAME(1, (char*)folderName.c_str()),
                          UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), oAttr, &this->ownedNodes, &createdNodeId);

  ua_mapInstantiatedNodes(createdNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), &this->ownedNodes);
  return createdNodeId;
}

UA_StatusCode ua_uaadapter::mapSelfToNamespace() {
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  UA_NodeId createdNodeId = UA_NODEID_NULL;

  if(UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE) {
    return 0;
  }

  // Create our toplevel instance
  UA_ObjectAttributes oAttr;
  UA_ObjectAttributes_init(&oAttr);
  // Classcast to prevent Warnings
  oAttr.displayName = UA_LOCALIZEDTEXT((char*)"en_US", (char*)this->serverConfig.rootFolder.c_str());
  oAttr.description = UA_LOCALIZEDTEXT((char*)"en_US", (char*)this->serverConfig.descriptionFolder.c_str());

  UA_Server_addObjectNode(this->mappedServer, UA_NODEID_STRING(1, ((char*) (this->serverConfig.rootFolder+"Dir").c_str())),
                          UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                          UA_QUALIFIEDNAME(1, (char*)this->serverConfig.rootFolder.c_str()),
                          UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_CTKMODULE), oAttr, &this->ownedNodes, &createdNodeId);

  this->ownNodeId = createdNodeId;
  ua_mapInstantiatedNodes(this->ownNodeId, UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_CTKMODULE), &this->ownedNodes);

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
  for(uint32_t i = 0; i < this->folderVector.size(); i++) {
    if((this->folderVector.at(i).folderName.compare(folder) == 0) &&
       (UA_NodeId_equal(&this->folderVector.at(i).prevFolderNodeId, &basenodeid))) {
      return this->folderVector.at(i).folderNodeId;
    }
  }
  return UA_NODEID_NULL;
}

UA_NodeId ua_uaadapter::createFolderPath(UA_NodeId basenodeid, std::vector<string> folderPath) {
  if(UA_NodeId_isNull(&basenodeid)) {
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
    for(uint32_t m = 0; m < folderPath.size(); m++) {
      for(uint32_t i = 0; i < this->folderVector.size(); i++) {
        // get correct folder NodeId from first folderPath element
        if(!setted && (folderPath.at(m).compare(this->folderVector.at(i).folderName) == 0) &&
           (UA_NodeId_equal(&this->folderVector.at(i).prevFolderNodeId, &nextNodeId)) &&
           ((m + 1) < folderPath.size())) {
          // remember on witch position the folder still exist
          setted = true;
          starter4Folder = m + 1;
          nextNodeId = this->folderVector.at(i).folderNodeId;
        }
        if(setted) {
          break;
        }
      }
      if(!setted) break;
      setted = false;
    }
  }
  else {
    // Path exist nothing to do
    return toCheckNodeId;
  }

  UA_NodeId prevNodeId = nextNodeId;
  // use the remembered position to start the loop
  for(uint32_t m = starter4Folder; m < folderPath.size(); m++) {
    prevNodeId = this->createFolder(prevNodeId, folderPath.at(m));
  }
  // return last created folder UA_NodeId
  return prevNodeId;
}

UA_NodeId ua_uaadapter::createFolder(UA_NodeId basenodeid, string folderName, string description) {
  if(UA_NodeId_isNull(&basenodeid)) {
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
  xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//process_variable");

  if(result) {
    xmlNodeSetPtr nodeset = result->nodesetval;
    for(int32_t i = 0; i < nodeset->nodeNr; i++) {
      //for(auto var:this->variables) {
      bool mapped = false;
      string mappedVar = this->fileHandler->getAttributeValueFromNode(nodeset->nodeTab[i], "sourceName");
      for(auto var : this->getVariables()) {
        if(var->getName().compare(mappedVar) == 0) {
          mapped = true;
        }
      }
      if(!mapped) {
        notMappableVariablesNames.push_back(mappedVar);
      }
    }

    xmlXPathFreeObject(result);
  }

  return notMappableVariablesNames;
}

UA_DateTime ua_uaadapter::getSourceTimeStamp() {
  return UA_DateTime_now();
}

UA_Server* ua_uaadapter::getMappedServer() {
  return this->mappedServer;
}
