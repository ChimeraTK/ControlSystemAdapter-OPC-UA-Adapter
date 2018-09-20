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

#include "ipc_managed_object.h"
#include <iostream>
#include <time.h>

void ipc_managed_object_callWorker(ipc_managed_object *theClass) {
  try {
    theClass->workerThread();
  }
  catch (...) {
    cout << "IPC: Managed Object; thread threw exception" << endl;
  }
}

ipc_managed_object::ipc_managed_object() {
  this->ipc_id = 0;
  this->threadTask = nullptr;
  this->manager    = nullptr;
  this->thread_run = false;
}

ipc_managed_object::~ipc_managed_object() {
  if (this->isRunning()) 
  {
    this->doStop();
  }	
}

ipc_manager *ipc_managed_object::getIpcManager() {
  return this->manager;
}

bool ipc_managed_object::isManaged() {
  if (this->manager != nullptr)
    return true;
  return false;
}

bool ipc_managed_object::assignManager(ipc_manager *manager) {
  if (manager == nullptr)
    return false;
  this->manager = manager;
  return true;
}

void ipc_managed_object::setIpcId(uint32_t ipc_id)
{
  this->ipc_id = ipc_id;
}

uint32_t ipc_managed_object::getIpcId()
{
  return this->ipc_id;
}

bool ipc_managed_object::isRunning() 
{
  return this->thread_run;
}

bool ipc_managed_object::taskRunningAttached() 
{
  return this->threadTask->joinable();
}

uint32_t ipc_managed_object::doStop()
{
  cout << "ipc_managed_object being stopped" << endl;
  this->thread_run = false;
  cout << "ipc_managed_object was stopped" << endl;
  return 0;
}

uint32_t ipc_managed_object::doStart()
{
  this->mtx_threadOperations.lock();
  if (this->isRunning()) {
    this->mtx_threadOperations.unlock();
    return 0;
  }
  this->thread_run = true;
  this->threadTask = new std::thread(ipc_managed_object_callWorker, this);
  this->mtx_threadOperations.unlock();
  return 0;
}
