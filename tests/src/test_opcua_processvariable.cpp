#include "open62541/plugin/log_stdout.h"

#include <boost/fusion/container/map.hpp>
#include <boost/test/included/unit_test.hpp>

#include <csa_opcua_adapter.h>
#include <string.h>
#include <test_sample_data.h>
#include <ua_adapter.h>

extern "C" {
#include "csa_namespace.h"
#include "unistd.h"
}

using namespace boost::unit_test_framework;
using namespace std;
namespace fusion = boost::fusion;

typedef fusion::map<fusion::pair<uint8_t, UA_DataType>, fusion::pair<int8_t, UA_DataType>,
    fusion::pair<uint16_t, UA_DataType>, fusion::pair<int16_t, UA_DataType>, fusion::pair<uint32_t, UA_DataType>,
    fusion::pair<int32_t, UA_DataType>, fusion::pair<uint64_t, UA_DataType>, fusion::pair<int64_t, UA_DataType>,
    fusion::pair<float, UA_DataType>, fusion::pair<double, UA_DataType>>
    TestTypesMap;

static TestTypesMap tMap{fusion::make_pair<uint8_t>(UA_TYPES[UA_TYPES_SBYTE]),
    fusion::make_pair<int8_t>(UA_TYPES[UA_TYPES_BYTE]), fusion::make_pair<uint16_t>(UA_TYPES[UA_TYPES_UINT16]),
    fusion::make_pair<int16_t>(UA_TYPES[UA_TYPES_INT16]), fusion::make_pair<uint32_t>(UA_TYPES[UA_TYPES_UINT32]),
    fusion::make_pair<int32_t>(UA_TYPES[UA_TYPES_INT32]), fusion::make_pair<uint64_t>(UA_TYPES[UA_TYPES_UINT64]),
    fusion::make_pair<int64_t>(UA_TYPES[UA_TYPES_INT64]), fusion::make_pair<float>(UA_TYPES[UA_TYPES_FLOAT]),
    fusion::make_pair<double>(UA_TYPES[UA_TYPES_DOUBLE])};

/*
 * ProcessVariableTest
 *
 */
class ProcessVariableTest {
 public:
  static void testClassSide();

  static void testClientSide();

  template<typename CTK_TYPE>
  static void testData(
      ProcessVariable::SharedPtr oneProcessVariable, TestFixturePVSet* pvSet, ua_processvariable* test);
};

template<typename CTK_TYPE>
void ProcessVariableTest::testData(
    ProcessVariable::SharedPtr oneProcessVariable, TestFixturePVSet* pvSet, ua_processvariable* test) {
  vector<CTK_TYPE> valueArray =
      pvSet->csManager->getProcessArray<CTK_TYPE>(oneProcessVariable->getName())->accessChannel(0);
  UA_Variant* var = UA_Variant_new();
  test->getValue<CTK_TYPE>(var, nullptr);
  if(valueArray.size() == 1) {
    CTK_TYPE newValue;
    BOOST_CHECK(*(CTK_TYPE*)(var->data) == 0);
    newValue = 100;
    UA_Variant_clear(var);
    UA_Variant_setScalarCopy(var, &newValue, &fusion::at_key<CTK_TYPE>(tMap));
    test->setValue<CTK_TYPE>(var);

    auto time = oneProcessVariable->getVersionNumber().getTime();
    auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count();
    BOOST_CHECK(test->getSourceTimeStamp() == usecs * UA_DATETIME_USEC + UA_DATETIME_UNIX_EPOCH);

    // Check value on controlsystemmanager side
    vector<CTK_TYPE> csValueArray =
        pvSet->csManager->getProcessArray<CTK_TYPE>(oneProcessVariable->getName())->accessChannel(0);
    BOOST_CHECK(csValueArray.size() == 1);
    BOOST_CHECK(csValueArray.at(0) == 100);
  }
  else {
    // if Array
    std::vector<CTK_TYPE> newVector(var->arrayLength);
    for(size_t i = 0; i < var->arrayLength; i++) {
      BOOST_CHECK(((CTK_TYPE*)(var->data))[i] == 0);
      newVector.at(i) = (100 - i);
    }
    UA_Variant_clear(var);
    UA_Variant_setArrayCopy(var, newVector.data(), newVector.size(), &fusion::at_key<CTK_TYPE>(tMap));
    test->setValue<CTK_TYPE>(var);

    // Check value on controlsystemmanager side
    vector<CTK_TYPE> valueArray =
        pvSet->csManager->getProcessArray<CTK_TYPE>(oneProcessVariable->getName())->accessChannel(0);
    size_t i = 0;
    for(auto value : valueArray) {
      BOOST_CHECK(value == (CTK_TYPE)(100 - i));
      i++;
    }

    // now a certain range
    UA_Variant_clear(var);
    UA_NumericRange range = UA_NUMERICRANGE("0:5");
    //    UA_NumericRange_parse(&range, UA_String_fromChars("0:5"));
    test->getValue<CTK_TYPE>(var, &range);
    BOOST_CHECK(var->arrayLength == 6);
    for(size_t i = 0; i < var->arrayLength; i++) {
      BOOST_CHECK(((CTK_TYPE*)(var->data))[i] == (CTK_TYPE)(100 - i));
    }
    UA_free(range.dimensions);
  }
  UA_Variant_delete(var);
}

void ProcessVariableTest::testClassSide() {
  std::cout << "Enter ProcessVariableTest with ClassSide" << std::endl;
  TestFixtureServerSet serverSet;
  TestFixturePVSet pvSet;

  thread* serverThread = new std::thread(UA_Server_run, serverSet.mappedServer, &serverSet.runUAServer);

  // check server
  if(serverSet.mappedServer == nullptr) {
    BOOST_CHECK(false);
  }

  // ua_processvariable *test;
  for(ProcessVariable::SharedPtr oneProcessVariable : pvSet.csManager->getAllProcessVariables()) {
    ua_processvariable test(
        serverSet.mappedServer, serverSet.baseNodeId, oneProcessVariable->getName(), pvSet.csManager, UA_Log_Stdout);

    BOOST_CHECK(test.getName() == oneProcessVariable->getName());

    BOOST_CHECK(test.getEngineeringUnit() == oneProcessVariable->getUnit());
    test.setEngineeringUnit("test");
    BOOST_CHECK(test.getEngineeringUnit() == "test");

    // Description
    string description = "";
    description = test.getDescription();
    BOOST_CHECK(description == oneProcessVariable->getDescription());

    //    auto time = oneProcessVariable->getVersionNumber().getTime();
    //    auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count();
    //    BOOST_CHECK(test.getSourceTimeStamp() == usecs * UA_DATETIME_USEC + UA_DATETIME_UNIX_EPOCH);
    std::string valueType = test.getType();

    cout << "Check Processvariable: " << test.getName() << endl;
    if(valueType == "int8_t") {
      BOOST_CHECK(valueType == "int8_t");
      testData<int8_t>(oneProcessVariable, &pvSet, &test);
    }
    else if(valueType == "uint8_t") {
      BOOST_CHECK(valueType == "uint8_t");
      testData<uint8_t>(oneProcessVariable, &pvSet, &test);
    }
    else if(valueType == "int16_t") {
      BOOST_CHECK(valueType == "int16_t");
      testData<int16_t>(oneProcessVariable, &pvSet, &test);
    }
    else if(valueType == "uint16_t") {
      BOOST_CHECK(valueType == "uint16_t");
      testData<uint16_t>(oneProcessVariable, &pvSet, &test);
    }
    else if(valueType == "int32_t") {
      BOOST_CHECK(valueType == "int32_t");
      testData<int32_t>(oneProcessVariable, &pvSet, &test);
    }
    else if(valueType == "uint32_t") {
      BOOST_CHECK(valueType == "uint32_t");
      testData<uint32_t>(oneProcessVariable, &pvSet, &test);
    }
    else if(valueType == "int64_t") {
      BOOST_CHECK(valueType == "int64_t");
      testData<int64_t>(oneProcessVariable, &pvSet, &test);
    }
    else if(valueType == "uint64_t") {
      BOOST_CHECK(valueType == "uint64_t");
      testData<uint64_t>(oneProcessVariable, &pvSet, &test);
    }
    else if(valueType == "float") {
      BOOST_CHECK(valueType == "float");
      testData<float>(oneProcessVariable, &pvSet, &test);
    }
    else if(valueType == "double") {
      BOOST_CHECK(valueType == "double");
      testData<double>(oneProcessVariable, &pvSet, &test);
    }
    else if(valueType == "string") {
    }
    else
      BOOST_CHECK(false);

    UA_NodeId nodeId = test.getOwnNodeId();
    BOOST_CHECK(!UA_NodeId_isNull(&nodeId));
    UA_NodeId_clear(&nodeId);

    string newName = "";
    newName = oneProcessVariable->getName();
    // Should not being changed
    BOOST_CHECK(test.getName() != "");
  }

  serverSet.runUAServer = UA_FALSE;

  if(serverThread->joinable()) {
    serverThread->join();
  }

  delete serverThread;
  serverThread = NULL;
};

void ProcessVariableTest::testClientSide() {
  std::cout << "Enter ProcessVariableTest with ExampleSet and ClientSide testing" << std::endl;

  TestFixtureServerSet serverSet;
  TestFixturePVSet pvSet;
  thread* serverThread = new std::thread(UA_Server_run, serverSet.mappedServer, &serverSet.runUAServer);

  // check server
  if(serverSet.mappedServer == nullptr) {
    BOOST_CHECK(false);
  }

  // add set
  vector<ua_processvariable*> varList;
  for(ProcessVariable::SharedPtr oneProcessVariable : pvSet.csManager->getAllProcessVariables()) {
    varList.push_back(new ua_processvariable(
        serverSet.mappedServer, serverSet.baseNodeId, oneProcessVariable->getName(), pvSet.csManager, UA_Log_Stdout));
  }

  // Create client to connect to server
  UA_Client* client = UA_Client_new();
  UA_ClientConfig* cc = UA_Client_getConfig(client);
  UA_ClientConfig_setDefault(cc);
  string endpointURL = "opc.tcp://localhost:" + to_string(serverSet.opcuaPort);
  UA_StatusCode retval = UA_Client_connect(client, endpointURL.c_str());
  sleep(1);

  int k = 1;
  while(retval != UA_STATUSCODE_GOOD & k < 10) {
    retval = UA_Client_connect(client, endpointURL.c_str());
    sleep(1);
    k++;
  }

  if(retval != UA_STATUSCODE_GOOD) {
    std::cout << "Failed to connect to server"
              << "opc.tcp://localhost:" << serverSet.opcuaPort << std::endl;
    BOOST_CHECK(false);
  }

  UA_BrowseRequest bReq;
  UA_BrowseRequest_init(&bReq);
  bReq.requestedMaxReferencesPerNode = 0;
  bReq.nodesToBrowse = UA_BrowseDescription_new();
  bReq.nodesToBrowseSize = 1;
  bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; // return everything
  UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);

  for(size_t i = 0; i < bResp.resultsSize; ++i) {
    for(size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
      UA_ReferenceDescription* ref = &(bResp.results[i].references[j]);
      if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
        size_t lenBrowseName = (int)ref->browseName.name.length;
        std::string browseNameFound(reinterpret_cast<char const*>(ref->browseName.name.data), lenBrowseName);

        UA_BrowseRequest bReq2;
        UA_BrowseRequest_init(&bReq2);
        bReq2.requestedMaxReferencesPerNode = 0;
        bReq2.nodesToBrowse = UA_BrowseDescription_new();
        bReq2.nodesToBrowseSize = 1;
        bReq2.nodesToBrowse[0].nodeId = ref->nodeId.nodeId;
        bReq2.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;
        UA_BrowseResponse bResp2 = UA_Client_Service_browse(client, bReq2);

        UA_NodeId valueNodeId = UA_NODEID_NULL;
        UA_NodeId typeNodeId = UA_NODEID_NULL;
        UA_NodeId nameNodeId = UA_NODEID_NULL;
        UA_NodeId engineeringUnitNodeId = UA_NODEID_NULL;
        UA_NodeId descriptionNodeId = UA_NODEID_NULL;
        string name = "";

        UA_ReferenceDescription* refe;
        for(size_t m = 0; m < bResp2.resultsSize; ++m) {
          for(size_t k = 0; k < bResp2.results[m].referencesSize; ++k) {
            refe = &(bResp2.results[m].references[k]);
            if(refe->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC ||
                refe->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
              size_t lenBrowseName2 = (int)refe->browseName.name.length;
              string browseNameFound2(reinterpret_cast<char const*>(refe->browseName.name.data), lenBrowseName2);

              if(browseNameFound2 == "Type") {
                UA_NodeId_copy(&refe->nodeId.nodeId, &typeNodeId);
              }

              if(browseNameFound2 == "Name") {
                UA_NodeId_copy(&refe->nodeId.nodeId, &nameNodeId);
              }

              if(browseNameFound2 == "Value") {
                UA_NodeId_copy(&refe->nodeId.nodeId, &valueNodeId);
                name = browseNameFound;
                cout << "Checking ProcessVariable: " << name << endl;
              }
              if(browseNameFound2 == "EngineeringUnit") {
                UA_NodeId_copy(&refe->nodeId.nodeId, &engineeringUnitNodeId);
              }

              if(browseNameFound2 == "Description") {
                UA_NodeId_copy(&refe->nodeId.nodeId, &descriptionNodeId);
              }
            }
          }
        }
        UA_BrowseRequest_clear(&bReq2);
        UA_BrowseResponse_clear(&bResp2);

        if(!UA_NodeId_isNull(&valueNodeId)) {
          UA_Variant* valueToCheck = UA_Variant_new();
          UA_Variant_init(valueToCheck);
          UA_StatusCode retvalValue = UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
          if(retvalValue != UA_STATUSCODE_GOOD) {
            BOOST_CHECK(false);
          }

          UA_Variant* datatypeToCheck = UA_Variant_new();
          UA_Variant_init(datatypeToCheck);
          UA_StatusCode retvalValueDatatype = UA_Client_readValueAttribute(client, typeNodeId, datatypeToCheck);
          if(retvalValueDatatype != UA_STATUSCODE_GOOD) {
            BOOST_CHECK(false);
          }

          UA_Variant* nameToCheck = UA_Variant_new();
          UA_Variant_init(nameToCheck);
          UA_StatusCode retvalValueName = UA_Client_readValueAttribute(client, nameNodeId, nameToCheck);
          if(retvalValueName != UA_STATUSCODE_GOOD) {
            BOOST_CHECK(false);
          }

          UA_Variant* euToCheck = UA_Variant_new();
          UA_Variant_init(euToCheck);
          UA_StatusCode retvalValueEu = UA_Client_readValueAttribute(client, engineeringUnitNodeId, euToCheck);
          if(retvalValueEu != UA_STATUSCODE_GOOD) {
            BOOST_CHECK(false);
          }

          UA_Variant* descToCheck = UA_Variant_new();
          UA_Variant_init(descToCheck);
          UA_StatusCode retvalValueDesc = UA_Client_readValueAttribute(client, descriptionNodeId, descToCheck);
          if(retvalValueDesc != UA_STATUSCODE_GOOD) {
            BOOST_CHECK(false);
          }

          UA_NodeId datatypeId;
          if(retvalValue == UA_STATUSCODE_GOOD) {
            UA_StatusCode retvalDatatype = UA_Client_readDataTypeAttribute(client, valueNodeId, &datatypeId);
            if(retvalDatatype != UA_STATUSCODE_GOOD) {
              BOOST_CHECK(false);
            }
          }

          if(retvalValue == UA_STATUSCODE_GOOD) {
            string datatype = "";
            string valName = "";
            string description = "";
            string engineeringUnit = "";

            UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);

            // Check EngineeringUnit -> for all the same
            UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)euToCheck->data)), engineeringUnit);
            // cout << "EngineeringUnit: " << valName << endl;
            if((valName.compare("/int8Scalar") == 0) || (valName.compare("/Mein/Name_ist#int8Array_s15") == 0) ||
                (valName.compare("/Unser/Name/ist_uint8Array_s10") == 0)) {
              BOOST_CHECK(engineeringUnit == "Einheit");
            }
            else {
              BOOST_CHECK(engineeringUnit == "n./a.");
            }

            // Write new engineering unit
            string merker = "mHensel/Iatrou";
            UA_String newEU = UA_String_fromChars((char*)merker.c_str());
            UA_Variant_init(euToCheck);
            UA_Variant_setScalarCopy(euToCheck, &newEU, &UA_TYPES[UA_TYPES_STRING]);
            UA_StatusCode retvalNewEU = UA_Client_writeValueAttribute(client, engineeringUnitNodeId, euToCheck);
            if(retvalNewEU != UA_STATUSCODE_GOOD) {
              BOOST_CHECK(false);
            }
            UA_String_clear(&newEU);
            UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)euToCheck->data)), engineeringUnit);
            BOOST_CHECK(engineeringUnit == "mHensel/Iatrou");

            // Check Description -> for all the same
            UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)descToCheck->data)), description);
            // cout << "Description: " << valName << endl;
            if((valName.compare("/int8Scalar") == 0) || (valName.compare("/Mein/Name_ist#int8Array_s15") == 0) ||
                (valName.compare("/Unser/Name/ist_uint8Array_s10") == 0)) {
              BOOST_CHECK(description == "Beschreibung der Variable");
            }
            else {
              BOOST_CHECK(description == "");
            }
            // Write new engineering unit
            merker = "Beschreibung";
            UA_String newDesc = UA_String_fromChars((char*)merker.c_str());
            UA_Variant_init(descToCheck);
            UA_Variant_setScalarCopy(descToCheck, &newDesc, &UA_TYPES[UA_TYPES_STRING]);
            UA_StatusCode retvalNewDesc = UA_Client_writeValueAttribute(client, descriptionNodeId, descToCheck);
            if(retvalNewDesc != UA_STATUSCODE_GOOD) {
              BOOST_CHECK(false);
            }
            UA_String_clear(&newDesc);
            UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)descToCheck->data)), valName);
            BOOST_CHECK(valName == "Beschreibung");

            // i know, this part is perfecly for makro stuff... but common, for maintainability reason we should use simple code...
            if(valueToCheck->arrayLength < 1) {
              switch(datatypeId.identifier.numeric - 1) {
                case UA_TYPES_SBYTE: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "int8_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/int8Scalar");
                  // Check Value
                  uint8_t value = (uint8_t)*((uint8_t*)valueToCheck->data);
                  BOOST_CHECK(value == 0);
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  uint8_t newValue = 123;
                  UA_Variant_init(valueToCheck);
                  UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_SBYTE]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (uint8_t)*((uint8_t*)valueToCheck->data);
                    BOOST_CHECK(value == newValue);
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_BYTE: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "uint8_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/uint8Scalar");
                  // Check Value
                  int8_t value = (int8_t)*((int8_t*)valueToCheck->data);
                  BOOST_CHECK(value == 0);
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  int8_t newValue = 122;
                  UA_Variant_init(valueToCheck);
                  UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_BYTE]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (int8_t)*((int8_t*)valueToCheck->data);
                    BOOST_CHECK(value == newValue);
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_INT16: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "int16_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/int16Scalar");
                  // Check Value
                  int16_t value = (int16_t)*((int16_t*)valueToCheck->data);
                  BOOST_CHECK(value == 0);
                  // cout << "Wert: " << std::to_string(value) << endl;
                  // set new value, should be wrong
                  int16_t newValue = 123;
                  UA_Variant_init(valueToCheck);
                  UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_INT16]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (int16_t)*((int16_t*)valueToCheck->data);
                    BOOST_CHECK(value == newValue);
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_UINT16: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "uint16_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/uint16Scalar");
                  // Check Value
                  uint16_t value = (uint16_t)*((uint16_t*)valueToCheck->data);
                  BOOST_CHECK(value == 0);
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  uint16_t newValue = 123;
                  UA_Variant_init(valueToCheck);
                  UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_UINT16]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (uint16_t)*((uint16_t*)valueToCheck->data);
                    BOOST_CHECK(value == newValue);
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_INT32: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "int32_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/Dein/Name/ist/int32Scalar");
                  // Check Value
                  int32_t value = (int32_t)*((int32_t*)valueToCheck->data);
                  BOOST_CHECK(value == 0);
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  int32_t newValue = 123;
                  UA_Variant_init(valueToCheck);
                  UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_INT32]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (int32_t)*((int32_t*)valueToCheck->data);
                    BOOST_CHECK(value == newValue);
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_UINT32: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "uint32_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/Mein/Name/ist/uint32Scalar");
                  // Check Value
                  uint32_t value = (uint32_t)*((uint32_t*)valueToCheck->data);
                  BOOST_CHECK(value == 0);
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  uint32_t newValue = 123;
                  UA_Variant_init(valueToCheck);
                  UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_UINT32]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (uint32_t)*((uint32_t*)valueToCheck->data);
                    BOOST_CHECK(value == newValue);
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_INT64: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "int64_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/Mein/Name/ist/int64Scalar");
                  // Check Value
                  int64_t value = (int64_t)*((int64_t*)valueToCheck->data);
                  BOOST_CHECK(value == 0);
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  int64_t newValue = 123;
                  UA_Variant_init(valueToCheck);
                  UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_INT64]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (int64_t)*((int64_t*)valueToCheck->data);
                    BOOST_CHECK(value == newValue);
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_UINT64: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "uint64_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/Mein/Name/ist/uint64Scalar");
                  // Check Value
                  uint64_t value = (uint64_t)*((uint64_t*)valueToCheck->data);
                  BOOST_CHECK(value == 0);
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  uint64_t newValue = 123;
                  UA_Variant_init(valueToCheck);
                  UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_UINT64]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (uint64_t)*((uint64_t*)valueToCheck->data);
                    BOOST_CHECK(value == newValue);
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_DOUBLE: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "double");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/Dieser/Name/ist/doubleScalar");
                  // Check Value
                  double value = (double)*((double*)valueToCheck->data);
                  BOOST_CHECK(value == 0);
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  double newValue = 123.12;
                  UA_Variant_init(valueToCheck);
                  UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_DOUBLE]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (double)*((double*)valueToCheck->data);
                    BOOST_CHECK(((int32_t)value * 100) == ((int32_t)newValue * 100));
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_FLOAT: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "float");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/floatScalar");
                  // Check Value
                  float value = (float)*((float*)valueToCheck->data);
                  BOOST_CHECK(value == 0);
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  float newValue = 123.12;
                  UA_Variant_init(valueToCheck);
                  UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_FLOAT]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (float)*((float*)valueToCheck->data);
                    BOOST_CHECK(((int32_t)value * 100) == ((int32_t)newValue * 100));
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                default: {
                  printf("Not define: %-16d\n", datatypeId.identifier.numeric);
                  BOOST_CHECK(false);
                }
              }
            }
            else {
              /*
               * Begin array section
               */
              switch(datatypeId.identifier.numeric - 1) {
                case UA_TYPES_SBYTE: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "int8_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  // cout << valName << endl;
                  BOOST_CHECK(valName == "/Mein/Name_ist#int8Array_s15");
                  // Check Value
                  size_t arrayLength = valueToCheck->arrayLength;
                  int8_t* value = (int8_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // set new value
                  int8_t varArray[arrayLength];
                  int8_t* newValue = varArray;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    newValue[i] = i;
                  }

                  UA_Variant_init(valueToCheck);
                  UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_SBYTE]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server -> should be the new one
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (int8_t*)valueToCheck->data;
                    for(uint32_t i = 0; i < arrayLength; i++) {
                      BOOST_CHECK(value[i] == newValue[i]);
                    }
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_BYTE: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "uint8_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  // cout << valName << endl;
                  BOOST_CHECK(valName == "/Dein/Name/ist/uint8Array_s10");
                  // Check Value
                  size_t arrayLength = valueToCheck->arrayLength;
                  uint8_t* value = (uint8_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // set new value
                  uint8_t varArray[arrayLength];
                  uint8_t* newValue = varArray;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    newValue[i] = i;
                  }

                  UA_Variant_init(valueToCheck);
                  UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_BYTE]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (uint8_t*)valueToCheck->data;
                    for(uint32_t i = 0; i < arrayLength; i++) {
                      BOOST_CHECK(value[i] == newValue[i]);
                    }
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_INT16: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "int16_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/Unser/Name/ist_int16Array_s10");
                  // Check Value
                  size_t arrayLength = valueToCheck->arrayLength;
                  int16_t* value = (int16_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // set new value
                  value = (int16_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // set new value
                  int16_t varArray[arrayLength];
                  int16_t* newValue = varArray;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    newValue[i] = i;
                  }

                  UA_Variant_init(valueToCheck);
                  UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_INT16]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (int16_t*)valueToCheck->data;
                    for(uint32_t i = 0; i < arrayLength; i++) {
                      BOOST_CHECK(value[i] == newValue[i]);
                    }
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_UINT16: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "uint16_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/Unser/Name/ist_uint8Array_s10");
                  // Check Value
                  size_t arrayLength = valueToCheck->arrayLength;
                  uint16_t* value = (uint16_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // set new value
                  value = (uint16_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  uint16_t varArray[arrayLength];
                  uint16_t* newValue = varArray;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    newValue[i] = i;
                  }

                  UA_Variant_init(valueToCheck);
                  UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_UINT16]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (uint16_t*)valueToCheck->data;
                    for(uint32_t i = 0; i < arrayLength; i++) {
                      BOOST_CHECK(value[i] == newValue[i]);
                    }
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_INT32: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "int32_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/int32Array_s15");
                  // Check Value
                  size_t arrayLength = valueToCheck->arrayLength;
                  int32_t* value = (int32_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // set new value
                  value = (int32_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  int32_t varArray[arrayLength];
                  int32_t* newValue = varArray;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    newValue[i] = i;
                  }

                  UA_Variant_init(valueToCheck);
                  UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_INT32]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (int32_t*)valueToCheck->data;
                    for(uint32_t i = 0; i < arrayLength; i++) {
                      BOOST_CHECK(value[i] == newValue[i]);
                    }
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_UINT32: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "uint32_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  // cout << valName << endl;
                  BOOST_CHECK(valName == "/uint32Array_s10");
                  // Check Value
                  size_t arrayLength = valueToCheck->arrayLength;
                  uint32_t* value = (uint32_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // set new value
                  uint32_t varArray[arrayLength];
                  uint32_t* newValue = varArray;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    newValue[i] = i;
                  }

                  UA_Variant_init(valueToCheck);
                  UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_UINT32]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (uint32_t*)valueToCheck->data;
                    for(uint32_t i = 0; i < arrayLength; i++) {
                      BOOST_CHECK(value[i] == newValue[i]);
                    }
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_INT64: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "int64_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/int64Array_s15");
                  // Check Value
                  size_t arrayLength = valueToCheck->arrayLength;
                  int64_t* value = (int64_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // set new value
                  value = (int64_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  int64_t varArray[arrayLength];
                  int64_t* newValue = varArray;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    newValue[i] = i;
                  }

                  UA_Variant_init(valueToCheck);
                  UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_INT64]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (int64_t*)valueToCheck->data;
                    for(uint32_t i = 0; i < arrayLength; i++) {
                      BOOST_CHECK(value[i] == newValue[i]);
                    }
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_UINT64: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "uint64_t");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/uint64Array_s10");
                  // Check Value
                  size_t arrayLength = valueToCheck->arrayLength;
                  uint64_t* value = (uint64_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // set new value
                  value = (uint64_t*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  uint64_t varArray[arrayLength];
                  uint64_t* newValue = varArray;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    newValue[i] = i;
                  }

                  UA_Variant_init(valueToCheck);
                  UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_UINT64]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (uint64_t*)valueToCheck->data;
                    for(uint32_t i = 0; i < arrayLength; i++) {
                      BOOST_CHECK(value[i] == newValue[i]);
                    }
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_DOUBLE: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "double");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  // cout << valName << endl;
                  BOOST_CHECK(valName == "/doubleArray_s15");
                  // Check Value
                  size_t arrayLength = valueToCheck->arrayLength;
                  double* value = (double*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  double varArray[arrayLength];
                  double* newValue = varArray;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    newValue[i] = i * 0.5;
                  }

                  UA_Variant_init(valueToCheck);
                  UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_DOUBLE]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (double*)valueToCheck->data;
                    for(uint32_t i = 0; i < arrayLength; i++) {
                      BOOST_CHECK(((int32_t)(value[i] * 100)) == ((int32_t)(newValue[i] * 100)));
                    }
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                case UA_TYPES_FLOAT: {
                  // Check Datatype
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)datatypeToCheck->data)), datatype);
                  BOOST_CHECK(datatype == "float");
                  // Check Name
                  UASTRING_TO_CPPSTRING(((UA_String) * ((UA_String*)nameToCheck->data)), valName);
                  BOOST_CHECK(valName == "/floatArray_s101234");
                  // Check Value
                  size_t arrayLength = valueToCheck->arrayLength;
                  float* value = (float*)valueToCheck->data;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    BOOST_CHECK(value[i] == 0);
                  }
                  // cout << "Wert: " << std::to_string(value) << endl;
                  //  set new value
                  float varArray[arrayLength];
                  float* newValue = varArray;
                  for(uint32_t i = 0; i < arrayLength; i++) {
                    newValue[i] = i * 0.5;
                  }

                  UA_Variant_init(valueToCheck);
                  UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_FLOAT]);
                  UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
                  if(retvalNewVar == UA_STATUSCODE_GOOD) {
                    // get value from server
                    UA_Variant_init(valueToCheck);
                    UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
                    value = (float*)valueToCheck->data;
                    for(uint32_t i = 0; i < arrayLength; i++) {
                      BOOST_CHECK(((int32_t)(value[i] * 100)) == ((int32_t)(newValue[i] * 100)));
                    }
                  }
                  else {
                    BOOST_CHECK(false);
                  }
                  break;
                }
                default: {
                  printf("Not define: %-16d\n", datatypeId.identifier.numeric);
                  BOOST_CHECK(false);
                }
              }
            }
          }
          UA_Variant_delete(valueToCheck);
          UA_Variant_delete(euToCheck);
          UA_Variant_delete(nameToCheck);
          UA_Variant_delete(datatypeToCheck);
          UA_Variant_delete(descToCheck);
        }
      }
    }
  }

  UA_BrowseRequest_clear(&bReq);
  UA_BrowseResponse_clear(&bResp);
  UA_Client_disconnect(client);
  /* Some times there is a double free corruption. */
  UA_Client_delete(client);

  serverSet.runUAServer = UA_FALSE;

  if(serverThread->joinable()) {
    serverThread->join();
  }

  cout << "Delete ServerThread" << endl;
  delete serverThread;
  serverThread = NULL;

  for(auto& ptr : varList) delete ptr;
}

class ProcessVariableTestSuite : public test_suite {
 public:
  ProcessVariableTestSuite() : test_suite("ua_processvariable Test Suite") {
    add(BOOST_TEST_CASE(&ProcessVariableTest::testClassSide));
    add(BOOST_TEST_CASE(&ProcessVariableTest::testClientSide));
  }
};

test_suite* init_unit_test_suite(int /*argc*/, char** /*argv[]*/) {
  framework::master_test_suite().add(new ProcessVariableTestSuite);
  return 0;
}
