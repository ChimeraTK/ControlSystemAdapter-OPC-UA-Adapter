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
 * Copyright (c) 2019-2021 Andreas Ebner  <Andreas.Ebner@iosb.fraunhofer.de>
 */

#pragma once

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/SupportedUserTypes.h"
#include "ua_mapped_class.h"

#include <boost/fusion/container/map.hpp>

#include <string>
using namespace std;
namespace fusion = boost::fusion;

namespace ChimeraTK {

  typedef fusion::map<fusion::pair<int8_t, uint>, fusion::pair<uint8_t, uint>, fusion::pair<int16_t, uint>,
      fusion::pair<uint16_t, uint>, fusion::pair<int32_t, uint>, fusion::pair<uint32_t, uint>,
      fusion::pair<int64_t, uint>, fusion::pair<uint64_t, uint>, fusion::pair<float, uint>, fusion::pair<double, uint>,
      fusion::pair<string, uint>>
      TypesMap;

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

    TypesMap typesMap{fusion::make_pair<int8_t>(UA_TYPES_SBYTE), fusion::make_pair<uint8_t>(UA_TYPES_BYTE),
        fusion::make_pair<int16_t>(UA_TYPES_INT16), fusion::make_pair<uint16_t>(UA_TYPES_UINT16),
        fusion::make_pair<int32_t>(UA_TYPES_INT32), fusion::make_pair<uint32_t>(UA_TYPES_UINT32),
        fusion::make_pair<int64_t>(UA_TYPES_INT64), fusion::make_pair<uint64_t>(UA_TYPES_UINT64),
        fusion::make_pair<float>(UA_TYPES_FLOAT), fusion::make_pair<double>(UA_TYPES_DOUBLE),
        fusion::make_pair<string>(UA_TYPES_STRING)};

   private:
    bool array;
    boost::shared_ptr<ControlSystemPVManager> csManager;
    UA_StatusCode addPVChildNodes(UA_NodeId pvNodeId, const string& baseNodePath, UA_DataSource_Map& map);

    /** @brief  This methode mapped all own nodes into the opcua server
     *
     * @return <UA_StatusCode>
     */
    UA_StatusCode mapSelfToNamespace(const UA_Logger* logger);

   public:
    /** @brief Constructor from ua_processvaribale for generic creation
     *
     * @param server A UA_Server type, with all server specific information from the used server
     * @param basenodeid Parent NodeId from OPC UA information model to add a new UA_ObjectNode
     * @param namePV Name of the process variable from control-system-adapter, is needed to fetch the rigth process
     * varibale from PV-Manager
     * @param csManager Provide the hole PVManager from control-system-adapter to map all processvariable to the OPC
     * UA-Model
     */
    ua_processvariable(UA_Server* server, UA_NodeId basenodeid, const string& namePV,
        boost::shared_ptr<ControlSystemPVManager> csManager, const UA_Logger* logger, string overwriteNodeString = "");

    /** @brief Destructor for ua_processvariable
     *
     */
    ~ua_processvariable();

    /** @brief Reimplement the sourcetimestamp for every processvariable
     *
     */
    UA_DateTime getSourceTimeStamp();

    static UA_StatusCode ua_readproxy_ua_processvariable_getName(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range, UA_DataValue* value);

    /** @brief  Get name of processvariable
     *
     * @return <String> of the name of processvariable
     */
    string getName();

    static UA_StatusCode ua_readproxy_ua_processvariable_getType(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range, UA_DataValue* value);

    /** @brief  Get type of processvariable
     *
     * @return <String> of the type
     */
    string getType();

    static UA_StatusCode ua_readproxy_ua_processvariable_getEngineeringUnit(UA_Server* server,
        const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId, void* nodeContext,
        UA_Boolean includeSourceTimeStamp, const UA_NumericRange* range, UA_DataValue* value);

    static UA_StatusCode ua_writeproxy_ua_processvariable_setEngineeringUnit(UA_Server* server,
        const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId, void* nodeContext,
        const UA_NumericRange* range, const UA_DataValue* value);

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

    static UA_StatusCode ua_writeproxy_ua_processvariable_setDescription(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, const UA_NumericRange* range,
        const UA_DataValue* value);

    static UA_StatusCode ua_readproxy_ua_processvariable_getDescription(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range, UA_DataValue* value);

    /** @brief  Get vadility  of processvariable*/
    static UA_StatusCode ua_readproxy_ua_processvariable_getValidity(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range, UA_DataValue* value);

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

    template<typename T>
    static UA_StatusCode ua_readproxy_ua_processvariable_getValue(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range, UA_DataValue* value);

    template<typename T>
    UA_StatusCode getValue(UA_Variant* v, const UA_NumericRange* range);

    template<typename T>
    static UA_StatusCode ua_writeproxy_ua_processvariable_setValue(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, const UA_NumericRange* range,
        const UA_DataValue* value);

    template<typename T>
    UA_StatusCode setValue(const UA_Variant* data);

    template<typename T>
    UA_UInt32 typeSpecificSetup(UA_DataSource_Map_Element& mapElem, const UA_NodeId nodeId);
  };
} // namespace ChimeraTK
