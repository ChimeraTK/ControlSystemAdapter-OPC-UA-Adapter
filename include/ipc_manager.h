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

#ifndef HAVE_IPC_MANAGER_H
#define HAVE_IPC_MANAGER_H

#include "stdint.h"
#include <list>
#include <condition_variable>
#include <ipc_managed_object.h>

class ipc_managed_object;

using namespace std;

/** @class ipc_manager
 *	@brief This class managed object who are assigned to this class. So you can start and stop all of these.
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

        /**
         * @brief Constructor for ipc-ipc_managed_object
         *
         */
        void workerThread();

public:
        /**
         * @brief Constructor for ipc_manager
         *
         */
        ipc_manager();

        /**
         * @brief Destructor for ipc_manager
         *
         */
        ~ipc_manager();

        /**
         * @brief Add a managed object to be managed
         *
         * @param object Pointer to a class who inherit from managed_object class
         *
         * @return Returns the unique ipc id of the new object
         */
        uint32_t addObject(ipc_managed_object *object) ;

        /**
         * @brief Delete a managed object with the given ipc id from manager
         *
         * @param rcp_id Unique number of the deleted object.
         *
         * @return Returns the id of the deleted object.
         */
        uint32_t deleteObject(uint32_t rpc_id);

        /**
         * @brief Getter to return the unique id
         *
         * @return Return the id of the manager
         */
        uint32_t getUniqueIpcId() ;

        /**
         * @brief Getter to get a object by a given id.
         *
         * @param id The number of the desired object.
         *
         * @return Pointer to a ipc_managed_object
         */
        ipc_managed_object* getObjectById(uint32_t id);

        /**
         * @brief Start all thread of assigned objects.
         *
         */
        void startAll();

        /**
         * @brief Stop all thread of assigned objects.
         *
         */
        void stopAll();

        /**
         * @brief Cycle worker, while thread is runing an notification is generated.
         *
         */
        void periodiclyKickWorkerThread();
};

#endif // HAVE_IPC_MANAGER_H
