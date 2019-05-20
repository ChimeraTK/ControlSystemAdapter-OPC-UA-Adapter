#include "ipc_manager.h"

#include <boost/test/included/unit_test.hpp>

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"

#include "ua_adapter.h"


using namespace boost::unit_test_framework;

class IPCManagerTest {
public:
    static void testManagerConnection();
};

void IPCManagerTest::testManagerConnection() {
    std::cout << "Enter IPCManagerTest" << std::endl;

    ipc_manager *manager = new ipc_manager();

    ua_uaadapter *adapterOne = new ua_uaadapter("../tests/uamapping_test_1.xml");
    ua_uaadapter *adapterTwo = new ua_uaadapter("../tests/uamapping_test_2.xml");

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

    ipc_manager *mgr = adapterOne->getIpcManager();
    BOOST_CHECK(mgr != NULL);

    manager->deleteObject(adapOneIpcId);
    BOOST_CHECK(manager->deleteObject(adapOneIpcId) == 0);

    ipc_manager *newManager = new ipc_manager();
    BOOST_CHECK(adapterOne->assignManager(newManager) == true);
    BOOST_CHECK(adapterOne->assignManager(nullptr) == false);

    newManager->~ipc_manager();
    BOOST_CHECK(true);
    manager->~ipc_manager();
    BOOST_CHECK(true);
    mgr = NULL;

}


/**
   * The boost test suite which executes the ProcessVariableTest.
   */
class IPCManagerTestSuite : public test_suite {
public:
    IPCManagerTestSuite() : test_suite("IPCManager Test Suite") {
        add(BOOST_TEST_CASE(&IPCManagerTest::testManagerConnection));
    }
};

test_suite *
init_unit_test_suite(int argc, char *argv[]) {
    framework::master_test_suite().add(new IPCManagerTestSuite);
    return 0;
}

