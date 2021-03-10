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

typedef enum {
    UA_PV_UNKNOWN = 0,
    UA_PV_INT8 = 1,
    UA_PV_UINT8 = 2,
    UA_PV_INT16 = 3,
    UA_PV_UINT16 = 4,
    UA_PV_INT32 = 5,
    UA_PV_UINT32 = 6,
    UA_PV_FLOAT = 8,
    UA_PV_DOUBLE = 9,
    UA_PV_STRING = 10,
    UA_PV_INT64 = 11,
    UA_PV_UINT64 = 12,
    UA_PV_BOOL = 13
} UA_Processvariable_Type;

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
        UA_NodeId ownNodeId = UA_NODEID_NULL;
        UA_Processvariable_Type type;
        bool array;

        boost::shared_ptr<ControlSystemPVManager> csManager;
        UA_StatusCode addPVChildNodes(UA_NodeId pvNodeId, string baseNodePath);

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

        static UA_StatusCode ua_readproxy_ua_processvariable_getName (void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value);

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

        static UA_StatusCode ua_readproxy_ua_processvariable_getType (void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value);

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

        static UA_StatusCode ua_readproxy_ua_processvariable_getEngineeringUnit (void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value);
        static UA_StatusCode ua_writeproxy_ua_processvariable_setEngineeringUnit (void *handle, const UA_NodeId nodeid,const UA_Variant *data, const UA_NumericRange *range);

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

        static UA_StatusCode ua_writeproxy_ua_processvariable_setDescription (void *handle, const UA_NodeId nodeid,const UA_Variant *data, const UA_NumericRange *range);
        static UA_StatusCode ua_readproxy_ua_processvariable_getDescription (void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value);

        /** @brief  Get vadility  of processvariable
        */
        static UA_StatusCode ua_readproxy_ua_processvariable_getValidity (void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value);


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

    static UA_StatusCode ua_readproxy_ua_processvariable_getValue(void *handle,
            const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp,
            const UA_NumericRange *range, UA_DataValue *value);
    UA_StatusCode getValue_int8(UA_Variant* v);
    UA_StatusCode getValue_uint8(UA_Variant* v);
    UA_StatusCode getValue_int16(UA_Variant* v);
    UA_StatusCode getValue_uint16(UA_Variant* v);
    UA_StatusCode getValue_int32(UA_Variant* v);
    UA_StatusCode getValue_uint32(UA_Variant* v);
    UA_StatusCode getValue_int64(UA_Variant* v);
    UA_StatusCode getValue_uint64(UA_Variant* v);
    UA_StatusCode getValue_float(UA_Variant* v);
    UA_StatusCode getValue_double(UA_Variant* v);
    UA_StatusCode getValue_string(UA_Variant* v);
    UA_StatusCode getValue_bool(UA_Variant* v);

    static UA_StatusCode ua_writeproxy_ua_processvariable_setValue(void *handle, const UA_NodeId nodeid,const UA_Variant *data, const UA_NumericRange *range);
    UA_StatusCode setValue_int8(const UA_Variant* data);
    UA_StatusCode setValue_uint8(const UA_Variant* data);
    UA_StatusCode setValue_int16(const UA_Variant* data);
    UA_StatusCode setValue_uint16(const UA_Variant* data);
    UA_StatusCode setValue_int32(const UA_Variant* data);
    UA_StatusCode setValue_uint32(const UA_Variant* data);
    UA_StatusCode setValue_int64(const UA_Variant* data);
    UA_StatusCode setValue_uint64(const UA_Variant* data);
    UA_StatusCode setValue_float(const UA_Variant* data);
    UA_StatusCode setValue_double(const UA_Variant* data);
    UA_StatusCode setValue_string(const UA_Variant* data);
    UA_StatusCode setValue_bool(const UA_Variant* data);
};

#endif // UA_PROCESSVARIABLE_H
