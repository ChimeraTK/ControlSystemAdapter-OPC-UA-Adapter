// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
// SPDX-FileCopyrightText: 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ua_adapter.h"

#include <memory>
#include <set>
#include <string>
#include <thread>
namespace ChimeraTK {
  /** @class csa_opcua_adapter
   *	@brief This class provide the two parts of the OPCUA Adapter.
   * First of all the OPCUA server starts with a port number defined in config-file (recommended 16664),
   * following the mapping process start. For this, the ProcessVariable from ControlSystemPVManager will be mapped to
   *the OPCUA Model. During the mapping process also all
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
    std::thread observer_thread;
    std::shared_ptr<ua_uaadapter> adapter;
    boost::shared_ptr<ControlSystemPVManager> csManager;
    std::set<std::string> unusedVariables;

   public:
    /**
     * @brief Constructor for ControlSystemAdapter-OPC-UA-Adapter
     *
     * @param csManager Provide the hole PVManager from control-system-adapter to map all processvariable to the OPC UA-Model
     * @param configFile Define the path to the mapping-file with config and mapping information
     */
    csa_opcua_adapter(boost::shared_ptr<ControlSystemPVManager> csManager, const std::string& configFile);

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
    std::shared_ptr<ua_uaadapter> getUAAdapter();

    const std::set<std::string>& getUnusedVariables() const;

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

    /**
     * @brief Get the adapter logger.
     *
     * @return The pointer of the logger used in the adapter.
     */
    const UA_Logger* getLogger();
  };
} // namespace ChimeraTK
