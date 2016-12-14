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
};
   
void UAAdapterTest::testExampleSet() { 
	cout << "UAAdapterTest with ExampleSet started." << endl;
	TestFixturePVSet tfExampleSet;
	 // Create the managers
	mtca_uaadapter *adapter = new mtca_uaadapter("./uamapping_test_2.xml");
	xml_file_handler *xmlHandler = new xml_file_handler("./uamapping_test_2.xml");
		
	// Test config handling
	BOOST_CHECK_THROW(mtca_uaadapter mtca_uaadapter("./uamapping_test_twoconfigs.xml"), runtime_error)
	cout << "Eingetretten" << endl;
	BOOST_CHECK_THROW(mtca_uaadapter mtca_uaadapter("./uamapping_test_applicationismissing.xml"), runtime_error)
		cout << "EingetrettenTTTTTTTTTTTTTTTTTTTTEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << endl;
	BOOST_CHECK_THROW(mtca_uaadapter mtca_uaadapter("./uamapping_test_configismissing.xml"), runtime_error)
	BOOST_CHECK_THROW(mtca_uaadapter mtca_uaadapter("./uamapping_test_portismissung.xml"), runtime_error)
	
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
		adapter->addVariable(processVar.get()->getName() , tfExampleSet.csManager);
	}

	adapter->addConstant("int8Scalar", tfExampleSet.csManager);

	BOOST_CHECK(adapter->getConstants().size() > 0);
	BOOST_CHECK(adapter->getVariables().size() > 0);
	
	/* Check if both var are not mapped */
	cout << "Größe von: " << adapter->getAllNotMappableVariablesNames().size() << endl;
	BOOST_CHECK(adapter->getAllNotMappableVariablesNames().size() == 6);
	// Check if timestamp is not enmpty
	string dateTime = "";
	UASTRING_TO_CPPSTRING(UA_DateTime_toString(adapter->getSourceTimeStamp()), dateTime);
	BOOST_CHECK(dateTime != "");
		
	adapter->~mtca_uaadapter();
	free(adapter);
};

/**
   * The boost test suite which executes the ProcessVariableTest.
   */
class UAAdapterTestSuite: public test_suite {
	public:
		UAAdapterTestSuite() : test_suite("mtca_uaadapter Test Suite") {
			add(BOOST_TEST_CASE(&UAAdapterTest::testExampleSet));
    }
};


test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	
	 framework::master_test_suite().add(new UAAdapterTestSuite);

    return 0;
}
