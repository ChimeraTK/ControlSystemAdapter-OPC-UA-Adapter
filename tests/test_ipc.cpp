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
	
	mtca_uaadapter *adapterOne = new mtca_uaadapter(16661);
	mtca_uaadapter *adapterTwo = new mtca_uaadapter(16662);
	
	//ipc_managed_object managedObj = new ipc_managed_object();
	//ipc_task *taskOne = new ipc_task();
// 	taskOne->execute();
// 	taskOne->hasCompleted();
// 	
// 	taskOne->getManagedObjectType();
// 	
// 	ipc_task *taskTwo = new ipc_task();
	
	//testTaskClass *taskOne = new testTaskClass();
	

	ipc_manager *manager = new ipc_manager();
	
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

