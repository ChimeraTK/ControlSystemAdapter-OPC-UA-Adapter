#include "ipc_manager.h"

#include <boost/test/included/unit_test.hpp>

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"

#include "mtca_uaadapter.h"


using namespace boost::unit_test_framework;

ipc_manager  *mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

struct TestFixtureEmptySet {
  

};


class IPCManagerTest {
	public:
		static void testEmptySet();
		static void testExampleSet();
};
   
void IPCManagerTest::testEmptySet(){ 
	std::cout << "Enter IPCManagerTest" << std::endl;
	
	ipc_manager *manager = new ipc_manager();
	
	mtca_uaadapter *adapterOne = new mtca_uaadapter(16660, "../../tests/uamapping_test.xml");
	adapterOne->setIpcId(100);
	mtca_uaadapter *adapterTwo = new mtca_uaadapter(16661, "../../tests/uamapping_test.xml");	
	
	manager->addObject(adapterOne);
	manager->addObject(adapterTwo);
	
	BOOST_CHECK(adapterTwo->getIpcId() != 0);
	
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
	
	//list<ipc_managed_object*> listOfManaedObjects = manager->getAllObjectsByType();
	
	/*
	 * Currently not supported, mehtod returns a nullptr
	 */
	//ipc_managed_object* adapter = manager->getObjectById(100);
	//BOOST_CHECK(adapter->getIpcId() == 100);

	BOOST_CHECK(adapterOne->taskRunningAttached() == 1);
	adapterOne->doStop();
 	BOOST_CHECK(adapterOne->terminate() == 0);
 	BOOST_CHECK(adapterTwo->terminate() == 0);
	
	/*
	 * currently not supported
	 */
	ipc_manager *mgr = adapterOne->getIpcManager();
	
	manager->deleteObject(100);
	
	ipc_manager *newManager = new ipc_manager();
	BOOST_CHECK(adapterOne->assignManager(newManager) == true);
	BOOST_CHECK(adapterOne->assignManager(nullptr) == false);
		
	
	
	
	//ipc_managed_object managedObj = new ipc_managed_object();
	//ipc_task *taskOne = new ipc_task();
// 	taskOne->execute();
// 	taskOne->hasCompleted();
// 	
// 	taskOne->getManagedObjectType();
// 	
// 	ipc_task *taskTwo = new ipc_task();
	
	//testTaskClass *taskOne = new testTaskClass();
 //	manager->addTask(taskOne);
	
	//taskOne->hasCompleted();
	//taskOne->getIpcId();
	//manager->addObject(taskOne);
// 	manager->addTask(taskTwo);


	manager->~ipc_manager();
};

void IPCManagerTest::testExampleSet(){ 

};

/**
   * The boost test suite which executes the ProcessVariableTest.
   */
class IPCManagerTestSuite: public test_suite {
	public:
		IPCManagerTestSuite() : test_suite("IPCManager Test Suite") {
			add(BOOST_TEST_CASE(&IPCManagerTest::testEmptySet));
			add(BOOST_TEST_CASE(&IPCManagerTest::testExampleSet));
    }
};


test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	
	 framework::master_test_suite().add(new IPCManagerTestSuite);

    return 0;
}

