/*
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
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

#ifndef MTCA_UAADAPTER_H
#define MTCA_UAADAPTER_H

#include <vector>

#include <ua_mapped_class.h>

#include "ipc_managed_object.h"

#include <mtca_processvariable.h>
#include <mtca_processscalar.h>
#include <mtca_processarray.h>
#include <mtca_timestamp.h>

#include <ControlSystemPVManager.h>

using namespace mtca4u;

typedef struct variablesTypes {
    std::list<mtca_processscalar *> scalar;
    std::list<mtca_processarray *> array;
  } variableTypes;
  
  
class mtca_uaadapter : ua_mapped_class, public ipc_managed_object {
	private:
		UA_ServerConfig       	server_config;
		UA_ServerNetworkLayer 	server_nl;
		UA_Logger             	logger;
		UA_NodeId             	scalarsListId;
		UA_NodeId				arraysListId;
		UA_NodeId             	constantsListId;
		
		variablesTypes variables;
		std::list<mtca_processvariable *> constants;
				
		void mtca_uaadapter_constructserver(uint16_t opcuaPort);
		UA_StatusCode mapSelfToNamespace() ;
		
	public:
		mtca_uaadapter(uint16_t opcuaPort);
		~mtca_uaadapter();
		
		
		void addVariable(std::string name, std::string value, std::type_info const &type, mtca4u::TimeStamp timestamp);
		void addVariable(std::string name, std::vector<std::string> value, std::type_info const &type, mtca4u::TimeStamp timestamp);
		void addConstant(std::string name, std::string value, std::type_info const &type, mtca4u::TimeStamp timestamp);
		
		void workerThread();
};

#endif // MTCA_UAADAPTER_H
