#include <csa_opcua_adapter.h>
#include <ua_adapter.h>
#include <xml_file_handler.h>
#include <test_sample_data.h>

#include <boost/test/included/unit_test.hpp>

#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "ChimeraTK/ControlSystemAdapter/DevicePVManager.h"
#include "ChimeraTK/ControlSystemAdapter/PVManager.h"
#include "ChimeraTK/ControlSystemAdapter/ControlSystemSynchronizationUtility.h"

using namespace boost::unit_test_framework;
using namespace std;

class UAMappingTest {
public:
    static void testExampleSet();
};

void UAMappingTest::testExampleSet() {
    cout << "General PV mapping test." << endl;
    TestFixturePVSet tfExampleSet;
    csa_opcua_adapter *csaOPCUA = new csa_opcua_adapter(tfExampleSet.csManager, "../tests/uamapping_test_general.xml");
    // is Server running?
    BOOST_CHECK(csaOPCUA->isRunning());
    // is csManager init
    BOOST_CHECK(csaOPCUA->getControlSystemManager()->getAllProcessVariables().size() == 21);

    ua_uaadapter *uaadapter = csaOPCUA->getUAAdapter();

    UA_NodeId result = UA_NODEID_NULL;
    UA_Server_readNodeId(uaadapter->getMappedServer(), UA_NODEID_STRING(1, (char *) "llrfCtrl_hzdr/1/FOLDER"), &result);
    BOOST_CHECK(UA_NodeId_isNull(&result) == UA_FALSE);
    // check if target folder for copy with source was created
    result = UA_NODEID_NULL;
    UA_Server_readNodeId(uaadapter->getMappedServer(), UA_NODEID_STRING(1, (char *) "llrfCtrl_hzdr/copyWithSourceTest"), &result);
    BOOST_CHECK(UA_NodeId_isNull(&result) == UA_FALSE);
    result = UA_NODEID_NULL;
    UA_Server_readNodeId(uaadapter->getMappedServer(), UA_NODEID_STRING(1, (char *) "llrfCtrl_hzdr/copyWithSourceTest/defaultSep"), &result);
    BOOST_CHECK(UA_NodeId_isNull(&result) == UA_FALSE);
    result = UA_NODEID_NULL;
    UA_Server_readNodeId(uaadapter->getMappedServer(), UA_NODEID_STRING(1, (char *) "llrfCtrl_hzdr/copyWithSourceTest/defaultSep/stringScalarValue"), &result);
    BOOST_CHECK(UA_NodeId_isNull(&result) == UA_FALSE);

    result = UA_NODEID_NULL;
    UA_Server_readNodeId(uaadapter->getMappedServer(), UA_NODEID_STRING(1, (char *) "llrfCtrl_hzdr/linkWithSourceTest"), &result);
    BOOST_CHECK(UA_NodeId_isNull(&result) == UA_FALSE);
    UA_BrowseDescription bd;
    bd.includeSubtypes = false;
    bd.nodeId = UA_NODEID_STRING(1, (char *) "llrfCtrl_hzdr/linkWithSourceTest");
    bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    bd.resultMask = UA_BROWSERESULTMASK_BROWSENAME;
    bd.nodeClassMask = UA_NODECLASS_OBJECT;
    bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    UA_BrowseResult br = UA_Server_browse(uaadapter->getMappedServer(), 1000, &bd);
    BOOST_CHECK(br.referencesSize > 0);
    for (size_t j = 0; j < br.referencesSize; ++j) {
        UA_String compareString = UA_STRING((char *) "defaultSep");
        BOOST_CHECK(UA_String_equal(&br.references[j].browseName.name, &compareString) == UA_TRUE);
    }
    UA_BrowseResult_deleteMembers(&br);

    bd.nodeId = UA_NODEID_STRING(1, (char *) "llrfCtrl_hzdr/1/FOLDER");
    bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    bd.resultMask = UA_BROWSERESULTMASK_BROWSENAME;
    bd.nodeClassMask = UA_NODECLASS_VARIABLE;
    bd.browseDirection = UA_BROWSEDIRECTION_BOTH;
    br = UA_Server_browse(uaadapter->getMappedServer(), 1000, &bd);
    BOOST_CHECK(br.referencesSize > 0);
    for (size_t j = 0; j < br.referencesSize; ++j) {
        UA_String compareString = UA_STRING((char *) "stringScalar");
        BOOST_CHECK(UA_String_equal(&br.references[j].browseName.name, &compareString) == UA_TRUE);
    }
    UA_BrowseResult_deleteMembers(&br);

    csaOPCUA->stop();
    BOOST_CHECK(csaOPCUA->isRunning() != true);
    csaOPCUA->~csa_opcua_adapter();
}

class UAMappingTestSuite : public test_suite {
public:
    UAMappingTestSuite() : test_suite("PV mapping Test Suite") {
        add(BOOST_TEST_CASE(&UAMappingTest::testExampleSet));
    }
};

test_suite *
init_unit_test_suite(int argc, char *argv[]) {
    framework::master_test_suite().add(new UAMappingTestSuite);
    return 0;
}
