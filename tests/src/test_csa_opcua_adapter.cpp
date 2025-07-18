#include <boost/test/included/unit_test.hpp>

#include <csa_opcua_adapter.h>
#include <test_sample_data.h>

#include <iostream>

extern "C" {
#include "unistd.h"
}

using namespace boost::unit_test_framework;
using namespace std;

class CSAOPCUATest {
 public:
  static void testWithoutPVSet();

  static void testWithPVSet();
};

void CSAOPCUATest::testWithoutPVSet() {
  cout << "Enter CSAOPCUATest without any pv" << std::endl;
  TestFixtureEmptySet tfEmptySet;
  // Create the managers
  csa_opcua_adapter csaOPCUA(tfEmptySet.csManager, "uamapping_test_1.xml");

  // is Server running?
  csaOPCUA.start();
  while(!csaOPCUA.getUAAdapter()->running) {
  }
  BOOST_CHECK(csaOPCUA.isRunning());
  BOOST_CHECK(csaOPCUA.getUAAdapter()->running);
  // is csManager init
  BOOST_CHECK(csaOPCUA.getControlSystemManager()->getAllProcessVariables().size() == 0);

  csaOPCUA.stop();
  BOOST_CHECK(!csaOPCUA.isRunning());
}

void CSAOPCUATest::testWithPVSet() {
  std::cout << "Enter CSAOPCUATest with ExampleSet" << std::endl;
  TestFixturePVSet tfExampleSet;
  csa_opcua_adapter csaOPCUA(tfExampleSet.csManager, "../tests/uamapping_test_2.xml");
  csaOPCUA.start();
  BOOST_CHECK(csaOPCUA.isRunning());
  while(!csaOPCUA.getUAAdapter()->running) {
  }
  BOOST_CHECK(csaOPCUA.getUAAdapter()->running);
  // is csManager init
  BOOST_CHECK(csaOPCUA.getControlSystemManager()->getAllProcessVariables().size() == 21);

  BOOST_CHECK(csaOPCUA.getUAAdapter() != NULL);

  csaOPCUA.stop();
  BOOST_CHECK(!csaOPCUA.isRunning());
}

/**
 * The boost test suite which executes the ProcessVariableTest.
 */
class CSAOPCUATestSuite : public test_suite {
 public:
  CSAOPCUATestSuite() : test_suite("csa_opcua_adapter Test Suite") {
    add(BOOST_TEST_CASE(&CSAOPCUATest::testWithoutPVSet));
    add(BOOST_TEST_CASE(&CSAOPCUATest::testWithPVSet));
  }
};

test_suite* init_unit_test_suite(int /*argc*/, char** /*argv[]*/) {
  framework::master_test_suite().add(new CSAOPCUATestSuite);
  return 0;
}
