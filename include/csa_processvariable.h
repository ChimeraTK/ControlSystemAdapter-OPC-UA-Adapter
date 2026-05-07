// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
// SPDX-FileCopyrightText: 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
// SPDX-FileCopyrightText: 2019-2021 Andreas Ebner <Andreas.Ebner@iosb.fraunhofer.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/SupportedUserTypes.h"
#include "ua_mapped_class.h"

#include <boost/fusion/container/map.hpp>

#include <string>
#include <vector>

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

    /** @brief  This method mapped all own nodes into the opcua server
     *
     * @return <UA_StatusCode>
     */
    UA_StatusCode mapSelfToNamespace(const UA_Logger* logger);

   public:
    /** @brief Constructor from ua_processvariable for generic creation
     *
     * @param server A UA_Server type, with all server specific information from the used server
     * @param basenodeid Parent NodeId from OPC UA information model to add a new UA_ObjectNode
     * @param namePV Name of the process variable from control-system-adapter, is needed to fetch the right process
     * variable from PV-Manager
     * @param csManager Provide the hole PVManager from control-system-adapter to map all processvariable to the OPC
     * UA-Model
     * @param logger Global logger used to send log messages
     * @param overwriteNodeString New node name
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
     * @return "string of the name of processvariable
     */
    string getName();

    static UA_StatusCode ua_readproxy_ua_processvariable_getType(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range, UA_DataValue* value);

    /** @brief  Get type of processvariable
     *
     * @return string of the type
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
     * @param engineeringUnit Define the engineering unit of the processvariable
     */
    void setEngineeringUnit(string engineeringUnit);
    /** @brief  Get engineering unit of processvariable
     *
     * @return string of engineering unit
     */
    string getEngineeringUnit();

    static UA_StatusCode ua_writeproxy_ua_processvariable_setDescription(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, const UA_NumericRange* range,
        const UA_DataValue* value);

    static UA_StatusCode ua_readproxy_ua_processvariable_getDescription(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range, UA_DataValue* value);

    /** @brief  Get validity  of processvariable*/
    static UA_StatusCode ua_readproxy_ua_processvariable_getValidity(UA_Server* server, const UA_NodeId* sessionId,
        void* sessionContext, const UA_NodeId* nodeId, void* nodeContext, UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range, UA_DataValue* value);

    /** @brief  Set description unit of processvariable
     */
    void setDescription(string description);
    /** @brief  Get description unit of processvariable
     *
     * @return string of description
     */
    string getDescription();

    /** @brief  Get node id of this processvariable instance
     *
     * @return UA_NodeId of this processvariable
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

  template<typename T>
  UA_StatusCode ua_processvariable::ua_readproxy_ua_processvariable_getValue(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      UA_Boolean includeSourceTimeStamp, const UA_NumericRange* range, UA_DataValue* value) {
    auto* thisObj = static_cast<ua_processvariable*>(nodeContext);
    UA_StatusCode rv = UA_STATUSCODE_GOOD;
    rv = thisObj->getValue<T>(&value->value, range);
    if(rv == UA_STATUSCODE_GOOD) {
      value->hasValue = true;
      if(includeSourceTimeStamp) {
        value->sourceTimestamp = thisObj->getSourceTimeStamp();
        value->hasSourceTimestamp = true;
      }
    }
    return rv;
  }

  template<typename T>
  UA_StatusCode ua_processvariable::ua_writeproxy_ua_processvariable_setValue(UA_Server* /*server*/,
      const UA_NodeId* /*sessionId*/, void* /*sessionContext*/, const UA_NodeId* /*nodeId*/, void* nodeContext,
      const UA_NumericRange* /*range*/, const UA_DataValue* value) {
    UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;
    auto* theClass = static_cast<ua_processvariable*>(nodeContext);
    retval = theClass->setValue<T>(&value->value);
    return retval;
  }

  template<typename T>
  UA_StatusCode ua_processvariable::getValue(UA_Variant* v, const UA_NumericRange* range) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<T>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0).size() == 1) {
      T ival = this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0).at(0);
      rv = UA_Variant_setScalarCopy(v, &ival, &UA_TYPES[fusion::at_key<T>(typesMap)]);
    }
    else {
      std::vector<T> iarr = this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0);
      v->type = &UA_TYPES[fusion::at_key<T>(typesMap)];
      if(range != nullptr) {
        // v->type = &UA_TYPES[fusion::at_key<T>(typesMap)];
        // rv = UA_Variant_setRangeCopy(v, iarr.data(), iarr.size(), *range);
        UA_Variant tmpVariant;
        UA_Variant_setArray(&tmpVariant, iarr.data(), iarr.size(), &UA_TYPES[fusion::at_key<T>(typesMap)]);
        rv = UA_Variant_copyRange(&tmpVariant, v, *range);
      }
      else {
        rv = UA_Variant_setArrayCopy(v, iarr.data(), iarr.size(), &UA_TYPES[fusion::at_key<T>(typesMap)]);
      }
    }
    return rv;
  }

  template<>
  inline UA_StatusCode ua_processvariable::getValue<string>(UA_Variant* v, const UA_NumericRange* range) {
    UA_StatusCode rv = UA_STATUSCODE_BADINTERNALERROR;
    if(this->csManager->getProcessVariable(this->namePV)->isReadable()) {
      this->csManager->getProcessArray<string>(this->namePV)->readLatest();
    }
    if(this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).size() == 1) {
      string sval = this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).at(0);
      UA_String ua_val = UA_String_fromChars((char*)sval.c_str());
      rv = UA_Variant_setScalarCopy(v, &ua_val, &UA_TYPES[UA_TYPES_STRING]);
      UA_String_clear(&ua_val);
    }
    else {
      std::vector<string> sarr = this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0);
      std::vector<UA_String> ua_sarr(sarr.size());
      for(size_t i = 0; i < sarr.size(); i++) {
        ua_sarr.at(i) = UA_String_fromChars((char*)sarr[i].c_str());
      }
      if(range != nullptr) {
        UA_Variant tmpVariant;
        UA_Variant_setArray(&tmpVariant, ua_sarr.data(), ua_sarr.size(), &UA_TYPES[UA_TYPES_STRING]);
        rv = UA_Variant_copyRange(&tmpVariant, v, *range);
      }
      else {
        rv = UA_Variant_setArrayCopy(v, ua_sarr.data(), ua_sarr.size(), &UA_TYPES[UA_TYPES_STRING]);
      }
      for(auto& i : ua_sarr) {
        UA_String_clear(&i);
      }
    }
    return rv;
  }

  template<typename T>
  UA_StatusCode ua_processvariable::setValue(const UA_Variant* data) {
    UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<T> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        T value = *((T*)data->data);
        valueArray.push_back(value);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        auto* v = (T*)data->data;
        valueArray.resize(data->arrayLength);
        for(size_t i = 0; i < valueArray.size(); i++) {
          valueArray.at(i) = v[i];
        }
        if(this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0).size() != data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<T>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }

    return retval;
  }

  template<>
  inline UA_StatusCode ua_processvariable::setValue<std::string>(const UA_Variant* data) {
    UA_StatusCode retval = UA_STATUSCODE_BADINTERNALERROR;

    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      vector<string> valueArray;
      if(UA_Variant_isScalar(data) && (!array)) {
        string cpps;
        UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)data->data)), cpps)
        // string value = *((string *)data->data);
        valueArray.push_back(cpps);
      }
      else if((!UA_Variant_isScalar(data)) && array) {
        // Array
        auto* vdata = (UA_String*)data->data;
        valueArray.resize(data->arrayLength);
        for(uint32_t i = 0; i < valueArray.size(); i++) {
          string cpps;
          UASTRING_TO_CPPSTRING(vdata[i], cpps)
          valueArray.at(i) = cpps;
        }
        if(this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0).size() != data->arrayLength) {
          return UA_STATUSCODE_BADINVALIDARGUMENT;
        }
      }
      this->csManager->getProcessArray<string>(this->namePV)->accessChannel(0) = valueArray;
      this->csManager->getProcessArray<string>(this->namePV)->write();
      retval = UA_STATUSCODE_GOOD;
    }
    else {
      retval = UA_STATUSCODE_BADNOTWRITABLE;
    }

    return retval;
  }

  template<typename T>
  UA_UInt32 ua_processvariable::typeSpecificSetup(UA_DataSource_Map_Element& mapElem, const UA_NodeId nodeId) {
    UA_Int32 arrayDims;
    mapElem.dataSource.read = ua_processvariable::ua_readproxy_ua_processvariable_getValue<T>;
    if(this->csManager->getProcessVariable(this->namePV)->isWriteable()) {
      mapElem.dataSource.write = ua_processvariable::ua_writeproxy_ua_processvariable_setValue<T>;
    }
    if(this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0).size() == 1) {
      this->array = false;
    }
    else {
      this->array = true;
      arrayDims = this->csManager->getProcessArray<T>(this->namePV)->accessChannel(0).size();
    }
    UA_Server_writeDataType(this->mappedServer, nodeId, UA_TYPES[fusion::at_key<T>(typesMap)].typeId);
    return arrayDims;
  }

} // namespace ChimeraTK
