#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include <test_sample_data.h>
#include <ua_adapter.h>
#include <xml_file_handler.h>

#include <boost/test/included/unit_test.hpp>

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
  ua_uaadapter* adapter = new ua_uaadapter("./uamapping_test_2.xml");
  xml_file_handler* xmlHandler = new xml_file_handler("./uamapping_test_2.xml");

  // Test config handling
  BOOST_CHECK_THROW(ua_uaadapter("./uamapping_test_twoconfigs.xml"), std::runtime_error);

  ua_uaadapter* ad1 = new ua_uaadapter("./uamapping_test_applicationismissing.xml");
  ad1->~ua_uaadapter();
  ad1 = new ua_uaadapter("./uamapping_test_configismissing.xml");
  ad1->~ua_uaadapter();
  ad1 = new ua_uaadapter("./uamapping_test_notwellformed.xml");
  ad1->~ua_uaadapter();
  ad1 = new ua_uaadapter("./uamapping_test_portismissing.xml");
  ad1->~ua_uaadapter();

  // is Server running?
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

  for(auto processVar : tfExampleSet.csManager.get()->getAllProcessVariables()) {
    adapter->implicitVarMapping(processVar.get()->getName(), tfExampleSet.csManager);
  }

  BOOST_CHECK(adapter->getVariables().size() > 0);

  /* Check if both var are not mapped */
  cout << "Größe von: " << adapter->getAllNotMappableVariablesNames().size() << endl;
  BOOST_CHECK(adapter->getAllNotMappableVariablesNames().size() == 7);
  // Check if timestamp is not enmpty
  string dateTime = "";
  BOOST_CHECK(adapter->getSourceTimeStamp() != 0);

  adapter->~ua_uaadapter();
  xmlHandler->~xml_file_handler();
}

class UAAdapterTestSuite : public test_suite {
 public:
  UAAdapterTestSuite() : test_suite("ua_uaadapter Test Suite") { add(BOOST_TEST_CASE(&UAAdapterTest::testExampleSet)); }
};

test_suite* init_unit_test_suite(int argc, char* argv[]) {
  framework::master_test_suite().add(new UAAdapterTestSuite);
  return 0;
}
