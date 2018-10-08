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

#ifndef UA_PROCESSVARIABLE_H
#define UA_PROCESSVARIABLE_H

#include "ua_mapped_class.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include <string>

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
        string nodeStringIdOverwrite;
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
        ua_processvariable(UA_Server *server, UA_NodeId basenodeid, string namePV, boost::shared_ptr<ControlSystemPVManager> csManager, std::string overwriteNodeString = "");

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
    CREATE_WRITE_FUNCTION_DEF(string)

    CREATE_READ_FUNCTION_DEF(int8_t)
    CREATE_READ_FUNCTION_DEF(uint8_t)
    CREATE_READ_FUNCTION_DEF(int16_t)
    CREATE_READ_FUNCTION_DEF(uint16_t)
    CREATE_READ_FUNCTION_DEF(int32_t)
    CREATE_READ_FUNCTION_DEF(uint32_t)
    CREATE_READ_FUNCTION_DEF(float)
    CREATE_READ_FUNCTION_DEF(double)
    CREATE_READ_FUNCTION_DEF(string)

    CREATE_WRITE_FUNCTION_ARRAY_DEF(int8_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(uint8_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(int16_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(uint16_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(int32_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(uint32_t)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(float)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(double)
    CREATE_WRITE_FUNCTION_ARRAY_DEF(string)

    CREATE_READ_FUNCTION_ARRAY_DEF(int8_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(uint8_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(int16_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(uint16_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(int32_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(uint32_t)
    CREATE_READ_FUNCTION_ARRAY_DEF(float)
    CREATE_READ_FUNCTION_ARRAY_DEF(double)
    CREATE_READ_FUNCTION_ARRAY_DEF(string)
};

#endif // UA_PROCESSVARIABLE_H
