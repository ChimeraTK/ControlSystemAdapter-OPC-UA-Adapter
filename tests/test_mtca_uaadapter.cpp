#include <mtca_uaadapter.h>

#include <boost/test/included/unit_test.hpp>

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"

using namespace boost::unit_test_framework;

struct TestFixtureInitManager {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers;
  boost::shared_ptr<ControlSystemPVManager> csManager;
  boost::shared_ptr<DevicePVManager> devManager;
  
  uint32_t opcuaPort;
  
  ControlSystemSynchronizationUtility csSyncUtil;

  TestFixtureInitManager() : pvManagers(createPVManager()),csManager(pvManagers.first), devManager(pvManagers.second), csSyncUtil(csManager) {
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
		
		ProcessScalar<int32_t>::SharedPtr intAdev = devManager->createProcessScalar<int32_t>(deviceToControlSystem, "intA");
		ProcessScalar<uint32_t>::SharedPtr intBdev = devManager->createProcessScalar<uint32_t>(controlSystemToDevice, "intB");
    
		ProcessArray<int32_t>::SharedPtr intC1dev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "intC1", 15);
		ProcessArray<int32_t>::SharedPtr intC2dev = devManager->createProcessArray<int32_t>(controlSystemToDevice, "intC2", 10);
	}
};


class UAAdapterTest {
	public:
		static void testEmptySet();
		static void testExampleSet();
};
   
void UAAdapterTest::testEmptySet(){ 
	TestFixtureInitManager tfInitManager;
	mtca_uaadapter *adapter = new mtca_uaadapter(tfInitManager.opcuaPort);
	
	// is Server running?
	adapter->doStart();
	BOOST_CHECK(adapter->isRunning() == true);
	
	adapter->doStop();
	BOOST_CHECK(adapter->isRunning() != true);
	
	adapter->doStart();
	BOOST_CHECK(adapter->isRunning() == true);
	
	// is csManager init
	//BOOST_CHECK(csaOPCUA->getControlSystemPVManager()->getAllProcessVariables().size() == 0);
	
	//BOOST_CHECK(csaOPCUA->getOPCUAPort() == 16664);
};

void UAAdapterTest::testExampleSet(){ 
	TestFixtureExampleSet tfExampleSet;
	 // Create the managers

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

