// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
namespace influxdb {
  class LocalHttpServer {
   public:
    explicit LocalHttpServer(uint16_t port = 0);

    ~LocalHttpServer();

    void reset();

    int port() const;
    std::string waitForRequest();
    std::string url() const;

   private:
    void acceptAndCapture();

    std::atomic<int> serverFd_{-1};
    int port_{0};
    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::string capturedRequest_;
  };
} // namespace influxdb