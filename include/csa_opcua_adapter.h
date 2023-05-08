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

#ifndef CSA_OPCUA_ADAPTER_H
#define CSA_OPCUA_ADAPTER_H

#include "ChimeraTK/ControlSystemAdapter/ApplicationBase.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "csa_processvariable.h"
#include "ua_adapter.h"

#include <thread>
#include <vector>

/** @class csa_opcua_adapter
 *	@brief This class provide the two parts of the OPCUA Adapter. 
 * First of all the OPCUA server starts with a port number defined in config-file (recommended 16664),
 * following the mapping process start. For this, the ProcessVariable from ControlSystemPVManager will be mapped to the OPCUA Model. During the mapping process also all 
 * \<additionalNodes\> will be mapped in OPC UA Model
 *   
 *  @author Chris Iatrou
 *  @author Julian Rahm
 *  @date 22.11.2016
 * 
 */

class csa_opcua_adapter {
 private:
  std::thread adapter_thread;
  ua_uaadapter* adapter;
  boost::shared_ptr<ControlSystemPVManager> csManager;

 public:
  /**
   * @brief Constructor for ControlSystemAdapter-OPC-UA-Adapter
   *
   * @param csManager Provide the hole PVManager from control-system-adapter to map all processvariable to the OPC UA-Model
   * @param configFile Define the path to the mapping-file with config and mapping information
   */
  csa_opcua_adapter(boost::shared_ptr<ControlSystemPVManager> csManager, string configFile);

  /**
   * @brief Destructor to stop the running thread, hence it stops the OPC UA server
   */
  ~csa_opcua_adapter();

  /**
   * @brief Return the ControlsystemPVManager of the class
   *
   * @return Returns a ControlSystemPVManager
   */
  boost::shared_ptr<ControlSystemPVManager> const& getControlSystemManager() const;

  /**
   * @brief Return the uaadapter, hence the OPC UA server
   *
   * @return Return the ua_uaadapter
   */
  ua_uaadapter* getUAAdapter();

  /**
   * @brief Start all objects in single threads for this case only the opc ua server
   */
  void start();

  /**
   * @brief Stop all objects in single threads for this case only the opc ua server
   */
  void stop();

  /**
   * @brief Checks if the opcua server is still running and return the suitable bool value
   *
   * @return The current running state in form of true/false
   */
  bool isRunning();
};

#endif // CSA_OPCUA_ADAPTER_H
