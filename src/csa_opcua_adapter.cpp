/*
 * Copyright (c) 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>, Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 * Chair for Process Systems Engineering
 * Technical University of Dresden
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


/** @class csa_opcua_adapter
 *	@brief This class provide the two parts of the OPCUA Adapter. 
 * First of all the OPCUA server starts with a port number defined in config-file (recommended 16664),
 * following the mapping process start. For this, the ProcessVariable from ControlSystemPVManager will be mapped to the OPCUA Model. During the mapping process also all 
 * \<additionalNodes\> will be mapped in OPC UA Model
 *   
 *  @author Chris Iatrou
 *	@author Julian Rahm
 *  @date 22.11.2016
 * 
 */

extern "C" {
#include "unistd.h"
}

#include "csa_opcua_adapter.h"

#include <iostream>
#include <math.h>
#include <typeinfo>       // std::bad_cast

#include "ipc_manager.h"
#include "ua_adapter.h"
#include "ua_processvariable.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ApplicationBase.h"

/**
 * @brief Constructor for ControlSystemAdapter-OPC-UA-Adapter
 * 
 * @param csManager Provide the hole PVManager from control-system-adapter to map all processvariable to the OPC UA-Model
 * @param configFile Define the path to the mapping-file with config and mapping information
 * 
 */ 
csa_opcua_adapter::csa_opcua_adapter(boost::shared_ptr<ControlSystemPVManager> csManager, string configFile) {
	
	this->csManager = csManager; 	
	this->csa_opcua_adapter_InitServer(configFile);
	this->csa_opcua_adapter_InitVarMapping();
	
}

/**
 * @brief This method starts the OPC UA Server with the defined port from mapping-file. 
 * For this, the server starts his own thread.
 * 
 * @param configFile Define the path to the mapping-file with config and mapping information
 *
 */ 
void csa_opcua_adapter::csa_opcua_adapter_InitServer(string configFile) {
	this->mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

	// Create new server adapter
	this->adapter = new ua_uaadapter(configFile);
	this->mgr->addObject(this->adapter);
	this->mgr->doStart(); // Implicit: Startet Worker-Threads aller ipc_managed_objects, die mit addObject registriert wurden  
}

/**
 * @brief To map all processvariables from controlsystemadapter to the OPC UA server the Method takes all variable 
 * and put these in a flat list. For better a better workflow in connection with the mapping-file the defined 
 * processvaribale are mapped in special folder, will be renamed, get description or a other engineering unit.
 * 
 */
void csa_opcua_adapter::csa_opcua_adapter_InitVarMapping() {
    // Get all ProcessVariables
    vector<ProcessVariable::SharedPtr> allProcessVariables = this->csManager->getAllProcessVariables();
  
    for(ProcessVariable::SharedPtr oneProcessVariable : allProcessVariables) {
        adapter->addVariable(oneProcessVariable->getName(), this->csManager);
    }
    
    vector<string> allNotMappedVariables = adapter->getAllNotMappableVariablesNames();
		if(allNotMappedVariables.size() > 0) {
			cout << "The following VariableNodes cant be mapped, because they are not member in PV-Manager:" << endl;
			for(string var:allNotMappedVariables) {
				cout << var << endl;
			}
		}
}

/**
 * @brief Destructor to stop the running thread, hence it stops the OPC UA server 
 * 
 */
csa_opcua_adapter::~csa_opcua_adapter() {
	
	//free(this->mgr);
	//delete this->mgr;
	//this->mgr = NULL;
	this->adapter->~ua_uaadapter();
	//free(this->adapter);	
	//delete this->adapter;
	//this->adapter = NULL;
	
	this->mgr->~ipc_manager();
}


/** 
 * @brief Return the ControlsystemPVManager
 * 
 * @return Returns a ControlSystemPVManager
 */
boost::shared_ptr<ControlSystemPVManager> const & csa_opcua_adapter::getControlSystemManager() const {
    return this->csManager;
}

/**
 * @brief Return the uaadapter, hence the OPC UA server 
 * 
 * @return Return the ua_uaadapter
 * 
 */
ua_uaadapter* csa_opcua_adapter::getUAAdapter() {
	return this->adapter;
}

/**
 * @brief Return the ipc_manager
 * 
 * @return Return the ipc_manager
 *
 */
ipc_manager* csa_opcua_adapter::getIPCManager() {
    return this->mgr;
}


/**
 * @brief Start all objects in single threads for this case only the opc ua server
 * 
 */
void csa_opcua_adapter::start() {
    this->mgr->startAll();
    return;
}

/**
 * @brief Stop all objects in single threads for this case only the opc ua server
 * 
 */
void csa_opcua_adapter::stop() {
    this->mgr->stopAll();
    return;
}

/**
 * @brief Terminate all objects in single threads for this case only the opc ua server
 * 
 */
void csa_opcua_adapter::terminate() {
    this->mgr->terminate();
    return;
}

/**
 * @brief Checks if the opcua server is still running and return the suitable bool value
 * 
 * @return The current running state in form of true/false
 */
bool csa_opcua_adapter::isRunning() {
    return this->mgr->isRunning();
}