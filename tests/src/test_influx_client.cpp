// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ChimeraTK/ControlSystemAdapter/ControlSystemPVManager.h"
#include "csa_opcua_adapter.h"
#include "history_backend/InfluxClient.h"
#include "LocalHttpServer.h"

#include <open62541/client_highlevel.h>
#include <open62541/plugin/historydata/history_data_gathering_default.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/util.h>

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
    static void testResetWorker();
  };

} // namespace detail

void detail::InfluxClientTest::testResetWorker() {
  InfluxConfig config;
  config.url = "http://127.0.0.1:1";
  config.token = "token";
  config.org = "org";
  config.bucket = "bucket";
  config.measurement = "measurement";
  config.precision = "ns";
  config.writeBatching.enabled = true;
  config.writeBatching.maxBatchPoints = 10;
  config.writeBatching.maxQueuePoints = 8;
  config.writeBatching.flushIntervalMs = 1000;
  config.writeBatching.maxRetries = 1;
  config.writeBatching.retryBackoffMs = 5;

  influxdb::InfluxClient client(config);
  std::string error;
  BOOST_REQUIRE(client.writePoint("pv", {"1", false}, {}, std::nullopt, &error));
  BOOST_REQUIRE(client.writePoint("pv", {"2", false}, {}, std::nullopt, &error));

  std::size_t queuedPoints = 0;
  for(int i = 0; i < 50; ++i) {
    queuedPoints = client.getWriteStats().queuedPoints;
    if(queuedPoints == 2) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  BOOST_REQUIRE_EQUAL(queuedPoints, 2);

  BOOST_REQUIRE(client.reset());
  const auto statsAfterReset = client.getWriteStats();
  BOOST_CHECK_EQUAL(statsAfterReset.queuedPoints, 0);
  BOOST_CHECK_EQUAL(statsAfterReset.pointsDropped, 0);
  BOOST_CHECK(!client.hasAsyncWriteError());

  BOOST_REQUIRE(client.writePoint("pv", {"3", false}, {}, std::nullopt, &error));
  const auto statsAfterRequeue = client.getWriteStats();
  BOOST_CHECK_EQUAL(statsAfterRequeue.queuedPoints, 1);
}

//**
//  Check the number of queued points using the OPC UA server PV InfluxHealth.QueuedPoints.
//
//  @param expectedValue The expected value.
//  @param server The OPC UA server.
//  @return true if the value is read, false otherwise.
// */
bool checkQueuedPoints(const uint64_t& expectedValue, UA_Server* server) {
  UA_Variant var;
  UA_Variant_init(&var);
  auto rt = UA_Server_readValue(server, UA_NODEID_STRING(1, const_cast<char*>("InfluxHealth.QueuedPoints")), &var);
  if(rt == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&var, &UA_TYPES[UA_TYPES_UINT64])) {
    const UA_UInt64 val = *(const UA_UInt64*)var.data;
    return val == expectedValue;
  }
  return false;
}

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
  // read initial values
  const std::string request = server.waitForRequest();
  server.reset();

  // Write data twice and check the response from the local HTTP server aka InfluxDB server
  while(csaOPCUA->isRunning() && i < 3) {
    devManager->getProcessArray<float>("dev/float")->accessChannel(0) = std::vector<float>{static_cast<float>(i)};
    devManager->getProcessArray<float>("dev/float")->write();
    while(!checkQueuedPoints(1, uaserver)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    devManager->getProcessArray<float>("dev/floatWithExtraTag")->accessChannel(0) =
        std::vector<float>{static_cast<float>(i)};
    devManager->getProcessArray<float>("dev/floatWithExtraTag")->write();
    while(!checkQueuedPoints(2, uaserver)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if(i == 2) {
      // writing always "" would not trigger a history update
      devManager->getProcessArray<std::string>("dev/testString1")->accessChannel(0) = std::vector<std::string>{""};
    }
    else {
      devManager->getProcessArray<std::string>("dev/testString1")->accessChannel(0) =
          std::vector<std::string>{std::to_string(i)};
    }
    devManager->getProcessArray<std::string>("dev/testString1")->write();
    while(!checkQueuedPoints(3, uaserver)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    devManager->getProcessArray<std::string>("dev/testString2")->accessChannel(0) =
        std::vector<std::string>{std::to_string(i)};
    devManager->getProcessArray<std::string>("dev/testString2")->write();

    const std::string request = server.waitForRequest();
    std::vector<std::string> requests;
    UA_LOG_INFO(config->logging, UA_LOGCATEGORY_USERLAND, "Received request from InfluxClient: %s", request.c_str());
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
    BOOST_CHECK(requestValue.find(std::string("float=") + std::to_string(i)) != std::string::npos);

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
    BOOST_CHECK(requestValue.find(std::string("floatWithExtraTag=") + std::to_string(i)) != std::string::npos);

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
    BOOST_CHECK(requestValue.find(std::string("testString1=")) != std::string::npos);

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
    BOOST_CHECK(
        requestValue.find(std::string("testString2=\"") + std::to_string(i) + std::string("\"")) != std::string::npos);

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
    add(BOOST_TEST_CASE(&detail::InfluxClientTest::testResetWorker));
    add(BOOST_TEST_CASE(&detail::InfluxClientTest::testLocalServer));
  }
};

boost::unit_test::test_suite* init_unit_test_suite(int /*argc*/, char** /*argv[]*/) {
  boost::unit_test::framework::master_test_suite().add(new InfluxClientTestSuite);
  return 0;
}
