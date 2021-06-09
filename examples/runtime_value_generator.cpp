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

#include "runtime_value_generator.h"

#include <iostream>
#include <math.h>
#include <sys/sysinfo.h> 
#include <unistd.h>

#include <ChimeraTK/ReadAnyGroup.h>

#include "csa_opcua_adapter.h"

using std::cout;
using std::endl;
using namespace ChimeraTK;

runtime_value_generator::runtime_value_generator(boost::shared_ptr<DevicePVManager> devManager) {
  this->valueGeneratorThread = std::thread(&runtime_value_generator::generateValues, this, devManager);
}

runtime_value_generator::~runtime_value_generator() {
    this->running = false;
    if(this->valueGeneratorThread.joinable()) {
        this->valueGeneratorThread.join();
    }
}

void runtime_value_generator::generateValues(boost::shared_ptr<DevicePVManager> devManager) {
  this->running = true;

  ReadAnyGroup readAnyGroup;
  for(auto& pv : devManager->getAllProcessVariables()) {
    if(pv->isReadable()) readAnyGroup.add(pv);
  }

  // Time meassureing
  clock_t start, end, im1, im2, writeTime;
  start = clock();
  end = clock();
  int counter = 0;
  float duration1 = 0.0f;
  devManager->getProcessArray<int32_t>("t")->accessChannel(0) = vector<int32_t>{(int32_t)start};

  while(this->running) {
    //  FIXME -Or maybe not: The Const M_PI from math.h generate senceless values, hence I use fix value 3.141
    // 	double double_sine = csManager->getProcessScalar<double>("amplitude")->accessChannel(0) * sin(((2*M_PI)/csManager->getProcessScalar<int32_t>("period")->accessChannel(0)) * csManager->getProcessScalar<int32_t>("t")->accessChannel(0));
    double double_sine = devManager->getProcessArray<double>("amplitude")->accessChannel(0).at(0) *
        sin((2 * 3.141) / devManager->getProcessArray<double>("period")->accessChannel(0).at(0) *
            devManager->getProcessArray<int32_t>("t")->accessChannel(0).at(0));
    int32_t int_sine = round(double_sine);
    // 	bool bool_sine = (double_sine > 0)? true : false;
    // 	std::cout << "double_sine: " << double_sine << std::endl;
    // 	std::cout << "int_sine: " << int_sine << std::endl;
    // 	std::cout << "bool_sine: " << bool_sine << std::endl;

    devManager->getProcessArray<double>("double_sine")->accessChannel(0) = vector<double>{double_sine};
    devManager->getProcessArray<double>("double_sine")->write();
    devManager->getProcessArray<int32_t>("int_sine")->accessChannel(0) = vector<int32_t>{int_sine};
    devManager->getProcessArray<int32_t>("int_sine")->write();
    devManager->getProcessArray<int32_t>("t")->accessChannel(0) =
        vector<int32_t>{(int32_t)((end - start) / (CLOCKS_PER_SEC / 1000))};
    devManager->getProcessArray<int32_t>("t")->write();

    usleep(devManager->getProcessArray<int32_t>("dt")->accessChannel(0).at(0));
    end = clock();

    counter = 0;
    writeTime = 0;
    for(int32_t i = 1000; i < 65535; i = i + 1000) {
      string nameDouble = "testDoubleArray_" + to_string(i);
      string nameInt = "testIntArray_" + to_string(i);
      ProcessArray<double>::SharedPtr testDoubleArray = devManager->getProcessArray<double>(nameDouble);
      ProcessArray<int32_t>::SharedPtr testIntArray = devManager->getProcessArray<int32_t>(nameInt);
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
      im1 = clock();
      testDoubleArray->write();
      testIntArray->write();
      im2 = clock();
      writeTime += im2 - im1;
      counter++;
    }
		
    duration1 = ((float)writeTime / CLOCKS_PER_SEC) * 1000.0f;
    printf("1: %d write passes, duration write: %.3f ms\n", counter, duration1);

    ProcessArray<double>::SharedPtr testDoubleArray = devManager->getProcessArray<double>("testDoubleArray_65535");
    ProcessArray<int32_t>::SharedPtr testIntArray = devManager->getProcessArray<int32_t>("testIntArray_65535");
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

    clock_t tmp2 = clock();

    testDoubleArray->write();
    testIntArray->write();

    clock_t tmp3 = clock();
    duration1 = ((float)(tmp3 - tmp2) / CLOCKS_PER_SEC) * 1000.0f;
    printf("write pass 2, duration write: %.3f ms\n", duration1);
	
    //while(readAnyGroup.readAnyNonBlocking().isValid()) continue;
  }
}
