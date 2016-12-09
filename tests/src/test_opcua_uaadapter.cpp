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
		static void testExampleSet();
		static void testConfigHandling();
};
   
void UAAdapterTest::testExampleSet() { 
	cout << "UAAdapterTest with ExampleSet started." << endl;
	TestFixturePVSet tfExampleSet;
	 // Create the managers
	mtca_uaadapter *adapter = new mtca_uaadapter("../../tests/uamapping_test_2.xml");
	xml_file_handler *xmlHandler = new xml_file_handler("../../tests/uamapping_test_2.xml");
	
// 	try {
// 		mtca_uaadapter *adapter12 = new mtca_uaadapter("../../tests/uamapping_test_3.xml");
// 	}
// 	catch(char const* msg) {
// 		cout << msg << endl;
// 		BOOST_CHECK(true);
// 	}
	BOOST_CHECK_THROW(mtca_uaadapter mtca_uaadapter("../../tests/uamapping_test_3.xml"), runtime_error)
	
	// is Server running?
	adapter->doStart();
	BOOST_CHECK(adapter->isRunning() == true);
	/*
	adapter->doStop();
	BOOST_CHECK(adapter->isRunning() != true);
	
	adapter->doStart();
	BOOST_CHECK(adapter->isRunning() == true);*/
	
	BOOST_CHECK(adapter->getIpcId() == 0);
	UA_NodeId ownNodeId = adapter->getOwnNodeId();
	BOOST_CHECK(!UA_NodeId_isNull(&ownNodeId));
	
	// Check folder functions
	vector<string> pathVector = xmlHandler->praseVariablePath("/test/test/");
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
	
	for(auto processVar:tfExampleSet.csManager.get()->getAllProcessVariables()) {
		adapter->addVariable(processVar.get()->getName() , tfExampleSet.csManager, tfExampleSet.syncDevUtility);
	}

	adapter->addConstant("int8Scalar", tfExampleSet.csManager, tfExampleSet.syncDevUtility);

	BOOST_CHECK(adapter->getConstants().size() > 0);
	BOOST_CHECK(adapter->getVariables().size() > 0);
	
	/* Check if both var are not mapped */
	cout << "Größe von: " << adapter->getAllNotMappableVariablesNames().size() << endl;
	BOOST_CHECK(adapter->getAllNotMappableVariablesNames().size() == 6);
		
	adapter->~mtca_uaadapter();
	free(adapter);
};

void UAAdapterTest::testConfigHandling() {
// 	cout << "UAAdapterTest with ExampleSet started." << endl;
// 	TestFixturePVSet tfExampleSet;
// 	 // Create the managers
// 	mtca_uaadapter *adapter = new mtca_uaadapter("../../tests/uamapping_test.xml", "10002");
// 	xml_file_handler *xmlHandler = new xml_file_handler("../../tests/uamapping_test.xml");
// 	
// 	adapter->doStart();
// 		
	//while(true) {}

	
};


/**
   * The boost test suite which executes the ProcessVariableTest.
   */
class UAAdapterTestSuite: public test_suite {
	public:
		UAAdapterTestSuite() : test_suite("mtca_uaadapter Test Suite") {
			add(BOOST_TEST_CASE(&UAAdapterTest::testExampleSet));
			add(BOOST_TEST_CASE(&UAAdapterTest::testConfigHandling));
    }
};


test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	
	 framework::master_test_suite().add(new UAAdapterTestSuite);

    return 0;
}
