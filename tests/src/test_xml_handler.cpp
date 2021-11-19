#include "xml_file_handler.h"

#include <boost/test/included/unit_test.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::unit_test_framework;
using namespace std;

class XMLFileHandlerTest {
 public:
  static void readDocFile();
  static void getContent();
};

void XMLFileHandlerTest::readDocFile() {
  std::cout << "Enter XMLFileHandlerTest - readDocFile" << std::endl;
  xml_file_handler* xmlHandlerOne = new xml_file_handler("../tests/uamapping_test_1.xml");
  BOOST_CHECK(xmlHandlerOne != NULL);

  // Emtpy path
  xml_file_handler* xmlHandlerTwo = new xml_file_handler("");
  BOOST_CHECK(xmlHandlerTwo->isDocSetted() == false);
  BOOST_CHECK(xmlHandlerTwo->getNodeSet("//process_variable") == NULL);
  // Set a document
  xmlHandlerTwo->createDoc("../tests/uamapping_test_2.xml");
  BOOST_CHECK(xmlHandlerTwo->isDocSetted() == true);
  BOOST_CHECK(xmlHandlerTwo->getNodeSet("//process_variable") != NULL);

  // Set a not wellformed document
  xml_file_handler* xmlHandlerThree = new xml_file_handler("../tests/uamapping_test_notwellformed.xml");
  BOOST_CHECK(xmlHandlerThree->isDocSetted() == false);
  BOOST_CHECK(xmlHandlerThree->getNodeSet("//process_variable") == NULL);

  xmlHandlerOne->~xml_file_handler();
  xmlHandlerTwo->~xml_file_handler();
  xmlHandlerThree->~xml_file_handler();
}

void XMLFileHandlerTest::getContent() {
  std::cout << "Enter XMLFileHandlerTest - getContent" << std::endl;
  xml_file_handler* xmlHandler = new xml_file_handler("../tests/uamapping_test_1.xml");

  xmlXPathObjectPtr result = xmlHandler->getNodeSet(
      //"//application[@name='TestCaseForXMLFileHandlerTest::getContent']//map");
      "//process_variable[@sourceName='int8Array__s15']");
  BOOST_CHECK(result != NULL);
  xmlNodeSetPtr nodeset = NULL;

  if(result) {
    nodeset = result->nodesetval;

    // Check if Tag <map> contain a Attribute "sourceVariableName" with value
    string sourceVariableName = xmlHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "sourceName");
    BOOST_CHECK(sourceVariableName != "");

    // We want to get a value from a not existing attribute
    BOOST_CHECK(xmlHandler->getAttributeValueFromNode(nodeset->nodeTab[0], "notExistingAttribute") == "");

    // Get a nodeList
    vector<xmlNodePtr> nodeList = xmlHandler->getNodesByName(nodeset->nodeTab[0]->children, "destination");
    BOOST_CHECK(nodeList.size() == 1);

    // Get a content from a tag element like: <tag>content</tag>
    string unrollPath = xmlHandler->getContentFromNode(nodeList[0]);
    BOOST_CHECK(unrollPath == "TestCaseForXMLFileHandlerTest::getContent/NorthSideLINAC/partB");

    // We want to get a conten from a not existing node
    string emptyContent = xmlHandler->getContentFromNode(NULL);
    BOOST_CHECK(emptyContent == "");

    vector<string> path = xmlHandler->praseVariablePath(
        sourceVariableName, xmlHandler->getAttributeValueFromNode(nodeList[0], "name"));
    BOOST_CHECK(path.size() == 1);
  }
  else {
    BOOST_CHECK(false);
  }

  // We want to get an nodeset from a node which not exist
  BOOST_CHECK(xmlHandler->getNodeSet("//test") == NULL);

  xmlHandler->~xml_file_handler();
}

/**
   * The boost test suite
   */
class XMLFileHandlerTestSuite : public test_suite {
 public:
  XMLFileHandlerTestSuite() : test_suite("XMLFileHandler Test Suite") {
    add(BOOST_TEST_CASE(&XMLFileHandlerTest::readDocFile));
    add(BOOST_TEST_CASE(&XMLFileHandlerTest::getContent));
  }
};

test_suite* init_unit_test_suite(int argc, char* argv[]) {
  framework::master_test_suite().add(new XMLFileHandlerTestSuite);
  return 0;
}
