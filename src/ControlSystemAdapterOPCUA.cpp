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

/**
* @mainpage This is the dokumentation of ControlSystemAdapter-OPC-UA-Adapter
* 
* @brief In some weeks... maybe there will be here a fancy description of the project...
*
*
* @author Chris Iatrou Chris_Paul.Iatrou@tu-dresden.de
* @author Julian Rahm Julian.Rahm@tu-dresden.de
*/ 

/** @class ControlSystemAdapterOPCUA
 *	@brief This class provide the two parts of the OPCUA Adapter. First of all the OPCUA server starts with a port number defined in config-file (recommended 16664),
 * following the mapping process start. For this, the ProcessVariable from ControlSystemPVManager will be mapped to the OPCUA Model. During the mapping process also all 
 * <additionalNodes> will be mapped in OPC UA Model
 *   
 *  @author Chris Iatrou
 *	@author Julian Rahm
 *  @date 22.11.2016
 * 
 */

extern "C" {
#include "unistd.h"
}

#include "ControlSystemAdapterOPCUA.h"

#include <iostream>
#include <math.h>
#include <typeinfo>       // std::bad_cast

#include "ipc_manager.h"
#include "mtca_uaadapter.h"
#include "mtca_processvariable.h"

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
ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA(boost::shared_ptr<ControlSystemPVManager> csManager, boost::shared_ptr<ControlSystemSynchronizationUtility> syncCsUtility, string configFile) {
	
	this->csManager = csManager; 	
	this->syncCsUtility = syncCsUtility;
	this->ControlSystemAdapterOPCUA_InitServer(configFile);
	this->ControlSystemAdapterOPCUA_InitVarMapping();
	
}

/**
 * @brief This method starts the OPC UA Server with the defined port from mapping-file. 
 * For this, the server starts his own thread.
 * 
 * @param configFile Define the path to the mapping-file with config and mapping information
 *
 */ 
void ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA_InitServer(string configFile) {
    this->mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

    // Create new server adapter
     this->adapter = new mtca_uaadapter(configFile);
  
    this->mgr->addObject(this->adapter);
    this->mgr->doStart(); // Implicit: Startet Worker-Threads aller ipc_managed_objects, die mit addObject registriert wurden  
}

/**
 * @brief To map all processvariables from controlsystemadapter to the OPC UA server the Method takes all variable 
 * and put these in a flat list. For better a better workflow in connection with the mapping-file the defined 
 * processvaribale are mapped in special folder, will be renamed, get description or a other engineering unit.
 * 
 */
void ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA_InitVarMapping() {
    // Get all ProcessVariables
    vector<ProcessVariable::SharedPtr> allProcessVariables = this->csManager->getAllProcessVariables();
  
    for(ProcessVariable::SharedPtr oneProcessVariable : allProcessVariables) {
        adapter->addVariable(oneProcessVariable->getName(), this->csManager, this->syncCsUtility);
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
ControlSystemAdapterOPCUA::~ControlSystemAdapterOPCUA() {
	this->mgr->~ipc_manager();
	free(this->mgr);
	this->adapter->~mtca_uaadapter();
	free(this->adapter);	
}


/** 
 * @brief Return the ControlsystemPVManager
 * 
 */
boost::shared_ptr<ControlSystemPVManager> const & ControlSystemAdapterOPCUA::getControlSystemManager() const {
    return this->csManager;
}

/**
 * @brief Return the uaadapter, hence the OPC UA server 
 * 
 */
mtca_uaadapter* ControlSystemAdapterOPCUA::getUAAdapter() {
	return this->adapter;
}

/**
 * @brief Return the ipc_manager
 *
 */
ipc_manager* ControlSystemAdapterOPCUA::getIPCManager() {
    return this->mgr;
}


/**
 * @brief Start all objects in single threads for this case only the opc ua server
 * 
 */
void ControlSystemAdapterOPCUA::start() {
    this->mgr->startAll();
    return;
}

/**
 * @brief Stop all objects in single threads for this case only the opc ua server
 * 
 */
void ControlSystemAdapterOPCUA::stop() {
    this->mgr->stopAll();
    return;
}

/**
 * @brief Terminate all objects in single threads for this case only the opc ua server
 * 
 */
void ControlSystemAdapterOPCUA::terminate() {
    this->mgr->terminate();
    return;
}

/**
 * @brief Checks if the opcua server is still running and return the suitable bool value
 * 
 */
bool ControlSystemAdapterOPCUA::isRunning() {
    return this->mgr->isRunning();
}