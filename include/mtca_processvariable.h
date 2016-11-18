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

class mtca_processvariable : ua_mapped_class {
private:
  string namePV;
	string nameNew;
	string engineeringUnit;
	string description;
	UA_NodeId ownNodeId;
	
	boost::shared_ptr<ChimeraTK::ControlSystemPVManager> csManager;
	UA_StatusCode mapSelfToNamespace();
	
public:
	mtca_processvariable(UA_Server *server, UA_NodeId basenodeid, std::string namePV, boost::shared_ptr<ChimeraTK::ControlSystemPVManager> csManager);
	mtca_processvariable(UA_Server *server, UA_NodeId basenodeid, string namePV, string nameNew, boost::shared_ptr<ChimeraTK::ControlSystemPVManager> csManager);
	
	~mtca_processvariable();
	
	void setTimeStampSeconds(uint32_t seconds);
	uint32_t getTimeStampSeconds();
	
	void setTimeStampNanoSeconds(uint32_t nanoSeconds);
	uint32_t getTimeStampNanoSeconds();
	
	void setName(string name);
	string getName();
	
	void setType(string type);
	string getType();
	
	void setEngineeringUnit(string engineeringUnit);
	string getEngineeringUnit();
	
	void setDescription(string description);
	string getDescription();
	
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
