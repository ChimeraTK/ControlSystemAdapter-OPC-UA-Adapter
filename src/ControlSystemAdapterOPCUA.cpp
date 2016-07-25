/*
 * Copyright (c) 2016
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

extern "C" {
#include "unistd.h"
#include "mtca_namespaceinit_generated.h" // Output des pyUANamespacecompilers
}

#include "ControlSystemAdapterOPCUA.h"

#include <iostream>
#include <math.h>
#include <typeinfo>       // std::bad_cast

#include "ipc_manager.h"

#include "mtca_uaadapter.h"
#include "mtca_processvariable.h"
#include "mtca_processscalar.h"
#include "mtca_processarray.h"

#include "ControlSystemPVManager.h"

ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA(uint16_t opcuaPort, boost::shared_ptr<mtca4u::ControlSystemPVManager> csManager) {
	this->ControlSystemAdapterOPCUA_InitServer(opcuaPort);
	this->ControlSystemAdapterOPCUA_InitVarMapping(csManager);
}

void ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA_InitServer(uint16_t opcuaPort) {
	this->mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

	// Create new server adapter
	this->adapter = new mtca_uaadapter(opcuaPort);
	std::cout << "Server aktiv..." << std::endl;
	
	this->mgr->addObject(adapter);
	this->mgr->doStart(); // Implicit: Startet Worker-Threads aller ipc_managed_objects, die mit addObject registriert wurden  
	
}

void ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA_InitVarMapping(boost::shared_ptr<mtca4u::ControlSystemPVManager> csManager) {
	// Get all ProcessVariables
	vector<ProcessVariable::SharedPtr> allProcessVariables = csManager->getAllProcessVariables();
	
	std::cout << "Start mapping..." << std::endl;
	
	
	/*
	 * FIXME: Derzeit werden nur <int> Datentypen unterstützt. Mittels Factory und Template sollten alle nötigen unterstützt werden.
	 */
	for(ProcessVariable::SharedPtr oneProcessVariable : allProcessVariables){
		try  {
		// Map to ProcessScalar
			if(!oneProcessVariable->isArray()) {
				adapter->addVariable(oneProcessVariable->getName(), std::to_string(csManager->getProcessScalar<int>(oneProcessVariable->getName())->get()), oneProcessVariable->getValueType(), oneProcessVariable->getTimeStamp());
				std::cout << "Scalar: " << oneProcessVariable->getName() << std::endl;
			}
			// Map to ProcessScalar
			if(oneProcessVariable->isArray()) {
				std::vector<int32_t> processVector (csManager->getProcessArray<int>(oneProcessVariable->getName())->get().size());
				// get array values from ProcessVariable
				for(int i=0; i< csManager->getProcessArray<int>(oneProcessVariable->getName())->get().size(); i++) { 
					processVector.at(i) = csManager->getProcessArray<int>(oneProcessVariable->getName())->get().at(i);
				}
				adapter->addVariable(oneProcessVariable->getName(), processVector, oneProcessVariable->getValueType(), oneProcessVariable->getTimeStamp());
				std::cout << "Array: " << oneProcessVariable->getName() << std::endl;
			}
		}
		catch (std::bad_cast& bc) {
			std::cerr << "bad_cast caught: " << bc.what() << '\n';
		}
	}
}
		

boost::shared_ptr<mtca4u::ControlSystemPVManager> const & ControlSystemAdapterOPCUA::getControlSystemPVManager() const {
	return this->csManager;
}

ipc_manager* ControlSystemAdapterOPCUA::getIPCManager() {
	return this->mgr;
}