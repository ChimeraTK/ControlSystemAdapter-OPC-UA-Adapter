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
	
	mtca_uaadapter *adapterOne = new mtca_uaadapter(16661);
	adapterOne->setIpcId(100);
	mtca_uaadapter *adapterTwo = new mtca_uaadapter(16662);	
	
	manager->addObject(adapterOne);
	manager->addObject(adapterTwo);
	
	BOOST_CHECK(adapterOne->isRunning() == true);
	BOOST_CHECK(adapterTwo->isRunning() == true);
	
	BOOST_CHECK(adapterOne->isManaged() == true);
	BOOST_CHECK(adapterTwo->isManaged() == true);
	
	std::cout << "test" << std::endl;
	
	//manager->startAll();
	//BOOST_CHECK(manager->isRunning() == true);
	//BOOST_CHECK(manager->isManaged() == true);
	
	//list<ipc_managed_object*> listOfManaedObjects = manager->getAllObjectsByType();
	
	/*
	 * Currently not supported, mehtod returns a nullptr
	 */
	//ipc_managed_object* adapter = manager->getObjectById(100);
	//BOOST_CHECK(adapter->getIpcId() == 100);
	
	//adapterOne->workerThread();
	//adapterOne->taskRunningAttached();
	//adapterOne->doStop();
	std::cout << "test2" << std::endl;
	//adapterTwo->terminate();
	
	/*
	 * currently not supported
	 */
	//ipc_manager *mgr = adapterOne->getIpcManager();
	
	std::cout << "test4" << std::endl;
	//manager->deleteObject(100);
	std::cout << "test5" << std::endl;
	//manager->doStop();
	
	//ipc_manager *newManager = new ipc_manager();
	//adapterOne->assignManager(newManager);
	std::cout << "test6" << std::endl;
	//adapterOne->terminate();
	
	//ipc_managed_object_type moType = adapterOne->getManagedObjectType();
	
	//manager->stopAll();
	//manager->terminate();
		
	std::cout << "test7" << std::endl;
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
// 	
	//manager.getAllObjectsByType();
	
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

