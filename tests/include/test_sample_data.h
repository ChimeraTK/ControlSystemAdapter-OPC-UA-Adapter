#ifndef _TEST_SAMPLE_DATA_H_
#define _TEST_SAMPLE_DATA_H_

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"
#include "ChimeraTK/ControlSystemAdapter/ProcessArray.h"
#include "ChimeraTK/ControlSystemAdapter/DeviceSynchronizationUtility.h"

#include "ipc_managed_object.h"

extern "C" {
	#include "unistd.h"
	#include "csa_namespaceinit_generated.h" // Output des pyUANamespacecompilers
}

using namespace ChimeraTK;

struct TestFixtureEmptySet {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers;
  boost::shared_ptr<ControlSystemPVManager> csManager;
  boost::shared_ptr<DevicePVManager> devManager;
    
	boost::shared_ptr<ControlSystemSynchronizationUtility> syncCsUtility;

  TestFixtureEmptySet() : pvManagers(createPVManager()),csManager(pvManagers.first), devManager(pvManagers.second) {
		syncCsUtility.reset(new ChimeraTK::ControlSystemSynchronizationUtility(csManager));
		syncCsUtility->receiveAll();
	}
};

struct TestFixturePVSet {

	std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers;
	boost::shared_ptr<ControlSystemPVManager> csManager;
	boost::shared_ptr<DevicePVManager> devManager;

	boost::shared_ptr<ControlSystemSynchronizationUtility> syncCsUtility;
	
  TestFixturePVSet() : pvManagers(createPVManager()),csManager(pvManagers.first), devManager(pvManagers.second){
		syncCsUtility.reset(new ChimeraTK::ControlSystemSynchronizationUtility(csManager));
		syncCsUtility->receiveAll();
	
	// Testset
	ProcessArray<int8_t>::SharedPtr intA8dev = devManager->createProcessArray<int8_t>(controlSystemToDevice, "int8Scalar", 1, "Einheit", "Beschreibung der Variable");
	ProcessArray<uint8_t>::SharedPtr intAu8dev = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "uint8Scalar", 1);
	ProcessArray<int16_t>::SharedPtr intA16dev = devManager->createProcessArray<int16_t>(controlSystemToDevice, "int16Scalar", 1);
	ProcessArray<uint16_t>::SharedPtr intAu16dev = devManager->createProcessArray<uint16_t>(controlSystemToDevice, "uint16Scalar", 1);
	ProcessArray<int32_t>::SharedPtr intA3devMap = devManager->createProcessArray<int32_t>(controlSystemToDevice, "Dein/Name/ist/int32Scalar", 1);
	ProcessArray<uint32_t>::SharedPtr intAu32devMap = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "Mein/Name/ist/uint32Scalar", 1);
	ProcessArray<float>::SharedPtr intAfdev = devManager->createProcessArray<float>(controlSystemToDevice, "floatScalar", 1);
	ProcessArray<double>::SharedPtr doubledevMap = devManager->createProcessArray<double>(controlSystemToDevice, "Dieser/Name/ist/doubleScalar", 1);
    
	ProcessArray<int8_t>::SharedPtr intB15A8devMap = devManager->createProcessArray<int8_t>(controlSystemToDevice, "Mein/Name_ist#int8Array_s15", 15, "Einheit", "Beschreibung der Variable");
	ProcessArray<uint8_t>::SharedPtr intB10Au8devMap1 = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "Dein/Name/ist/uint8Array_s10", 10);
	ProcessArray<int16_t>::SharedPtr intB10Au16devMap2 = devManager->createProcessArray<int16_t>(controlSystemToDevice, "Unser/Name/ist_int16Array_s10", 12);
	ProcessArray<uint16_t>::SharedPtr intB10Au8devMap2 = devManager->createProcessArray<uint16_t>(controlSystemToDevice, "Unser/Name/ist_uint8Array_s10", 10, "Einheit", "Beschreibung der Variable");
	ProcessArray<int32_t>::SharedPtr intB15A32dev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "int32Array_s15", 15);
	ProcessArray<uint32_t>::SharedPtr intB10Au32dev = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "uint32Array_s10", 10);
	ProcessArray<double>::SharedPtr intB15Afdev = devManager->createProcessArray<double>(controlSystemToDevice, "doubleArray_s15", 15);
	ProcessArray<float>::SharedPtr intB10Addev = devManager->createProcessArray<float>(controlSystemToDevice, "floatArray_s101234", 10);
	}
};

struct TestFixtureServerSet {
	
	uint32_t opcuaPort;
	/* Create new Server */
	UA_ServerConfig       server_config;
	UA_ServerNetworkLayer server_nl;
	UA_Server *mappedServer;
	UA_NodeId baseNodeId;
	UA_Boolean runUAServer;
	
	TestFixtureServerSet() {
		
		opcuaPort = 16663;
		server_config = UA_ServerConfig_standard;
		server_nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, opcuaPort);
		server_config.logger = UA_Log_Stdout;
		server_config.networkLayers = &server_nl;
		server_config.networkLayersSize = 1;
		
		runUAServer = UA_TRUE;
		
		mappedServer = UA_Server_new(server_config);
    baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
		
		csa_namespaceinit_generated(mappedServer);
	}
};


#endif // _TEST_SAMPLE_DATA_H_