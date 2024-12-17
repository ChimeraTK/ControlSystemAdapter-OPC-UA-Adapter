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
 * Copyright (c) 2023 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
 */

/*! @mainpage
 *
 * @section build_sec Build Options
 * To generate the project, it is possible to do this with different parameter. In the table below there are all
 * possible options with a shirt description. <table> <caption id="multi_row">Build Options</caption> <tr><th>Option
 * <th>Mode <th>Description <tr><td colspan="3">Code quality control options <tr><td>ENABLE_COVERAGE_REPORT 	<td> OFF
 * <td> Enable coverage statistics. After succsesfully generation you can see all the results in /build/coverage_html.
 * There are html-files, so you can view the statistics in any browser. <tr><td>BUILD_STATIC_ANALYSIS
 * <td> OFF <td> Instead of building a binary, perform a static code analysis using clangs analyzer. <tr><td>ENABLE_LINTING
 * <td> OFF <td> Enables the cppcheck static analyzer, the result will be stored in /build/static_analysis.
 * <tr><td>ENABLE_RUNTIME_ANALYSIS 	<td> OFF <td> Build Runtime analysis. This test check all testcases if there are
 * some mem leaks, heap corruptions. <tr><td>ENABLE_UNIT_TESTS 				<td> ON  <td> Compile all unit tests, this
 * tests are used by all above analysis. <tr><td>CHECK_CTK_STYLE          <td> OFF <td> Check ChimeraTK code style and
 * license information. <tr><td colspan="3"> Miscellaneous Options <tr><td>SUPPRESS_AUTO_DOC_BUILD	<td> OFF <td>
 * Suppress generating doxygen documentation, especially this one you read now... <tr><td>INSTALL_SAMPLE_ADAPTER   <td>
 * OFF <td> Install the sample application called ControlSystem-OPCUA_Sample_Adapter
 *  * </table>
 *
 * @section Miscellaneous
 *
 * @author Julian Rahm \<Julian.Rahm@tu-dresden.de>
 * @author Chris Iatrou \<Chris_Paul.Iatrou@tu-dresden.de>
 * @date 22.11.2016
 */
extern "C" {
#include <signal.h>
#include <unistd.h>
}

#include "ChimeraTK/ControlSystemAdapter/ApplicationBase.h"
#include "csa_opcua_adapter.h"
#include "open62541/plugin/log_stdout.h"

#include <atomic>
#include <iostream>
#include <string>

boost::shared_ptr<ChimeraTK::ControlSystemPVManager> csManager;
boost::shared_ptr<ChimeraTK::DevicePVManager> devManager;
ChimeraTK::csa_opcua_adapter* csaOPCUA;

std::atomic<bool> terminateMain;

static void SigHandler_Int(int sign) {
  UA_LOG_INFO(csaOPCUA->getLogger(), UA_LOGCATEGORY_USERLAND, "Received SIGINT... terminating");
  terminateMain = true;
  if(csaOPCUA) {
    csaOPCUA->stop();
    csaOPCUA->~csa_opcua_adapter();
  }
  ChimeraTK::ApplicationBase::getInstance().shutdown();
  std::cout << "OPC UA adapter termianted." << std::endl;
}

int main() {
  signal(SIGINT, SigHandler_Int);  // Registriert CTRL-C/SIGINT
  signal(SIGTERM, SigHandler_Int); // Registriert SIGTERM

  /* Block SIGINT until the OPC UA Adapter is running.
   * So the adapter is in a consistent state when we shut it down. */
  sigset_t intmask;
  sigemptyset(&intmask);
  sigaddset(&intmask, SIGINT);
  sigprocmask(SIG_BLOCK, &intmask, nullptr);

  std::pair<boost::shared_ptr<ChimeraTK::ControlSystemPVManager>, boost::shared_ptr<ChimeraTK::DevicePVManager>>
      pvManagers = ChimeraTK::createPVManager();

  devManager = pvManagers.second;
  csManager = pvManagers.first;

  csManager->enablePersistentDataStorage();
  ChimeraTK::ApplicationBase::getInstance().setPVManager(devManager);
  ChimeraTK::ApplicationBase::getInstance().initialise();

  string pathToConfig = ChimeraTK::ApplicationBase::getInstance().getName() + "_mapping.xml";
  csaOPCUA = new ChimeraTK::csa_opcua_adapter(csManager, pathToConfig);

  UA_LOG_INFO(csaOPCUA->getLogger(), UA_LOGCATEGORY_USERLAND, "Optimize unmapped variables in the application base");
  ChimeraTK::ApplicationBase::getInstance().optimiseUnmappedVariables(csaOPCUA->getUnusedVariables());

  UA_LOG_INFO(csaOPCUA->getLogger(), UA_LOGCATEGORY_USERLAND, "Run the application instance");
  ChimeraTK::ApplicationBase::getInstance().run();

  UA_LOG_INFO(csaOPCUA->getLogger(), UA_LOGCATEGORY_USERLAND, "Start the OPC UA Adapter");
  csaOPCUA->start();

  /* Unblock SIGINT */
  sigprocmask(SIG_UNBLOCK, &intmask, nullptr);

  while(!terminateMain) sleep(3600); // sleep will be interrupted when signal is received
  csManager.reset();

  std::cout << "Application termianted." << std::endl;
}
