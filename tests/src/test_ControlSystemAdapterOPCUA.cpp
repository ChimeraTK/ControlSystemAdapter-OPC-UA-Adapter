
#include <mtca_uaadapter.h>
#include <ControlSystemAdapterOPCUA.h>
#include "ipc_manager.h"

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

ipc_manager  *mgr = new ipc_manager(); // Global, um vom Signalhandler stopbar zu sein

struct TestFixtureEmptySet {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers;
  boost::shared_ptr<ControlSystemPVManager> csManager;
  boost::shared_ptr<DevicePVManager> devManager;
  
  uint32_t opcuaPort;
  
  ControlSystemSynchronizationUtility csSyncUtil;

  TestFixtureEmptySet() : pvManagers(createPVManager()),csManager(pvManagers.first), devManager(pvManagers.second), csSyncUtil(csManager) {
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
		ProcessArray<uint32_t>::SharedPtr intB10Au32dev = devManager->createProcessArray<uint32_t>(controlSystemToDevice, "uint32Array_s10", 10);
		ProcessArray<double>::SharedPtr intB15Afdev = devManager->createProcessArray<double>(controlSystemToDevice, "doubleArray_s15", 15);
		ProcessArray<float>::SharedPtr intB10Addev = devManager->createProcessArray<float>(controlSystemToDevice, "floatArray_s10", 10);
		
		
	}
};


class CSAOPCUATest {
	public:
		static void testEmptySet();
		static void testExampleSet();
};
   
void CSAOPCUATest::testEmptySet(){ 
	std::cout << "Enter CSAOPCUATest with EmptySet" << std::endl;
	TestFixtureEmptySet tfEmptySet;
	 // Create the managers
	ControlSystemAdapterOPCUA *csaOPCUA = new ControlSystemAdapterOPCUA(tfEmptySet.opcuaPort, tfEmptySet.csManager, "../../tests/uamapping_test.xml");
	// is Server running?
	csaOPCUA->start();
	BOOST_CHECK(csaOPCUA->isRunning() == true);
	// is csManager init
	BOOST_CHECK(csaOPCUA->getControlSystemPVManager()->getAllProcessVariables().size() == 0);
	
	BOOST_CHECK(csaOPCUA->getOPCUAPort() == 16664);	
	
	csaOPCUA->stop();
	csaOPCUA->terminate();
	BOOST_CHECK(csaOPCUA->isRunning() != true);
};

void CSAOPCUATest::testExampleSet(){ 
	std::cout << "Enter CSAOPCUATest with ExampleSet" << std::endl;
	TestFixtureExampleSet tfExampleSet;
	 // Create the managers
	ControlSystemAdapterOPCUA *csaOPCUA = new ControlSystemAdapterOPCUA(tfExampleSet.opcuaPort, tfExampleSet.csManager, "../../tests/uamapping_test.xml");
	// is Server running?
	BOOST_CHECK(csaOPCUA->isRunning() == true);
	
	// is csManager init
	BOOST_CHECK(csaOPCUA->getControlSystemPVManager()->getAllProcessVariables().size() == 16);
	
	BOOST_CHECK(csaOPCUA->getOPCUAPort() == 16664);
			
	BOOST_CHECK(csaOPCUA->getIPCManager() != NULL);
	
	BOOST_CHECK(csaOPCUA->getUAAdapter() != NULL);
	
	csaOPCUA->stop();
	csaOPCUA->terminate();
	BOOST_CHECK(csaOPCUA->isRunning() != true);
};


/**
   * The boost test suite which executes the ProcessVariableTest.
   */
class CSAOPCUATestSuite: public test_suite {
	public:
		CSAOPCUATestSuite() : test_suite("ControlSystemAdapterOPCUA Test Suite") {
			add(BOOST_TEST_CASE(&CSAOPCUATest::testEmptySet));
			add(BOOST_TEST_CASE(&CSAOPCUATest::testExampleSet));
    }
};


test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	
	 framework::master_test_suite().add(new CSAOPCUATestSuite);

    return 0;
}

