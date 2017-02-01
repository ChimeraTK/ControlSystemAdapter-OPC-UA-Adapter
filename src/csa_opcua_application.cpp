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

/*! @mainpage 
 *
 * @section build_sec Build Options
 * To generate the project, it is possible to do this with different parameter. In the table below there are all possible options with a shirt description.
 * <table>
 * <caption id="multi_row">Build Options</caption>
 * <tr><th>Option                		<th>Mode <th>Description
 * <tr><td colspan="3">Code quality control options 	
 * <tr><td>ENABLE_COVERAGE					<td> OFF <td> Enable coverage statistics. After succsesfully generation you can see all the results in /build/coverage_html. There are html-files, so you can view the statistics in any browser.
 * <tr><td>BUILD_STATIC_ANALYSIS		<td> OFF <td> Instead of building a binary, perform a static code analysis using clangs analyzer.
 * <tr><td>ENABLE_LINTING						<td> OFF <td> Enables the cppcheck static analyzer, the result will be stored in /build/static_analysis.
 * <tr><td>ENABLE_RUNTIME_ANALYSIS 	<td> OFF <td> Build Runtime analysis. This test check all testcases if there are some mem leaks, heap corruptions.
 * <tr><td>ENABLE_UNIT_TESTS 				<td> ON  <td> Compile all unit tests, this tests are used by all above analysis.
 * <tr><td colspan="3"> Miscellaneous Options 
 * <tr><td>ENABLE_DOCUMENTATION			<td> OFF <td> Generate Doxygen documentation, especially this one you read now...
 * <tr><td>ENABLE_CREATEMODEL				<td> OFF <td> Create model from XML description, this is only needed if you change the opc ua information model. In all other cases there is a pre build model in /include/model_prebuilt.
 * <tr><td>UPDATE_OPEN62541					<td> OFF <td> Build new open62541 from git repository. This option should only be used during development time. Because there is no warranty, that the adapter works with a brand new open62541-stack. So you need some time to check the whole adaper.
 * </table>
 * 
 * @section Miscellaneous
 *  
 * @author Julian Rahm \<Julian.Rahm@tu-dresden.de>
 * @author Chris Iatrou \<Chris_Paul.Iatrou@tu-dresden.de>
 * @date 22.11.2016
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
#include "ChimeraTK/ControlSystemAdapter/DeviceSynchronizationUtility.h"
#include "csa_opcua_adapter.h"

boost::shared_ptr<ControlSystemPVManager> csManager;
boost::shared_ptr<DevicePVManager> devManager;
boost::shared_ptr<ControlSystemSynchronizationUtility> syncCsUtility;
boost::shared_ptr<DeviceSynchronizationUtility> syncDevUtility;
csa_opcua_adapter *csaOPCUA;

std::atomic<bool> terminateMain;

static void SigHandler_Int(int sign) {
	cout << "Received SIGINT... terminating" << endl;
	terminateMain = true;
	csaOPCUA->stop();
	csaOPCUA->~csa_opcua_adapter();
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
	csaOPCUA = new csa_opcua_adapter(csManager, pathToConfig);
	
	ChimeraTK::ApplicationBase::getInstance().run();

	while(!terminateMain) sleep(3600);  // sleep will be interrupted when signal is received
	csManager.reset();
	cout << "Application termianted." << endl;
}
