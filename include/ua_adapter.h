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
 * Copyright (c) 2019-2021 Andreas Ebner  <Andreas.Ebner@iosb.fraunhofer.de>
 */

#pragma once

#include "node_historizing.h"
#include <open62541/config.h>
#include <open62541/plugin/accesscontrol.h>
#include <open62541/plugin/accesscontrol_default.h>
#include <open62541/plugin/historydata/history_data_gathering.h>
#include <open62541/server_config_default.h>

using namespace std;

namespace ChimeraTK {
  struct ServerConfig {
    string rootFolder;
    string descriptionFolder;
    UA_Boolean UsernamePasswordLogin = UA_FALSE;
    string password;
    string username;
    string applicationName = "OPCUA-Adapter";
    uint16_t opcuaPort = 16664;
    bool enableSecurity = false;
    bool unsecure = false;
    UA_LogLevel logLevel = UA_LOGLEVEL_INFO;
    string certPath;
    string keyPath;
    string allowListFolder;
    string blockListFolder;
    string issuerListFolder;
    vector<AdapterHistorySetup> history{};
    vector<AdapterFolderHistorySetup> historyfolders{};
    vector<AdapterPVHistorySetup> historyvariables{};
  };

  /** @struct FolderInfo
   *	@brief This struct represents a folder in OPCUA with its own node id and with his parent and child node id. For
   *this FolderInfo are a singly-linked list
   *
   *  @author Chris Iatrou, Julian Rahm
   *  @date 03.12.2016
   *
   */
  struct FolderInfo {
    /** @brief Name of the folder
     *
     */
    string folderName;
    /** @brief NodeId from the folder from opcua server */
    UA_NodeId folderNodeId = UA_NODEID_NULL;
    /** @brief NodeId from the child folder */
    UA_NodeId nextFolderNodeId = UA_NODEID_NULL;
    /** @brief NodeId from the parent folder */
    UA_NodeId prevFolderNodeId = UA_NODEID_NULL;
  };

  /** @struct ServerConfig
   *	@brief This struct represents a server config. If the hole config file is prased, all information will be stored
   *in die struct. Additionally for every necessary variable a default value is set.
   *
   *  @author Chris Iatrou, Julian Rahm
   *  @date 03.12.2016
   *
   */

  /** @class ua_uaadapter
   *	@brief This class provide the opcua server and manage the variable mapping.
   *
   * This Class create and start the opcua server also it contain all variables of the server.
   * Especially it reads the config-file and add all Variables from
   * a pv-manager and additional variables. For config purpose. The
   * config-file parameter will parsed and set to the right variable.
   *
   *  @author Chris Iatrou
   *	@author Julian Rahm
   *
   *  @date 22.11.2016
   */
  class ua_uaadapter : ua_mapped_class {
   private:
    UA_ServerConfig* server_config;

    UA_NodeId variablesListId;
    UA_NodeId constantsListId;

    vector<FolderInfo> folderVector;
    UA_NodeId ownNodeId;
    string pvSeperator;
    UA_Boolean mappingExceptions;

    ServerConfig serverConfig;
    UA_Logger logger;

    vector<ua_processvariable*> variables;
    vector<ua_additionalvariable*> additionalVariables;
    vector<ua_processvariable*> mappedVariables;

    xml_file_handler* fileHandler;

    /** @brief This methode construct the parameter for the opcua server, depending of the <serverConfig> struct
     */
    void constructServer();

    /** @brief This methode mapped all own nodes into the opcua server
     *
     * @return <UA_StatusCode>
     */
    UA_StatusCode mapSelfToNamespace();

    /** @brief Creates a folder in the given parent node
     *
     * @param basenodeId Node id of the parent node
     * @param folderName Name of the new folder
     * @param description A short description of the folder
     *
     * @return UA_NodeId is the node id of the new created folder
     */
    UA_NodeId createUAFolder(UA_NodeId basenodeId, const string& folderName, const string& description = "");

   public:
    bool running = false;
    // TODO move this field to private and add getter and setter
    vector<string> exclude;
    vector<string> folder_with_history;

    /** @brief Constructor of the class.
     *
     * During the construction of the class it instanciate a xml_file_handler and read the config, after that the server
     * will be sonstructed and the namespace ist added to them. Concluding all additional nodes which are defined in the
     * configFile are mapped into the server.
     *
     * @param configFile This file provide the configuration and the mapping of the server
     */
    ua_uaadapter(const string& configPath);

    /** @brief Destrructor of the class.
     *
     * It stop the server and delete the managed object.
     *
     */
    virtual ~ua_uaadapter();

    /** @brief Return the timestamp of the node
     *
     * @return UA_DateTime
     */
    UA_DateTime getSourceTimeStamp();

    /** @brief Return the OPC UA Server instance
     *
     * @return UA_Server *
     */
    UA_Server* getMappedServer();

    /** @brief Create a path of folders in the given parent node
     *
     * @param basenodeid Node id of the parent node
     * @param folderPathVector Every single string is a folder name, the path ist createt in the given order
     */
    UA_NodeId createFolderPath(UA_NodeId basenodeid, vector<string> folderPathVector);

    /** @brief Creates a folder in the given parent node
     *
     * @param basenodeId Node id of the parent node
     * @param folderName Name of the new folder
     * @param description A short description of the folder
     */
    UA_NodeId createFolder(UA_NodeId basenodeid, const string& folderName, const string& description = "");

    /** @brief Check if a folder path exist in opcua server
     *
     * @param basenodeId Node id of the parent node
     * @param folderPathVector Every single string is a folder name, the path ist checked in the given order
     */
    UA_NodeId existFolderPath(UA_NodeId basenodeid, const vector<string>& folderPath);

    /** @brief Check if a folder exist in opcua server
     *
     * @param basenodeId Node id of the parent node
     * @param folderName The name of folder, that be checked
     */
    UA_NodeId existFolder(UA_NodeId basenodeid, const string& folderName);

    /** @brief Create folder structure based on the given path.
     *
     * @param path The complete path
     * @param seperator Delimiters with are used to split the path
     *
     * @return nodeId of the last created folder (last part of path)
     */
    UA_NodeId enrollFolderPathFromString(const string& path, const string& seperator);

    /** @brief Start implicit mapping process
     *
     * @param basenodeId Node id of the parent node
     * @param folderPathVector Every single string is a folder name, the path ist checked in the given order
     */
    void implicitVarMapping(const std::string& varName, const boost::shared_ptr<ControlSystemPVManager>& csManager);

    /**
     * @brief Read mapping file and apply the contained folders, additional variables and pv mappings.
     * Order -> Folder (without source) -> Folder (with source) -> additional variables -> PV mappings
     *
     * @param csManager control system manager
     */
    void applyMapping(const boost::shared_ptr<ControlSystemPVManager>& csManager);

    /**
     * @brief Read mapping file and apply contained PV mappings.
     *
     * @param csManager control system manager
     */
    void explicitVarMapping(const boost::shared_ptr<ControlSystemPVManager>& csManager);

    /**
     * @brief Read mapping file and add contained additional variables to the server.
     */
    void addAdditionalVariables();

    /**
     * @brief Read mapping file and and add contained folders to the server.
     *
     * @param csManager control system manager
     */
    void buildFolderStructure(const boost::shared_ptr<ControlSystemPVManager>& csManager);

    /**
     * @brief Copy (recursively) the content of a folder to a new location
     *
     * @param csManager control system manager
     * @param layer source folder
     * @param target destination folder
     */
    void deepCopyHierarchicalLayer(
        const boost::shared_ptr<ControlSystemPVManager>& csManager, UA_NodeId layer, UA_NodeId target);

    /** @brief Methode that returns the node id of the instanced class
     *
     * @return UA_NodeId
     */
    UA_NodeId getOwnNodeId();

    /** @brief Methode that returns all <ua_processvariable> of the class.
     *
     * @return A vector of all <ua_processvariable>
     */
    vector<ua_processvariable*> getVariables();

    /** @brief Create and start a thread for the opcua server instance
     *
     */

    void workerThread();

    /** @brief This Methode reads the config-tag form the given <variableMap.xml>.
     *
     */
    void readConfig();

    /** @brief Methode to get all names from all potential VarableNodes from XML-Mappingfile which could not allocated.
     *
     * @return vector<string> notMappableVariablesNames List with all VariableNodes which could not allocated a Varaible
     * in PV-Manager.
     */
    vector<string> getAllNotMappableVariablesNames();

    // ToDo documentation
    vector<string> getAllMappedPvSourceNames();

    /** @brief Fill server build information.
     *
     * @param config The server configuration to be modified.
     */
    void fillBuildInfo(UA_ServerConfig* config) const;

    /** @brief get ServerConfig.
     *
     * @param returns the serveConfig.
     */
    ServerConfig get_server_config();

    /**
     * @brief callback function used to read the servers logging level.
     *
     * Read the current logging level stored in adapter->serverConfig.logLevel.
     */
    static UA_StatusCode readLogLevel(UA_Server* server, const UA_NodeId* sessionId, void* sessionContext,
        const UA_NodeId* nodeId, void* nodeContext, UA_Boolean sourceTimeStamp, const UA_NumericRange* range,
        UA_DataValue* dataValue);

    /**
     * @brief callback function used to change the servers logging level.
     *
     * Set the new logging level entered by the user. Replace the servers logger by a new one.
     */
    static UA_StatusCode writeLogLevel(UA_Server* server, const UA_NodeId* sessionId, void* sessionContext,
        const UA_NodeId* nodeId, void* nodeContext, const UA_NumericRange* range, const UA_DataValue* data);
    // friend class declaration
    friend class csa_opcua_adapter;
  };
} // namespace ChimeraTK
