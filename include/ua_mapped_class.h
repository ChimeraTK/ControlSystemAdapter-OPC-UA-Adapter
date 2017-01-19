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

#ifndef UA_MAPPED_CLASS_H
#define UA_MAPPED_CLASS_H

extern "C" 
{
  #include "open62541.h"
}

#include "ua_proxies.h"

/** @class ua_mapped_class
 *	@brief This class mapped all inforamtion into the opca server
 *   
 *  @author Chris Iatrou
 *	@author Julian Rahm
 *  @date 22.11.2016
 * 
 */
class ua_mapped_class {
	
protected:
	
	nodePairList ownedNodes;
	UA_NodeId baseNodeId;
	UA_DateTime sourceTimeStamp;
	
public:
	
		UA_Client *mappedClient;
	UA_Server *mappedServer;
	
	/** @brief Constructor of the class with parameter
	* 
	* @param server opcua server
	* @param baseNodeId Node id from the parent node
	* 
	* @return Returns a element pointer if some was found, in other cases it will return NULL
	*/
	ua_mapped_class(UA_Server *server, UA_NodeId baseNodeId);
	
	/** @brief Constructor of the class with parameter
	*/
	ua_mapped_class();
	
	/** @brief Destructor of the class
	*/
	~ua_mapped_class();
	 
	/** @brief This methode mapped all own nodes into the opcua server
	* 
	* @return UA_StatusCode
	*/
	UA_StatusCode ua_mapSelfToNamespace();
	
	/** @brief This methode unmapped all nodes
	*
	* @return UA_StatusCode
	*/
	UA_StatusCode ua_unmapSelfFromNamespace();
	
	/** @brief This methode return a pointer of a xPath element depending of the given xPathString
	* 
 * @param map	Contains all Node from the class the should instantiated into the server 
 * @param scrClass Pointer to our class instance 
	* 
	* @return UA_StatusCode
	*/
	UA_StatusCode ua_mapDataSources(void* srcClass, UA_DataSource_Map* map);
	
	/** @brief Get the SourceTimeStamp from node in the OPC UA server
	 * Virtual methode which returned Timestamp is setted into the node with the help of the proxy_callback.h
	 * 
	 * @return Returns a UA_DateTime
	 */
	virtual UA_DateTime getSourceTimeStamp()=0;
	
};

#endif // UA_MAPPED_CLASS_H
