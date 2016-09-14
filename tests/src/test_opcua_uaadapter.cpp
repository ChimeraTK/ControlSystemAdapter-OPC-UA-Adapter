#include <mtca_uaadapter.h>
#include <xml_file_handler.h>

#include <boost/test/included/unit_test.hpp>

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"

using namespace boost::unit_test_framework;
using namespace std;

struct TestFixtureEmptySet {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers;
  boost::shared_ptr<ControlSystemPVManager> csManager;
  boost::shared_ptr<DevicePVManager> devManager;
  
  uint32_t opcuaPort;
  
  ControlSystemSynchronizationUtility csSyncUtil;

  TestFixtureEmptySet() : pvManagers(createPVManager()),csManager(pvManagers.first), devManager(pvManagers.second), csSyncUtil(csManager) {
		csSyncUtil.receiveAll();
		opcuaPort = 16661;
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
		opcuaPort = 16660;
		
		ProcessScalar<int8_t>::SharedPtr intA8dev = devManager->createProcessScalar<int8_t>(controlSystemToDevice, "int8Scalar");
		ProcessScalar<uint8_t>::SharedPtr intAu8dev = devManager->createProcessScalar<uint8_t>(deviceToControlSystem, "uint8Scalar");
		ProcessScalar<int16_t>::SharedPtr intA16dev = devManager->createProcessScalar<int16_t>(deviceToControlSystem, "int16Scalar");
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
		ProcessArray<uint32_t>::SharedPtr intB10Au32dev = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "int32Array_s10", 10);
		ProcessArray<double>::SharedPtr intB15Afdev = devManager->createProcessArray<double>(controlSystemToDevice, "doubleArray_s15", 15);
		ProcessArray<float>::SharedPtr intB10Addev = devManager->createProcessArray<float>(controlSystemToDevice, "floatArray_s10", 10);
	}
};


class UAAdapterTest {
	public:
		static void testEmptySet();
		static void testExampleSet();
};
   
void UAAdapterTest::testEmptySet(){ 
	cout << "UAAdapterTest with EmptySet started." << endl;
	TestFixtureEmptySet tfEmptySet;
	mtca_uaadapter *adapter = new mtca_uaadapter(tfEmptySet.opcuaPort, "../../tests/uamapping_test.xml");
	
	// is Server running?
	adapter->doStart();
	BOOST_CHECK(adapter->isRunning() == true);
	
	adapter->doStop();
	BOOST_CHECK(adapter->isRunning() != true);
	
	adapter->doStart();
	BOOST_CHECK(adapter->isRunning() == true);
	
	BOOST_CHECK(adapter->getIpcId() == 0);
	
	UA_NodeId nodeId = adapter->getOwnNodeId();
	BOOST_CHECK(!UA_NodeId_isNull(&nodeId));
	
	adapter->doStop();
	BOOST_CHECK(adapter->isRunning() != true);
	
};

void UAAdapterTest::testExampleSet(){ 
	cout << "UAAdapterTest with ExampleSet started." << endl;
	TestFixtureExampleSet tfExampleSet;
	 // Create the managers
	mtca_uaadapter *adapter = new mtca_uaadapter(tfExampleSet.opcuaPort, "../../tests/uamapping_test.xml");
	xml_file_handler *xmlHandler = new xml_file_handler("../../tests/uamapping_test.xml");
	
	// is Server running?
	adapter->doStart();
	BOOST_CHECK(adapter->isRunning() == true);
	
	adapter->doStop();
	BOOST_CHECK(adapter->isRunning() != true);
	
	adapter->doStart();
	BOOST_CHECK(adapter->isRunning() == true);
	
	BOOST_CHECK(adapter->getIpcId() == 0);
	UA_NodeId ownNodeId = adapter->getOwnNodeId();
	BOOST_CHECK(!UA_NodeId_isNull(&ownNodeId));
	
	// Check folder functions
	vector<string> pathVector = xmlHandler->praseVariablePath("/test/test/");
	//vector<string> pathVector = {"tee", "tea"};
	// Check if path exist
	UA_NodeId folderNodeId = adapter->existFolderPath(ownNodeId, pathVector);
	BOOST_CHECK(UA_NodeId_isNull(&folderNodeId));
	
	// create path
	folderNodeId = adapter->createFolderPath(ownNodeId, pathVector);
	BOOST_CHECK(!UA_NodeId_isNull(&folderNodeId));
	// cheack if path exist now
	folderNodeId = adapter->existFolderPath(ownNodeId, pathVector);
	BOOST_CHECK(!UA_NodeId_isNull(&folderNodeId));
	
	// Check if path partly exist and create it 
	pathVector = xmlHandler->praseVariablePath("/test/test1/");
	folderNodeId = adapter->createFolderPath(UA_NODEID_NULL, pathVector);
	BOOST_CHECK(UA_NodeId_isNull(&folderNodeId));
	
	folderNodeId = adapter->createFolderPath(ownNodeId, pathVector);
	BOOST_CHECK(!UA_NodeId_isNull(&folderNodeId));
	
	folderNodeId = adapter->existFolderPath(ownNodeId, pathVector);
	BOOST_CHECK(!UA_NodeId_isNull(&folderNodeId));
	
	// Double creation of folder, should be the same folder nodeid
	UA_NodeId existingFolderNodeId = adapter->createFolderPath(ownNodeId, pathVector);
	BOOST_CHECK(UA_NodeId_equal(&existingFolderNodeId, &folderNodeId));
		
	folderNodeId = adapter->existFolderPath(UA_NODEID_NULL, pathVector);
	BOOST_CHECK(UA_NodeId_isNull(&folderNodeId));
	
	adapter->addConstant("int8Scalar", tfExampleSet.csManager);
	adapter->addVariable("int32Array_s15", tfExampleSet.csManager);
	adapter->addVariable("uint8Array_s10", tfExampleSet.csManager);
	adapter->addVariable("uint16Array_s10", tfExampleSet.csManager);
	adapter->addVariable("int8Array_s15", tfExampleSet.csManager);
	adapter->addVariable("floatScalar", tfExampleSet.csManager);

	BOOST_CHECK(adapter->getConstants().size() > 0);
	BOOST_CHECK(adapter->getVariables().size() > 0);
		
	adapter->~mtca_uaadapter();
		
};


/**
   * The boost test suite which executes the ProcessVariableTest.
   */
class UAAdapterTestSuite: public test_suite {
	public:
		UAAdapterTestSuite() : test_suite("mtca_uaadapter Test Suite") {
			add(BOOST_TEST_CASE(&UAAdapterTest::testEmptySet));
			add(BOOST_TEST_CASE(&UAAdapterTest::testExampleSet));
    }
};


test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	
	 framework::master_test_suite().add(new UAAdapterTestSuite);

    return 0;
}
