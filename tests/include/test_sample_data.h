#ifndef _TEST_SAMPLE_DATA_H_
#define _TEST_SAMPLE_DATA_H_

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ProcessArray.h"

extern "C" {
#include "csa_namespace.h"
#include "unistd.h"
}

#include <open62541/server_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_config_default.h>
#include <open62541/client.h>

using namespace ChimeraTK;
using namespace std;

struct TestFixtureEmptySet {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers;
  boost::shared_ptr<ControlSystemPVManager> csManager;
  boost::shared_ptr<DevicePVManager> devManager;

  TestFixtureEmptySet() : pvManagers(createPVManager()), csManager(pvManagers.first), devManager(pvManagers.second) {
    std::cout << "TestFixtureEmptySet BEGIN" << std::endl;
    std::cout << "TestFixtureEmptySet END" << std::endl;
  }
};

struct TestFixturePVSet {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager>> pvManagers;
  boost::shared_ptr<ControlSystemPVManager> csManager;
  boost::shared_ptr<DevicePVManager> devManager;

  TestFixturePVSet() : pvManagers(createPVManager()), csManager(pvManagers.first), devManager(pvManagers.second) {
    std::cout << "TestFixturePVSet BEGIN" << std::endl;

    // Testset
    ProcessArray<int8_t>::SharedPtr intA8dev = devManager->createProcessArray<int8_t>(
        SynchronizationDirection::controlSystemToDevice, "int8Scalar", 1, "Einheit", "Beschreibung der Variable");
    ProcessArray<uint8_t>::SharedPtr intAu8dev =
        devManager->createProcessArray<uint8_t>(SynchronizationDirection::controlSystemToDevice, "uint8Scalar", 1);
    ProcessArray<int16_t>::SharedPtr intA16dev =
        devManager->createProcessArray<int16_t>(SynchronizationDirection::controlSystemToDevice, "int16Scalar", 1);
    ProcessArray<uint16_t>::SharedPtr intAu16dev = devManager->createProcessArray<uint16_t>(
        SynchronizationDirection::controlSystemToDevice, "uint16Scalar", 1);
    ProcessArray<int32_t>::SharedPtr intA3devMap = devManager->createProcessArray<int32_t>(
        SynchronizationDirection::controlSystemToDevice, "Dein/Name/ist/int32Scalar", 1);
    ProcessArray<uint32_t>::SharedPtr intAu32devMap = devManager->createProcessArray<uint32_t>(
        SynchronizationDirection::controlSystemToDevice, "Mein/Name/ist/uint32Scalar", 1);
    ProcessArray<int64_t>::SharedPtr intA64devMap = devManager->createProcessArray<int64_t>(
        SynchronizationDirection::controlSystemToDevice, "Mein/Name/ist/int64Scalar", 1);
    ProcessArray<uint64_t>::SharedPtr intAu64devMap = devManager->createProcessArray<uint64_t>(
        SynchronizationDirection::controlSystemToDevice, "Mein/Name/ist/uint64Scalar", 1);
    ProcessArray<float>::SharedPtr intAfdev =
        devManager->createProcessArray<float>(SynchronizationDirection::controlSystemToDevice, "floatScalar", 1);
    ProcessArray<double>::SharedPtr doubledevMap = devManager->createProcessArray<double>(
        SynchronizationDirection::controlSystemToDevice, "Dieser/Name/ist/doubleScalar", 1);
    ProcessArray<string>::SharedPtr stringAddev2 =
        devManager->createProcessArray<string>(SynchronizationDirection::deviceToControlSystem,
                                               "1/FOLDER/defaultSep/stringScalar", 1, "my description", "desc");

    ProcessArray<int8_t>::SharedPtr intB15A8devMap =
        devManager->createProcessArray<int8_t>(SynchronizationDirection::controlSystemToDevice,
                                               "Mein/Name_ist#int8Array_s15", 15, "Einheit", "Beschreibung der Variable");
    ProcessArray<uint8_t>::SharedPtr intB10Au8devMap1 = devManager->createProcessArray<uint8_t>(
        SynchronizationDirection::controlSystemToDevice, "Dein/Name/ist/uint8Array_s10", 10);
    ProcessArray<int16_t>::SharedPtr intB10Au16devMap2 = devManager->createProcessArray<int16_t>(
        SynchronizationDirection::controlSystemToDevice, "Unser/Name/ist_int16Array_s10", 12);
    ProcessArray<uint16_t>::SharedPtr intB10Au8devMap2 =
        devManager->createProcessArray<uint16_t>(SynchronizationDirection::controlSystemToDevice,
                                                 "Unser/Name/ist_uint8Array_s10", 10, "Einheit", "Beschreibung der Variable");
    ProcessArray<int32_t>::SharedPtr intB15A32dev = devManager->createProcessArray<int32_t>(
        SynchronizationDirection::controlSystemToDevice, "int32Array_s15", 15);
    ProcessArray<uint32_t>::SharedPtr intB10Au32dev = devManager->createProcessArray<uint32_t>(
        SynchronizationDirection::controlSystemToDevice, "uint32Array_s10", 10);
    ProcessArray<int64_t>::SharedPtr intB15A64dev = devManager->createProcessArray<int64_t>(
        SynchronizationDirection::controlSystemToDevice, "int64Array_s15", 15);
    ProcessArray<uint64_t>::SharedPtr intB15Au64dev = devManager->createProcessArray<uint64_t>(
        SynchronizationDirection::controlSystemToDevice, "uint64Array_s10", 10);
    ProcessArray<double>::SharedPtr intB15Afdev = devManager->createProcessArray<double>(
        SynchronizationDirection::controlSystemToDevice, "doubleArray_s15", 15);
    ProcessArray<float>::SharedPtr intB10Addev = devManager->createProcessArray<float>(
        SynchronizationDirection::controlSystemToDevice, "floatArray_s101234", 10);

    std::cout << "TestFixturePVSet END" << std::endl;
  }
};

struct TestFixtureServerSet {
  uint32_t opcuaPort = 16663;
  /* Create new Server */
  UA_ServerConfig* server_config;
  UA_ServerNetworkLayer server_nl;
  UA_Server* mappedServer;
  UA_NodeId baseNodeId;
  UA_Boolean runUAServer;

  TestFixtureServerSet() {
    this->mappedServer = UA_Server_new();
    UA_ServerConfig_setMinimal(UA_Server_getConfig(this->mappedServer), opcuaPort, NULL);
    server_config = UA_Server_getConfig(mappedServer);

    runUAServer = UA_TRUE;
    baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);

    csa_namespace_init(mappedServer);
  }
};

#endif // _TEST_SAMPLE_DATA_H_
