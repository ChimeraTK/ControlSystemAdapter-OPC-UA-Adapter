#include <mtca_uaadapter.h>
#include <ControlSystemAdapterOPCUA.h>
#include <ipc_manager.h>
#include <test_sample_data.h>

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

class CSAOPCUATest {
	public:
		static void testWithoutPVSet();
		static void testWithPVSet();
};
   
void CSAOPCUATest::testWithoutPVSet(){ 
	std::cout << "Enter CSAOPCUATest without any pv" << std::endl;
	TestFixtureEmptySet tfEmptySet;
	 // Create the managers
	ControlSystemAdapterOPCUA *csaOPCUA = new ControlSystemAdapterOPCUA(tfEmptySet.csManager, tfEmptySet.syncDevUtility, "../tests/uamapping_test_1.xml");
	// is Server running?
	csaOPCUA->start();
	BOOST_CHECK(csaOPCUA->isRunning() == true);
	// is csManager init
	BOOST_CHECK(csaOPCUA->getControlSystemManager()->getAllProcessVariables().size() == 0);
		
	csaOPCUA->stop();
	csaOPCUA->terminate();
	BOOST_CHECK(csaOPCUA->isRunning() != true);
	
	csaOPCUA->~ControlSystemAdapterOPCUA(); 
};

void CSAOPCUATest::testWithPVSet(){ 
	std::cout << "Enter CSAOPCUATest with ExampleSet" << std::endl;
	TestFixturePVSet tfExampleSet;
	 // Create the managers
	ControlSystemAdapterOPCUA *csaOPCUA = new ControlSystemAdapterOPCUA(tfExampleSet.csManager, tfExampleSet.syncDevUtility, "../tests/uamapping_test_2.xml");
	// is Server running?
	BOOST_CHECK(csaOPCUA->isRunning() == true);
	
	// is csManager init
	BOOST_CHECK(csaOPCUA->getControlSystemManager()->getAllProcessVariables().size() == 16);
			
	BOOST_CHECK(csaOPCUA->getIPCManager() != NULL);
	
	BOOST_CHECK(csaOPCUA->getUAAdapter() != NULL);
	
	csaOPCUA->stop();
	csaOPCUA->terminate();
	BOOST_CHECK(csaOPCUA->isRunning() != true);
	
	csaOPCUA->~ControlSystemAdapterOPCUA();
};


/**
   * The boost test suite which executes the ProcessVariableTest.
   */
class CSAOPCUATestSuite: public test_suite {
	public:
		CSAOPCUATestSuite() : test_suite("ControlSystemAdapterOPCUA Test Suite") {
			add(BOOST_TEST_CASE(&CSAOPCUATest::testWithoutPVSet));
			add(BOOST_TEST_CASE(&CSAOPCUATest::testWithPVSet));
    }
};


test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	
	 framework::master_test_suite().add(new CSAOPCUATestSuite);

    return 0;
}

