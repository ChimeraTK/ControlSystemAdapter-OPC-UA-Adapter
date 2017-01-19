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

#ifndef HAVE_IPC_MANAGER_H
#define HAVE_IPC_MANAGER_H

#include "stdint.h"
#include <list>
#include <condition_variable>
#include <ipc_managed_object.h>

class ipc_managed_object;

using namespace std;

/** @class ipc_manager
 *	@brief This class represent a additional variable from <variableMap.xml> in the information model of a OPC UA Server 
 *   
 *  @author Chris Iatrou, Julian Rahm
 *  @date 22.11.2016
 * 
 */
class ipc_manager : public ipc_managed_object {
private:
  list<ipc_managed_object*> objects;
  uint32_t nxtId;
  std::condition_variable   notifier;

  void workerThread();
public:
  ipc_manager();
  ~ipc_manager();
  
  uint32_t addObject(ipc_managed_object *object) ;
  uint32_t deleteObject(uint32_t rpc_id);
  
  bool hasTaskCompleted(uint32_t ipc_id);
  uint32_t getUniqueIpcId() ;
  
  ipc_managed_object* getObjectById(uint32_t id);
  
  void startAll();
  void stopAll();
  
  void periodiclyKickWorkerThread();
};

#endif // HAVE_IPC_MANAGER_H
