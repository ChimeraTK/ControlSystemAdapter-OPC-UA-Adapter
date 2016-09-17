#include "xml_file_handler.h"

#include <boost/test/included/unit_test.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::unit_test_framework;


class XMLFileHandlerTest {
	public:
		static void readDocFile();
		static void getContent();
};
   
void XMLFileHandlerTest::readDocFile(){ 
	std::cout << "Enter XMLFileHandlerTest" << std::endl;
	xml_file_handler *xmlHandlerOne = new xml_file_handler("../../tests/uamapping_test.xml");
	
	BOOST_CHECK(xmlHandlerOne->createDoc("") == NULL);
	
	xml_file_handler *xmlHandlerTwo = new xml_file_handler("");
	BOOST_CHECK(xmlHandlerTwo->getNodeSet("//application") == NULL);
	
	BOOST_CHECK(xmlHandlerTwo->createDoc("../../tests/uamapping_test_notwellformed.xml") == NULL);
	
	BOOST_CHECK(xmlHandlerTwo->getNodeSet("//application") == NULL);
};


void XMLFileHandlerTest::getContent(){ 
	std::cout << "Enter XMLFileHandlerTest" << std::endl;
	xml_file_handler *xmlHandler = new xml_file_handler("../../tests/uamapping_test.xml");
	
	xmlXPathObjectPtr result = xmlHandler->getNodeSet("//map");
	BOOST_CHECK(result != NULL);
	xmlNodeSetPtr nodeset;
	
	if(result) {
		nodeset = result->nodesetval;
			std::string srcVarName = xmlHandler->getContentFromNode(nodeset->nodeTab[0]);
			BOOST_CHECK(srcVarName == "");
	}
	else {
		BOOST_CHECK(false);
	}
	
	BOOST_CHECK(xmlHandler->getNodeSet("//test") == NULL);
	
	xmlNode* node;
	BOOST_CHECK(xmlHandler->getContentFromNode(node) == "");
	node = nullptr;
	BOOST_CHECK(xmlHandler->getContentFromNode(node) == "");
	
	
};


/**
   * The boost test suite
   */
class XMLFileHandlerTestSuite: public test_suite {
	public:
		XMLFileHandlerTestSuite() : test_suite("IPCManager Test Suite") {
			add(BOOST_TEST_CASE(&XMLFileHandlerTest::readDocFile));
			add(BOOST_TEST_CASE(&XMLFileHandlerTest::getContent));
			
    }
};


test_suite*
init_unit_test_suite( int argc, char* argv[] ) {
	
	 framework::master_test_suite().add(new XMLFileHandlerTestSuite);

    return 0;
}

