#ifndef _TEST_SAMPLE_DATA_H_
#define _TEST_SAMPLE_DATA_H_

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"

#include "ipc_managed_object.h"

extern "C" {
	#include "unistd.h"
	#include "mtca_namespaceinit_generated.h" // Output des pyUANamespacecompilers
}


struct TestFixtureEmptySet {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers;
  boost::shared_ptr<ControlSystemPVManager> csManager;
  boost::shared_ptr<DevicePVManager> devManager;
    
  ControlSystemSynchronizationUtility csSyncUtil;

  TestFixtureEmptySet() : pvManagers(createPVManager()),csManager(pvManagers.first), devManager(pvManagers.second), csSyncUtil(csManager) {
		csSyncUtil.receiveAll();
	}
};

struct TestFixturePVSet {

	std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers;
	boost::shared_ptr<ControlSystemPVManager> csManager;
	boost::shared_ptr<DevicePVManager> devManager;
		
	ControlSystemSynchronizationUtility csSyncUtil;
	
  TestFixturePVSet() : pvManagers(createPVManager()),csManager(pvManagers.first), devManager(pvManagers.second), csSyncUtil(csManager) {
		csSyncUtil.receiveAll();
		
	ProcessScalar<int8_t>::SharedPtr intA8dev = devManager->createProcessScalar<int8_t>(controlSystemToDevice, "int8Scalar");
	ProcessScalar<uint8_t>::SharedPtr intAu8dev = devManager->createProcessScalar<uint8_t>(controlSystemToDevice, "uint8Scalar");
	ProcessScalar<int16_t>::SharedPtr intA16dev = devManager->createProcessScalar<int16_t>(controlSystemToDevice, "int16Scalar");
	ProcessScalar<uint16_t>::SharedPtr intAu16dev = devManager->createProcessScalar<uint16_t>(controlSystemToDevice, "uint16Scalar");
	ProcessScalar<int32_t>::SharedPtr intA32dev = devManager->createProcessScalar<int32_t>(controlSystemToDevice, "int32Scalar");
	ProcessScalar<uint32_t>::SharedPtr intAu32dev = devManager->createProcessScalar<uint32_t>(controlSystemToDevice, "uint32Scalar");
	ProcessScalar<float>::SharedPtr intAfdev = devManager->createProcessScalar<float>(controlSystemToDevice, "floatScalar");
	ProcessScalar<double>::SharedPtr intAddev = devManager->createProcessScalar<double>(controlSystemToDevice, "doubleScalar");
    
	ProcessArray<int8_t>::SharedPtr intB15A8dev = devManager->createProcessArray<int8_t>(controlSystemToDevice, "int8Array_s15", 15);
	ProcessArray<uint8_t>::SharedPtr intB10Au8dev = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "uint8Array_s10", 10);
	ProcessArray<int16_t>::SharedPtr intB15A16dev = devManager->createProcessArray<int16_t>(controlSystemToDevice, "int16Array_s15", 15);
	ProcessArray<uint16_t>::SharedPtr intB10Au16dev = devManager->createProcessArray<uint16_t>(controlSystemToDevice, "uint16Array_s10", 10);
	ProcessArray<int32_t>::SharedPtr intB15A32dev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "int32Array_s15", 15);
	ProcessArray<uint32_t>::SharedPtr intB10Au32dev = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "uint32Array_s10", 10);
	ProcessArray<double>::SharedPtr intB15Afdev = devManager->createProcessArray<double>(controlSystemToDevice, "doubleArray_s15", 15);
	ProcessArray<float>::SharedPtr intB10Addev = devManager->createProcessArray<float>(controlSystemToDevice, "floatArray_s10", 10);
		
	// example mapping variable
	ProcessArray<int8_t>::SharedPtr intB15A8devMap = devManager->createProcessArray<int8_t>(controlSystemToDevice, "Mein/Name_ist#int8Array_s15", 15);
	ProcessArray<uint8_t>::SharedPtr intB10Au8devMap1 = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "Dein/Name/ist/uint8Array_s10", 10);
	ProcessArray<uint8_t>::SharedPtr intB10Au8devMap2 = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "Unser/Name/ist_uint8Array_s10", 10);
	ProcessScalar<uint32_t>::SharedPtr intAu32devMap = devManager->createProcessScalar<uint32_t>(controlSystemToDevice, "Ist/Name/dieser/uint32Scalar");
	ProcessScalar<int32_t>::SharedPtr intA32devMap = devManager->createProcessScalar<int32_t>(controlSystemToDevice, "Ist/Name/dieser/int32Scalar");
	ProcessScalar<double>::SharedPtr doubledevMap = devManager->createProcessScalar<double>(controlSystemToDevice, "Ist/Name/dieser/doubleScalar");
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
		
		opcuaPort = 16660;
		server_config = UA_ServerConfig_standard;
		server_nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, opcuaPort);
		server_config.logger = UA_Log_Stdout;
		server_config.networkLayers = &server_nl;
		server_config.networkLayersSize = 1;
		server_config.enableAnonymousLogin = UA_TRUE;
		
		runUAServer = UA_TRUE;
		
		mappedServer = UA_Server_new(server_config);
    baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
		
		mtca_namespaceinit_generated(mappedServer);
	}
};


#endif // _TEST_SAMPLE_DATA_H_