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