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

/** @class ipc_managed_object
 *	@brief This abstract class manages the given object in a seperate threat and controlls them.
 * Other classes can inherit from this class and get the ability to start and stop the thread in a easy way.
 *
 *  @author Chris Iatrou
 *	@author Julian Rahm
 *  @date 22.11.2016
 *
 */
class ipc_managed_object {
protected:
        uint32_t     ipc_id;
        bool         thread_run;
        ipc_manager *manager;
        std::thread *threadTask;
        std::mutex   mtx_threadOperations;

public:
        /**
         * @brief Constructor for ipc_managed_object
         *
         */
        ipc_managed_object();
        /**
         * @brief Constructor for ipc_managed_object
         *
         * @param ipc_id Unique number to identifie the object inner the manager
         *
         */
        ipc_managed_object(uint32_t ipc_id);

        /**
         * @brief Destructor to stop the running thread
         *
         */
        ~ipc_managed_object();

        /**
         * @brief Getter methode which rurn the setted pointer to the assigned manager.
         *
         * @return Return a pointer to the assigned manager
         */
        ipc_manager *getIpcManager();

        /**
         * @brief Getter mehtode which returns the setted ipc id
         *
         * @return Retuns the assigned id
         */
        uint32_t getIpcId();

        /**
         * @brief Set a new unique ipc id
         *
         * @param ipc_id It have to be a unique number for the manager
         */
        void setIpcId(uint32_t ipc_id);

        /**
         * @brief This methode checks if any manager is related to this object
         *
         * @return True or false depending if a manager is set or not.
         */
        bool isManaged();

        /**
         * @brief Assigne a specific ipc_manager to this ipc object.
         *
         * @param manager Pointer to the new assigning manager
         *
         * @return Return true if the new manager is assigned in a correct way, in other cases it return false.
         */
        bool assignManager(ipc_manager *manager);

        /**
         * @brief Join the thread.
         *
         * @return Return true if the join methode return true, in other cases it will return false.
         *
         */
        bool taskRunningAttached();

        /**
         * @brief Check if the thread is running
         *
         * @return Returns true if the thread ist working, in other cases it will return false.
         */
        bool isRunning();

        /**
         * @brief Stop the thread
         *
         * @return 0
         */
        uint32_t doStop();

        /**
         * @brief Starts the thread
         *
         * @return 0
         */
        uint32_t doStart();

        /**
         * @brief Virtual methode, every inherit class have to implement and specified the functionality of this methode to ensure the correct start procedure of the thread.
         *
         */
        virtual void workerThread() = 0;
};

/**
 * @brief
 *
 */
void ipc_managed_object_callWorker(ipc_managed_object *theClass);

#endif // IPC_MANAGED_OBJECT_H
