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
#include <time.h>
#include <errno.h>
#include <stdlib.h>

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "csa_opcua_adapter.h"
#include "runtime_value_generator.h"

using namespace std;
using namespace ChimeraTK;

runtime_value_generator* valGen;
csa_opcua_adapter* csaOPCUA;

/* FUNCTIONS */
static void SigHandler_Int(int sign) {
  cout << "Received SIGINT... terminating" << endl;
  valGen->~runtime_value_generator();
  csaOPCUA->stop();
  csaOPCUA->~csa_opcua_adapter();
  cout << "terminated threads" << endl;
}

int main() {
  signal(SIGINT, SigHandler_Int);  // Registriert CTRL-C/SIGINT
  signal(SIGTERM, SigHandler_Int); // Registriert SIGTERM

  clock_t start = clock();

  // Create the managers
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers =
      createPVManager();

  boost::shared_ptr<DevicePVManager> devManager = pvManagers.second;
  boost::shared_ptr<ControlSystemPVManager> csManager = pvManagers.first;

  // Generate dummy data
  ProcessArray<int8_t>::SharedPtr intA8dev = devManager->createProcessArray<int8_t>(
      SynchronizationDirection::controlSystemToDevice, "int8Scalar", 1, "Iatrou^2/Rahm");
  ProcessArray<uint8_t>::SharedPtr intAu8dev = devManager->createProcessArray<uint8_t>(
      SynchronizationDirection::controlSystemToDevice, "uint8Scalar", 1, "mIatrou*Rahm");
  ProcessArray<int16_t>::SharedPtr intA16dev =
      devManager->createProcessArray<int16_t>(SynchronizationDirection::controlSystemToDevice, "int16Scalar", 1);
  ProcessArray<uint16_t>::SharedPtr intAu16dev =
      devManager->createProcessArray<uint16_t>(SynchronizationDirection::controlSystemToDevice, "uint16Scalar", 1);
  ProcessArray<int32_t>::SharedPtr intA32dev =
      devManager->createProcessArray<int32_t>(SynchronizationDirection::controlSystemToDevice, "int32Scalar", 1);
  ProcessArray<uint32_t>::SharedPtr intAu32dev =
      devManager->createProcessArray<uint32_t>(SynchronizationDirection::controlSystemToDevice, "uint32Scalar", 1);
  ProcessArray<float>::SharedPtr intAfdev =
      devManager->createProcessArray<float>(SynchronizationDirection::controlSystemToDevice, "floatScalar", 1);
  ProcessArray<double>::SharedPtr intAddev =
      devManager->createProcessArray<double>(SynchronizationDirection::controlSystemToDevice, "doubleScalar", 1);
  ProcessArray<string>::SharedPtr stringAddev =
      devManager->createProcessArray<string>(SynchronizationDirection::controlSystemToDevice, "stringScalar", 1);
  ProcessArray<string>::SharedPtr stringAddev2 =
      devManager->createProcessArray<string>(SynchronizationDirection::deviceToControlSystem,
                                             "1/FOLDER/defaultSep/stringScalar", 1, "my description", "desc");

  ProcessArray<int8_t>::SharedPtr intB15A8dev =
      devManager->createProcessArray<int8_t>(SynchronizationDirection::controlSystemToDevice, "int8Array_s15", 15,
                                             "mIatrou*Rahm", "Die Einheit ist essentiel und sollte SI Einheit sein...");
  ProcessArray<uint8_t>::SharedPtr intB10Au8dev = devManager->createProcessArray<uint8_t>(
      SynchronizationDirection::controlSystemToDevice, "uint8Array_s10", 10, "1/Rahm");
  ProcessArray<int16_t>::SharedPtr intB15A16dev = devManager->createProcessArray<int16_t>(
      SynchronizationDirection::controlSystemToDevice, "int16Array_s15", 15, "Iatrou", "Beschreibung eines Iatrous");
  ProcessArray<uint16_t>::SharedPtr intB10Au16dev = devManager->createProcessArray<uint16_t>(
      SynchronizationDirection::controlSystemToDevice, "uint16Array_s10", 10);
  ProcessArray<int32_t>::SharedPtr intB15A32dev =
      devManager->createProcessArray<int32_t>(SynchronizationDirection::controlSystemToDevice, "int32Array_s15", 15);
  ProcessArray<uint32_t>::SharedPtr intB10Au32dev = devManager->createProcessArray<uint32_t>(
      SynchronizationDirection::controlSystemToDevice, "uint32Array_s10", 10);
  ProcessArray<double>::SharedPtr intB15Afdev =
      devManager->createProcessArray<double>(SynchronizationDirection::controlSystemToDevice, "doubleArray_s15", 15);
  ProcessArray<float>::SharedPtr intB10Addev =
      devManager->createProcessArray<float>(SynchronizationDirection::controlSystemToDevice, "floatArray_s10", 10);

  // data generation cycle time in ms
  ProcessArray<int32_t>::SharedPtr dtDev =
      devManager->createProcessArray<int32_t>(SynchronizationDirection::controlSystemToDevice, "dt", 1);
  // time since server start in ms
  ProcessArray<int32_t>::SharedPtr tDev =
      devManager->createProcessArray<int32_t>(SynchronizationDirection::deviceToControlSystem, "t", 1);
  ProcessArray<double>::SharedPtr periodDev =
      devManager->createProcessArray<double>(SynchronizationDirection::controlSystemToDevice, "period", 1);
  ProcessArray<double>::SharedPtr amplitudeDev =
      devManager->createProcessArray<double>(SynchronizationDirection::controlSystemToDevice, "amplitude", 1);
  ProcessArray<double>::SharedPtr double_sineDev =
      devManager->createProcessArray<double>(SynchronizationDirection::deviceToControlSystem, "double_sine", 1);
  ProcessArray<int32_t>::SharedPtr int_sineDev =
      devManager->createProcessArray<int32_t>(SynchronizationDirection::deviceToControlSystem, "int_sine", 1);

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

  time_t t;
  time(&t);
  srand((unsigned int)t);
  for(int32_t i = 1000; i < 65535; i = i + 1000) {
    string nameDouble = "testDoubleArray_" + to_string(i);
    string nameInt = "testIntArray_" + to_string(i);
    ProcessArray<double>::SharedPtr testDoubleArray =
        devManager->createProcessArray<double>(SynchronizationDirection::deviceToControlSystem, nameDouble, i);
    ProcessArray<int32_t>::SharedPtr testIntArray =
        devManager->createProcessArray<int32_t>(SynchronizationDirection::deviceToControlSystem, nameInt, i);
    for(int32_t k = 0; k < i; k++) {
      if(k % 2 == 0) {
        testDoubleArray->accessChannel(0).at(k) = rand() % 6 + 1;
        testIntArray->accessChannel(0).at(k) = rand() % 6 + 1;
      }
      else {
        testDoubleArray->accessChannel(0).at(k) = rand() % 50 + 10;
        testIntArray->accessChannel(0).at(k) = rand() % 50 + 10;
      }
    }
    testDoubleArray->write();
    testIntArray->write();
  }

  /* Adding some folder with variables
  *
  */
  time(&t);
  srand((unsigned int)t);
  for(int32_t n = 1; n < 10; n++) {
    for(int32_t i = 1000; i < 65535; i = i + 10000) {
      string nameDouble = "folder/f" + to_string(n) + "/testDoubleArray_" + to_string(i);
      string nameInt = "folder/f" + to_string(n) + "/testIntArray_" + to_string(i);
      ProcessArray<double>::SharedPtr testDoubleArray =
          devManager->createProcessArray<double>(SynchronizationDirection::deviceToControlSystem, nameDouble, i);
      ProcessArray<int32_t>::SharedPtr testIntArray =
          devManager->createProcessArray<int32_t>(SynchronizationDirection::deviceToControlSystem, nameInt, i);
      for(int32_t k = i - 10; k < i; k++) {
        if(k % 2 == 0) {
          testDoubleArray->accessChannel(0).at(k) = rand() % 6 + 1;
          testIntArray->accessChannel(0).at(k) = rand() % 6 + 1;
        }
        else {
          testDoubleArray->accessChannel(0).at(k) = rand() % 50 + 10;
          testIntArray->accessChannel(0).at(k) = rand() % 50 + 10;
        }
      }
      testDoubleArray->write();
      testIntArray->write();
    }
  }

  ProcessArray<double>::SharedPtr testDoubleArray = devManager->createProcessArray<double>(
      SynchronizationDirection::deviceToControlSystem, "testDoubleArray_65535", 65535);
  ProcessArray<int32_t>::SharedPtr testIntArray = devManager->createProcessArray<int32_t>(
      SynchronizationDirection::deviceToControlSystem, "testIntArray_65535", 65535);
  for(int32_t i = 0; i < 65535; i++) {
    if(i % 2 == 0) {
      testDoubleArray->accessChannel(0).at(i) = rand() % 6 + 1;
      testIntArray->accessChannel(0).at(i) = rand() % 6 + 1;
    }
    else {
      testDoubleArray->accessChannel(0).at(i) = rand() % 50 + 10;
      testIntArray->accessChannel(0).at(i) = rand() % 50 + 10;
    }
  }
  testDoubleArray->write();
  testIntArray->write();

  stringAddev2->accessChannel(0).at(0) = "Test value";
  stringAddev2->write();
  // start values
  int32_t microseconds = 1000000;
  double period = 3.141;
  double amplitude = 10;

  csManager->getProcessArray<string>("stringScalar")->accessChannel(0) = vector<string>{"Merk"};
  // Set values
  csManager->getProcessArray<int32_t>("dt")->accessChannel(0) = vector<int32_t>{microseconds};
  csManager->getProcessArray<int32_t>("dt")->write();
  csManager->getProcessArray<double>("period")->accessChannel(0) = vector<double>{period};
  csManager->getProcessArray<double>("period")->write();
  csManager->getProcessArray<double>("amplitude")->accessChannel(0) = vector<double>{amplitude};
  csManager->getProcessArray<double>("amplitude")->write();

  csManager->getProcessArray<int8_t>("int8Scalar")->accessChannel(0) = vector<int8_t>{12};
  cout << "write dummy Data..." << std::endl;

  string pathToConfig = "opcuaAdapter_mapping.xml";
  csaOPCUA = new csa_opcua_adapter(csManager, pathToConfig);
  csaOPCUA->start();
  while(!csaOPCUA->getUAAdapter()->running) {
  };

  // Only for Sin ValueGenerator
  valGen = new runtime_value_generator(devManager);

  float duration = ((float)(clock() - start) / CLOCKS_PER_SEC) * 1000.0f;
  printf("Start duration: %.3f ms\n", duration);

  // Server is running
  std::cout << "server is running..." << std::endl;
  while(csaOPCUA->isRunning()) {
    sleep(2);
  }

  return 0;
}
