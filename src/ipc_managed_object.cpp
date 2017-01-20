/*
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 * Chair for Process Systems Engineering
 * Technical University of Dresden
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/** @class ipc_managed_object
 *	@brief 
 *   
 *  @author Chris Iatrou
 *	@author Julian Rahm
 *  @date 22.11.2016
 * 
 */

#include "ipc_managed_object.h"
#include <iostream>
#include <time.h>

void ipc_managed_object_callWorker(ipc_managed_object *theClass)
{
  try {
    theClass->workerThread();
  }
  catch (...) {
    cout << "IPC: Managed Object; thread threw exception" << endl;
  }
}

ipc_managed_object::ipc_managed_object()
{
  this->ipc_id = 0;
  this->threadTask = nullptr;
  this->manager    = nullptr;
  this->thread_run = false;
}

ipc_managed_object::~ipc_managed_object()
{
  if (this->isRunning()) 
  {
    this->doStop();
  }
//   this->threadTask->~thread();
//   delete this->threadTask;
// 	this->threadTask = NULL;
	
  this->terminate();
}

ipc_manager *ipc_managed_object::getIpcManager()
{
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

int32_t ipc_managed_object::terminate() {
  return 0;
}