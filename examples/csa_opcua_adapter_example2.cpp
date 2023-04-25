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
 * Copyright (c) 2018-2019 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
 */

extern "C" {
#include "csa_namespace.h"

#include <signal.h>
#include <unistd.h>
}

#include <iostream>

#include <chrono>
#include <errno.h>
#include <stdlib.h>

#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ApplicationBase.h"

using namespace std;
using namespace ChimeraTK;

struct MyApp : public ApplicationBase {
  MyApp() : ApplicationBase("opcuaAdapter") {}
  ~MyApp() { shutdown(); }

  void run() { cout << "Application run..." << endl; }

  void shutdown() {
    std::lock_guard<std::mutex> lock(instance_mutex);
    instance = nullptr;
    hasBeenShutdown = true;
    cout << "Application shutdown..." << endl;
  }

  void initialise() {
    boost::shared_ptr<DevicePVManager> devManager = this->_processVariableManager;

    //Generate dummy data
    ProcessArray<int8_t>::SharedPtr intA8dev = this->_processVariableManager->createProcessArray<int8_t>(
        SynchronizationDirection::controlSystemToDevice, "int8Scalar", 1, "Iatrou^2/Rahm");
    ProcessArray<uint8_t>::SharedPtr intAu8dev = this->_processVariableManager->createProcessArray<uint8_t>(
        SynchronizationDirection::controlSystemToDevice, "uint8Scalar", 1, "mIatrou*Rahm");
    ProcessArray<int16_t>::SharedPtr intA16dev = this->_processVariableManager->createProcessArray<int16_t>(
        SynchronizationDirection::controlSystemToDevice, "int16Scalar", 1);
    ProcessArray<uint16_t>::SharedPtr intAu16dev = this->_processVariableManager->createProcessArray<uint16_t>(
        SynchronizationDirection::controlSystemToDevice, "uint16Scalar", 1);
    ProcessArray<int32_t>::SharedPtr intA32dev = this->_processVariableManager->createProcessArray<int32_t>(
        SynchronizationDirection::controlSystemToDevice, "int32Scalar", 1);
    ProcessArray<uint32_t>::SharedPtr intAu32dev = this->_processVariableManager->createProcessArray<uint32_t>(
        SynchronizationDirection::controlSystemToDevice, "uint32Scalar", 1);
    ProcessArray<float>::SharedPtr intAfdev = this->_processVariableManager->createProcessArray<float>(
        SynchronizationDirection::controlSystemToDevice, "floatScalar", 1);
    ProcessArray<double>::SharedPtr intAddev = this->_processVariableManager->createProcessArray<double>(
        SynchronizationDirection::controlSystemToDevice, "doubleScalar", 1);
    ProcessArray<string>::SharedPtr stringAddev =
        devManager->createProcessArray<string>(SynchronizationDirection::controlSystemToDevice, "stringScalar", 1);
    ProcessArray<string>::SharedPtr stringAddev2 = devManager->createProcessArray<string>(
        SynchronizationDirection::deviceToControlSystem, "1/FOLDER/defaultSep/stringScalar", 1, "my unit", "desc");

    ProcessArray<int8_t>::SharedPtr intB15A8dev =
        this->_processVariableManager->createProcessArray<int8_t>(SynchronizationDirection::controlSystemToDevice,
                                                                  "int8Array_s15", 15, "mIatrou*Rahm", "Die Einheit ist essentiel und sollte SI Einheit sein...");
    ProcessArray<uint8_t>::SharedPtr intB10Au8dev = this->_processVariableManager->createProcessArray<uint8_t>(
        SynchronizationDirection::controlSystemToDevice, "uint8Array_s10", 10, "1/Rahm");
    ProcessArray<int16_t>::SharedPtr intB15A16dev =
        this->_processVariableManager->createProcessArray<int16_t>(SynchronizationDirection::controlSystemToDevice,
                                                                   "int16Array_s15", 15, "Iatrou", "Beschreibung eines Iatrous");
    ProcessArray<uint16_t>::SharedPtr intB10Au16dev = this->_processVariableManager->createProcessArray<uint16_t>(
        SynchronizationDirection::controlSystemToDevice, "uint16Array_s10", 10);
    ProcessArray<int32_t>::SharedPtr intB15A32dev = this->_processVariableManager->createProcessArray<int32_t>(
        SynchronizationDirection::controlSystemToDevice, "int32Array_s15", 15);
    ProcessArray<uint32_t>::SharedPtr intB10Au32dev = this->_processVariableManager->createProcessArray<uint32_t>(
        SynchronizationDirection::controlSystemToDevice, "uint32Array_s10", 10);
    ProcessArray<double>::SharedPtr intB15Afdev = this->_processVariableManager->createProcessArray<double>(
        SynchronizationDirection::controlSystemToDevice, "doubleArray_s15", 15);
    ProcessArray<float>::SharedPtr intB10Addev = this->_processVariableManager->createProcessArray<float>(
        SynchronizationDirection::controlSystemToDevice, "floatArray_s10", 10);

    // example mapping variable
    ProcessArray<int8_t>::SharedPtr intB15A8devMap = devManager->createProcessArray<int8_t>(
        SynchronizationDirection::controlSystemToDevice, "Mein/Name_ist#int8Array_s15", 15, "Iatrou^2/Rahm");
    ProcessArray<int8_t>::SharedPtr intB15A8devMap2 = devManager->createProcessArray<int8_t>(
        SynchronizationDirection::controlSystemToDevice, "Mein/Name_ist#int8Array", 15, "Iatrou^2/Rahm");
    ProcessArray<uint8_t>::SharedPtr intB10Au8devMap1 =
        devManager->createProcessArray<uint8_t>(SynchronizationDirection::controlSystemToDevice,
                                                "/Dein/Name//ist/uint8Array_s10", 10, "Iatrou^2/Rahm", "Beschreibung");
    ProcessArray<uint8_t>::SharedPtr intB10Au8devMap2 = devManager->createProcessArray<uint8_t>(
        SynchronizationDirection::controlSystemToDevice, "Unser/Name/ist_uint8Array_s10", 10);
    ProcessArray<uint32_t>::SharedPtr intAu32devMap = devManager->createProcessArray<uint32_t>(
        SynchronizationDirection::controlSystemToDevice, "Ist/Name/dieser/uint32Scalar", 1);
    ProcessArray<int32_t>::SharedPtr intA32devMap = devManager->createProcessArray<int32_t>(
        SynchronizationDirection::controlSystemToDevice, "Ist/Name/dieser/int32Scalar", 1);
    ProcessArray<double>::SharedPtr doubledevMap = devManager->createProcessArray<double>(
        SynchronizationDirection::controlSystemToDevice, "Ist/Name/dieser/doubleScalar", 1);
  }
};

MyApp myApp;
