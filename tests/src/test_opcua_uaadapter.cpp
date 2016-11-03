#include <mtca_uaadapter.h>
#include <xml_file_handler.h>

#include <test_sample_data.h>

#include <boost/test/included/unit_test.hpp>

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"

using namespace boost::unit_test_framework;
using namespace std;


class UAAdapterTest {
	public:
		static void testEmptySet();
		static void testExampleSet();
		static void testConfigHandling();
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

void UAAdapterTest::testExampleSet() { 
	cout << "UAAdapterTest with ExampleSet started." << endl;
	TestFixturePVSet tfExampleSet;
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

void UAAdapterTest::testConfigHandling() {
	
	
	
};


/**
   * The boost test suite which executes the ProcessVariableTest.
   */
class UAAdapterTestSuite: public test_suite {
	public:
		UAAdapterTestSuite() : test_suite("mtca_uaadapter Test Suite") {
			add(BOOST_TEST_CASE(&UAAdapterTest::testEmptySet));
			add(BOOST_TEST_CASE(&UAAdapterTest::testExampleSet));
			add(BOOST_TEST_CASE(&UAAdapterTest::testConfigHandling));
    }
};


test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	
	 framework::master_test_suite().add(new UAAdapterTestSuite);

    return 0;
}
