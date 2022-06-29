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
#include "csa_opcua_adapter.h"

boost::shared_ptr<ControlSystemPVManager> csManager;
boost::shared_ptr<DevicePVManager> devManager;
csa_opcua_adapter* csaOPCUA;

std::atomic<bool> terminateMain;

static void SigHandler_Int(int sign) {
  cout << "Received SIGINT... terminating" << endl;
  terminateMain = true;
  if(csaOPCUA) {
    csaOPCUA->stop();
    csaOPCUA->~csa_opcua_adapter();
  }
  ChimeraTK::ApplicationBase::getInstance().shutdown();
  cout << "OPC UA adapter termianted." << endl;
}

int main() {
  signal(SIGINT, SigHandler_Int);  // Registriert CTRL-C/SIGINT
  signal(SIGTERM, SigHandler_Int); // Registriert SIGTERM

  /* Block SIGINT until the OPC UA Adapter is running.
   * So the adapter is in a consistent state when we shut it down. */
  sigset_t intmask;
  sigemptyset(&intmask);
  sigaddset(&intmask, SIGINT);
  sigprocmask(SIG_BLOCK, &intmask, NULL);

  cout << "Create the Managers" << endl;
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();

  devManager = pvManagers.second;
  csManager = pvManagers.first;

  csManager->enablePersistentDataStorage();
  ChimeraTK::ApplicationBase::getInstance().setPVManager(devManager);
  ChimeraTK::ApplicationBase::getInstance().initialise();

  cout << "Start the mapping" << endl;
  string pathToConfig = ChimeraTK::ApplicationBase::getInstance().getName() + "_mapping.xml";
  cout << pathToConfig << endl;

  cout << "Create the adapter" << endl;
  csaOPCUA = new csa_opcua_adapter(csManager, pathToConfig);

  cout << "Run the application instance" << endl;
  ChimeraTK::ApplicationBase::getInstance().run();

  cout << "Start the OPC UA Adapter" << endl;
  csaOPCUA->start();

  /* Unblock SIGINT */
  sigprocmask(SIG_UNBLOCK, &intmask, NULL);

  while(!terminateMain) sleep(3600); // sleep will be interrupted when signal is received
  csManager.reset();

  cout << "Application termianted." << endl;
}
