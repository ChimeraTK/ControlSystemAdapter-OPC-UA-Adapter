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

#include "csa_opcua_adapter.h"

#include "ipc_manager.h"

using std::cout;
using std::endl;
using namespace ChimeraTK;
	 
runtime_value_generator::runtime_value_generator(boost::shared_ptr<DevicePVManager> devManager) {
	this->devManager = devManager;
//	this->syncDevUtility = syncDevUtility;
	this->doStart();
}

runtime_value_generator::~runtime_value_generator() {
	if (this->isRunning()) {
		this->doStop();
	}
}

void runtime_value_generator::generateValues(boost::shared_ptr<DevicePVManager> devManager) {
	// Time meassureing
	clock_t start, end;
	start = clock();
	end = clock();
	devManager->getProcessArray<int32_t>("t")->accessChannel(0) = vector<int32_t> {(int32_t)start};
	
	while(true) {
//  FIXME -Or maybe not: The Const M_PI from math.h generate senceless values, hence I use fix value 3.141
// 	double double_sine = csManager->getProcessScalar<double>("amplitude")->accessChannel(0) * sin(((2*M_PI)/csManager->getProcessScalar<int32_t>("period")->accessChannel(0)) * csManager->getProcessScalar<int32_t>("t")->accessChannel(0));
		double double_sine = devManager->getProcessArray<double>("amplitude")->accessChannel(0).at(0) * sin((2*3.141)/devManager->getProcessArray<double>("period")->accessChannel(0).at(0) * devManager->getProcessArray<int32_t>("t")->accessChannel(0).at(0));
		int32_t int_sine = round(double_sine);
// 	bool bool_sine = (double_sine > 0)? true : false;
// 	std::cout << "double_sine: " << double_sine << std::endl;
// 	std::cout << "int_sine: " << int_sine << std::endl;
// 	std::cout << "bool_sine: " << bool_sine << std::endl;
			
		devManager->getProcessArray<double>("double_sine")->accessChannel(0) = vector<double> {double_sine};
		devManager->getProcessArray<double>("double_sine")->write();
		devManager->getProcessArray<int32_t>("int_sine")->accessChannel(0) = vector<int32_t> {int_sine};
		devManager->getProcessArray<int32_t>("int_sine")->write();
		devManager->getProcessArray<int32_t>("t")->accessChannel(0) = vector<int32_t> {(int32_t)((end - start)/(CLOCKS_PER_SEC/1000))};
		devManager->getProcessArray<int32_t>("t")->write();
		
		usleep(devManager->getProcessArray<int32_t>("dt")->accessChannel(0).at(0));
		end = clock();
		
		
		for(int32_t i=1000; i < 65535; i=i+1000) {
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
			testDoubleArray->write();
			testIntArray->write();
		}
		
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
		testDoubleArray->write();
		testIntArray->write();
		
//		syncDevUtility->receiveAll();
	}
	
}

void runtime_value_generator::workerThread() {
	bool run = true;
	
	thread *valueGeneratorThread = new std::thread(generateValues, this->devManager);
	
	while (run == true) {
		if (! this->isRunning()) {
			run = false;
		}
		sleep(1);
	}
	
	if(valueGeneratorThread->joinable()) {
		valueGeneratorThread->join();
	}
	delete valueGeneratorThread;
}


