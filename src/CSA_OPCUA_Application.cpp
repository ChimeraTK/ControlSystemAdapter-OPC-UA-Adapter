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
* @mainpage Apllication
*
*
* @author Julian Rahm
*/ 

#include <iostream>
#include <string>

#include "ChimeraTK/ControlSystemAdapter/ApplicationBase.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"
#include "ControlSystemAdapterOPCUA.h"


	boost::shared_ptr<ControlSystemPVManager> csManager;
	boost::shared_ptr<DevicePVManager> devManager;
	boost::shared_ptr<ControlSystemSynchronizationUtility> syncUtility;
	ControlSystemAdapterOPCUA *csaOPCUA;
	
	
void csa_opcua_prolog() {
	// Create the managers
	std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();
    
	devManager = pvManagers.second;
	csManager = pvManagers.first;

	syncUtility.reset(new ChimeraTK::ControlSystemSynchronizationUtility(csManager));

	csManager->enablePersistentDataStorage();
	
	ChimeraTK::ApplicationBase::getInstance().setPVManager(devManager);
	ChimeraTK::ApplicationBase::getInstance().initialise();
	
}

ControlSystemAdapterOPCUA* csa_opcua_main() {
	
	string pathToConfig = ChimeraTK::ApplicationBase::getInstance().getName() + "_mapping.xml";
	csaOPCUA = new ControlSystemAdapterOPCUA(csManager, pathToConfig);
	
	return csaOPCUA;
}

void csa_opcua_epilog() {
	
	ChimeraTK::ApplicationBase::getInstance().run();
}
