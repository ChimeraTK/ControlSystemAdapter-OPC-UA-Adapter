// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "LocalHttpServer.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace influxdb {

  LocalHttpServer::LocalHttpServer(uint16_t port) {
    serverFd_ = socket(AF_INET, SOCK_STREAM, 0);
    // BOOST_REQUIRE(serverFd_ >= 0);

    int enableReuse = 1;
    setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(enableReuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);

    if(bind(serverFd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
      throw std::runtime_error("Failed to bind server socket");
    };

    if(port == 0) {
      socklen_t addressLength = sizeof(address);
      if(getsockname(serverFd_, reinterpret_cast<sockaddr*>(&address), &addressLength) != 0) {
        throw std::runtime_error("Failed to get server socket name");
      };
      port_ = ntohs(address.sin_port);
    }
    else {
      port_ = port;
    }
    if(listen(serverFd_, 1) != 0) {
      throw std::runtime_error("Failed to listen on server socket");
    };

    worker_ = std::thread([this]() { acceptAndCapture(); });
  }

  LocalHttpServer::~LocalHttpServer() {
    if(serverFd_ >= 0) {
      shutdown(serverFd_, SHUT_RDWR);
      close(serverFd_);
      serverFd_ = -1;
    }
    if(worker_.joinable()) {
      worker_.join();
    }
  }

  void LocalHttpServer::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    capturedRequest_.clear();
    condition_.notify_all();
    if(worker_.joinable()) {
      worker_.join();
    }
    worker_ = std::thread([this]() { acceptAndCapture(); });
  }

  int LocalHttpServer::port() const {
    return port_;
  }

  std::string LocalHttpServer::url() const {
    return "http://127.0.0.1:" + std::to_string(port_);
  }

  std::string LocalHttpServer::waitForRequest() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this]() { return !capturedRequest_.empty(); });
    return capturedRequest_;
  }

  void LocalHttpServer::acceptAndCapture() {
    int clientFd = accept(serverFd_, nullptr, nullptr);
    if(clientFd < 0) {
      return;
    }

    std::string request;
    char buffer[4096];
    while(true) {
      const ssize_t received = recv(clientFd, buffer, sizeof(buffer), 0);
      if(received <= 0) {
        break;
      }
      request.append(buffer, static_cast<size_t>(received));

      const std::string::size_type headerEnd = request.find("\r\n\r\n");
      if(headerEnd == std::string::npos) {
        continue;
      }

      const std::string headers = request.substr(0, headerEnd);
      const std::string::size_type contentLengthPos = headers.find("Content-Length:");
      if(contentLengthPos == std::string::npos) {
        break;
      }

      const std::string::size_type lineStart = contentLengthPos + std::string("Content-Length:").size();
      const std::string::size_type lineEnd = headers.find("\r\n", lineStart);
      const std::string value = headers.substr(lineStart, lineEnd - lineStart);
      const long contentLength = std::strtol(value.c_str(), nullptr, 10);
      const std::string::size_type bodyStart = headerEnd + std::string("\r\n\r\n").size();
      const std::string::size_type bodyEnd = bodyStart + static_cast<size_t>(contentLength);
      if(request.size() >= bodyEnd) {
        request = request.substr(bodyStart, contentLength);
        break;
      }
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      capturedRequest_ = request;
    }
    condition_.notify_one();

    const std::string response = "HTTP/1.1 204 No Content\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    send(clientFd, response.c_str(), response.size(), 0);
    close(clientFd);
  }

} // namespace influxdb