#include "ipc_manager.h"

#include <boost/test/included/unit_test.hpp>

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"

#include "mtca_uaadapter.h"


using namespace boost::unit_test_framework;

class IPCManagerTest {
	public:
		static void testManagerConnection();
};
   
void IPCManagerTest::testManagerConnection(){ 
	std::cout << "Enter IPCManagerTest" << std::endl;
	
	ipc_manager *manager = new ipc_manager();
	
	mtca_uaadapter *adapterOne = new mtca_uaadapter(16660, "../../tests/uamapping_test.xml");
	mtca_uaadapter *adapterTwo = new mtca_uaadapter(16661, "../../tests/uamapping_test.xml");	
	
	BOOST_CHECK(adapterTwo->isManaged() == false);
	
	uint32_t adapOneIpcId = manager->addObject(adapterOne);
	uint32_t adapTwoIpcId = manager->addObject(adapterTwo);
	
	BOOST_CHECK(adapterTwo->isManaged() == true);

	BOOST_CHECK(adapterOne->getIpcId() == adapOneIpcId);
	BOOST_CHECK(adapterTwo->getIpcId() == adapTwoIpcId);
	
 	adapterTwo->setIpcId(20);
	BOOST_CHECK(adapterTwo->getIpcId() == 20);
	
	BOOST_CHECK(adapterOne->isRunning() == true);
	BOOST_CHECK(adapterTwo->isRunning() == true);
	
	BOOST_CHECK(adapterOne->isManaged() == true);
	BOOST_CHECK(adapterTwo->isManaged() == true);
	
	manager->stopAll();
	BOOST_CHECK(adapterOne->isRunning() == false);
	BOOST_CHECK(adapterTwo->isRunning() == false);
	
	manager->startAll();
	BOOST_CHECK(adapterOne->isRunning() == true);
	BOOST_CHECK(adapterTwo->isRunning() == true);

	BOOST_CHECK(adapterOne->taskRunningAttached() == 1);
	adapterOne->doStop();
 	BOOST_CHECK(adapterOne->terminate() == 0);
 	BOOST_CHECK(adapterTwo->terminate() == 0);

	ipc_manager *mgr = adapterOne->getIpcManager();
	manager->deleteObject(adapOneIpcId);
	BOOST_CHECK(manager->deleteObject(adapOneIpcId) == 0);
	
	ipc_manager *newManager = new ipc_manager();
	BOOST_CHECK(adapterOne->assignManager(newManager) == true);
 	BOOST_CHECK(adapterOne->assignManager(nullptr) == false);
		
	//mtca_uaadapter *adapterThree = new mtca_uaadapter(16662, "../../tests/uamapping_test.xml");
	BOOST_CHECK(true);
	//manager->addObject(adapterThree);
	BOOST_CHECK(true);
	manager->~ipc_manager();
};


/**
   * The boost test suite which executes the ProcessVariableTest.
   */
class IPCManagerTestSuite: public test_suite {
	public:
		IPCManagerTestSuite() : test_suite("IPCManager Test Suite") {
			add(BOOST_TEST_CASE(&IPCManagerTest::testManagerConnection));
    }
};


test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	
	 framework::master_test_suite().add(new IPCManagerTestSuite);

    return 0;
}

