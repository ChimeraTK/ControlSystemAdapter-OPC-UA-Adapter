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

extern "C" {
    #include <unistd.h>
    #include <signal.h>
}

#include <iostream>
#include <string>
#include <atomic>

#include "ChimeraTK/ControlSystemAdapter/ApplicationBase.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"
#include "ControlSystemAdapterOPCUA.h"

boost::shared_ptr<ControlSystemPVManager> csManager;
boost::shared_ptr<DevicePVManager> devManager;
	boost::shared_ptr<ControlSystemSynchronizationUtility> syncCsUtility;
	boost::shared_ptr<DeviceSynchronizationUtility> syncDevUtility;
ControlSystemAdapterOPCUA *csaOPCUA;

std::atomic<bool> terminateMain;

static void SigHandler_Int(int sign) {
    cout << "Received SIGINT... terminating" << endl;
    terminateMain = true;
    csaOPCUA->stop();
    csaOPCUA->terminate();
    csaOPCUA->~ControlSystemAdapterOPCUA();
    cout << "OPC UA adapter termianted." << endl;
}

int main() {
    signal(SIGINT,  SigHandler_Int); // Registriert CTRL-C/SIGINT
    signal(SIGTERM, SigHandler_Int); // Registriert SIGTERM

    // Create the managers
    std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers = createPVManager();

    devManager = pvManagers.second;
    csManager = pvManagers.first;

	syncCsUtility.reset(new ChimeraTK::ControlSystemSynchronizationUtility(csManager));
	syncDevUtility.reset(new ChimeraTK::DeviceSynchronizationUtility(devManager));

    csManager->enablePersistentDataStorage();

    ChimeraTK::ApplicationBase::getInstance().setPVManager(devManager);
    ChimeraTK::ApplicationBase::getInstance().initialise();

    string pathToConfig = ChimeraTK::ApplicationBase::getInstance().getName() + "_mapping.xml";
	csaOPCUA = new ControlSystemAdapterOPCUA(csManager, syncDevUtility, pathToConfig);


    ChimeraTK::ApplicationBase::getInstance().run();

    while(!terminateMain) sleep(3600);  // sleep will be interrupted when signal is received
    csManager.reset();
    cout << "Application termianted." << endl;
}
