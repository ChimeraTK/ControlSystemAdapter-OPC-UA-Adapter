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
#include "mtca_processscalar.h"
#include "mtca_processarray.h"

#include "ControlSystemPVManager.h"

/**
 * This class provide the two parts of the OPCUA Adapter. First of all the OPCUA server starts with a random port number (recommended 16664),
 * following the mapping process start. For this, the ProcessVariable from ControlSystemPVManager will be mapped to the OPCUA Model 
 * 
 */ 
ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA(uint16_t opcuaPort, boost::shared_ptr<mtca4u::ControlSystemPVManager> csManager) {
  this->ControlSystemAdapterOPCUA_InitServer(opcuaPort);
  this->ControlSystemAdapterOPCUA_InitVarMapping(csManager);
}

// Init the OPCUA server
void ControlSystemAdapterOPCUA::ControlSystemAdapterOPCUA_InitServer(uint16_t opcuaPort) {
  this->mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

  // Create new server adapter
  this->adapter = new mtca_uaadapter(opcuaPort);
  std::cout << "Server aktiv..." << std::endl;
  
  this->mgr->addObject(adapter);
  this->mgr->doStart(); // Implicit: Startet Worker-Threads aller ipc_managed_objects, die mit addObject registriert wurden  
  
}

// Mapping the ProcessVariables to the server
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
			std::type_info const & valueType = oneProcessVariable->getValueType();
			if(!oneProcessVariable->isArray()) {
				std::string value;
				// cast the correct processVariable type
				if (valueType == typeid(int8_t)) {
					value = std::to_string(csManager->getProcessScalar<int8_t>(oneProcessVariable->getName())->get());
				} else if (valueType == typeid(uint8_t)) {
					value = std::to_string(csManager->getProcessScalar<uint8_t>(oneProcessVariable->getName())->get());
				} else if (valueType == typeid(int16_t)) {
					value = std::to_string(csManager->getProcessScalar<int16_t>(oneProcessVariable->getName())->get());
				} else if (valueType == typeid(uint16_t)) {
					value = std::to_string(csManager->getProcessScalar<uint16_t>(oneProcessVariable->getName())->get());
				} else if (valueType == typeid(int32_t)) {
					value = std::to_string(csManager->getProcessScalar<int32_t>(oneProcessVariable->getName())->get());
				} else if (valueType == typeid(uint32_t)) {
					value = std::to_string(csManager->getProcessScalar<u_int32_t>(oneProcessVariable->getName())->get());
				} else if (valueType == typeid(float)) {
					value = std::to_string(csManager->getProcessScalar<float>(oneProcessVariable->getName())->get());
				} else if (valueType == typeid(double)) {
					value = std::to_string(csManager->getProcessScalar<double>(oneProcessVariable->getName())->get());
				} else {
					throw std::invalid_argument("unsupported value type");
				}
				adapter->addVariable(oneProcessVariable->getName(), value, valueType, oneProcessVariable->getTimeStamp());
				std::cout << "Scalar: " << oneProcessVariable->getName() << std::endl;
			}
			// Map to ProcessScalar
			if(oneProcessVariable->isArray()) {
				std::vector<std::string> valueVector;
				// get array values from ProcessVariable
				if (valueType == typeid(int8_t)) {
					for(int i=0; i< csManager->getProcessArray<int8_t>(oneProcessVariable->getName())->get().size(); i++) { 
						valueVector.push_back(std::to_string(csManager->getProcessArray<int8_t>(oneProcessVariable->getName())->get().at(i)));
					}
				} else if (valueType == typeid(uint8_t)) {
					for(int i=0; i< csManager->getProcessArray<u_int8_t>(oneProcessVariable->getName())->get().size(); i++) { 
						valueVector.push_back(std::to_string(csManager->getProcessArray<u_int8_t>(oneProcessVariable->getName())->get().at(i)));
					}
				} else if (valueType == typeid(int16_t)) {
					for(int i=0; i< csManager->getProcessArray<int16_t>(oneProcessVariable->getName())->get().size(); i++) { 
						valueVector.push_back(std::to_string(csManager->getProcessArray<int16_t>(oneProcessVariable->getName())->get().at(i)));
					}
				} else if (valueType == typeid(uint16_t)) {
					for(int i=0; i< csManager->getProcessArray<u_int16_t>(oneProcessVariable->getName())->get().size(); i++) { 
						valueVector.push_back(std::to_string(csManager->getProcessArray<uint16_t>(oneProcessVariable->getName())->get().at(i)));
					}
				} else if (valueType == typeid(int32_t)) {
					for(int i=0; i< csManager->getProcessArray<int32_t>(oneProcessVariable->getName())->get().size(); i++) { 
						valueVector.push_back(std::to_string(csManager->getProcessArray<int32_t>(oneProcessVariable->getName())->get().at(i)));
					}
				} else if (valueType == typeid(uint32_t)) {
					for(int i=0; i< csManager->getProcessArray<u_int32_t>(oneProcessVariable->getName())->get().size(); i++) { 
						valueVector.push_back(std::to_string(csManager->getProcessArray<uint32_t>(oneProcessVariable->getName())->get().at(i)));
					}
				} else if (valueType == typeid(float)) {
					for(int i=0; i< csManager->getProcessArray<float>(oneProcessVariable->getName())->get().size(); i++) { 
						valueVector.push_back(std::to_string(csManager->getProcessArray<float>(oneProcessVariable->getName())->get().at(i)));
					}
				} else if (valueType == typeid(double)) {
					for(int i=0; i< csManager->getProcessArray<double>(oneProcessVariable->getName())->get().size(); i++) { 
						valueVector.push_back(std::to_string(csManager->getProcessArray<double>(oneProcessVariable->getName())->get().at(i)));
					}
				} else {
					throw std::invalid_argument("unsupported value type");
				}
				
				adapter->addVariable(oneProcessVariable->getName(), valueVector, valueType, oneProcessVariable->getTimeStamp());
				std::cout << "Array: " << oneProcessVariable->getName() << std::endl;
			}
		}
		catch (std::bad_cast& bc) {
			std::cerr << "bad_cast caught: " << bc.what() << '\n';
		}
	}
}
    

// Maybe needed, return the ControlSystemPVManager
boost::shared_ptr<mtca4u::ControlSystemPVManager> const & ControlSystemAdapterOPCUA::getControlSystemPVManager() const {
  return this->csManager;
}

void ControlSystemAdapterOPCUA::start() {
  this->mgr->startAll();
  return;
}

void ControlSystemAdapterOPCUA::stop() {
  this->mgr->stopAll();
  return;
}

void ControlSystemAdapterOPCUA::terminate() {
  this->mgr->terminate();
  return;
}

bool ControlSystemAdapterOPCUA::isRunning() {
  return this->mgr->isRunning();
}