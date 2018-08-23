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
#include "ua_processvariable.h"
#include "ua_additionalvariable.h"
#include "xml_file_handler.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"

using namespace ChimeraTK;
using namespace std;

/** @struct FolderInfo
 *	@brief This struct represents a folder in OPCUA with its own node id and with his parent and child node id. For this FolderInfo are a singly-linked list
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
        /** @brief NodeId from the folder from opcua server
         */
        UA_NodeId folderNodeId = UA_NODEID_NULL;
        /** @brief NodeId from the child folder
         */
        UA_NodeId nextFolderNodeId = UA_NODEID_NULL;
        /** @brief NodeId from the parent folder
         */
        UA_NodeId prevFolderNodeId = UA_NODEID_NULL;
};

/** @struct ServerConfig
 *	@brief This struct represents a server config. If the hole config file is prased, all information will be stored in die struct.
 * Additionally for every necessary variable a default value is set.
 *
 *  @author Chris Iatrou, Julian Rahm
 *  @date 03.12.2016
 *
 */
struct ServerConfig {
        string rootFolder = "Module";
        string descriptionFolder = "No description is set";
        UA_Boolean	UsernamePasswordLogin = UA_FALSE;
        string password = "";
        string username = "";
        string applicationName = "OPCUA Adapter";
        uint16_t opcuaPort = 16664;
};


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
class ua_uaadapter : ua_mapped_class, public ipc_managed_object {
private:
        UA_ServerConfig					server_config;
        UA_ServerNetworkLayer 	server_nl;
        UA_Logger 							logger;

        UA_NodeId 							variablesListId;
        UA_NodeId 							constantsListId;

        vector<FolderInfo>			folderVector;
        UA_NodeId								ownNodeId;

        ServerConfig 						serverConfig;

        vector<ua_processvariable *> 			variables;
        vector<ua_additionalvariable *> 	additionalVariables;
        vector<ua_processvariable *>			mappedVariables;

        xml_file_handler *fileHandler;

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
        UA_NodeId createUAFolder(UA_NodeId basenodeId, string folderName, string description = "");

public:

        /** @brief Constructor of the class.
 *
 * During the construction of the class it instanciate a xml_file_handler and read the config, after that the server will be sonstructed and the namespace ist added to them.
 * Concluding all additional nodes which are defined in the configFile are mapped into the server.
 *
 * @param configFile This file provide the configuration and the mapping of the server
 */
        ua_uaadapter(string configPath);

        /** @brief Destrructor of the class.
        *
        * It stop the server and delete the managed object.
        *
        */
        ~ua_uaadapter();

        /** @brief Return the timestamp of the node
        *
        * @return UA_DateTime
        */
        UA_DateTime getSourceTimeStamp();

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
        UA_NodeId createFolder(UA_NodeId basenodeid, string folderName, string description = "");

        /** @brief Check if a folder path exist in opcua server
        *
        * @param basenodeId Node id of the parent node
        * @param folderPathVector Every single string is a folder name, the path ist checked in the given order
        */
        UA_NodeId existFolderPath(UA_NodeId basenodeid, vector<string> folderPath);

        /** @brief Check if a folder exist in opcua server
        *
        * @param basenodeId Node id of the parent node
        * @param folderName The name of folder, that be checked
        */
        UA_NodeId existFolder(UA_NodeId basenodeid, string folderName);

        /** @brief Check if a folder ath exist in opcua server
        *
        * @param basenodeId Node id of the parent node
        * @param folderPathVector Every single string is a folder name, the path ist checked in the given order
        */
        void addVariable(string name, boost::shared_ptr<ControlSystemPVManager> csManager);

        /** @brief Methode that returns the node id of the instanced class
        *
        * @return UA_NodeId
        */
        UA_NodeId getOwnNodeId();

        /** @brief Methode that returns all <ua_processvariable> of the class.
        *
        * @return A vector of all <ua_processvariable>
        */
        vector<ua_processvariable *> getVariables();

        /** @brief Create and start a thread for the opcua server instance
        *
        */
        void workerThread();

        /** @brief This Methode reads the config-tag form the given <variableMap.xml>.
        *
        */
        void readConfig();

        /** @brief This Methode reads the additionaNode-tag from the given <variableMap.xml>
        *
        */
        void readAdditionalNodes();

        /** @brief Methode to get all names from all potential VarableNodes from XML-Mappingfile which could not allocated.
        *
        * @return vector<string> notMappableVariablesNames List with all VariableNodes which could not allocated a Varaible in PV-Manager.
        */
        vector<string> getAllNotMappableVariablesNames();
};

#endif // MTCA_UAADAPTER_H
