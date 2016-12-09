#include <open62541.h>

#include <string.h>
#include <test_sample_data.h>
#include <ua_proxies_typeconversion.h>
#include <mtca_additionalvariable.h>

#include <boost/test/included/unit_test.hpp>


extern "C" {
	#include "unistd.h"
	#include "mtca_namespaceinit_generated.h" // Output des pyUANamespacecompilers
}


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
	if (serverSet->mappedServer == nullptr) {
		BOOST_CHECK(false);
	}
	
	mtca_additionalvariable *addVar1 = new mtca_additionalvariable(serverSet->mappedServer, serverSet->baseNodeId, "Name", "Value", "Description"); 
	
	BOOST_CHECK(addVar1->getSourceTimeStamp() != 0);
	BOOST_CHECK(addVar1->getValue() == "Value");
	
	addVar1->~mtca_additionalvariable();
	
	serverSet->runUAServer = UA_FALSE;
	UA_Server_delete(serverSet->mappedServer);

	if (serverThread->joinable()) {
		serverThread->join();
	}

	delete serverThread;
	delete(serverSet); 
};

void AdditionalVariableTest::testClientSide(){ 
	std::cout << "Enter AdditionalVariableTest with ExampleSet and ClientSide testing" << std::endl;

	TestFixtureServerSet *serverSet = new TestFixtureServerSet;	
	TestFixturePVSet pvSet;
	thread *serverThread = new std::thread(UA_Server_run, serverSet->mappedServer, &serverSet->runUAServer);

	// check server
	if (serverSet->mappedServer == nullptr) {
		BOOST_CHECK(false);
	}

	// add set
	mtca_additionalvariable *addVar1 = new mtca_additionalvariable(serverSet->mappedServer, serverSet->baseNodeId, "Name", "Value", "Description"); 
		
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
									UA_Variant *valueToCheck = UA_Variant_new();
									UA_Variant_init(valueToCheck);
									UA_StatusCode retvalValue = UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
									if(retvalValue != UA_STATUSCODE_GOOD) {
										BOOST_CHECK(false);
									}
									
									UA_Variant *datatypeToCheck = UA_Variant_new();
									UA_Variant_init(datatypeToCheck);
									UA_StatusCode retvalValueDatatype = UA_Client_readValueAttribute(client, valueNodeId, datatypeToCheck);
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
										UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) valueToCheck->data)), valName);
										//cout << "Description: " << valName << endl;
										BOOST_CHECK(valName == "Value");
										// Write new engineering unit
										UA_String newValue;
										UA_String_init(&newValue);
										string merker = "Beschreibung";
										CPPSTRING_TO_UASTRING(newValue, merker);
										UA_Variant_init(valueToCheck);
										UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_STRING]);
										UA_StatusCode retvalNewDesc = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
										UA_String_deleteMembers(&newValue);
										UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) valueToCheck->data)), valName);
										BOOST_CHECK(valName == "Beschreibung");
										
										BOOST_CHECK((datatypeId.identifier.numeric -1) == UA_TYPES_STRING);
									}
									else {
										BOOST_CHECK(false);
									}
									UA_Variant_delete(valueToCheck);
									UA_Variant_delete(datatypeToCheck);
						}
					}
		}
	}

	UA_BrowseRequest_deleteMembers(&bReq);
	UA_BrowseResponse_deleteMembers(&bResp);
	  
	
	serverSet->runUAServer = UA_FALSE;
	UA_Server_delete(serverSet->mappedServer);
 	
 	if (serverThread->joinable()) {
  		serverThread->join();
  	}
 
 	delete serverThread;
	delete(serverSet); 

};


class AdditionalVariableTestSuite: public test_suite {
	public:
		AdditionalVariableTestSuite() : test_suite("mtca_additionalvariable Test Suite") {
			add(BOOST_TEST_CASE(&AdditionalVariableTest::testEmptySet));
			add(BOOST_TEST_CASE(&AdditionalVariableTest::testClientSide));
    }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] ) {

	 framework::master_test_suite().add(new AdditionalVariableTestSuite);

    return 0;
}