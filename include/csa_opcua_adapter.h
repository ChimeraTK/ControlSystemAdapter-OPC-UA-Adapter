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

#ifndef CSA_OPCUA_ADAPTER_H
#define CSA_OPCUA_ADAPTER_H

#include <vector>
#include "ipc_managed_object.h"

#include "ua_adapter.h"
#include "ua_processvariable.h"

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ApplicationBase.h"

class csa_opcua_adapter {
private:
	ipc_manager    *mgr;
	ua_uaadapter *adapter;
	boost::shared_ptr<ControlSystemPVManager> csManager;
	
	void csa_opcua_adapter_InitServer(string configFile);
	void csa_opcua_adapter_InitVarMapping();
    
public:
	csa_opcua_adapter(boost::shared_ptr<ControlSystemPVManager> csManager, string configFile);
	~csa_opcua_adapter();
    
	boost::shared_ptr<ControlSystemPVManager> const & getControlSystemManager() const;
	ua_uaadapter* getUAAdapter();
	
	ipc_manager* getIPCManager();
	void start();
	void stop();
	void terminate();
	bool isRunning();
};

#endif // CSA_OPCUA_ADAPTER_H
