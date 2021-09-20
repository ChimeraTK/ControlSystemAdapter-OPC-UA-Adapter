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

#ifndef RUN_TIME_VALUE_GENERATOR_H
#define RUN_TIME_VALUE_GENERATOR_H

#include <thread>

#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>

using namespace ChimeraTK;

class runtime_value_generator {
   private:
    bool running;
    std::thread valueGeneratorThread;

   public:
    runtime_value_generator(boost::shared_ptr<DevicePVManager> devManager);
    ~runtime_value_generator();
    void generateValues(boost::shared_ptr<DevicePVManager> devManager);
};

#endif // RUN_TIME_VALUE_GENERATOR_H
