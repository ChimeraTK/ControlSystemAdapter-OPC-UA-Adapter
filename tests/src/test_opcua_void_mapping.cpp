// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <open62541/types.h>

#include <boost/test/included/unit_test.hpp>

#include <csa_opcua_adapter.h>
#include <test_sample_data.h>
#include <ua_adapter.h>
#include <xml_file_handler.h>

#include <string>

using namespace boost::unit_test_framework;
using namespace std;

class UAMappingTest {
 public:
  static void testExampleSetBool();
  static void testExampleSetMethod();
};

void checkNode(const std::string& node, std::shared_ptr<ua_uaadapter> uaadapter) {
  UA_NodeId result = UA_NODEID_NULL;
  UA_Server_readNodeId(uaadapter->getMappedServer(), UA_NODEID_STRING(1, const_cast<char*>(node.c_str())), &result);
  BOOST_CHECK(UA_NodeId_isNull(&result) == UA_FALSE);
  UA_NodeId_clear(&result);
}

void checkReference(const std::string& node, const std::string& folder, const std::string& reference,
    std::shared_ptr<ua_uaadapter> uaadapter) {
  UA_BrowseDescription bd;
  bd.nodeId = UA_NODEID_STRING(1, const_cast<char*>(folder.c_str()));
  bd.referenceTypeId = UA_NS0ID(HASCOMPONENT);
  bd.resultMask = UA_BROWSERESULTMASK_BROWSENAME;
  bd.nodeClassMask = UA_NODECLASS_VARIABLE | UA_NODECLASS_METHOD;
  bd.browseDirection = UA_BROWSEDIRECTION_BOTH;
  UA_BrowseResult br = UA_Server_browse(uaadapter->getMappedServer(), 1000, &bd);
  BOOST_CHECK(br.referencesSize > 0);
  auto referenceNode = UA_NODEID_STRING(1, const_cast<char*>(reference.c_str()));
  for(size_t j = 0; j < br.referencesSize; ++j) {
    UA_String compareString = UA_STRING((char*)node.c_str());
    if(UA_String_equal(&br.references[j].browseName.name, &compareString) == UA_TRUE) {
      if(UA_NodeId_equal(&br.references[j].nodeId.nodeId, &referenceNode) == UA_TRUE) {
        UA_BrowseResult_clear(&br);
        BOOST_CHECK(true);
        return;
      }
    }
  }
  UA_BrowseResult_clear(&br);
  BOOST_CHECK(false);
}

void UAMappingTest::testExampleSetBool() {
  cout << "General PV mapping test." << endl;
  TestFixturePVSet tfExampleSet;
  csa_opcua_adapter csaOPCUA(tfExampleSet.csManager, "uamapping_test_void_bool.xml");
  // is Server running?
  csaOPCUA.start();
  BOOST_CHECK(csaOPCUA.isRunning());
  while(!csaOPCUA.getUAAdapter()->running) {
  };
  BOOST_CHECK(csaOPCUA.getUAAdapter()->running);
  // is csManager init
  BOOST_CHECK(csaOPCUA.getControlSystemManager()->getAllProcessVariables().size() == 25);

  std::shared_ptr<ua_uaadapter> uaadapter = csaOPCUA.getUAAdapter();

  checkNode("test_void/Dieser/Name/ist/voidScalar", uaadapter);
  checkNode("test_void/MappedDir", uaadapter);
  // checkNode("test_void/Mapped/voidScalar", uaadapter);
  checkReference("voidScalar", "test_void/MappedDir", "test_void/Dieser/Name/ist/voidScalar", uaadapter);
  checkNode("test_void/MappedCopyDir", uaadapter);
  checkNode("test_void/MappedCopy/voidScalarCopy", uaadapter);
  checkNode("test_void/MappedFolderDir", uaadapter);
  // checkNode("test_void/MappedFolder/voidScalar", uaadapter);
  checkReference("voidScalar", "test_void/MappedFolderDir", "test_void/Dieser/Name/ist/voidScalar", uaadapter);
  checkNode("test_void/MappedFolderCopyDir", uaadapter);
  checkNode("test_void/MappedFolderCopy/voidScalar", uaadapter);

  csaOPCUA.stop();
  BOOST_CHECK(csaOPCUA.isRunning() != true);
}

void UAMappingTest::testExampleSetMethod() {
  cout << "General PV mapping test." << endl;
  TestFixturePVSet tfExampleSet;
  csa_opcua_adapter csaOPCUA(tfExampleSet.csManager, "uamapping_test_void_method.xml");
  // is Server running?
  csaOPCUA.start();
  BOOST_CHECK(csaOPCUA.isRunning());
  while(!csaOPCUA.getUAAdapter()->running) {
  };
  BOOST_CHECK(csaOPCUA.getUAAdapter()->running);
  // is csManager init
  BOOST_CHECK(csaOPCUA.getControlSystemManager()->getAllProcessVariables().size() == 25);

  std::shared_ptr<ua_uaadapter> uaadapter = csaOPCUA.getUAAdapter();

  checkNode("test_void/Dieser/Name/ist/voidScalar", uaadapter);
  checkNode("test_void/MappedDir", uaadapter);
  // checkNode("test_void/Mapped/voidScalar", uaadapter);
  checkReference("voidScalar", "test_void/MappedDir", "test_void/Dieser/Name/ist/voidScalar", uaadapter);
  checkNode("test_void/MappedCopyDir", uaadapter);
  checkNode("test_void/MappedCopy/voidScalarCopy", uaadapter);
  checkNode("test_void/MappedFolderDir", uaadapter);
  // checkNode("test_void/MappedFolder/voidScalar", uaadapter);
  checkReference("voidScalar", "test_void/MappedFolderDir", "test_void/Dieser/Name/ist/voidScalar", uaadapter);
  checkNode("test_void/MappedFolderCopyDir", uaadapter);
  // We don't copy in case folders are used
  checkReference("voidScalar", "test_void/MappedFolderCopyDir", "test_void/Dieser/Name/ist/voidScalar", uaadapter);

  csaOPCUA.stop();
  BOOST_CHECK(csaOPCUA.isRunning() != true);
}

class UAMappingTestSuite : public test_suite {
 public:
  UAMappingTestSuite() : test_suite("PV mapping Test Suite") {
    add(BOOST_TEST_CASE(&UAMappingTest::testExampleSetBool));
    add(BOOST_TEST_CASE(&UAMappingTest::testExampleSetMethod));
  }
};

test_suite* init_unit_test_suite(int /*argc*/, char** /*argv[]*/) {
  framework::master_test_suite().add(new UAMappingTestSuite);
  return 0;
}