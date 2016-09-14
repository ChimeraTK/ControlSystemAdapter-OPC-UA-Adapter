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

thread *serverThread = 0;

struct TestFixturePVSet {
  std::pair<boost::shared_ptr<ControlSystemPVManager>, boost::shared_ptr<DevicePVManager> > pvManagers;
  boost::shared_ptr<ControlSystemPVManager> csManager;
  boost::shared_ptr<DevicePVManager> devManager;
    
  ControlSystemSynchronizationUtility csSyncUtil;

  TestFixturePVSet() : pvManagers(createPVManager()),csManager(pvManagers.first), devManager(pvManagers.second), csSyncUtil(csManager) {
		csSyncUtil.receiveAll();
		
		ProcessScalar<int8_t>::SharedPtr intA8dev = devManager->createProcessScalar<int8_t>(controlSystemToDevice, "int8Scalar");
 		ProcessScalar<uint8_t>::SharedPtr intAu8dev = devManager->createProcessScalar<uint8_t>(controlSystemToDevice, "uint8Scalar");
 		ProcessScalar<int16_t>::SharedPtr intA16dev = devManager->createProcessScalar<int16_t>(controlSystemToDevice, "int16Scalar");
 		ProcessScalar<uint16_t>::SharedPtr intAu16dev = devManager->createProcessScalar<uint16_t>(controlSystemToDevice, "uint16Scalar");
		ProcessScalar<int32_t>::SharedPtr intA32dev = devManager->createProcessScalar<int32_t>(controlSystemToDevice, "int32Scalar");
 		ProcessScalar<uint32_t>::SharedPtr intAu32dev = devManager->createProcessScalar<uint32_t>(controlSystemToDevice, "uint32Scalar");
 		ProcessScalar<float>::SharedPtr intAfdev = devManager->createProcessScalar<float>(controlSystemToDevice, "floatScalar");
 		ProcessScalar<double>::SharedPtr intAddev = devManager->createProcessScalar<double>(controlSystemToDevice, "doubleScalar");
    
		csManager->getProcessScalar<uint8_t>("uint8Scalar")->set(12);
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

struct TestFixtureServerSet {
	
	uint32_t opcuaPort;
	/* Create new Server */
	UA_ServerConfig       server_config;
	UA_ServerNetworkLayer server_nl;
	UA_Server *mappedServer;
	UA_NodeId baseNodeId;
	UA_Boolean runUAServer;
	
	TestFixtureServerSet() {
		
		opcuaPort = 16660;
		
		server_config = UA_ServerConfig_standard;
		server_nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, opcuaPort);
		server_config.logger = UA_Log_Stdout;
		server_config.networkLayers = &server_nl;
		server_config.networkLayersSize = 1;
		
		runUAServer = UA_TRUE;
		
		mappedServer = UA_Server_new(server_config);
    baseNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
		
		mtca_namespaceinit_generated(mappedServer);
	}
};

/*
 * ProcessVariableTest
 * 
 */
class ProcessVariableTest {
	public:
		static void testEmptySet();
		static void testClientSide();
		static void testExampleSet();
		static UA_StatusCode readAttribueValue(UA_Client *client, const UA_NodeId nodeId, UA_Variant *outValue);
};

UA_StatusCode ProcessVariableTest::readAttribueValue(UA_Client *client, const UA_NodeId nodeId, UA_Variant *outValue) {
	UA_ReadValueId item;
	UA_ReadValueId_init(&item);
	item.nodeId = nodeId;
	item.attributeId = UA_ATTRIBUTEID_VALUE;
	UA_ReadRequest requested;
	UA_ReadRequest_init(&requested);
	requested.nodesToRead = &item;
	requested.nodesToReadSize = 1;
	UA_ReadResponse responseed = UA_Client_Service_read(client, requested);
	UA_StatusCode retval = responseed.responseHeader.serviceResult;
	if(retval == UA_STATUSCODE_GOOD && responseed.resultsSize != 1)
		retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
	if(retval != UA_STATUSCODE_GOOD) {
		UA_ReadResponse_deleteMembers(&responseed);
		return retval;
	}
	UA_DataValue *res = responseed.results;
	if(res->hasStatus != UA_STATUSCODE_GOOD)
		retval = res->hasStatus;
	else if(!res->hasValue) //|| !UA_Variant_isScalar(&res->value))
		retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
	if(retval != UA_STATUSCODE_GOOD) {
		UA_ReadResponse_deleteMembers(&responseed);
		return retval;
	}
									
	if(UA_ATTRIBUTEID_VALUE == UA_ATTRIBUTEID_VALUE) {
		memcpy(outValue, &res->value, sizeof(UA_Variant));
		UA_Variant_init(&res->value);
	} else if(UA_Variant_isScalar(&res->value) && res->value.type == &UA_TYPES[UA_TYPES_VARIANT]) {
		memcpy(&outValue, res->value.data, res->value.type->memSize);
		free(res->value.data);
		res->value.data = NULL;
	} else {
		retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
	}

	UA_ReadResponse_deleteMembers(&responseed);
	
	return retval;	
}
   
void ProcessVariableTest::testEmptySet(){ 
	std::cout << "Enter ProcessVariableTest with EmptySet" << std::endl;
	TestFixtureServerSet serverSet;
	TestFixturePVSet pvSet;
	
	serverThread = new std::thread(UA_Server_run, serverSet.mappedServer, &serverSet.runUAServer);
	
	// check server
	if (serverSet.mappedServer == nullptr) {
		BOOST_CHECK(false);
	}
	
	//mtca_processvariable *test;
	for(ProcessVariable::SharedPtr oneProcessVariable : pvSet.csManager->getAllProcessVariables()) {
		std::cout << "Checking ProcessVariable: " <<  oneProcessVariable->getName() << std::endl;
		mtca_processvariable *test = new mtca_processvariable(serverSet.mappedServer, serverSet.baseNodeId, oneProcessVariable->getName(), pvSet.csManager);

		BOOST_CHECK(test->getName() == oneProcessVariable->getName());
		
		// TODO: Add some stter function for timestamp
		BOOST_CHECK(test->getTimeStamp().index0 == 0);
		BOOST_CHECK(test->getTimeStamp().index1 == 0);
		BOOST_CHECK(test->getTimeStamp().nanoSeconds == 0);
		BOOST_CHECK(test->getTimeStamp().seconds == 0);
		
		BOOST_CHECK(test->getTimeStampIndex0() == 0);
		BOOST_CHECK(test->getTimeStampIndex1() == 0);
		BOOST_CHECK(test->getTimeStampNanoSeconds() == 0);
		BOOST_CHECK(test->getTimeStampSeconds() == 0);
		
		BOOST_CHECK(test->getEngineeringUnit() == "");
		test->setEngineeringUnit("test");
		BOOST_CHECK(test->getEngineeringUnit() == "test");
		
		std::string valueType = test->getType();
		if(!oneProcessVariable->isArray()) {
			if (valueType == "int8_t") {
 				BOOST_CHECK(valueType == "int8_t");
 				BOOST_CHECK(test->getValue_int8_t() == 0);
 				test->setValue_int8_t(111);
 				BOOST_CHECK(test->getValue_int8_t() == 111);
			}
			else if (valueType == "uint8_t") {
				BOOST_CHECK(valueType == "uint8_t");
				BOOST_CHECK(test->getValue_uint8_t() == 12);
				test->setValue_uint8_t(111);
				BOOST_CHECK(test->getValue_uint8_t() == 111);
			}
			else if (valueType == "int16_t") {
				BOOST_CHECK(valueType == "int16_t");
				BOOST_CHECK(test->getValue_int16_t() == 0);
				test->setValue_int16_t(987);
				BOOST_CHECK(test->getValue_int16_t() == 987);
			}
			else if (valueType == "uint16_t") {
				BOOST_CHECK(valueType == "uint16_t");
				BOOST_CHECK(test->getValue_uint16_t() == 0);
				test->setValue_uint16_t(987);
				BOOST_CHECK(test->getValue_uint16_t() == 987);
			}
			else if (valueType == "int32_t") {
				BOOST_CHECK(valueType == "int32_t");
				BOOST_CHECK(test->getValue_int32_t() == 0);
				test->setValue_int32_t(987);
				BOOST_CHECK(test->getValue_int32_t() == 987);
			}
			else if (valueType == "uint32_t") {
				BOOST_CHECK(valueType == "uint32_t");
				BOOST_CHECK(test->getValue_uint32_t() == 0);
				test->setValue_uint32_t(987);
				BOOST_CHECK(test->getValue_uint32_t() == 987);
			}
			else if (valueType == "float") {
				BOOST_CHECK(valueType == "float");
				BOOST_CHECK(test->getValue_float() == 0);
				test->setValue_float(987);
				BOOST_CHECK(test->getValue_float() == 987);
			}
			else if (valueType == "double") {
				BOOST_CHECK(valueType == "double");
				BOOST_CHECK(test->getValue_double() == 0);
				test->setValue_double(987);
				BOOST_CHECK(test->getValue_double() == 987);
			}
			else BOOST_CHECK(false);	
		}
		else {
			if (valueType == "int8_t") {
				BOOST_CHECK(valueType == "int8_t");
				int32_t i = 0;
				std::vector<int8_t> newVector(test->getValue_Array_int8_t().size());
				for(int8_t value: test->getValue_Array_int8_t()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_int8_t(newVector);
			}
			else if (valueType == "uint8_t") {
				BOOST_CHECK(valueType == "uint8_t");
				int32_t i = 0;
				std::vector<uint8_t> newVector(test->getValue_Array_uint8_t().size());
				for(uint8_t value: test->getValue_Array_uint8_t()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_uint8_t(newVector);
			}
			else if (valueType == "int16_t") {
				BOOST_CHECK(valueType == "int16_t");
				int32_t i = 0;
				std::vector<int16_t> newVector(test->getValue_Array_int16_t().size());
				for(int16_t value: test->getValue_Array_int16_t()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_int16_t(newVector);
			}
			else if (valueType == "uint16_t") {
				BOOST_CHECK(valueType == "uint16_t");
				int32_t i = 0;
				std::vector<uint16_t> newVector(test->getValue_Array_uint16_t().size());
				for(uint16_t value: test->getValue_Array_uint16_t()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_uint16_t(newVector);
			}
			else if (valueType == "int32_t") {
				BOOST_CHECK(valueType == "int32_t");
				int32_t i = 0;
				std::vector<int32_t> newVector(test->getValue_Array_int32_t().size());
				for(int32_t value : test->getValue_Array_int32_t()){
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_int32_t(newVector);
			}
			else if (valueType == "uint32_t") {
				BOOST_CHECK(valueType == "uint32_t");
				int32_t i = 0;
				std::vector<uint32_t> newVector(test->getValue_Array_uint32_t().size());
				for(uint32_t value: test->getValue_Array_uint32_t()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_uint32_t(newVector);
			}
			else if (valueType == "float") {
				BOOST_CHECK(valueType == "float");
				int32_t i = 0;
				std::vector<float> newVector(test->getValue_Array_float().size());
				for(float value: test->getValue_Array_float()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_float(newVector);
			}
			else if (valueType == "double") {
				BOOST_CHECK(valueType == "double");
				int32_t i = 0;
				std::vector<double> newVector(test->getValue_Array_double().size());
				for(double value: test->getValue_Array_double()) {
					BOOST_CHECK(value == 0);
					newVector.at(i) = 100-i;
					i++;
				}
				test->setValue_Array_double(newVector);
			}
			else BOOST_CHECK(false);
		}
		const UA_NodeId nodeId = test->getOwnNodeId();
		BOOST_CHECK(!UA_NodeId_isNull(&nodeId));
		
		std::string newName = "";
		newName = oneProcessVariable->getName();
		// Should not being changed
		BOOST_CHECK(test->getName() != "");
		
  }  
  
	UA_Server_run_shutdown(serverSet.mappedServer);
	UA_Server_delete(serverSet.mappedServer);
	
	//delete serverThread;
	//serverThread = nullptr;
};

void ProcessVariableTest::testClientSide(){ 
	std::cout << "Enter ProcessVariableTest with ExampleSet and ClientSide testing" << std::endl;
	
	TestFixtureServerSet serverSet;
	TestFixturePVSet pvSet;
	
	serverThread = new std::thread(UA_Server_run, serverSet.mappedServer, &serverSet.runUAServer);
	
	// add set
	for(ProcessVariable::SharedPtr oneProcessVariable : pvSet.csManager->getAllProcessVariables()) {
		mtca_processvariable *test = new mtca_processvariable(serverSet.mappedServer, serverSet.baseNodeId, oneProcessVariable->getName(), pvSet.csManager); 
	}
	
	// check server
	if (serverSet.mappedServer == nullptr) {
		BOOST_CHECK(false);
	}
	
	// Create client to connect to server
	UA_Client *client = UA_Client_new(UA_ClientConfig_standard);
	string endpointURL = "opc.tcp://localhost:" + to_string(serverSet.opcuaPort);
	UA_StatusCode retval = UA_Client_connect(client, endpointURL.c_str());
		
	if (retval != UA_STATUSCODE_GOOD) {
		std::cout << "Failed to connect to server" << "opc.tcp://localhost:" << serverSet.opcuaPort << std::endl;
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
			UA_NodeId typeNodeId = UA_NODEID_NULL;
			UA_NodeId nameNodeId = UA_NODEID_NULL;
			UA_NodeId engineeringUnitNodeId = UA_NODEID_NULL;
			string name = "";
			
					UA_ReferenceDescription *refe;
					for (size_t m = 0; m < bResp2.resultsSize; ++m) {
						for (size_t k = 0; k < bResp2.results[m].referencesSize; ++k) {
							refe = &(bResp2.results[m].references[k]);
							if(refe->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
							
								size_t lenBrowseName2 = (int)refe->browseName.name.length;
								string browseNameFound2(reinterpret_cast<char const*>(refe->browseName.name.data), lenBrowseName2);
								
								if(browseNameFound2 == "Type") {
									typeNodeId = refe->nodeId.nodeId;
								}
								
								if(browseNameFound2 == "Name") {
									nameNodeId  = refe->nodeId.nodeId;
								}
								
								if(browseNameFound2 == "Value") {
									valueNodeId = refe->nodeId.nodeId;
									name = browseNameFound;
									cout << "Checking ProcessVariable: " <<  name << endl;
								}
								
								if(browseNameFound2 == "EngineeringUnit") {
									engineeringUnitNodeId = refe->nodeId.nodeId;
								}
								
								
								
								/* timeStamp Begin */
								if(browseNameFound2 == "timeStamp") {
									// TimeStamp Request
									UA_BrowseRequest bReq4TimeStamp;
									UA_BrowseRequest_init(&bReq4TimeStamp);
									bReq4TimeStamp.requestedMaxReferencesPerNode = 0;
									bReq4TimeStamp.nodesToBrowse = UA_BrowseDescription_new();
									bReq4TimeStamp.nodesToBrowseSize = 1;
									bReq4TimeStamp.nodesToBrowse[0].nodeId = refe->nodeId.nodeId; 
									bReq4TimeStamp.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; 
									UA_BrowseResponse bResp4TimeStamp = UA_Client_Service_browse(client, bReq4TimeStamp);		
									
									UA_NodeId tsSecondsNodeId = UA_NODEID_NULL;
									UA_NodeId tsNanoSecondsNodeId = UA_NODEID_NULL;
									UA_NodeId tsIndex0NodeId = UA_NODEID_NULL;
									UA_NodeId tsIndex1NodeId = UA_NODEID_NULL;
									
									UA_ReferenceDescription *refDes4TimeStamp;
									for (size_t o = 0; o < bResp4TimeStamp.resultsSize; ++o) {
										for (size_t r = 0; r < bResp4TimeStamp.results[o].referencesSize; ++r) {
											refDes4TimeStamp = &(bResp4TimeStamp.results[o].references[r]);
											if(refDes4TimeStamp->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
											
												size_t lenBrowseName4TimeStamp = (int)refDes4TimeStamp->browseName.name.length;
												string browseName4TimeStamp(reinterpret_cast<char const*>(refDes4TimeStamp->browseName.name.data), lenBrowseName4TimeStamp);
												if(browseName4TimeStamp == "seconds") {
													tsSecondsNodeId = refDes4TimeStamp->nodeId.nodeId;
												}
												if(browseName4TimeStamp == "nanoseconds") {
													tsNanoSecondsNodeId = refDes4TimeStamp->nodeId.nodeId;
												}
												if(browseName4TimeStamp == "index0") {
													tsIndex0NodeId = refDes4TimeStamp->nodeId.nodeId;
												}
												if(browseName4TimeStamp == "index1") {
													tsIndex1NodeId = refDes4TimeStamp->nodeId.nodeId;
												}
												
											}
										}
									}
								
									/* timeStamp check begin */
									// set wrong value
									uint32_t value = 123;
									UA_StatusCode retvalValue = UA_STATUSCODE_BADUNEXPECTEDERROR;
									UA_Variant *timeStampToCheck = UA_Variant_new();
									// Seconds
									UA_Variant_init(timeStampToCheck);
									retvalValue = UA_Client_readValueAttribute(client, tsSecondsNodeId, timeStampToCheck);
									if(retvalValue != UA_STATUSCODE_GOOD) {
										BOOST_CHECK(false);
									}
									value = (uint32_t) *((uint32_t*) timeStampToCheck->data);
									BOOST_CHECK(value == 0);
									
									// NanoSeconds
									UA_Variant_init(timeStampToCheck);
									retvalValue = UA_Client_readValueAttribute(client, tsNanoSecondsNodeId, timeStampToCheck);
									if(retvalValue != UA_STATUSCODE_GOOD) {
										BOOST_CHECK(false);
									}
									value = (uint32_t) *((uint32_t*) timeStampToCheck->data);
									BOOST_CHECK(value == 0);
									
									// Index0
									UA_Variant_init(timeStampToCheck);
									retvalValue = UA_Client_readValueAttribute(client, tsIndex0NodeId, timeStampToCheck);
									if(retvalValue != UA_STATUSCODE_GOOD) {
										BOOST_CHECK(false);
									}
									value = (uint32_t) *((uint32_t*) timeStampToCheck->data);
									BOOST_CHECK(value == 0);
									
									// Index1
									UA_Variant_init(timeStampToCheck);
									retvalValue = UA_Client_readValueAttribute(client, tsIndex1NodeId, timeStampToCheck);
									if(retvalValue != UA_STATUSCODE_GOOD) {
										BOOST_CHECK(false);
									}
									value = (uint32_t) *((uint32_t*) timeStampToCheck->data);
									BOOST_CHECK(value == 0);
										
									/* timeStamp check end */
								}
								/* timeStamp End */
							}
						}
					}
					UA_BrowseRequest_deleteMembers(&bReq2);
					UA_BrowseResponse_deleteMembers(&bResp2);
					
					if(!UA_NodeId_isNull(&valueNodeId)) {
					// Currently only scalar are suppored 
									UA_Variant *valueToCheck = UA_Variant_new();
									UA_Variant_init(valueToCheck);
									//UA_StatusCode retvalValue = UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
									UA_StatusCode retvalValue = ProcessVariableTest::readAttribueValue(client, valueNodeId, valueToCheck);
									if(retvalValue != UA_STATUSCODE_GOOD) {
										BOOST_CHECK(false);
									}
									
									UA_Variant *datatypeToCheck = UA_Variant_new();
									UA_Variant_init(datatypeToCheck);
									UA_StatusCode retvalValueDatatype = UA_Client_readValueAttribute(client, typeNodeId, datatypeToCheck);
									if(retvalValueDatatype != UA_STATUSCODE_GOOD) {
										BOOST_CHECK(false);
									}

									UA_Variant *nameToCheck = UA_Variant_new();
									UA_Variant_init(nameToCheck);
									UA_StatusCode retvalValueName = UA_Client_readValueAttribute(client, nameNodeId, nameToCheck);
									if(retvalValueName != UA_STATUSCODE_GOOD) {
										BOOST_CHECK(false);
									}
									
									UA_Variant *euToCheck = UA_Variant_new();
									UA_Variant_init(euToCheck);
									UA_StatusCode retvalValueEu = UA_Client_readValueAttribute(client, engineeringUnitNodeId, euToCheck);
									if(retvalValueEu != UA_STATUSCODE_GOOD) {
										BOOST_CHECK(false);
									}

									UA_NodeId datatypeId;
									if(retvalValue == UA_STATUSCODE_GOOD) {
										UA_StatusCode retvalDatatype = UA_Client_readDataTypeAttribute(client, valueNodeId, &datatypeId);
										if(retvalDatatype != UA_STATUSCODE_GOOD) {
											BOOST_CHECK(false);
										}
									}	
									
									if(retvalValue == UA_STATUSCODE_GOOD) {
										string datatype = "";
										string valName = "";
										
										// Check EngineeringUnit -> for all the same
										UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) euToCheck->data)), valName);
										BOOST_CHECK(valName == "");
										
										// i know, this part is perfecly for makro stuff... but common, for maintainability reason we should use simple code... 
										if(valueToCheck->arrayLength < 1) {
											switch(datatypeId.identifier.numeric -1) {
												case UA_TYPES_SBYTE: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "int8_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName == "int8Scalar");
													// Check Value
													uint8_t value = (uint8_t) *((uint8_t*) valueToCheck->data);
													BOOST_CHECK(value == 0);
													cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													uint8_t newValue = 123;
													UA_Variant_init(valueToCheck);
													UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_SBYTE]);
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													value = (uint8_t) *((uint8_t*) valueToCheck->data);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
														value = (uint8_t) *((uint8_t*) valueToCheck->data);
														BOOST_CHECK(value == newValue);
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_BYTE: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "uint8_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "");
													// Check Value
													int8_t value = (int8_t) *((int8_t*) valueToCheck->data);
													BOOST_CHECK(value == 12);
													cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													int8_t newValue = 123;
													valueToCheck = UA_Variant_new();
													UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_BYTE]);
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
														value = (int8_t) *((int8_t*) valueToCheck->data);
														BOOST_CHECK(value == newValue);
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_INT16: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "int16_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													int16_t value = (int16_t) *((int16_t*) valueToCheck->data);
													BOOST_CHECK(value == 0);
													cout << "Wert: " << std::to_string(value) << endl;
													// set new value, should be wrong
													int16_t newValue = 123;
													valueToCheck = UA_Variant_new();
													UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_INT16]);
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
														value = (int16_t) *((int16_t*) valueToCheck->data);
														BOOST_CHECK(value == newValue);
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_UINT16: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "uint16_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													uint16_t value = (uint16_t) *((uint16_t*) valueToCheck->data);
													BOOST_CHECK(value == 0);
													cout << "Wert: " << std::to_string(value) << endl;
													// set new value
												uint16_t newValue = 123;
													valueToCheck = UA_Variant_new();
													UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_UINT16]);
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
														value = (uint16_t) *((uint16_t*) valueToCheck->data);
														BOOST_CHECK(value == newValue);
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_INT32: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "int32_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													int32_t value = (int32_t) *((int32_t*) valueToCheck->data);
													BOOST_CHECK(value == 0);
													cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													int32_t newValue = 123;
													valueToCheck = UA_Variant_new();
													UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_INT32]);
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
														value = (int32_t) *((int32_t*) valueToCheck->data);
														BOOST_CHECK(value == newValue);
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_UINT32: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "uint32_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													uint32_t value = (uint32_t) *((uint32_t*) valueToCheck->data);
													BOOST_CHECK(value == 0);
													cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													uint32_t newValue = 123;
													valueToCheck = UA_Variant_new();
													UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_UINT32]);
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
														value = (uint32_t) *((uint32_t*) valueToCheck->data);
														BOOST_CHECK(value == newValue);
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_DOUBLE: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "double");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													double value = (double) *((double*) valueToCheck->data);
													BOOST_CHECK(value == 0);
													cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													double newValue = 123.123;
													valueToCheck = UA_Variant_new();
													UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_DOUBLE]);
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
														value = (double) *((double*) valueToCheck->data);
														BOOST_CHECK(((int32_t)value*100) == ((int32_t)newValue*100));
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_FLOAT: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "float");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													float value = (float) *((float*) valueToCheck->data);
													BOOST_CHECK(value == 0);
													cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													float newValue = 123.123;
													valueToCheck = UA_Variant_new();
													UA_Variant_setScalarCopy(valueToCheck, &newValue, &UA_TYPES[UA_TYPES_FLOAT]);
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														UA_Client_readValueAttribute(client, valueNodeId, valueToCheck);
														value = (float) *((float*) valueToCheck->data);
														BOOST_CHECK(((int32_t)value*100) == ((int32_t)newValue*100));
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												default: printf("Not define: %-16d\n", datatypeId.identifier.numeric);
											}
										}
										else {
											/*
											* Begin array section
											*/
											switch(datatypeId.identifier.numeric -1) {
												case UA_TYPES_SBYTE: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "int8_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName == "int8Array_s15");
													// Check Value							
													size_t arrayLength = valueToCheck->arrayLength;
													int8_t* value = (int8_t*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}
													// set new value
													int8_t varArray[arrayLength];
													int8_t* newValue = varArray;
													for(int i=0; i < arrayLength; i++) {
														newValue[i] = i;
													}
													
													valueToCheck = UA_Variant_new();
													UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_UINT16]);													
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														ProcessVariableTest::readAttribueValue(client, valueNodeId, valueToCheck);
														value = (int8_t*) valueToCheck->data; 
														for(int i=0; i < arrayLength; i++) {
															BOOST_CHECK(value[i] == newValue[i]);
														}
													}										
													break;
												}
												case UA_TYPES_BYTE: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "uint8_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "");
													// Check Value
													size_t arrayLength = valueToCheck->arrayLength;
													uint8_t* value = (uint8_t*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}
													// set new value
													uint8_t varArray[arrayLength];
													uint8_t* newValue = varArray;
													for(int i=0; i < arrayLength; i++) {
														newValue[i] = i;
													}
													
													valueToCheck = UA_Variant_new();
													UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_UINT16]);													
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														ProcessVariableTest::readAttribueValue(client, valueNodeId, valueToCheck);
														value = (uint8_t*) valueToCheck->data; 
														for(int i=0; i < arrayLength; i++) {
															BOOST_CHECK(value[i] == newValue[i]);
														}
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_INT16: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "int16_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													size_t arrayLength = valueToCheck->arrayLength;
													int16_t* value = (int16_t*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}
													// set new value
													value = (int16_t*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}
													//cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													int16_t varArray[arrayLength];
													int16_t* newValue = varArray;
													for(int i=0; i < arrayLength; i++) {
														newValue[i] = i;
													}
													
													valueToCheck = UA_Variant_new();
													UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_INT16]);													
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														ProcessVariableTest::readAttribueValue(client, valueNodeId, valueToCheck);
														value = (int16_t*) valueToCheck->data; 
														for(int i=0; i < arrayLength; i++) {
															BOOST_CHECK(value[i] == newValue[i]);
														}
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_UINT16: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "uint16_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													size_t arrayLength = valueToCheck->arrayLength;
													uint16_t* value = (uint16_t*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}
													// set new value
													value = (uint16_t*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}
													//cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													uint16_t varArray[arrayLength];
													uint16_t* newValue = varArray;
													for(int i=0; i < arrayLength; i++) {
														newValue[i] = i;
													}
													
													valueToCheck = UA_Variant_new();
													UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_UINT16]);													
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														ProcessVariableTest::readAttribueValue(client, valueNodeId, valueToCheck);
														value = (uint16_t*) valueToCheck->data; 
														for(int i=0; i < arrayLength; i++) {
															BOOST_CHECK(value[i] == newValue[i]);
														}
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_INT32: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "int32_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													size_t arrayLength = valueToCheck->arrayLength;
													int32_t* value = (int32_t*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}													
													// set new value
													value = (int32_t*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}
													//cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													int32_t varArray[arrayLength];
													int32_t* newValue = varArray;
													for(int i=0; i < arrayLength; i++) {
														newValue[i] = i;
													}
													
													valueToCheck = UA_Variant_new();
													UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_INT32]);													
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														ProcessVariableTest::readAttribueValue(client, valueNodeId, valueToCheck);
														value = (int32_t*) valueToCheck->data; 
														for(int i=0; i < arrayLength; i++) {
															BOOST_CHECK(value[i] == newValue[i]);
														}
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_UINT32: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "uint32_t");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													size_t arrayLength = valueToCheck->arrayLength;
													uint32_t* value = (uint32_t*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}
													// set new value
													uint32_t varArray[arrayLength];
													uint32_t* newValue = varArray;
													for(int i=0; i < arrayLength; i++) {
														newValue[i] = i;
													}
													
													valueToCheck = UA_Variant_new();
													UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_UINT32]);													
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														ProcessVariableTest::readAttribueValue(client, valueNodeId, valueToCheck);
														value = (uint32_t*) valueToCheck->data; 
														for(int i=0; i < arrayLength; i++) {
															BOOST_CHECK(value[i] == newValue[i]);
														}
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_DOUBLE: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "double");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													size_t arrayLength = valueToCheck->arrayLength;
													double* value = (double*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}
													//cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													double varArray[arrayLength];
													double* newValue = varArray;
													for(int i=0; i < arrayLength; i++) {
														newValue[i] = i*0.5;
													}
													
													valueToCheck = UA_Variant_new();
													UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_DOUBLE]);													
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														ProcessVariableTest::readAttribueValue(client, valueNodeId, valueToCheck);
														value = (double*) valueToCheck->data; 
														for(int i=0; i < arrayLength; i++) {
															BOOST_CHECK(((int32_t)(value[i]*100)) == ((int32_t)(newValue[i]*100)));
														}
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												case UA_TYPES_FLOAT: {
													// Check Datatype
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) datatypeToCheck->data)), datatype);
													BOOST_CHECK(datatype == "float");
													// Check Name 
													UASTRING_TO_CPPSTRING(((UA_String) *((UA_String *) nameToCheck->data)), valName);
													BOOST_CHECK(valName != "int8Scalar");
													// Check Value
													size_t arrayLength = valueToCheck->arrayLength;
													float* value = (float*) valueToCheck->data; 
													for(int i=0; i < arrayLength; i++) {
														BOOST_CHECK(value[i] == 0);
													}
													//cout << "Wert: " << std::to_string(value) << endl;
													// set new value
													float varArray[arrayLength];
													float* newValue = varArray;
													for(int i=0; i < arrayLength; i++) {
														newValue[i] = i*0.5;
													}
													
													valueToCheck = UA_Variant_new();
													UA_Variant_setArrayCopy(valueToCheck, newValue, arrayLength, &UA_TYPES[UA_TYPES_FLOAT]);													
													UA_StatusCode retvalNewVar = UA_Client_writeValueAttribute(client, valueNodeId, valueToCheck);
													if(retvalNewVar == UA_STATUSCODE_GOOD) {
														// get value from server
														UA_Variant_init(valueToCheck);
														ProcessVariableTest::readAttribueValue(client, valueNodeId, valueToCheck);
														value = (float*) valueToCheck->data; 
														for(int i=0; i < arrayLength; i++) {
															BOOST_CHECK(((int32_t)(value[i]*100)) == ((int32_t)(newValue[i]*100)));
														}
													}
													else {
														BOOST_CHECK(false);
													}
													break;
												}
												default: printf("Not define: %-16d\n", datatypeId.identifier.numeric);
											}
										}
									}
			
									UA_Variant_delete(valueToCheck);
					}
					}
		}
	}

	UA_BrowseRequest_deleteMembers(&bReq);
	UA_BrowseResponse_deleteMembers(&bResp);

};


class ProcessVariableTestSuite: public test_suite {
	public:
		ProcessVariableTestSuite() : test_suite("mtca_processvariable Test Suite") {
			add(BOOST_TEST_CASE(&ProcessVariableTest::testEmptySet));
			add(BOOST_TEST_CASE(&ProcessVariableTest::testClientSide));
    }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] ) {

	 framework::master_test_suite().add(new ProcessVariableTestSuite);

    return 0;
}