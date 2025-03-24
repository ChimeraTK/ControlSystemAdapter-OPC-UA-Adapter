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
 * Copyright (c) 2018-2023 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
 */
#include "void_type.h"
#include <open62541/plugin/historydata/history_data_backend_memory.h>
#include <open62541/plugin/historydata/history_data_gathering_default.h>
#include <open62541/plugin/historydata/history_database_default.h>
#include <open62541/plugin/historydatabase.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>

extern "C" {
#include "csa_namespace.h"
#include "loggingLevel_type.h"
#include "unistd.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
}

#include "csa_additionalvariable.h"
#include "csa_config.h"
#include "csa_processvariable.h"
#include "ua_adapter.h"
#include "ua_map_types.h"
#include "xml_file_handler.h"

#include <functional>
#include <regex>
#include <string>

// These defines allow the use of an define as a string
#define xstr(a) str(a)
#define str(a) #a

using namespace std;

namespace ChimeraTK {
  ua_uaadapter::ua_uaadapter(const string& configFile) : ua_mapped_class() {
    this->mappingExceptions = UA_FALSE;
    this->fileHandler = std::make_shared<xml_file_handler>(configFile);
    this->readConfig();
    this->constructServer();
    this->mapSelfToNamespace();
  }

  ua_uaadapter::~ua_uaadapter() {
    UA_Server_delete(this->mappedServer);
    for(auto ptr : variables) delete ptr;
    for(auto ptr : additionalVariables) delete ptr;
    for(auto ptr : mappedVariables) delete ptr;
    for(auto dir : folderVector) {
      UA_NodeId_clear(&dir.folderNodeId);
    }
    UA_NodeId_clear(&this->ownNodeId);
    UA_NodeId_clear(&this->configNodeId);
  }

  static string cleanUri(string s) {
    // replace multiple with a single space
    std::regex r1("\\s+");
    s = std::regex_replace(s, r1, " ");
    s = std::regex_replace(s, r1, "_");
    // remove all not printable basic ASCII character
    std::regex r2("[^ -~]+");
    s = std::regex_replace(s, r2, "");
    return s;
  }

  static UA_ByteString loadFile(const char* const path) {
    UA_ByteString fileContents = UA_STRING_NULL;

    /* Open the file */
    FILE* fp = fopen(path, "rb");
    if(!fp) {
      errno = 0; /* We read errno also from the tcp layer... */
      return fileContents;
    }

    /* Get the file length, allocate the data and read */
    fseek(fp, 0, SEEK_END);
    fileContents.length = (size_t)ftell(fp);
    fileContents.data = (UA_Byte*)UA_malloc(fileContents.length * sizeof(UA_Byte));
    if(fileContents.data) {
      fseek(fp, 0, SEEK_SET);
      size_t read = fread(fileContents.data, sizeof(UA_Byte), fileContents.length, fp);
      if(read != fileContents.length) UA_ByteString_clear(&fileContents);
    }
    else {
      fileContents.length = 0;
    }
    fclose(fp);

    return fileContents;
  }

  void ua_uaadapter::fillBuildInfo(UA_ServerConfig* config) const {
    /*get hostname */
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    string hostname_uri = "urn:";
    hostname_uri.append(hostname);
    hostname_uri.append(":ChimeraTK:" + this->serverConfig.applicationName);

    config->applicationDescription.applicationType = UA_APPLICATIONTYPE_SERVER;
    UA_String* hostName = UA_String_new();
    *hostName = UA_STRING_ALLOC(hostname);
    config->applicationDescription.discoveryUrls = hostName;
    config->applicationDescription.discoveryUrlsSize = 1;
    UA_String_clear(&config->applicationDescription.applicationUri);
    config->applicationDescription.applicationUri = UA_STRING_ALLOC(const_cast<char*>(cleanUri(hostname_uri).c_str()));
    UA_String_clear(&config->buildInfo.manufacturerName);
    config->buildInfo.manufacturerName = UA_STRING_ALLOC(const_cast<char*>("ChimeraTK Team"));
    std::string
        versionString =
            "open62541: " xstr(UA_OPEN62541_VER_MAJOR) "." xstr(UA_OPEN62541_VER_MINOR) "." xstr(UA_OPEN62541_VER_PATCH) ", ControlSystemAdapter-OPC-UA-Adapter: " xstr(
                PROJECT_VER_MAJOR) "." xstr(PROJECT_VER_MINOR) "." xstr(PTOJECT_VER_PATCH) "";
    UA_String_clear(&config->buildInfo.softwareVersion);
    config->buildInfo.softwareVersion = UA_STRING_ALLOC(const_cast<char*>(versionString.c_str()));
    config->buildInfo.buildDate = UA_DateTime_now();
    UA_String_clear(&config->buildInfo.buildNumber);
    config->buildInfo.buildNumber = UA_STRING_ALLOC(const_cast<char*>(""));

    UA_String_clear(&config->buildInfo.productName);
    config->buildInfo.productName = UA_STRING_ALLOC(const_cast<char*>(this->serverConfig.applicationName.c_str()));
    string product_urn = "urn:ChimeraTK:" + this->serverConfig.applicationName;
    UA_String_clear(&config->buildInfo.productUri);
    config->buildInfo.productUri = UA_STRING_ALLOC(const_cast<char*>(cleanUri(product_urn).c_str()));
    UA_LocalizedText_clear(&config->applicationDescription.applicationName);
    config->applicationDescription.applicationName = UA_LOCALIZEDTEXT_ALLOC(
        const_cast<char*>("en_US"), const_cast<char*>(this->serverConfig.applicationName.c_str()));
  }

  void ua_uaadapter::constructServer() {
    auto config = (UA_ServerConfig*)UA_calloc(1, sizeof(UA_ServerConfig));
    config->logging = &logger;
    if(!this->serverConfig.enableSecurity) {
      UA_ServerConfig_setMinimal(config, this->serverConfig.opcuaPort, nullptr);
    }
    config->eventLoop->logger = &logger;
    if(this->serverConfig.enableSecurity) {
      UA_ByteString certificate = UA_BYTESTRING_NULL;
      UA_ByteString privateKey = UA_BYTESTRING_NULL;
      size_t trustListSize = 0;
      UA_ByteString* trustList = nullptr;
      size_t issuerListSize = 0;
      UA_ByteString* issuerList = nullptr;
      size_t blockListSize = 0;
      UA_ByteString* blockList = nullptr;

      /* Load certificate and private key */
      certificate = loadFile(this->serverConfig.certPath.c_str());
      privateKey = loadFile(this->serverConfig.keyPath.c_str());
      if(UA_ByteString_equal(&certificate, &UA_BYTESTRING_NULL) ||
          UA_ByteString_equal(&privateKey, &UA_BYTESTRING_NULL)) {
        UA_LOG_WARNING(server_config->logging, UA_LOGCATEGORY_USERLAND,
            "Invalid security configuration. Can't load private key or certificate.");
      }

      /* Load trust list */
      struct dirent* entry = nullptr;
      DIR* dp = nullptr;
      dp = opendir(this->serverConfig.allowListFolder.c_str());
      if(dp != nullptr) {
        while((entry = readdir(dp))) {
          if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
          trustListSize++;
          trustList = (UA_ByteString*)UA_realloc(trustList, sizeof(UA_ByteString) * trustListSize);
          char sbuf[1024];
          sprintf(sbuf, "%s/%s", this->serverConfig.allowListFolder.c_str(), entry->d_name);
          printf("Trust List entry:  %s\n", entry->d_name);
          trustList[trustListSize - 1] = loadFile(sbuf);
        }
      }
      closedir(dp);

      /* Load Issuer list */
      dp = nullptr;
      dp = opendir(this->serverConfig.issuerListFolder.c_str());
      if(dp != nullptr) {
        while((entry = readdir(dp))) {
          if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
          issuerListSize++;
          issuerList = (UA_ByteString*)UA_realloc(issuerList, sizeof(UA_ByteString) * issuerListSize);
          char sbuf[1024];
          sprintf(sbuf, "%s/%s", this->serverConfig.issuerListFolder.c_str(), entry->d_name);
          printf("Issuer List entry:  %s\n", entry->d_name);
          issuerList[issuerListSize - 1] = loadFile(sbuf);
        }
      }
      closedir(dp);

      /* Load Block list */
      dp = nullptr;
      dp = opendir(this->serverConfig.blockListFolder.c_str());
      if(dp != nullptr) {
        while((entry = readdir(dp))) {
          if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
          blockListSize++;
          blockList = (UA_ByteString*)UA_realloc(blockList, sizeof(UA_ByteString) * blockListSize);
          char sbuf[1024];
          sprintf(sbuf, "%s/%s", this->serverConfig.blockListFolder.c_str(), entry->d_name);
          printf("Block List entry:  %s\n", entry->d_name);
          blockList[blockListSize - 1] = loadFile(sbuf);
        }
      }
      closedir(dp);

      // setup encrypted endpoints
      UA_StatusCode retval = UA_ServerConfig_setDefaultWithSecurityPolicies(config, this->serverConfig.opcuaPort,
          &certificate, &privateKey, trustList, trustListSize, issuerList, issuerListSize, blockList, blockListSize);

      if(retval != UA_STATUSCODE_GOOD) {
        throw std::runtime_error("Failed setting up server endpoints.");
      }

      if(!this->serverConfig.unsecure) {
        for(size_t i = 0; i < config->endpointsSize; i++) {
          UA_EndpointDescription* ep = &config->endpoints[i];
          if(ep->securityMode != UA_MESSAGESECURITYMODE_NONE) continue;

          UA_EndpointDescription_clear(ep);
          // Move the last to this position
          if(i + 1 < config->endpointsSize) {
            config->endpoints[i] = config->endpoints[config->endpointsSize - 1];
            i--;
          }
          config->endpointsSize--;
        }
        // Delete the entire array if the last Endpoint was removed
        if(config->endpointsSize == 0) {
          UA_free(config->endpoints);
          config->endpoints = nullptr;
        }
      }

      UA_ByteString_clear(&certificate);
      UA_ByteString_clear(&privateKey);
      for(size_t i = 0; i < trustListSize; i++) UA_ByteString_clear(&trustList[i]);
    }
    fillBuildInfo(config);
    for(size_t i = 0; i < config->endpointsSize; ++i) {
      UA_ApplicationDescription_clear(&config->endpoints[i].server);
      UA_ApplicationDescription_copy(&config->applicationDescription, &config->endpoints[i].server);
    }

    this->mappedServer = UA_Server_newWithConfig(config);
    this->server_config = UA_Server_getConfig(this->mappedServer);

    customType[0] = LoggingLevelType;
    this->customDataTypes.reset(new UA_DataTypeArray{this->server_config->customDataTypes, 1, customType, UA_FALSE});
    this->server_config->customDataTypes = customDataTypes.get();

    // Username/Password handling
    auto* usernamePasswordLogins = new UA_UsernamePasswordLogin; //!< Brief description after the member
    usernamePasswordLogins->password = UA_STRING_ALLOC(const_cast<char*>(this->serverConfig.password.c_str()));
    usernamePasswordLogins->username = UA_STRING_ALLOC(const_cast<char*>(this->serverConfig.username.c_str()));
    UA_AccessControl_default(this->server_config, !this->serverConfig.UsernamePasswordLogin,
        &this->server_config->securityPolicies[this->server_config->securityPoliciesSize - 1].policyUri, 1,
        usernamePasswordLogins);

    this->baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    csa_namespace_init(this->mappedServer);
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

      /*
       * result = this->fileHandler->getNodeSet(xpath + "//process_variable_hierarchy");
  if(result) {
    xmlNodeSetPtr nodeset = result->nodesetval;
    vector<xmlNodePtr> nodeVectorUnrollPathPV =
        xml_file_handler::getNodesByName(nodeset->nodeTab[0]->children, "unroll");
    for(auto nodeUnrollPath : nodeVectorUnrollPathPV) {
      string unrollSepEnabled = xml_file_handler::getContentFromNode(nodeUnrollPath);
      transform(unrollSepEnabled.begin(), unrollSepEnabled.end(), unrollSepEnabled.begin(), ::toupper);
      if(unrollSepEnabled == "TRUE") {
        this->pvSeperator += xml_file_handler::getAttributeValueFromNode(nodeUnrollPath, "pathSep");
      }
    }
       */
      vector<xmlNodePtr> mappingExceptionsVector =
          xml_file_handler::getNodesByName(nodeset->nodeTab[0]->children, "mapping_exceptions");
      if(mappingExceptionsVector.empty()) {
        this->mappingExceptions = UA_FALSE;
      }
      else {
        placeHolder = xml_file_handler::getContentFromNode(mappingExceptionsVector[0]);
        transform(placeHolder.begin(), placeHolder.end(), placeHolder.begin(), ::toupper);
        if(placeHolder == "TRUE") {
          this->mappingExceptions = UA_TRUE;
        }
        else {
          this->mappingExceptions = UA_FALSE;
        }
      }

      xmlXPathObjectPtr sub_result = this->fileHandler->getNodeSet(xpath + "//server");
      if(result) {
        xmlNodeSetPtr nodeset = sub_result->nodesetval;
        if(nodeset->nodeNr > 1) {
          throw std::runtime_error("To many <server>-Tags in config file");
        }

        placeHolder = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "logLevel");
        if(!placeHolder.empty()) {
          transform(placeHolder.begin(), placeHolder.end(), placeHolder.begin(), ::toupper);
          if(placeHolder.compare("TRACE") == 0) {
            this->serverConfig.logLevel = UA_LOGLEVEL_TRACE;
          }
          else if(placeHolder.compare("DEBUG") == 0) {
            this->serverConfig.logLevel = UA_LOGLEVEL_DEBUG;
          }
          else if(placeHolder.compare("INFO") == 0) {
            this->serverConfig.logLevel = UA_LOGLEVEL_INFO;
          }
          else if(placeHolder.compare("WARNING") == 0) {
            this->serverConfig.logLevel = UA_LOGLEVEL_WARNING;
          }
          else if(placeHolder.compare("ERROR") == 0) {
            this->serverConfig.logLevel = UA_LOGLEVEL_ERROR;
          }
          else if(placeHolder.compare("FATAL") == 0) {
            this->serverConfig.logLevel = UA_LOGLEVEL_FATAL;
          }
          else {
            // See default set in ServerConfig
            // Use UA_Log_Stdout because the logger does not yet exist, but log level INFO is used so
            // writing a warning is ok here.
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Unknown logLevel (\"%s\") found in config file. INFO is used instead!", placeHolder.c_str());
          }
        }
        else {
          // Use UA_Log_Stdout because the logger does not yet exist, but log level INFO is used so
          // writing a warning is ok here.
          UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
              "No 'logLevel'-Attribute set in config file. Use default logging level: info");
        }
        logger = UA_Log_Stdout_withLevel(this->serverConfig.logLevel);

        string opcuaPort = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "port");
        if(!opcuaPort.empty()) {
          this->serverConfig.opcuaPort = std::stoi(opcuaPort);
        }
        else {
          UA_LOG_WARNING(
              &logger, UA_LOGCATEGORY_USERLAND, "No 'port'-Attribute in config file is set. Use default Port: 16664");
        }

        placeHolder = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "applicationName");
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
            this->serverConfig.applicationName = applicationName;
          }
          catch(ChimeraTK::logic_error) {
          }
          UA_LOG_WARNING(&logger, UA_LOGCATEGORY_USERLAND,
              "No 'applicationName'-Attribute is set in config file. Use default application-name.");
        }

        // if no root folder name is set, use application name
        if(this->serverConfig.rootFolder.empty()) {
          this->serverConfig.rootFolder = this->serverConfig.applicationName;
        }
        xmlXPathFreeObject(sub_result);
      }
      else {
        logger = UA_Log_Stdout_withLevel(this->serverConfig.logLevel);
        UA_LOG_WARNING(&logger, UA_LOGCATEGORY_USERLAND,
            "No <server>-Tag in config file. Use default port 16664 and application name configuration.");
        this->serverConfig.rootFolder = this->serverConfig.applicationName;
      }

      // check if historizing is configured
      vector<xmlNodePtr> historizing = xml_file_handler::getNodesByName(nodeset->nodeTab[0]->children, "historizing");
      if(!historizing.empty()) {
        xmlXPathObjectPtr sub_sub_result = this->fileHandler->getNodeSet("//historizing");
        if(sub_sub_result) {
          xmlNodeSetPtr node_set = sub_sub_result->nodesetval;
          for(int32_t i = 0; i < node_set->nodeNr; i++) {
            vector<xmlNodePtr> nodeVectorhistorizingSetUp =
                xml_file_handler::getNodesByName(node_set->nodeTab[i]->children, "setup");
            string val;
            for(auto& nodeHistorizingPath : nodeVectorhistorizingSetUp) {
              string history_name = xml_file_handler::getAttributeValueFromNode(nodeHistorizingPath, "name");
              AdapterHistorySetup temp;
              char* c;
              bool incomplete = false;
              if(!history_name.empty()) {
                temp.name = history_name;
              }
              else {
                raiseError("Missing history parameter 'name'.",
                    "History configuration is not added to the list of available history configurations",
                    nodeHistorizingPath->line);
                incomplete = true;
              }
              string history_buffer_length =
                  xml_file_handler::getAttributeValueFromNode(nodeHistorizingPath, "buffer_length");
              if(!history_buffer_length.empty()) {
                sscanf(history_buffer_length.c_str(), "%zu", &temp.buffer_length);
              }
              else {
                raiseError("Missing history parameter 'buffer_length'.",
                    "History configuration " + history_name +
                        " is not added to the list of available history configurations",
                    nodeHistorizingPath->line);
                incomplete = true;
              }
              string history_entries_per_response =
                  xml_file_handler::getAttributeValueFromNode(nodeHistorizingPath, "entries_per_response");
              if(!history_entries_per_response.empty()) {
                sscanf(history_entries_per_response.c_str(), "%zu", &temp.entries_per_response);
              }
              else {
                raiseError("Missing history parameter 'entries_per_response'.",
                    "History configuration " + history_name +
                        " is not added to the list of available history configurations",
                    nodeHistorizingPath->line);
                incomplete = true;
              }
              string history_interval = xml_file_handler::getAttributeValueFromNode(nodeHistorizingPath, "interval");
              if(!history_interval.empty()) {
                sscanf(history_interval.c_str(), "%zu", &temp.interval);
              }
              else {
                raiseError("Missing history parameter 'interval'.",
                    "History configuration " + history_name +
                        " is not added to the list of available history configurations",
                    nodeHistorizingPath->line);
                incomplete = true;
              }

              // check if a configuration is redefined
              bool existing = false;
              for(size_t j = 0; j < this->serverConfig.history.size(); j++) {
                if(history_name == this->serverConfig.history[j].name) {
                  existing = true;
                }
              }
              if(!existing) {
                if(!incomplete) {
                  this->serverConfig.history.insert(this->serverConfig.history.end(), temp);
                }
              }
              else {
                raiseError("Redefinition of history configuration.",
                    "History configuration " + history_name +
                        " is not added to the list of available history configurations");
              }
            }
          }
        }
      }

      placeHolder = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "rootFolder");
      if(!placeHolder.empty()) {
        this->serverConfig.rootFolder = placeHolder;
      }

      placeHolder = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "description");
      if(!placeHolder.empty()) {
        this->serverConfig.descriptionFolder = placeHolder;
      }
      xmlXPathFreeObject(result);
    }
    else {
      logger = UA_Log_Stdout_withLevel(this->serverConfig.logLevel);
    }
    result = this->fileHandler->getNodeSet(xpath + "//login");
    if(result) {
      xmlNodeSetPtr nodeset = result->nodesetval;
      if(nodeset->nodeNr > 1) {
        throw std::runtime_error("To many <login>-Tags in config file");
      }
      this->serverConfig.UsernamePasswordLogin = UA_TRUE;
      placeHolder = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "password");
      if(!placeHolder.empty()) {
        this->serverConfig.password = placeHolder;
      }
      else {
        throw std::runtime_error("<login>-Tag requires username");
      }

      placeHolder = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "username");
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

    result = this->fileHandler->getNodeSet(xpath + "//security");
    if(result) {
      xmlNodeSetPtr nodeset = result->nodesetval;
      if(nodeset->nodeNr > 1) {
        throw std::runtime_error("To many <security>-Tags in config file");
      }
      this->serverConfig.enableSecurity = true;
      string unsecure = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "unsecure");
      if(!unsecure.empty()) {
        transform(unsecure.begin(), unsecure.end(), unsecure.begin(), ::toupper);
        if(unsecure.compare("TRUE") == 0) {
          this->serverConfig.unsecure = true;
        }
        else {
          this->serverConfig.unsecure = false;
        }
      }
      else {
        this->serverConfig.unsecure = false;
        UA_LOG_WARNING(&logger, UA_LOGCATEGORY_USERLAND,
            "No 'unsecure'-Attribute in config file is set. Disable unsecure endpoints");
      }
      string certPath = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "certificate");
      if(!certPath.empty()) {
        this->serverConfig.certPath = certPath;
      }
      else {
        UA_LOG_WARNING(&logger, UA_LOGCATEGORY_USERLAND,
            "Invalid security configuration. No 'certificate'-Attribute in config file is set.");
      }
      string keyPath = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "privatekey");
      if(!keyPath.empty()) {
        this->serverConfig.keyPath = keyPath;
      }
      else {
        UA_LOG_WARNING(&logger, UA_LOGCATEGORY_USERLAND,
            "Invalid security configuration. No 'privatekey'-Attribute in config file is set.");
      }
      string allowListFolder = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "trustlist");
      if(!allowListFolder.empty()) {
        this->serverConfig.allowListFolder = allowListFolder;
      }
      else {
        UA_LOG_WARNING(&logger, UA_LOGCATEGORY_USERLAND,
            "Invalid security configuration. No 'trustlist'-Attribute in config file is set.");
      }
      string blockListFolder = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "blocklist");
      if(!blockListFolder.empty()) {
        this->serverConfig.blockListFolder = blockListFolder;
      }
      else {
        UA_LOG_WARNING(&logger, UA_LOGCATEGORY_USERLAND, "No 'blockListFolder'-Attribute in config file is set.");
      }
      string issuerListFolder = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[0], "issuerlist");
      if(!issuerListFolder.empty()) {
        this->serverConfig.issuerListFolder = issuerListFolder;
      }
      else {
        UA_LOG_WARNING(&logger, UA_LOGCATEGORY_USERLAND, "No 'issuerListFolder'-Attribute in config file is set.");
      }
      xmlXPathFreeObject(result);
    }
    else {
      UA_LOG_WARNING(&logger, UA_LOGCATEGORY_USERLAND,
          "No <security>-Tag in config file. Use default configuration with unsecure endpoint.");
      this->serverConfig.enableSecurity = false;
    }
    result = this->fileHandler->getNodeSet(xpath + "//process_variable_hierarchy");
    if(result) {
      xmlNodeSetPtr nodeset = result->nodesetval;
      vector<xmlNodePtr> nodeVectorUnrollPathPV =
          xml_file_handler::getNodesByName(nodeset->nodeTab[0]->children, "unroll");
      for(auto nodeUnrollPath : nodeVectorUnrollPathPV) {
        string unrollSepEnabled = xml_file_handler::getContentFromNode(nodeUnrollPath);
        transform(unrollSepEnabled.begin(), unrollSepEnabled.end(), unrollSepEnabled.begin(), ::toupper);
        if(unrollSepEnabled == "TRUE") {
          this->pvSeperator += xml_file_handler::getAttributeValueFromNode(nodeUnrollPath, "pathSep");
        }
      }
      xmlXPathFreeObject(result);
    }
    else {
      UA_LOG_WARNING(&logger, UA_LOGCATEGORY_USERLAND,
          "No <process_variable_hierarchy>-Tag in config file. Use default hierarchical mapping with '/'.");
      this->pvSeperator = "/";
    }

    xmlXPathObjectPtr result_exclude = this->fileHandler->getNodeSet("//exclude");
    xmlNodeSetPtr nodeset;
    if(result_exclude) {
      nodeset = result_exclude->nodesetval;
      for(int32_t i = 0; i < nodeset->nodeNr; i++) {
        string exclude_string = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[i], "sourceName");
        if(!exclude_string.empty()) {
          this->exclude.insert(this->exclude.begin(), "/" + exclude_string);
        }
      }
    }
    xmlXPathFreeObject(result_exclude);

    xmlXPathObjectPtr folder_with_his = this->fileHandler->getNodeSet("//folder");
    xmlNodeSetPtr nodeset_folder_with_history;
    if(folder_with_his) {
      nodeset_folder_with_history = folder_with_his->nodesetval;
      for(int32_t i = 0; i < nodeset_folder_with_history->nodeNr; i++) {
        string folder =
            xml_file_handler::getAttributeValueFromNode(nodeset_folder_with_history->nodeTab[i], "sourceName");
        string history =
            xml_file_handler::getAttributeValueFromNode(nodeset_folder_with_history->nodeTab[i], "history");
        if(!folder.empty() && !history.empty()) {
          this->folder_with_history.insert(this->folder_with_history.begin(), "/" + folder);
        }
      }
    }
    xmlXPathFreeObject(folder_with_his);
  }

  ServerConfig ua_uaadapter::get_server_config() {
    return this->serverConfig;
  }

  void ua_uaadapter::applyMapping(const boost::shared_ptr<ControlSystemPVManager>& csManager) {
    // build folder structure
    this->buildFolderStructure(csManager);
    // start explicit mapping
    this->explicitVarMapping(csManager);
    // add configured additional variables
    this->addAdditionalVariables();
  }

  void ua_uaadapter::workerThread() {
    if(this->mappedServer == nullptr) {
      UA_LOG_DEBUG(server_config->logging, UA_LOGCATEGORY_USERLAND, "No server mapped");
      return;
    }

    /*for(auto & variable : this->variables){
      check_void_type(variable);
    }*/

    vector<UA_NodeId> historizing_nodes;
    vector<string> historizing_setup;
    UA_HistoryDataGathering gathering =
        add_historizing_nodes(historizing_nodes, historizing_setup, this->mappedServer, this->server_config,
            this->serverConfig.history, this->serverConfig.historyfolders, this->serverConfig.historyvariables);
    UA_LOG_INFO(server_config->logging, UA_LOGCATEGORY_USERLAND, "Starting the server worker thread");
    UA_Server_run_startup(this->mappedServer);
    this->running = true;
    while(this->running) {
      UA_Server_run_iterate(this->mappedServer, true);
    }
    clear_history(gathering, historizing_nodes, historizing_setup, this->mappedServer,
        this->serverConfig.historyfolders, this->serverConfig.historyvariables, this->server_config);

    UA_Server_run_shutdown(this->mappedServer);
    UA_LOG_INFO(server_config->logging, UA_LOGCATEGORY_USERLAND, "Stopped the server worker thread");
  }

  UA_NodeId ua_uaadapter::enrollFolderPathFromString(const string& path, const string& seperator) {
    vector<string> varPathVector;
    if(!seperator.empty()) {
      vector<string> newPathVector = xml_file_handler::parseVariablePath(path, seperator);
      varPathVector.insert(varPathVector.end(), newPathVector.begin(), newPathVector.end());
    }
    if(!varPathVector.empty()) { // last element is the variable name itself
      varPathVector.pop_back();
      return this->createFolderPath(this->ownNodeId, varPathVector);
    }
    return UA_NODEID_NULL;
  }

  void ua_uaadapter::implicitVarMapping(
      const std::string& varName, const boost::shared_ptr<ControlSystemPVManager>& csManager) {
    UA_NodeId folderPathNodeId = enrollFolderPathFromString(varName, this->pvSeperator);
    ua_processvariable* processvariable;
    if(!UA_NodeId_isNull(&folderPathNodeId)) {
      processvariable =
          new ua_processvariable(this->mappedServer, folderPathNodeId, varName.substr(1, varName.size() - 1), csManager,
              server_config->logging, xml_file_handler::parseVariablePath(varName, this->pvSeperator).back());
    }
    else {
      processvariable = new ua_processvariable(this->mappedServer, this->ownNodeId,
          varName.substr(1, varName.size() - 1), csManager, server_config->logging);
    }
    this->variables.push_back(processvariable);
    UA_NodeId tmpNodeId = processvariable->getOwnNodeId();
    UA_Server_writeDisplayName(this->mappedServer, tmpNodeId,
        UA_LOCALIZEDTEXT(const_cast<char*>("en_US"),
            const_cast<char*>(xml_file_handler::parseVariablePath(varName, this->pvSeperator).back().c_str())));
    UA_NodeId_clear(&tmpNodeId);
  }

  void ua_uaadapter::deepCopyHierarchicalLayer(
      const boost::shared_ptr<ControlSystemPVManager>& csManager, UA_NodeId layer, UA_NodeId target) {
    // copy pv's of current layer
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
      // get unit, desc, ..
      UA_LocalizedText foundPVName;
      string foundPVNameCPP;
      UA_Server_readDisplayName(this->mappedServer, rd.nodeId.nodeId, &foundPVName);
      UASTRING_TO_CPPSTRING(foundPVName.text, foundPVNameCPP)

      string pvSourceNameid;
      UA_String foundPVSourceName;
      string foundPVSourceNameCPP;
      UA_Variant value;
      UA_STRING_TO_CPPSTRING_COPY(&rd.nodeId.nodeId.identifier.string, &pvSourceNameid)
      UA_Server_readValue(
          this->mappedServer, UA_NODEID_STRING(1, const_cast<char*>((pvSourceNameid + "/Name").c_str())), &value);
      foundPVSourceName = *((UA_String*)value.data);
      UASTRING_TO_CPPSTRING(foundPVSourceName, foundPVSourceNameCPP)
      string varName = xml_file_handler::parseVariablePath(foundPVNameCPP, "/").back();
      auto* processvariable = new ua_processvariable(
          this->mappedServer, target, foundPVSourceNameCPP, csManager, server_config->logging, foundPVNameCPP);
      this->variables.push_back(processvariable);
      UA_Variant_clear(&value);
      UA_LocalizedText_clear(&foundPVName);
    }
    UA_BrowseResult_clear(&br);
    // copy folders of current layer
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
      UASTRING_TO_CPPSTRING(foundFolderName.text, foundFolderNameCPP)
      if(result != UA_STATUSCODE_GOOD) {
        // error handling
        continue;
      }
      UA_NodeId newRootFolder = createFolder(target, foundFolderNameCPP);
      if(foundFolderDescription.text.length > 0)
        UA_Server_writeDescription(this->mappedServer, newRootFolder, foundFolderDescription);
      deepCopyHierarchicalLayer(csManager, rd.nodeId.nodeId, newRootFolder);
      UA_LocalizedText_clear(&foundFolderName);
      UA_LocalizedText_clear(&foundFolderDescription);
    }
    UA_BrowseResult_clear(&br);
  }

  void ua_uaadapter::buildFolderStructure(const boost::shared_ptr<ControlSystemPVManager>& csManager) {
    xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//folder");
    xmlNodeSetPtr nodeset;
    if(result) {
      nodeset = result->nodesetval;

      for(int32_t i = 0; i < nodeset->nodeNr; i++) {
        UA_NodeId folderPathNodeId;
        string destination, description, folder;
        vector<xmlNodePtr> nodeFolderPath =
            xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "destination");
        vector<xmlNodePtr> nodeFolder = xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "name");
        vector<xmlNodePtr> nodeDescription =
            xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "description");
        string sourceName = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[i], "sourceName");
        string copy = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[i], "copy");
        transform(copy.begin(), copy.end(), copy.begin(), ::toupper);
        if(!nodeFolderPath.empty()) {
          destination = xml_file_handler::getContentFromNode(nodeFolderPath[0]);
        }
        if(!nodeDescription.empty()) {
          description = xml_file_handler::getContentFromNode(nodeDescription[0]);
        }
        if(!nodeFolder.empty()) {
          folder = xml_file_handler::getContentFromNode(nodeFolder[0]);
        }
        string history = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[i], "history");
        if(!history.empty()) {
          // After discussion we only allow history if source name is set.
          // Else the exclude logic would become more complex!
          if(sourceName.empty()) {
            raiseError("Can not add history if no sourceName attribute is set.", "", nodeset->nodeTab[i]->line);
          }
          if(!nodeFolder.empty()) {
            string folderNodeId;
            if(!nodeFolderPath.empty()) {
              if(strlen(destination.c_str()) == 0) {
                folderNodeId = this->serverConfig.rootFolder + "/" + folder + "Dir";
              }
              else {
                folderNodeId = this->serverConfig.rootFolder + "/" + destination + "/" + folder + "Dir";
              }
            }
            else {
              folderNodeId = this->serverConfig.rootFolder + "/" + folder + "Dir";
            }

            AdapterFolderHistorySetup temp;
            temp.folder_historizing = history;
            UA_NodeId id = UA_NODEID_STRING(1, const_cast<char*>(folderNodeId.c_str()));
            UA_NodeId_copy(&id, &temp.folder_id);
            this->serverConfig.historyfolders.insert(this->serverConfig.historyfolders.end(), temp);
            UA_String out = UA_STRING_NULL;
            UA_print(&temp.folder_id, &UA_TYPES[UA_TYPES_NODEID], &out);
            UA_LOG_INFO(server_config->logging, UA_LOGCATEGORY_USERLAND,
                "Adding history for folder from destination and name %.*s ", (int)out.length, out.data);
            UA_String_clear(&out);
          }
          else if(copy.empty() || copy == "FALSE") {
            AdapterFolderHistorySetup temp;
            temp.folder_historizing = history;
            string folderNodeId = this->serverConfig.rootFolder + "/" + sourceName + "Dir";
            UA_NodeId id = UA_NODEID_STRING(1, const_cast<char*>(folderNodeId.c_str()));
            UA_NodeId_copy(&id, &temp.folder_id);
            this->serverConfig.historyfolders.insert(this->serverConfig.historyfolders.end(), temp);
            UA_String out = UA_STRING_NULL;
            UA_print(&temp.folder_id, &UA_TYPES[UA_TYPES_NODEID], &out);
            UA_LOG_INFO(server_config->logging, UA_LOGCATEGORY_USERLAND,
                "Adding history for folder from source name %.*s ", (int)out.length, out.data);
            UA_String_clear(&out);
          }
          else {
            raiseError(
                "Can not add history if copy is true and no source name is given.", "", nodeset->nodeTab[i]->line);
          }
        }

        if(folder.empty()) {
          // only if no history is set raise error/warning
          if(history.empty()) {
            raiseError("Folder creation failed. Name is missing.", "Skipping Folder.", nodeset->nodeTab[i]->line);
          }
          continue;
        }
        // check if a pv with the requested node id exists
        UA_NodeId tmpOutput = UA_NODEID_NULL;
        string pvNodeString;
        if(destination.empty()) {
          pvNodeString += this->serverConfig.rootFolder + "/" + folder + "Dir";
        }
        else {
          pvNodeString += this->serverConfig.rootFolder + "/" + destination + "/" + folder + "Dir";
        }
        UA_NodeId pvNode = UA_NODEID_STRING(1, const_cast<char*>(pvNodeString.c_str()));
        UA_Server_readNodeId(this->mappedServer, pvNode, &tmpOutput);
        if(!UA_NodeId_isNull(&tmpOutput)) {
          UA_NodeId_clear(&tmpOutput);
          // set folder description
          if(sourceName.empty() && !description.empty()) {
            UA_Server_writeDescription(this->mappedServer, pvNode,
                UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(description.c_str())));
            continue;
          }
          raiseError("Folder with source mapping failed. Target folder node id exists.",
              std::string("Skipping folder: ") + sourceName, nodeset->nodeTab[i]->line);
          continue;
        }
        if(!copy.empty() && sourceName.empty()) {
          raiseError(std::string("Source 'name: ") + folder + "' folder missing.", "Skipping Folder.",
              nodeset->nodeTab[i]->line);
          continue;
        }
        folderPathNodeId = enrollFolderPathFromString(destination + "/replacePart", "/");
        if(UA_NodeId_isNull(&folderPathNodeId)) {
          folderPathNodeId = UA_NODEID_STRING(1, const_cast<char*>((this->serverConfig.rootFolder + "Dir").c_str()));
        }
        // check if source name is set -> map complete hierarchical structure to the destination
        if(!sourceName.empty()) {
          if((destination.empty() && sourceName == folder) ||
              (!destination.empty() && sourceName == destination + "/" + folder)) {
            raiseError(
                "Folder creation failed. Source and Destination equal.", "Skipping Folder.", nodeset->nodeTab[i]->line);
            continue;
          }
          // check if the src is a folder
          string sourceFolder = this->serverConfig.rootFolder + "/" + sourceName + "Dir";
          bool isFolderType = false;
          UA_NodeId sourceFolderId = UA_NODEID_STRING(1, const_cast<char*>(sourceFolder.c_str()));

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
            raiseError(std::string("Folder creation failed. No corresponding source folder: ") + sourceName,
                "Skipping Folder.", nodeset->nodeTab[i]->line);
            continue;
          }
          // enroll path destination -> copy / link the complete tree to this place
          if(copy == "TRUE") {
            bool sourceAndDestinationEqual = false;
            if(!destination.empty()) {
              if(sourceName == destination + "/" + folder) sourceAndDestinationEqual = true;
            }
            else {
              if(sourceName == folder) sourceAndDestinationEqual = true;
            }
            if(sourceAndDestinationEqual) {
              raiseError(std::string("Folder creation failed. Source and destination must be different for '") +
                      sourceName + "' folder.",
                  "Skipping Folder.", nodeset->nodeTab[i]->line);
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
                existingDestinationFolderString = this->serverConfig.rootFolder +=
                    "/" + destination + "/" + folder + "Dir";
              }
              copyRoot = UA_NODEID_STRING(1, const_cast<char*>(existingDestinationFolderString.c_str()));
            }
            deepCopyHierarchicalLayer(csManager, sourceFolderId, copyRoot);
            if(!description.empty()) {
              UA_Server_writeDescription(this->mappedServer, copyRoot,
                  UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(description.c_str())));
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
                existingDestinationFolderString = this->serverConfig.rootFolder + "/" + destination + "/" + folder;
              }
              copyRoot = UA_NODEID_STRING(1, const_cast<char*>(existingDestinationFolderString.c_str()));
            }
            else {
              if(!description.empty()) {
                UA_Server_writeDescription(this->mappedServer, copyRoot,
                    UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(description.c_str())));
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

        UA_NodeId retnode = createFolder(folderPathNodeId, folder);
        // set folder description
        if(copy.empty() && sourceName.empty() && !description.empty()) {
          UA_Server_writeDescription(this->mappedServer, retnode,
              UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(description.c_str())));
        }
      }

      xmlXPathFreeObject(result);
    }
  }

  void ua_uaadapter::explicitVarMapping(const boost::shared_ptr<ControlSystemPVManager>& csManager) {
    xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//process_variable");
    xmlNodeSetPtr nodeset;
    if(result) {
      nodeset = result->nodesetval;
      for(int32_t i = 0; i < nodeset->nodeNr; i++) {
        string sourceName, copy, destination, name, unit, description, unrollPath, history;
        vector<xmlNodePtr> nodeDestination =
            xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "destination");
        vector<xmlNodePtr> nodeName = xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "name");

        vector<xmlNodePtr> nodeUnit = xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "unit");
        vector<xmlNodePtr> nodeDescription =
            xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "description");

        copy = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[i], "copy");
        transform(copy.begin(), copy.end(), copy.begin(), ::toupper);

        sourceName = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[i], "sourceName");
        history = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[i], "history");
        if(!history.empty()) {
          AdapterPVHistorySetup temp;
          temp.variable_historizing = history;
          string targetNodeId;
          // get the name of the variable
          if(!nodeName.empty()) {
            if(nodeDestination.empty()) {
              targetNodeId = this->serverConfig.rootFolder + "/" + xml_file_handler::getContentFromNode(nodeName[0]);
              UA_NodeId id = UA_NODEID_STRING(1, const_cast<char*>(targetNodeId.c_str()));
              UA_NodeId_copy(&id, &temp.variable_id);
              this->serverConfig.historyvariables.insert(this->serverConfig.historyvariables.end(), temp);
            }
            else {
              targetNodeId = this->serverConfig.rootFolder + "/" +
                  xml_file_handler::getContentFromNode(nodeDestination[0]) + "/" +
                  xml_file_handler::getContentFromNode(nodeName[0]);
              UA_NodeId id = UA_NODEID_STRING(1, const_cast<char*>(targetNodeId.c_str()));
              UA_NodeId_copy(&id, &temp.variable_id);
              this->serverConfig.historyvariables.insert(this->serverConfig.historyvariables.end(), temp);
            }
          }
          if(!sourceName.empty() && (copy.empty() || copy == "FALSE")) {
            name = sourceName;
            targetNodeId = this->serverConfig.rootFolder + "/" + sourceName;
            UA_NodeId id = UA_NODEID_STRING(1, const_cast<char*>(targetNodeId.c_str()));
            UA_NodeId_copy(&id, &temp.variable_id);
            this->serverConfig.historyvariables.insert(this->serverConfig.historyvariables.end(), temp);
          }
        }

        if(!nodeDestination.empty()) {
          destination = xml_file_handler::getContentFromNode(nodeDestination[0]);
          unrollPath = xml_file_handler::getAttributeValueFromNode(nodeDestination[0], "unrollPath");
        }
        if(!nodeName.empty()) {
          name = xml_file_handler::getContentFromNode(nodeName[0]);
        }
        if(!nodeUnit.empty()) {
          unit = xml_file_handler::getContentFromNode(nodeUnit[0]);
        }
        if(!nodeDescription.empty()) {
          description = xml_file_handler::getContentFromNode(nodeDescription[0]);
        }

        if(sourceName.empty()) {
          if(!name.empty()) {
            raiseError(std::string("PV mapping failed. SourceName missing for 'name' : '") + name,
                "Skipping PV mapping.", nodeset->nodeTab[i]->line);
          }
          else {
            raiseError("PV mapping failed. SourceName is missing.", "Skipping PV mapping.", nodeset->nodeTab[i]->line);
          }
          continue;
        }
        // check if the pv still exists -> update requested
        if(((destination + "/" + name) == sourceName) && copy == "FALSE") {
          // check if the source var exists
          string parentSourceFolder = this->serverConfig.rootFolder + "/" +
              (sourceName.substr(
                  0, sourceName.length() - xml_file_handler::parseVariablePath(sourceName, "/").back().length() - 1));
          UA_NodeId parentSourceFolderId = UA_NODEID_STRING(1, const_cast<char*>(parentSourceFolder.c_str()));
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
            if(name == xml_file_handler::parseVariablePath(sourceName, "/").back()) {
              UA_NodeId_copy(&br.references[j].nodeId.nodeId, &pvNodeId);
            }
          }
          UA_BrowseResult_clear(&br);
          if(UA_NodeId_isNull(&pvNodeId)) {
            if(!name.empty()) {
              raiseError(std::string("PV mapping failed. No corresponding source pv for 'name' : '") + name,
                  "Skipping PV mapping.", nodeset->nodeTab[i]->line);
            }
            else {
              raiseError(
                  "PV mapping failed. No corresponding source pv.", "Skipping PV mapping.", nodeset->nodeTab[i]->line);
            }
            continue;
          }
          if(!unit.empty()) {
            // update unit
            for(auto& variable : this->variables) {
              UA_NodeId tmpNodeId = variable->getOwnNodeId();
              if(UA_NodeId_equal(&tmpNodeId, &pvNodeId)) {
                variable->setEngineeringUnit(unit);
              }
              UA_NodeId_clear(&tmpNodeId);
            }
          }
          if(!description.empty()) {
            // update description
            for(auto& variable : this->variables) {
              UA_NodeId tmpNodeId = variable->getOwnNodeId();
              if(UA_NodeId_equal(&tmpNodeId, &pvNodeId)) {
                variable->setDescription(description);
              }
              UA_NodeId_clear(&tmpNodeId);
            }
          }

          if(!UA_NodeId_isNull(&pvNodeId)) {
            UA_NodeId_clear(&pvNodeId);
          }
          continue;
        }
        // check if the source pv exists
        string parentSourceString = this->serverConfig.rootFolder + "/" + sourceName;
        UA_NodeId parentSourceId = UA_NODEID_STRING(1, const_cast<char*>(parentSourceString.c_str()));
        UA_NodeId tmpOutput = UA_NODEID_NULL;
        UA_Server_readNodeId(this->mappedServer, parentSourceId, &tmpOutput);
        if(UA_NodeId_isNull(&tmpOutput)) {
          if(!name.empty()) {
            raiseError(std::string("PV mapping failed. Source PV not found 'name' : '") + name, "Skipping PV mapping.",
                nodeset->nodeTab[i]->line);
          }
          else {
            raiseError("PV mapping failed. Source PV not found", "Skipping PV mapping.", nodeset->nodeTab[i]->line);
          }
          continue;
        }
        else {
          UA_NodeId_clear(&tmpOutput);
          UA_NodeId_init(&tmpOutput);
        }
        // check the pv copy attribute -> copy of pv requested; false -> reference to original pv requested
        UA_NodeId createdNodeId = UA_NODEID_NULL;
        if(copy == "TRUE") {
          if(sourceName == (destination + "/" + name)) {
            raiseError("PV mapping failed. Source and destination must be different if copy='true'.",
                std::string("Skipping PV ") + sourceName, nodeset->nodeTab[i]->line);
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
              // legacy code to make old mapping possible
              if(destination.find("Variables" + unrollPath) == 0) {
                string requestedBrowseName = destination;
                requestedBrowseName.erase(0, ("Variables" + unrollPath).length());
                // check if the requested path still exists in the Variables folder
                string requestedPVBrowseName = this->serverConfig.rootFolder + "/Variables/" + requestedBrowseName;
                UA_NodeId requestedPVBrowseId = UA_NODEID_STRING(1, const_cast<char*>(requestedPVBrowseName.c_str()));
                UA_NodeId tmpOutput = UA_NODEID_NULL;
                UA_Server_readNodeId(this->mappedServer, requestedPVBrowseId, &tmpOutput);
                if(!UA_NodeId_isNull(&tmpOutput)) {
                  UA_LOG_WARNING(server_config->logging, UA_LOGCATEGORY_USERLAND,
                      "Using legacy code. Mapped PV is used as PV mapping target");
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
                this->mappedServer, destinationFolderNodeId, sourceName, csManager, server_config->logging, name);
            UA_NodeId tmpProcessVariableNodeId = processvariable->getOwnNodeId();
            if(UA_NodeId_isNull(&tmpProcessVariableNodeId)) {
              raiseError("PV creation failed. PV with same name mapped.", std::string("Skipping PV ") + sourceName,
                  nodeset->nodeTab[i]->line);
              continue;
            }
            else {
              UA_NodeId_clear(&tmpProcessVariableNodeId);
            }
            this->variables.push_back(processvariable);
          }
          else {
            raiseError("Folder creation failed.", std::string("Skipping PV ") + sourceName, nodeset->nodeTab[i]->line);
            continue;
          }
          UA_NodeId tmpPVNodeId = processvariable->getOwnNodeId();
          UA_NodeId_copy(&tmpPVNodeId, &createdNodeId);
          UA_NodeId_clear(&tmpPVNodeId);
        }
        else {
          // get node id of the source node
          string sourceVarName = xml_file_handler::parseVariablePath(sourceName, "/").back();
          if(name.empty()) {
            name = sourceVarName;
          }
          if(sourceVarName != name) {
            if(history.empty()) {
              raiseError("PV mapping failed. The pv name can't changed if copy is false.",
                  std::string("Skipping PV mapping of pv ") + name);
            }
            continue;
          }
          // create destination folder
          UA_NodeId destinationFolder = enrollFolderPathFromString(destination + "/removedpart", "/");
          UA_ExpandedNodeId enid;
          enid.serverIndex = 0;
          enid.namespaceUri = UA_STRING_NULL;
          enid.nodeId = parentSourceId;
          // add reference to the source node
          UA_StatusCode addRef = UA_Server_addReference(
              this->mappedServer, destinationFolder, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), enid, true);
          if(sourceVarName != name) {
            raiseError("PV mapping failed. Can't create reference to original pv.",
                std::string("Skipping PV mapping of pv ") + name);
            continue;
          }
          UA_NodeId_copy(&parentSourceId, &createdNodeId);
        }
        if(!unit.empty()) {
          for(auto& variable : this->variables) {
            UA_NodeId tmpNodeId = variable->getOwnNodeId();
            if(UA_NodeId_equal(&tmpNodeId, &createdNodeId)) {
              variable->setEngineeringUnit(unit);
            }
            UA_NodeId_clear(&tmpNodeId);
          }
        }
        if(!description.empty()) {
          for(auto& variable : this->variables) {
            UA_NodeId tmpNodeId = variable->getOwnNodeId();
            if(UA_NodeId_equal(&tmpNodeId, &createdNodeId)) {
              variable->setDescription(description);
            }
            UA_NodeId_clear(&tmpNodeId);
          }
        }
        UA_Server_writeDisplayName(this->mappedServer, createdNodeId,
            UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(name.c_str())));
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
            xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "destination");
        vector<xmlNodePtr> nodeName = xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "name");
        vector<xmlNodePtr> nodeDescription =
            xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "description");
        vector<xmlNodePtr> nodeValue = xml_file_handler::getNodesByName(nodeset->nodeTab[i]->children, "value");

        if(!nodeDestination.empty()) {
          destination = xml_file_handler::getContentFromNode(nodeDestination[0]);
        }
        if(!nodeName.empty()) {
          name = xml_file_handler::getContentFromNode(nodeName[0]);
        }
        if(!nodeDescription.empty()) {
          description = xml_file_handler::getContentFromNode(nodeDescription[0]);
        }
        if(!nodeValue.empty()) {
          value = xml_file_handler::getContentFromNode(nodeValue[0]);
        }
        // check if name is empty
        if(name.empty()) {
          raiseError("Additional variable node creation failed. Additional variable name is mandatory.",
              "Skipping additional variable.", nodeset->nodeTab[i]->line);
          continue;
        }
        // check if the av node still exists
        UA_NodeId tmpOutput = UA_NODEID_NULL;
        string avNodeString;
        if(destination.empty()) {
          avNodeString = this->serverConfig.rootFolder + "/" + name + "AdditionalVariable";
        }
        else {
          avNodeString = this->serverConfig.rootFolder + "/" + destination + "/" + name + "AdditionalVariable";
        }
        UA_NodeId avNode = UA_NODEID_STRING(1, const_cast<char*>(avNodeString.c_str()));
        UA_Server_readNodeId(this->mappedServer, avNode, &tmpOutput);
        if(!UA_NodeId_isNull(&tmpOutput)) {
          UA_NodeId_clear(&tmpOutput);
          UA_NodeId_init(&tmpOutput);
          raiseError("Additional variable node creation failed. Additional variable already exists.",
              std::string("Skipping additional variable ") + name, nodeset->nodeTab[i]->line);
          continue;
        }
        // check if pv with same name exists in the target folder
        UA_NodeId_clear(&tmpOutput);
        string pvNodeString;
        if(destination.empty()) {
          pvNodeString = this->serverConfig.rootFolder + "/" + name + "Value";
        }
        else {
          pvNodeString = this->serverConfig.rootFolder + "/" + destination + "/" + name + "Value";
        }
        UA_NodeId pvNode = UA_NODEID_STRING(1, const_cast<char*>(pvNodeString.c_str()));
        UA_Server_readNodeId(this->mappedServer, pvNode, &tmpOutput);
        if(!UA_NodeId_isNull(&tmpOutput)) {
          UA_NodeId_clear(&tmpOutput);
          UA_NodeId_init(&tmpOutput);
          raiseError("Additional variable node creation failed. PV with same name already exists.",
              std::string("Skipping additional variable ") + name, nodeset->nodeTab[i]->line);
          continue;
        }

        UA_NodeId additionalVarFolderPath = UA_NODEID_NULL;
        string additionalVarFolderPathNodeId;
        if(!destination.empty()) {
          additionalVarFolderPath = enrollFolderPathFromString(destination + "/removePart", "/");
        }
        else {
          additionalVarFolderPathNodeId += this->serverConfig.rootFolder + "Dir";
          additionalVarFolderPath = UA_NODEID_STRING_ALLOC(1, const_cast<char*>(additionalVarFolderPathNodeId.c_str()));
        }
        if(UA_NodeId_isNull(&additionalVarFolderPath)) {
          raiseError("Creation of additional variable folder failed.", "Skipping additional variable.",
              nodeset->nodeTab[i]->line);
          continue;
        }
        auto* additionalvariable =
            new ua_additionalvariable(this->mappedServer, additionalVarFolderPath, name, value, description);
        this->additionalVariables.push_back(additionalvariable);
        if(destination.empty()) {
          UA_NodeId_clear(&additionalVarFolderPath);
        }
      }

      xmlXPathFreeObject(result);
    }
  }

  vector<ua_processvariable*> ua_uaadapter::getVariables() {
    return this->variables;
  }

  UA_NodeId ua_uaadapter::createUAFolder(
      UA_NodeId basenodeid, const std::string& folderName, const std::string& description) {
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
    oAttr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(folderName.c_str()));
    oAttr.description = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(description.c_str()));

    string parentNodeIdString;
    if(basenodeid.identifierType == UA_NODEIDTYPE_STRING) {
      UASTRING_TO_CPPSTRING(basenodeid.identifier.string, parentNodeIdString)
      if(!parentNodeIdString.empty()) {
        parentNodeIdString.resize(parentNodeIdString.size() - 3);
      }
      parentNodeIdString += '/' + folderName + "Dir";
    }
    else if(basenodeid.identifierType == UA_NODEIDTYPE_NUMERIC) {
      parentNodeIdString += '/' + std::to_string(basenodeid.identifier.numeric) + "Dir";
    }

    UA_Server_addObjectNode(this->mappedServer,
        UA_NODEID_STRING(1, const_cast<char*>(parentNodeIdString.c_str())), // UA_NODEID_NUMERIC(1,0)
        basenodeid, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, const_cast<char*>(folderName.c_str())), UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), oAttr,
        &this->ownedNodes, &createdNodeId);

    ua_mapInstantiatedNodes(createdNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), &this->ownedNodes);
    return createdNodeId;
  }

  void ua_uaadapter::raiseError(std::string errorMesssage, std::string consequenceMessage, const int& line) {
    std::string lineMessage("");
    if(line > 0) {
      lineMessage = std::string(" Mapping line number: ") + std::to_string(line) + ".";
    }
    std::string tmp[2] = {errorMesssage, consequenceMessage};
    if(errorMesssage.back() != '.') {
      errorMesssage += ".";
    }
    if(consequenceMessage.back() != '.') {
      consequenceMessage += ".";
    }
    if(this->mappingExceptions) {
      throw std::runtime_error(std::string("Error! ") + errorMesssage + lineMessage);
    }
    if(server_config) {
      UA_LOG_WARNING(server_config->logging, UA_LOGCATEGORY_USERLAND, "%s %s", errorMesssage.c_str(),
          (consequenceMessage + lineMessage).c_str());
    }
    else {
      UA_LOG_WARNING(
          &logger, UA_LOGCATEGORY_USERLAND, "%s %s", errorMesssage.c_str(), (consequenceMessage + lineMessage).c_str());
    }
  }

  UA_StatusCode ua_uaadapter::mapSelfToNamespace() {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_NodeId createdNodeId = UA_NODEID_NULL;

    if(UA_NodeId_equal(&this->baseNodeId, &createdNodeId) == UA_TRUE) {
      return 0;
    }

    {
      // Create our toplevel instance
      UA_ObjectAttributes oAttr;
      UA_ObjectAttributes_init(&oAttr);
      // Classcast to prevent Warnings
      oAttr.displayName =
          UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(this->serverConfig.rootFolder.c_str()));
      oAttr.description =
          UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>(this->serverConfig.descriptionFolder.c_str()));

      UA_Server_addObjectNode(this->mappedServer,
          UA_NODEID_STRING(1, (const_cast<char*>((this->serverConfig.rootFolder + "Dir").c_str()))),
          UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
          UA_QUALIFIEDNAME(1, const_cast<char*>(this->serverConfig.rootFolder.c_str())),
          UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_CTKMODULE), oAttr, &ownedNodes, &createdNodeId);

      this->ownNodeId = createdNodeId;
      ua_mapInstantiatedNodes(this->ownNodeId, UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_CTKMODULE), &ownedNodes);
    }
    {
      // Create our toplevel instance for configuration
      UA_ObjectAttributes oAttr;
      UA_ObjectAttributes_init(&oAttr);
      // Classcast to prevent Warnings
      oAttr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>("ServerConfiguration"));
      oAttr.description =
          UA_LOCALIZEDTEXT(const_cast<char*>("en_US"), const_cast<char*>("Here adapter configurations are placed."));
      UA_Server_addObjectNode(this->mappedServer, UA_NODEID_STRING(1, const_cast<char*>("ServerConfigurationDir")),
          UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
          UA_QUALIFIEDNAME(1, const_cast<char*>("ServerConfiguration")),
          UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_CTKMODULE), oAttr, &ownedNodes, &createdNodeId);
      configNodeId = createdNodeId;
      ua_mapInstantiatedNodes(configNodeId, UA_NODEID_NUMERIC(CSA_NSID, UA_NS2ID_CTKMODULE), &ownedNodes);

      // create log level node
      UA_VariableAttributes attr = UA_VariableAttributes_default;
      attr.displayName = UA_LOCALIZEDTEXT(const_cast<char*>("en-US"), const_cast<char*>("logLevel"));
      attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
      attr.valueRank = -1;
      attr.dataType = LoggingLevelType.typeId;
      UA_LoggingLevel l = UA_LOGGINGLEVEL_INFO;
      auto status = UA_Variant_setScalarCopy(&attr.value, &l, &LoggingLevelType);

      UA_NodeId currentNodeId = UA_NODEID_STRING(1, const_cast<char*>("logLevel"));
      UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, const_cast<char*>("logLevel"));
      UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
      UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

      UA_DataSource logLevelDataSource;
      logLevelDataSource.read = readLogLevel;
      logLevelDataSource.write = writeLogLevel;
      UA_Server_addDataSourceVariableNode(this->mappedServer, currentNodeId, createdNodeId, parentReferenceNodeId,
          currentName, variableTypeNodeId, attr, logLevelDataSource, this, NULL);
      UA_Variant_clear(&attr.value);
    }

    return UA_STATUSCODE_GOOD;
  }

  UA_NodeId ua_uaadapter::getOwnNodeId() {
    return this->ownNodeId;
  }

  UA_NodeId ua_uaadapter::existFolderPath(UA_NodeId basenodeid, const std::vector<string>& folderPath) {
    UA_NodeId lastNodeId = basenodeid;
    for(const std::string& t : folderPath) {
      lastNodeId = this->existFolder(lastNodeId, t);
      if(UA_NodeId_isNull(&lastNodeId)) {
        return UA_NODEID_NULL;
      }
    }
    return lastNodeId;
  }

  UA_NodeId ua_uaadapter::existFolder(UA_NodeId basenodeid, const string& folder) {
    UA_NodeId lastNodeId = UA_NODEID_NULL;
    for(auto& i : this->folderVector) {
      if((i.folderName == folder) && (UA_NodeId_equal(&i.prevFolderNodeId, &basenodeid))) {
        return i.folderNodeId;
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
        for(auto& i : this->folderVector) {
          // get correct folder NodeId from first folderPath element
          if(!setted && (folderPath.at(m) == i.folderName) && (UA_NodeId_equal(&i.prevFolderNodeId, &nextNodeId)) &&
              ((m + 1) < folderPath.size())) {
            // remember on witch position the folder still exist
            setted = true;
            starter4Folder = m + 1;
            nextNodeId = i.folderNodeId;
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

  UA_NodeId ua_uaadapter::createFolder(UA_NodeId basenodeid, const string& folderName, const string& description) {
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

  vector<string> ua_uaadapter::getAllMappedPvSourceNames() {
    vector<string> mappedPvSources;
    xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//process_variable");
    if(result) {
      xmlNodeSetPtr nodeset = result->nodesetval;
      for(int32_t i = 0; i < nodeset->nodeNr; i++) {
        mappedPvSources.insert(
            mappedPvSources.begin(), xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[i], "sourceName"));
      }
    }
    xmlXPathFreeObject(result);
    return mappedPvSources;
  }

  vector<string> ua_uaadapter::getAllNotMappableVariablesNames() {
    vector<string> notMappableVariablesNames;
    xmlXPathObjectPtr result = this->fileHandler->getNodeSet("//process_variable");

    if(result) {
      xmlNodeSetPtr nodeset = result->nodesetval;
      for(int32_t i = 0; i < nodeset->nodeNr; i++) {
        // for(auto var:this->variables) {
        bool mapped = false;
        string mappedVar = xml_file_handler::getAttributeValueFromNode(nodeset->nodeTab[i], "sourceName");
        for(auto var : this->getVariables()) {
          if(var->getName() == mappedVar) {
            mapped = true;
            break;
          }
        }
        if(!mapped) {
          for(auto histVar : this->serverConfig.historyvariables) {
            string mappedVarHist = serverConfig.rootFolder + "/" + mappedVar;
            std::string tmp;
            UASTRING_TO_CPPSTRING(histVar.variable_id.identifier.string, tmp);
            if(tmp.compare(mappedVarHist) == 0) {
              mapped = true;
              break;
            }
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

  UA_StatusCode ua_uaadapter::readLogLevel(UA_Server* /*server*/, const UA_NodeId* /*sessionId*/,
      void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext, UA_Boolean /*sourceTimeStamp*/,
      const UA_NumericRange* /*range*/, UA_DataValue* dataValue) {
    auto adapter = (ua_uaadapter*)nodeContext;
    UA_LoggingLevel l = toLoggingLevel(adapter->serverConfig.logLevel);
    UA_Variant_setScalarCopy(&dataValue->value, &l, &LoggingLevelType);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
  }

  UA_StatusCode ua_uaadapter::writeLogLevel(UA_Server* /*server*/, const UA_NodeId* /*sessionId*/,
      void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext, const UA_NumericRange* /*range*/,
      const UA_DataValue* data) {
    auto adapter = (ua_uaadapter*)nodeContext;
    const UA_Variant* var = &data->value;
    adapter->serverConfig.logLevel = toLogLevel(*((UA_LoggingLevel*)var->data));
    adapter->logger = UA_Log_Stdout_withLevel(adapter->serverConfig.logLevel);
    adapter->server_config->logging = &adapter->logger;
    UA_LOG_INFO(adapter->server_config->logging, UA_LOGCATEGORY_USERLAND, "Set log level to %d",
        adapter->serverConfig.logLevel);
    return UA_STATUSCODE_GOOD;
  }

} // namespace ChimeraTK
