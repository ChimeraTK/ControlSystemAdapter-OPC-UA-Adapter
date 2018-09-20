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

#include "csa_config.h"

#include "ipc_manager.h"
#include "ipc_managed_object.h"

extern "C" {
#ifdef WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif
}

ipc_manager::ipc_manager() {
  this->nxtId = 0;
}

ipc_manager::~ipc_manager() {
	this->stopAll();
	this->doStop();
	//for(auto ptr : this->objects) delete ptr;
}

static void call_periodiclyKickWorkerThread(ipc_manager *mgr) {mgr->periodiclyKickWorkerThread();}
void ipc_manager::periodiclyKickWorkerThread()
{
  //timespec t_periodic = (const struct timespec) {.tv_sec=0, .tv_nsec=IPC_MANAGER_PERIODIC_IDLEUPATEINTERVAL*1000000};
  
  while(this->thread_run) {
    this->notifier.notify_all();
    sleep(1);
  }
}

void ipc_manager::workerThread()
{
  std::unique_lock<std::mutex> lock(this->mtx_threadOperations);
  std::thread kicker(call_periodiclyKickWorkerThread, this);
  while(this->thread_run) 
  {
    this->notifier.wait(lock);
    
    // Check Tasks
  }
  if (kicker.joinable())
    kicker.join();
  return;
}

uint32_t ipc_manager::addObject(ipc_managed_object *object) 
{
  std::unique_lock<std::mutex> lock(this->mtx_threadOperations);
  
  if (object==nullptr) return 0; //Used to kick the worker thread for shutdown;

	object->setIpcId(this->getUniqueIpcId());
  object->assignManager(this);
  this->objects.push_back(object);
  object->doStart();
  
  this->notifier.notify_all();
  
  return object->getIpcId();
}

uint32_t ipc_manager::deleteObject(uint32_t rpc_id) {
  std::list<ipc_managed_object *> deleteThese;
  for(list<ipc_managed_object*>::iterator j = this->objects.begin(); j != this->objects.end(); ++j) {
    ipc_managed_object *obj = *j;
		
    if (obj->getIpcId() == rpc_id) {
      deleteThese.push_back(obj);
    }
  }
  
  list<ipc_managed_object*>::iterator j ;
  j = deleteThese.begin();
  while (j != deleteThese.end()) {
    ipc_managed_object *obj = *j;
    obj->doStop();
    this->objects.remove(obj);
    deleteThese.remove(obj);
    j = deleteThese.begin();
  }
  return 0;
}

uint32_t ipc_manager::getUniqueIpcId() 
{
  return ++nxtId;
}

void ipc_manager::startAll() {
	for (auto j : this->objects) {
		j->doStart();
	}
	this->doStart();
	return;
}

void ipc_manager::stopAll() {
  for(list<ipc_managed_object*>::iterator j = this->objects.begin(); j != this->objects.end(); ++j) {
    ipc_managed_object *obj = *j;
    (*obj).doStop();
		//delete obj;
		//obj = NULL;
  }
  
  // Stop our own thread
  this->thread_run = false;
  sleep(1);
  this->notifier.notify_all();
  sleep(1);
  this->notifier.notify_all();
  this->doStop();
  
  return;
}
