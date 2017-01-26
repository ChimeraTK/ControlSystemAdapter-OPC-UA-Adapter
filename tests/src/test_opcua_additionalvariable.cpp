#include <string.h>
#include <test_sample_data.h>
#include <ua_proxies_typeconversion.h>
#include <ua_additionalvariable.h>
#include "open62541.h"

#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test_framework;

/*
 * ProcessVariableTest
 * 
 */
class AdditionalVariableTest {
	public:
		static void testEmptySet();
		static void testClientSide();
		static void testExampleSet();
		
};
   
void AdditionalVariableTest::testEmptySet(){ 
	std::cout << "Enter AdditionalVariableTest with EmptySet" << std::endl;
	
	TestFixtureServerSet *serverSet = new TestFixtureServerSet;		
	thread *serverThread = new std::thread(UA_Server_run, serverSet->mappedServer, &serverSet->runUAServer);
		
	// check server
	if(serverSet->mappedServer == nullptr) {
		BOOST_CHECK(false);
	}
	
	ua_additionalvariable *addVar1 = new ua_additionalvariable(serverSet->mappedServer, serverSet->baseNodeId, "Name", "Value", "Description"); 
	
	BOOST_CHECK(addVar1->getSourceTimeStamp() != 0);
	BOOST_CHECK(addVar1->getValue() == "Value");
	
		
	serverSet->runUAServer = UA_FALSE;
		
 	if (serverThread->joinable()) {
  		serverThread->join();
  	}
	
	UA_Server_delete(serverSet->mappedServer);

	serverSet->server_nl.deleteMembers(&serverSet->server_nl);

	delete serverSet;
	serverSet = NULL;
	
 	delete serverThread;
 	serverThread = NULL;
	
	//addVar1->~ua_additionalvariable();

}

void AdditionalVariableTest::testClientSide(){ 
	std::cout << "Enter AdditionalVariableTest with ExampleSet and ClientSide testing" << std::endl;
		
	TestFixtureServerSet *serverSet = new TestFixtureServerSet;	
			
	thread *serverThread = new std::thread(UA_Server_run, serverSet->mappedServer, &serverSet->runUAServer);

	// check server
	if(serverSet->mappedServer == nullptr) {
		BOOST_CHECK(false);
	}

	// add set
	ua_additionalvariable *addVar1 = new ua_additionalvariable(serverSet->mappedServer, serverSet->baseNodeId, "Name", "Value", "Description"); 

	// Create client to connect to server
	UA_Client *client = UA_Client_new(UA_ClientConfig_standard);
	string endpointURL = "opc.tcp://localhost:" + to_string(serverSet->opcuaPort);
	UA_StatusCode retval = UA_Client_connect(client, endpointURL.c_str());
		
	if (retval != UA_STATUSCODE_GOOD) {
		std::cout << "Failed to connect to server" << "opc.tcp://localhost:" << serverSet->opcuaPort << std::endl;
		BOOST_CHECK(false);
	}
		
	UA_BrowseRequest bReq;
	UA_BrowseRequest_init(&bReq);
	bReq.requestedMaxReferencesPerNode = 0;
	bReq.nodesToBrowse = UA_BrowseDescription_new();
	bReq.nodesToBrowseSize = 1;
	bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); 
	bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; // return everything
	UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
	
	for (size_t i = 0; i < bResp.resultsSize; ++i) {
		for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {		
		UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
		if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
						
			size_t lenBrowseName = (int)ref->browseName.name.length;
			std::string browseNameFound(reinterpret_cast<char const*>(ref->browseName.name.data), lenBrowseName);
			
			UA_BrowseRequest bReq2;
			UA_BrowseRequest_init(&bReq2);
			bReq2.requestedMaxReferencesPerNode = 0;
			bReq2.nodesToBrowse = UA_BrowseDescription_new();
			bReq2.nodesToBrowseSize = 1;
			bReq2.nodesToBrowse[0].nodeId = ref->nodeId.nodeId; 
			bReq2.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; 
			UA_BrowseResponse bResp2 = UA_Client_Service_browse(client, bReq2);		
			
			UA_NodeId valueNodeId = UA_NODEID_NULL;
			string name = "";
			
			UA_ReferenceDescription *refe;
			for (size_t m = 0; m < bResp2.resultsSize; ++m) {
				for (size_t k = 0; k < bResp2.results[m].referencesSize; ++k) {
					refe = &(bResp2.results[m].references[k]);
					if(refe->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {			
						size_t lenBrowseName2 = (int)refe->browseName.name.length;
						string browseNameFound2(reinterpret_cast<char const*>(refe->browseName.name.data), lenBrowseName2);
							
						if(browseNameFound2 == "Value") {
							valueNodeId = refe->nodeId.nodeId;
							name = browseNameFound;
							//cout << "Checking ProcessVariable: " <<  name << endl;
						}
					}
				}
			}
			UA_BrowseRequest_deleteMembers(&bReq2);
			UA_BrowseResponse_deleteMembers(&bResp2);
					
				if(!UA_NodeId_isNull(&valueNodeId)) {
					UA_Variant valueToCheck;
					UA_Variant_init(&valueToCheck);
					UA_StatusCode retvalValue = UA_Client_readValueAttribute(client, valueNodeId, &valueToCheck);
					if(retvalValue != UA_STATUSCODE_GOOD) {
						BOOST_CHECK(false);
					}
									
					UA_Variant datatypeToCheck;
					UA_Variant_init(&datatypeToCheck);
					UA_StatusCode retvalValueDatatype = UA_Client_readValueAttribute(client, valueNodeId, &datatypeToCheck);
					if(retvalValueDatatype != UA_STATUSCODE_GOOD) {
						BOOST_CHECK(false);
					}
									
					UA_NodeId datatypeId;
					if(retvalValue == UA_STATUSCODE_GOOD) {
						UA_StatusCode retvalDatatype = UA_Client_readDataTypeAttribute(client, valueNodeId, &datatypeId);
						if(retvalDatatype != UA_STATUSCODE_GOOD) {
							BOOST_CHECK(false);
						}
					}
					else {
						BOOST_CHECK(false);
					}
					
					if(retvalValue == UA_STATUSCODE_GOOD) {
						string datatype = "";
						string valName = "";
									
						// Check Description -> for all the same
						UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) valueToCheck.data)), valName);
						//cout << "Description: " << valName << endl;
						BOOST_CHECK(valName == "Value");
						
						BOOST_CHECK((datatypeId.identifier.numeric -1) == UA_TYPES_STRING);
					}
					else {
						BOOST_CHECK(false);
					}
					UA_Variant_deleteMembers(&valueToCheck);
					UA_Variant_deleteMembers(&datatypeToCheck);
				}
			}
		}
	}
	
	UA_BrowseRequest_deleteMembers(&bReq);
	UA_BrowseResponse_deleteMembers(&bResp);

	UA_Client_disconnect(client);
	UA_Client_delete(client);
	
	serverSet->runUAServer = UA_FALSE;
	
	if (serverThread->joinable()) {
		serverThread->join();
	}
	
	UA_Server_delete(serverSet->mappedServer);

	serverSet->server_nl.deleteMembers(&serverSet->server_nl);

	delete serverSet;
	serverSet = NULL;
	
 	delete serverThread;
 	serverThread = NULL;
	
	//addVar1->~ua_additionalvariable();

}


class AdditionalVariableTestSuite: public test_suite {
	public:
		AdditionalVariableTestSuite() : test_suite("ua_additionalvariable Test Suite") {
			add(BOOST_TEST_CASE(&AdditionalVariableTest::testEmptySet));
			add(BOOST_TEST_CASE(&AdditionalVariableTest::testClientSide));
    }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	 framework::master_test_suite().add(new AdditionalVariableTestSuite);
	 return 0;
}