#include <ControlSystemAdapterOPCUA.h>
#include <mtca_uaadapter.h>
#include "ipc_manager.h"

#include <boost/test/included/unit_test.hpp>

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"

extern "C" {
#include "unistd.h"
#include "mtca_namespaceinit_generated.h" // Output des pyUANamespacecompilers
}


using namespace boost::unit_test_framework;

ipc_manager  *mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

struct TestFixtureEmptySet {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers;
  boost::shared_ptr<ControlSystemPVManager> csManager;
  boost::shared_ptr<DevicePVManager> devManager;
  
  uint32_t opcuaPort;
  
  ControlSystemSynchronizationUtility csSyncUtil;

  TestFixtureEmptySet() : pvManagers(createPVManager()),csManager(pvManagers.first), devManager(pvManagers.second), csSyncUtil(csManager) {
		csSyncUtil.receiveAll();
		opcuaPort = 16664;
	}
};

struct TestFixtureExampleSet {

	std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers;
	boost::shared_ptr<ControlSystemPVManager> csManager;
	boost::shared_ptr<DevicePVManager> devManager;
	
	uint32_t opcuaPort;
	
	ControlSystemSynchronizationUtility csSyncUtil;
	
  TestFixtureExampleSet() : pvManagers(createPVManager()),csManager(pvManagers.first), devManager(pvManagers.second), csSyncUtil(csManager) {
		csSyncUtil.receiveAll();
		opcuaPort = 16664;
		
		ProcessScalar<int8_t>::SharedPtr intA8dev = devManager->createProcessScalar<int8_t>(controlSystemToDevice, "intA8");
		ProcessScalar<uint8_t>::SharedPtr intAu8dev = devManager->createProcessScalar<uint8_t>(controlSystemToDevice, "intAu8");
		ProcessScalar<int16_t>::SharedPtr intA16dev = devManager->createProcessScalar<int16_t>(controlSystemToDevice, "intA16");
		ProcessScalar<uint16_t>::SharedPtr intAu16dev = devManager->createProcessScalar<uint16_t>(controlSystemToDevice, "intAu16");
		ProcessScalar<int32_t>::SharedPtr intA32dev = devManager->createProcessScalar<int32_t>(controlSystemToDevice, "intA32");
		ProcessScalar<uint32_t>::SharedPtr intAu32dev = devManager->createProcessScalar<uint32_t>(controlSystemToDevice, "intAu32");
		ProcessScalar<float>::SharedPtr intAfdev = devManager->createProcessScalar<float>(controlSystemToDevice, "intAf");
		ProcessScalar<double>::SharedPtr intAddev = devManager->createProcessScalar<double>(controlSystemToDevice, "intAd");
    
		
		ProcessArray<int8_t>::SharedPtr intB15A8dev = devManager->createProcessArray<int8_t>(controlSystemToDevice, "intB15A8", 15);
		ProcessArray<uint8_t>::SharedPtr intB10Au8dev = devManager->createProcessArray<uint8_t>(controlSystemToDevice, "intB10Au8", 10);
		ProcessArray<int16_t>::SharedPtr intB15A16dev = devManager->createProcessArray<int16_t>(controlSystemToDevice, "intB15A16", 15);
		ProcessArray<uint16_t>::SharedPtr intB10Au16dev = devManager->createProcessArray<uint16_t>(controlSystemToDevice, "intB10Au16", 10);
		ProcessArray<int32_t>::SharedPtr intB15A32dev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "intB15A32", 15);
		ProcessArray<uint32_t>::SharedPtr intB10Au32dev = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "intB10Au32", 10);
		ProcessArray<double>::SharedPtr intB15Afdev = devManager->createProcessArray<double>(controlSystemToDevice, "intB15Af", 15);
		ProcessArray<float>::SharedPtr intB10Addev = devManager->createProcessArray<float>(controlSystemToDevice, "intB10Ad", 10);
		

		int x[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
		std::vector<int> v(x, x + sizeof x / sizeof x[0]);
		csManager->getProcessArray<int32_t>("intB15A32")->set(v); 
		csManager->getProcessScalar<int8_t>("intA8")->set(12);
		
	}
};




class CSAOPCUATest {
	public:
		static void testEmptySet();
		static void testExampleSet();
};
   
void CSAOPCUATest::testEmptySet(){ 
	std::cout << "Enter CSAOPCUATest with EmptySet" << std::endl;
	TestFixtureEmptySet tfEmptySet;
	 // Create the managers
	ControlSystemAdapterOPCUA *csaOPCUA = new ControlSystemAdapterOPCUA(tfEmptySet.opcuaPort, tfEmptySet.csManager);
	// is Server running?
	csaOPCUA->start();
	BOOST_CHECK(csaOPCUA->isRunning() == true);
	// is csManager init
	BOOST_CHECK(csaOPCUA->getControlSystemPVManager()->getAllProcessVariables().size() == 0);
	
	BOOST_CHECK(csaOPCUA->getOPCUAPort() == 16664);	
	
	csaOPCUA->stop();
	csaOPCUA->terminate();
	BOOST_CHECK(csaOPCUA->isRunning() != true);
};

void CSAOPCUATest::testExampleSet(){ 
	std::cout << "Enter CSAOPCUATest with ExampleSet" << std::endl;
	TestFixtureExampleSet tfExampleSet;
	 // Create the managers
	ControlSystemAdapterOPCUA *csaOPCUA = new ControlSystemAdapterOPCUA(tfExampleSet.opcuaPort, tfExampleSet.csManager);
	// is Server running?
	BOOST_CHECK(csaOPCUA->isRunning() == true);
	
	// is csManager init
	BOOST_CHECK(csaOPCUA->getControlSystemPVManager()->getAllProcessVariables().size() == 16);
	
	BOOST_CHECK(csaOPCUA->getOPCUAPort() == 16664);
	
	//mtca_uaadapter *adapter = csaOPCUA->getUAAdapter();
	
// 	adapter.addVariable("Heinz", tfExampleSet.csManager);
// 	adapter.addConstant("Berndt", tfExampleSet.csManager);
		
	csaOPCUA->stop();
	csaOPCUA->terminate();
	BOOST_CHECK(csaOPCUA->isRunning() != true);
};


/*
 * UAAdapterTest
 * 
 */
class UAAdapterTest {
	public:
		static void testEmptySet();
		static void testExampleSet();
};
   
void UAAdapterTest::testEmptySet(){ 
	std::cout << "Enter UAAdapterTest with EmptySet" << std::endl;
	TestFixtureEmptySet tfEmptySet;
	
	mtca_uaadapter *adapter = new mtca_uaadapter(16664);
  
	mgr->addObject(adapter);

	mgr->doStart(); 
	//std::cout << adapter->getIpcId() << std::endl;
	//mgr->deleteObject(adapter->getIpcId());
	mgr->stopAll();
	
	//adapter->doStop();
	//adapter->terminate();
	//BOOST_CHECK(adapter->isRunning() != true);
	
	//adapter->~mtca_uaadapter();
};

void UAAdapterTest::testExampleSet(){ 
	std::cout << "Enter UAAdapterTest with ExampleSet" << std::endl;
	TestFixtureExampleSet tfExampleSet;
  		
	mtca_uaadapter *adapter = new mtca_uaadapter(16664);
	
	mgr->addObject(adapter);
	mgr->doStart();
	
	// is Server running?
	adapter->addConstant("intA8", tfExampleSet.csManager);
	adapter->addVariable("intAu8", tfExampleSet.csManager);

	
	//adapter->doStop();
	//adapter->terminate();
	//BOOST_CHECK(adapter->isRunning() != true);
	
	//adapter->~mtca_uaadapter();

};

/*
 * ProcessVariableTest
 * 
 */
class ProcessVariableTest {
	public:
		static void testEmptySet();
		static void testExampleSet();
};
   
void ProcessVariableTest::testEmptySet(){ 
	std::cout << "Enter ProcessVariableTest with EmptySet" << std::endl;
	TestFixtureEmptySet tfEmptySet;
	TestFixtureExampleSet tfExampleSet;
	
	/* Create new Server */
	UA_ServerConfig       server_config;
	UA_ServerNetworkLayer server_nl;
	
	server_config = UA_ServerConfig_standard;
    server_nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, 16665);
    server_config.logger = UA_Log_Stdout;
    server_config.networkLayers = &server_nl;
    server_config.networkLayersSize = 1;
  
    UA_Server *mappedServer = UA_Server_new(server_config);
    UA_NodeId baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  
    mtca_namespaceinit_generated(mappedServer);
	UA_NodeId constantsListId;
	
	//mtca_processvariable *test;
	for(ProcessVariable::SharedPtr oneProcessVariable : tfExampleSet.csManager->getAllProcessVariables()) {
		std::cout << "Check ProcessVariable: " <<  oneProcessVariable->getName() << std::endl;
		mtca_processvariable *test = new mtca_processvariable(mappedServer, baseNodeId, oneProcessVariable->getName(), tfExampleSet.csManager);
		BOOST_CHECK(test->getName() == oneProcessVariable->getName());
		
		BOOST_CHECK(test->getTimeStamp().index0 == 0);
		BOOST_CHECK(test->getTimeStamp().index1 == 0);
		BOOST_CHECK(test->getTimeStamp().nanoSeconds == 0);
		BOOST_CHECK(test->getTimeStamp().seconds == 0);
		
		BOOST_CHECK(test->getTimeStampIndex0() == 0);
		BOOST_CHECK(test->getTimeStampIndex1() == 0);
		BOOST_CHECK(test->getTimeStampNanoSeconds() == 0);
		BOOST_CHECK(test->getTimeStampSeconds() == 0);
		
		std::string valueType = test->getType();
		if(oneProcessVariable->isArray()) {
			if (valueType == "int8_t") {
				BOOST_CHECK(valueType == "int8_t");
				std::cout << test->getValue_int8_t() << std::endl;
				BOOST_CHECK(test->getValue_int8_t() == 12);
				test->setValue_int8_t(13);
				BOOST_CHECK(test->getValue_int8_t() == 13);
			}
			else if (valueType == "uint8_t") {
				BOOST_CHECK(valueType == "uint8_t");
				std::cout << test->getValue_uint8_t() << std::endl;
				BOOST_CHECK(test->getValue_uint8_t() == 0);
				test->setValue_uint8_t(13);
				BOOST_CHECK(test->getValue_uint8_t() == 13);
			}
			else if (valueType == "int16_t") {
				BOOST_CHECK(valueType == "int16_t");
				BOOST_CHECK(test->getValue_int16_t() == 0);
				test->setValue_int16_t(13);
				BOOST_CHECK(test->getValue_int16_t() == 13);
			}
			else if (valueType == "uint16_t") {
				BOOST_CHECK(valueType == "uint16_t");
				BOOST_CHECK(test->getValue_uint16_t() == 0);
				test->setValue_uint16_t(13);
				BOOST_CHECK(test->getValue_uint16_t() == 13);
			}
			else if (valueType == "int32_t") {
				BOOST_CHECK(valueType == "int32_t");
				BOOST_CHECK(test->getValue_int32_t() == 0);
				test->setValue_int32_t(13);
				BOOST_CHECK(test->getValue_int32_t() == 13);
			}
			else if (valueType == "uint32_t") {
				BOOST_CHECK(valueType == "uint32_t");
				BOOST_CHECK(test->getValue_uint32_t() == 0);
				test->setValue_uint32_t(13);
				BOOST_CHECK(test->getValue_uint32_t() == 13);
			}
			else if (valueType == "float") {
				BOOST_CHECK(valueType == "float");
				BOOST_CHECK(test->getValue_float() == 0);
				test->setValue_float(13);
				BOOST_CHECK(test->getValue_float() == 13);
			}
			else if (valueType == "double") {
				BOOST_CHECK(valueType == "double");
				BOOST_CHECK(test->getValue_double() == 0);
				test->setValue_double(13);
				BOOST_CHECK(test->getValue_double() == 13);
			}
			else BOOST_CHECK(false);	
		}
		else {
			if (valueType == "int8_t") {
				BOOST_CHECK(valueType == "int8_t");
				int32_t i = 0;
				std::vector<int8_t> newVector(test->getValue_Array_int8_t().size());
				for(int8_t value: test->getValue_Array_int8_t()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_int8_t(newVector);
			}
			else if (valueType == "uint8_t") {
				BOOST_CHECK(valueType == "uint8_t");
				int32_t i = 0;
				std::vector<uint8_t> newVector(test->getValue_Array_uint8_t().size());
				for(uint8_t value: test->getValue_Array_uint8_t()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
					
				}
				test->setValue_Array_uint8_t(newVector);
			}
			else if (valueType == "int16_t") {
				BOOST_CHECK(valueType == "int16_t");
				int32_t i = 0;
				std::vector<int16_t> newVector(test->getValue_Array_int16_t().size());
				for(int16_t value: test->getValue_Array_int16_t()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_int16_t(newVector);
			}
			else if (valueType == "uint16_t") {
				BOOST_CHECK(valueType == "uint16_t");
				int32_t i = 0;
				std::vector<uint16_t> newVector(test->getValue_Array_uint16_t().size());
				for(uint16_t value: test->getValue_Array_uint16_t()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_uint16_t(newVector);
			}
			else if (valueType == "int32_t") {
				BOOST_CHECK(valueType == "int32_t");
				int32_t i = 0;
				std::vector<int32_t> newVector(test->getValue_Array_int32_t().size());
				for(int32_t value : test->getValue_Array_int32_t()){
					BOOST_CHECK(value == i);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_int32_t(newVector);
			}
			else if (valueType == "uint32_t") {
				BOOST_CHECK(valueType == "uint32_t");
				int32_t i = 0;
				std::vector<uint32_t> newVector(test->getValue_Array_uint32_t().size());
				for(uint32_t value: test->getValue_Array_uint32_t()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_uint32_t(newVector);
			}
			else if (valueType == "float") {
				BOOST_CHECK(valueType == "float");
				int32_t i = 0;
				std::vector<float> newVector(test->getValue_Array_float().size());
				for(float value: test->getValue_Array_float()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_float(newVector);
			}
			else if (valueType == "double") {
				BOOST_CHECK(valueType == "double");
				int32_t i = 0;
				std::vector<double> newVector(test->getValue_Array_double().size());
				for(double value: test->getValue_Array_double()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_double(newVector);
			}
			else BOOST_CHECK(false);
		}
		
		 		
		std::string newName;
		newName = oneProcessVariable->getName();
		newName.append("_1");
		
		//std::cout << newName << std::endl;
		test->setName(newName);
		//BOOST_CHECK(test->getName() == newName);
		
    }
    
    
    	
	 if (mappedServer == nullptr) {
        return;
    }
  
    UA_Boolean runUAServer = UA_TRUE;
    thread *serverThread = new std::thread(UA_Server_run, mappedServer, &runUAServer);
	
	
	// Create client to connect to server
		UA_Client *client = UA_Client_new(UA_ClientConfig_standard);
		UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:16665");
		
		if (retval != UA_STATUSCODE_GOOD) {
			std::cout << "Failed to connect to server" << "opc.tcp://localhost:16665" << std::endl;
		}
		
		
		printf("Browsing nodes in objects folder:\n");
		UA_BrowseRequest bReq;
		UA_BrowseRequest_init(&bReq);
		bReq.requestedMaxReferencesPerNode = 0;
		bReq.nodesToBrowse = UA_BrowseDescription_new();
		bReq.nodesToBrowseSize = 1;
		bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
		bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
		UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
		printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
		for (size_t i = 0; i < bResp.resultsSize; ++i) {
			for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
						
				UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
				if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
					printf("%-9d %-16d %-16.*s %-16.*s\n", ref->browseName.namespaceIndex,
					ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
					ref->browseName.name.data, (int)ref->displayName.text.length,
					ref->displayName.text.data);
					
					
					size_t lenBrowseName = (int)ref->browseName.name.length;
					std::string browseNameFound(reinterpret_cast<char const*>(ref->browseName.name.data), lenBrowseName);
					
					//if(browseNameFound == "intA8") {
					
						UA_BrowseRequest bReq2;
						UA_BrowseRequest_init(&bReq2);
						bReq2.requestedMaxReferencesPerNode = 0;
						bReq2.nodesToBrowse = UA_BrowseDescription_new();
						bReq2.nodesToBrowseSize = 1;
						bReq2.nodesToBrowse[0].nodeId = ref->nodeId.nodeId; 
						bReq2.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; 
						UA_BrowseResponse bResp2 = UA_Client_Service_browse(client, bReq2);
		
						printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
					
						for (size_t m = 0; m < bResp2.resultsSize; ++m) {
							for (size_t k = 0; k < bResp2.results[m].referencesSize; ++k) {
								UA_ReferenceDescription *refe = &(bResp2.results[m].references[k]);
								if(refe->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
									
									printf("%-9d %-16d %-16.*s %-16.*s\n", refe->browseName.namespaceIndex,
										refe->nodeId.nodeId.identifier.numeric, (int)refe->browseName.name.length,
										refe->browseName.name.data, (int)refe->displayName.text.length,
										refe->displayName.text.data);
									
									size_t lenBrowseName = (int)refe->browseName.name.length;
									std::string browseNameFound(reinterpret_cast<char const*>(refe->browseName.name.data), lenBrowseName);
					
									if(browseNameFound == "Value") {
										UA_Variant *val = UA_Variant_new();
										UA_StatusCode retval = UA_Client_readValueAttribute(client, refe->nodeId.nodeId, val);
									
										int8_t value;
										if (retval == UA_STATUSCODE_GOOD) {
											printf("%-16d\n", refe->nodeId.nodeId.identifier.numeric);
											value = *(int8_t*) val->data;
											std::cout << "Wert: " << std::to_string(value) << std::endl;
										}
										
										val = UA_Variant_new();
										int32_t merk = 123;
										UA_Variant_setScalarCopy(val, &merk, &UA_TYPES[UA_TYPES_INT32]);
										UA_StatusCode retva = UA_Client_writeValueAttribute(client, refe->nodeId.nodeId, val);
										
										UA_Variant_delete(val);
									}
									/*
									if(browseNameFound == "Name") {
										
										UA_Variant *val = UA_Variant_new();
										
										UA_StatusCode retval = UA_Client_readValueAttribute(client, refe->nodeId.nodeId, val);
									
										const char *value;
										
										if (retval == UA_STATUSCODE_GOOD) {
											printf("%-16d\n", refe->nodeId.nodeId.identifier.numeric);
											//std::string s(static_cast<const char*>(val->data), 1);
											value = (const char*) val->data;
											std::cout << "Name Wert: " << value << std::endl;
										}
									
										
										val = UA_Variant_new();
										const char *merk = "Ulf";
										//val->data = &merk;h
										UA_Variant_setScalarCopy(val, &merk, &UA_TYPES[UA_TYPES_STRING]);
										UA_StatusCode retva = UA_Client_writeValueAttribute(client, refe->nodeId.nodeId, val);
										
										if (retva != UA_STATUSCODE_GOOD) {
											std::cout << "Failed to write var" << std::endl;
										}
										
										UA_Variant_delete(val);
									}
									*/
								}
							}
						}
						UA_BrowseRequest_deleteMembers(&bReq2);
						UA_BrowseResponse_deleteMembers(&bResp2);
				//	}	
				}
			}
		}
		UA_BrowseRequest_deleteMembers(&bReq);
		UA_BrowseResponse_deleteMembers(&bResp);
		

/*
      while (true) {
          sleep(1);
      }*/
	//adapter->doStop();
	//adapter->terminate();
	//BOOST_CHECK(adapter->isRunning() != true);
	
	//adapter->~mtca_uaadapter();
};

void ProcessVariableTest::testExampleSet(){ 
	std::cout << "Enter ProcessVariableTest with ExampleSet" << std::endl;
	TestFixtureExampleSet tfExampleSet;
  		
	
	
	//adapter->doStop();
	//adapter->terminate();
	//BOOST_CHECK(adapter->isRunning() != true);
	
	//adapter->~mtca_uaadapter();

};

/**
   * The boost test suite which executes the ProcessVariableTest.
   */
class CSAOPCUATestSuite: public test_suite {
	public:
		CSAOPCUATestSuite() : test_suite("ControlSystemAdapterOPCUA Test Suite") {
			add(BOOST_TEST_CASE(&CSAOPCUATest::testEmptySet));
			add(BOOST_TEST_CASE(&CSAOPCUATest::testExampleSet));
    }
};

class UAAdapterTestSuite: public test_suite {
	public:
		UAAdapterTestSuite() : test_suite("mtca_uaadapter Test Suite") {
			add(BOOST_TEST_CASE(&UAAdapterTest::testEmptySet));
			add(BOOST_TEST_CASE(&UAAdapterTest::testExampleSet));
    }
};

class ProcessVariableTestSuite: public test_suite {
	public:
		ProcessVariableTestSuite() : test_suite("mtca_processvariable Test Suite") {
			add(BOOST_TEST_CASE(&ProcessVariableTest::testEmptySet));
			add(BOOST_TEST_CASE(&ProcessVariableTest::testExampleSet));
    }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	
	 framework::master_test_suite().add(new CSAOPCUATestSuite);
	 framework::master_test_suite().add(new UAAdapterTestSuite);
	 framework::master_test_suite().add(new ProcessVariableTestSuite);

    return 0;
}

