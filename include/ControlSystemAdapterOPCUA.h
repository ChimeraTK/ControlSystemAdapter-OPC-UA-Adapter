/*
 * Copyright (c) 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
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

#ifndef CONTROLSYSTEMADAPTER_OPCUA_H
#define CONTROLSYSTEMADAPTER_OPCUA_H

#include <vector>
#include "ipc_managed_object.h"

#include "mtca_uaadapter.h"
#include "mtca_processvariable.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ApplicationBase.h"

class ControlSystemAdapterOPCUA {
private:
	ipc_manager    *mgr;
	mtca_uaadapter *adapter;
	boost::shared_ptr<ControlSystemPVManager> csManager;
	
	void ControlSystemAdapterOPCUA_InitServer(string configFile);
	void ControlSystemAdapterOPCUA_InitVarMapping();
    
public:
	ControlSystemAdapterOPCUA(boost::shared_ptr<ControlSystemPVManager> csManager, string configFile);
	~ControlSystemAdapterOPCUA();
    
	boost::shared_ptr<ControlSystemPVManager> const & getControlSystemManager() const;
	mtca_uaadapter* getUAAdapter();
	
	ipc_manager* getIPCManager();
	void start();
	void stop();
	void terminate();
	bool isRunning();
};

#endif // CONTROLSYSTEMADAPTER_H
