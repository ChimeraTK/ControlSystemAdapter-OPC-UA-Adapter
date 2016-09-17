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

#ifndef IPC_MANAGED_OBJECT_H
#define IPC_MANAGED_OBJECT_H
#include <thread>
#include <mutex>
#include <iostream>
#include "stdint.h"

using namespace std;

// Early definition of manager class
class ipc_manager;

typedef enum {
  IPC_MANAGED_OBJECT_TYPE_GENERICOBJECT      = 0x01,
  IPC_MANAGED_OBJECT_TYPE_GENERICTASK        = 0x02,
  IPC_MANAGED_OBJECT_TYPE_AGGREGATINGSERVER  = 0x04,
  IPC_MANAGED_OBJECT_TYPE_AGGREGATEDSYSTEM   = 0x08,
  IPC_MANAGED_OBJECT_TYPE_SIMULATOR          = 0x10,
  IPC_MANAGED_OBJECT_TYPE_BENCHMARKSIMULATOR = 0x20,
  IPC_MANAGED_OBJECT_TYPE_UDPBEACON          = 0x40
} ipc_managed_object_type;

class ipc_managed_object
{
protected:
  uint32_t     ipc_id;
  bool         thread_run;
  ipc_manager *manager;
  std::thread *threadTask;
  std::mutex   mtx_threadOperations;
  
public:
  ipc_managed_object();
  ipc_managed_object(uint32_t ipc_id);
  virtual ~ipc_managed_object();
  
  ipc_manager *getIpcManager();
  uint32_t getIpcId();
	void setIpcId(uint32_t ipc_id);
  bool isManaged();
  bool assignManager(ipc_manager *manager);
  //virtual ipc_managed_object_type getManagedObjectType();
  
  bool taskRunningAttached();
  bool isRunning();
  uint32_t doStop();
  uint32_t doStart();
  int32_t terminate();
  
  virtual void workerThread() = 0;
};

void ipc_managed_object_callWorker(ipc_managed_object *theClass);

#endif // IPC_MANAGED_OBJECT_H
