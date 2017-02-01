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

#ifndef MTCA_PROCESSVARIABLE_H
#define MTCA_PROCESSVARIABLE_H

#include "ua_mapped_class.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"

using namespace std;
using namespace ChimeraTK;

/** @class ua_processvariable 
 *	@brief This class represent a processvariable of the controlsystemadapter in the information model of a OPC UA Server 
 *   
 *  @author Chris Iatrou, Julian Rahm
 *  @date 22.11.2016
 * 
 */
class ua_processvariable : ua_mapped_class {
private:
  string namePV;
	string nameNew;
	string engineeringUnit;
	string description;
	UA_NodeId ownNodeId;
	
	boost::shared_ptr<ControlSystemPVManager> csManager;
	
	
	/** @brief  This methode mapped all own nodes into the opcua server
	* 
	* @return <UA_StatusCode> 
	*/
	UA_StatusCode mapSelfToNamespace();
	
public:
	/** @brief Constructor from ua_processvaribale for generic creation
	* 
	* @param server A UA_Server type, with all server specific information from the used server
	* @param basenodeid Parent NodeId from OPC UA information model to add a new UA_ObjectNode
	* @param namePV Name of the process variable from control-system-adapter, is needed to fetch the rigth process varibale from PV-Manager
	* @param csManager Provide the hole PVManager from control-system-adapter to map all processvariable to the OPC UA-Model
	*/
	ua_processvariable(UA_Server *server, UA_NodeId basenodeid, string namePV, boost::shared_ptr<ControlSystemPVManager> csManager);
	
	/** @brief Destructor for ua_processvariable
	* 
	*/
	~ua_processvariable();
	
	/** @brief Reimplement the sourcetimestamp for every processvariable
	* 
	*/
	UA_DateTime getSourceTimeStamp();
	
	/** @brief Set name of processvariable
	* 
	* @param name Name of the processvariable
	*/
	void setName(string name);
	/** @brief  Get name of processvariable
	* 
	* @return <String> of the name of processvariable
	*/
	string getName();
	
	/** @brief  Set type of processvariable
	* 
	* @param type Define the type of the processvariable
	*/
	void setType(string type);
	/** @brief  Get type of processvariable
	* 
	* @return <String> of the type
	*/
	string getType();
	
	/** @brief  Set engineering unit of processvariable
	* 
	* @param type Define the engineering unit of the processvariable
	*/
	void setEngineeringUnit(string engineeringUnit);
	/** @brief  Get engineering unit of processvariable
	* 
	* @return <String> of engineering unit
	*/
	string getEngineeringUnit();
	
	/** @brief  Get description unit of processvariable
	* 
	* @return <String> of description
	*/
	void setDescription(string description);
	/** @brief  Get description unit of processvariable
	* 
	* @return <String> of description
	*/
	string getDescription();
	
	/** @brief  Get node id of this processvariable instance
	* 
	* @return <UA_NodeId> of this processvariable
	*/
	UA_NodeId getOwnNodeId();
    
	#define CREATE_READ_FUNCTION_ARRAY_DEF(_p_type)  std::vector<_p_type>  getValue_Array_##_p_type();    
	#define CREATE_WRITE_FUNCTION_ARRAY_DEF(_p_type) void setValue_Array_##_p_type(std::vector<_p_type> value);
	#define CREATE_READ_FUNCTION_DEF(_p_type)  _p_type  getValue_##_p_type();    
	#define CREATE_WRITE_FUNCTION_DEF(_p_type) void setValue_##_p_type(_p_type value);

    CREATE_WRITE_FUNCTION_DEF(int8_t)
    CREATE_WRITE_FUNCTION_DEF(uint8_t)
    CREATE_WRITE_FUNCTION_DEF(int16_t)
    CREATE_WRITE_FUNCTION_DEF(uint16_t)
    CREATE_WRITE_FUNCTION_DEF(int32_t)
    CREATE_WRITE_FUNCTION_DEF(uint32_t)
    CREATE_WRITE_FUNCTION_DEF(float)
    CREATE_WRITE_FUNCTION_DEF(double)
    
    CREATE_READ_FUNCTION_DEF(int8_t)
    CREATE_READ_FUNCTION_DEF(uint8_t)
    CREATE_READ_FUNCTION_DEF(int16_t)
    CREATE_READ_FUNCTION_DEF(uint16_t)
    CREATE_READ_FUNCTION_DEF(int32_t)
    CREATE_READ_FUNCTION_DEF(uint32_t)
    CREATE_READ_FUNCTION_DEF(float)
    CREATE_READ_FUNCTION_DEF(double)
    
    CREATE_WRITE_FUNCTION_ARRAY_DEF(int8_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(uint8_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(int16_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(uint16_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(int32_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(uint32_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(float)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(double)
    
    CREATE_READ_FUNCTION_ARRAY_DEF(int8_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(uint8_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(int16_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(uint16_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(int32_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(uint32_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(float)
    CREATE_READ_FUNCTION_ARRAY_DEF(double)
};

#endif // MTCA_PROCESSVARIABLE_H
