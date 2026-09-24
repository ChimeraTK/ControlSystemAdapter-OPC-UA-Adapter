// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "csa_opcua_adapter.h"
#include "LocalHttpServer.h"

#include <open62541/plugin/historydata/history_data_gathering_default.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>          // Include for boost::split
#include <boost/test/included/unit_test.hpp>

#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

namespace detail {

  namespace {
    class ScopedInfluxConfig {
     public:
      explicit ScopedInfluxConfig(const std::string& serverUrl)
      : configPath_(std::filesystem::current_path() / "influx_config.xml") {
        originalContent_ = readFile(configPath_);

        const std::string startTag = "<csa:url>";
        const std::string endTag = "</csa:url>";
        const std::size_t startPos = originalContent_.find(startTag);
        const std::size_t endPos = originalContent_.find(endTag, startPos == std::string::npos ? 0 : startPos);
        if(startPos == std::string::npos || endPos == std::string::npos) {
          throw std::runtime_error("Failed to locate <csa:url> in influx_config.xml");
        }

        std::string updatedContent = originalContent_;
        updatedContent.replace(startPos + startTag.size(), endPos - (startPos + startTag.size()), serverUrl);
        writeFile(updatedContent);
      }

      ~ScopedInfluxConfig() {
        if(!originalContent_.empty()) {
          writeFile(originalContent_);
        }
      }

     private:
      static std::string readFile(const std::filesystem::path& path) {
        std::ifstream input(path);
        if(!input.is_open()) {
          throw std::runtime_error("Failed to open influx_config.xml for reading");
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
      }

      void writeFile(const std::string& content) const {
        std::ofstream output(configPath_, std::ios::trunc);
        if(!output.is_open()) {
          throw std::runtime_error("Failed to open influx_config.xml for writing");
        }
        output << content;
      }

      std::filesystem::path configPath_;
      std::string originalContent_;
    };
  } // namespace

  std::pair<std::string, std::string> splitInfluxRequest(const std::string& request) {
    for(std::size_t i = 0; i < request.size(); ++i) {
      if(request[i] == '\\' && i + 1 < request.size() && request[i + 1] == ' ') {
        ++i;
        continue;
      }
      if(request[i] == ' ') {
        return {request.substr(0, i), request.substr(i + 1)};
      }
    }
    return {request, {}};
  }

  class InfluxClientTest {
   public:
    static void testLocalServer();
  };

} // namespace detail

void detail::InfluxClientTest::testLocalServer() {
  influxdb::LocalHttpServer server;
  ScopedInfluxConfig scopedInfluxConfig(server.url());
  std::cout << "Local HTTP server running at: " << server.url() << " (port " << server.port() << ")" << std::endl;

  std::pair<boost::shared_ptr<ChimeraTK::ControlSystemPVManager>, boost::shared_ptr<ChimeraTK::DevicePVManager>>
      pvManagers = ChimeraTK::createPVManager();
  boost::shared_ptr<ChimeraTK::DevicePVManager> devManager = pvManagers.second;
  boost::shared_ptr<ChimeraTK::ControlSystemPVManager> csManager = pvManagers.first;
  ChimeraTK::ProcessArray<float>::SharedPtr pvFloat1 = devManager->createProcessArray<float>(
      ChimeraTK::SynchronizationDirection::deviceToControlSystem, "dev/float", 1, "unit1", "desc");
  ChimeraTK::ProcessArray<float>::SharedPtr pvFloat2 = devManager->createProcessArray<float>(
      ChimeraTK::SynchronizationDirection::deviceToControlSystem, "dev/floatWithExtraTag", 1, "unit2", "desc");
  ChimeraTK::ProcessArray<std::string>::SharedPtr pvStr1 = devManager->createProcessArray<std::string>(
      ChimeraTK::SynchronizationDirection::deviceToControlSystem, "dev/testString1", 1, "unit2", "empty string");
  ChimeraTK::ProcessArray<std::string>::SharedPtr pvStr2 = devManager->createProcessArray<std::string>(
      ChimeraTK::SynchronizationDirection::deviceToControlSystem, "dev/testString2", 1, "", "");
  std::string pathToConfig = "uamapping_test_influx.xml";
  std::unique_ptr<ChimeraTK::csa_opcua_adapter> csaOPCUA(new ChimeraTK::csa_opcua_adapter(csManager, pathToConfig));
  csaOPCUA->start();
  while(!csaOPCUA->getUAAdapter()->running) {
  };

  // Server is running
  std::cout << "server is running..." << std::endl;
  size_t i = 1;
  UA_Server* uaserver = csaOPCUA->getUAAdapter()->getMappedServer();
  UA_ServerConfig* config = UA_Server_getConfig(uaserver);
  // Write data twice and check the response from the local HTTP server aka InfluxDB server
  while(csaOPCUA->isRunning() && i < 3) {
    sleep(2);
    devManager->getProcessArray<float>("dev/float")->accessChannel(0) = std::vector<float>{static_cast<float>(i)};
    devManager->getProcessArray<float>("dev/float")->write();
    devManager->getProcessArray<float>("dev/floatWithExtraTag")->accessChannel(0) =
        std::vector<float>{static_cast<float>(i)};
    devManager->getProcessArray<float>("dev/floatWithExtraTag")->write();
    if(i % 1 == 1) {
      // writing always "" would not trigger a history update
      devManager->getProcessArray<std::string>("dev/testString1")->accessChannel(0) = std::vector<std::string>{""};
    }
    else {
      devManager->getProcessArray<std::string>("dev/testString1")->accessChannel(0) =
          std::vector<std::string>{std::to_string(i)};
    }
    devManager->getProcessArray<std::string>("dev/testString1")->write();
    devManager->getProcessArray<std::string>("dev/testString2")->accessChannel(0) =
        std::vector<std::string>{std::to_string(i)};
    devManager->getProcessArray<std::string>("dev/testString2")->write();

    const std::string request = server.waitForRequest();
    std::vector<std::string> requests;
    std::cout << "Received request from InfluxClient: " << request << std::endl;
    boost::split(requests, request, boost::is_any_of("\n"), boost::token_compress_on);
    auto pos = request.find('\n');
    BOOST_CHECK(requests.size() == 4);

    // check float
    const auto splitRequest1 = splitInfluxRequest(requests.at(0));
    std::string requestMetadata = splitRequest1.first;
    std::string requestValue = splitRequest1.second;

    UA_LOG_INFO(
        config->logging, UA_LOGCATEGORY_USERLAND, "Received request for float metadata: %s", requestMetadata.c_str());
    UA_LOG_INFO(config->logging, UA_LOGCATEGORY_USERLAND, "Received request for float value: %s", requestValue.c_str());

    BOOST_REQUIRE(!requestMetadata.empty());
    BOOST_REQUIRE(!requestValue.empty());
    BOOST_CHECK(requestMetadata.find("demo_measurement") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("unittest=influx") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("unit=unit1") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("name=float") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("extra=special1") != std::string::npos);
    BOOST_CHECK(requestValue.find("float=") != std::string::npos);

    // now check floatWithExtraTag
    const auto splitRequest2 = splitInfluxRequest(requests.at(1));
    requestMetadata = splitRequest2.first;
    requestValue = splitRequest2.second;

    UA_LOG_INFO(config->logging, UA_LOGCATEGORY_USERLAND, "Received request for floatWithExtraTag metadata: %s",
        requestMetadata.c_str());
    UA_LOG_INFO(config->logging, UA_LOGCATEGORY_USERLAND, "Received request for floatWithExtraTag value: %s",
        requestValue.c_str());

    BOOST_REQUIRE(!requestMetadata.empty());
    BOOST_REQUIRE(!requestValue.empty());
    BOOST_CHECK(requestMetadata.find("demo_measurement") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("unittest=influx") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("extra=special1") == std::string::npos);
    BOOST_CHECK(requestMetadata.find("extra=special2") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("unit=unit2") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("name=floatWithExtraTag") != std::string::npos);
    BOOST_CHECK(requestValue.find("floatWithExtraTag=") != std::string::npos);

    const auto splitRequest3 = splitInfluxRequest(requests.at(2));
    requestMetadata = splitRequest3.first;
    requestValue = splitRequest3.second;

    UA_LOG_INFO(
        config->logging, UA_LOGCATEGORY_USERLAND, "Received request for string1 metadata: %s", requestMetadata.c_str());
    UA_LOG_INFO(
        config->logging, UA_LOGCATEGORY_USERLAND, "Received request for string1 value: %s", requestValue.c_str());

    BOOST_REQUIRE(!requestMetadata.empty());
    BOOST_REQUIRE(!requestValue.empty());
    BOOST_CHECK(requestMetadata.find("demo_measurement") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("unittest=influx") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("extra=special1") == std::string::npos);
    BOOST_CHECK(requestMetadata.find("extra=special2") == std::string::npos);
    BOOST_CHECK(requestMetadata.find("unit=unit2") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("name=floatWithExtraTag") == std::string::npos);
    BOOST_CHECK(requestValue.find("floatWithExtraTag=") == std::string::npos);

    const auto splitRequest4 = splitInfluxRequest(requests.at(3));
    requestMetadata = splitRequest4.first;
    requestValue = splitRequest4.second;

    UA_LOG_INFO(
        config->logging, UA_LOGCATEGORY_USERLAND, "Received request for string2 metadata: %s", requestMetadata.c_str());
    UA_LOG_INFO(
        config->logging, UA_LOGCATEGORY_USERLAND, "Received request for string2 value: %s", requestValue.c_str());

    BOOST_REQUIRE(!requestMetadata.empty());
    BOOST_REQUIRE(!requestValue.empty());
    BOOST_CHECK(requestMetadata.find("demo_measurement") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("unittest=influx") != std::string::npos);
    BOOST_CHECK(requestMetadata.find("extra=special1") == std::string::npos);
    BOOST_CHECK(requestMetadata.find("extra=special2") == std::string::npos);
    BOOST_CHECK(requestMetadata.find("unit=") == std::string::npos);
    BOOST_CHECK(requestMetadata.find("name=floatWithExtraTag") == std::string::npos);
    BOOST_CHECK(requestValue.find("floatWithExtraTag=") == std::string::npos);

    server.reset();
    i++;
  }
  csaOPCUA->stop();
  std::cout << "server stopped..." << std::endl;
  csaOPCUA.reset();
}

class InfluxClientTestSuite : public boost::unit_test::test_suite {
 public:
  InfluxClientTestSuite() : boost::unit_test::test_suite("InfluxClient Test Suite") {
    add(BOOST_TEST_CASE(&detail::InfluxClientTest::testLocalServer));
  }
};

boost::unit_test::test_suite* init_unit_test_suite(int /*argc*/, char** /*argv[]*/) {
  boost::unit_test::framework::master_test_suite().add(new InfluxClientTestSuite);
  return 0;
}
