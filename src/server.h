#ifndef SRC_SERVER_H
#define SRC_SERVER_H

#include <list>
#include <memory>
#include <unordered_map>

#include "common/fd_guard.h"
#include "connection.h"
#include "epoll.h"
#include "glog/logging.h"

#include "protocols/echo.h"

struct ServerOption {
  uint32_t port;
};

class Server {
 public:
  Server(const ServerOption& options)
      : options_(options),
        protocol_(new Echo()) {}
  bool Start();
  void Stop();
  void RunUtilAskedToStop() { el_.RunUtilAskedToStop(); }
  void Receive(int fd, void* msg);
  void Send(int fd, void* msg, const char* dat, int len);
  // int AddService(Service* s, const string& uri_method_mappings);

 private:
  void Accept(int fd, void* data);
  void RemoveConnection(Connection* conn);
  void SayGoodbye(int sock, const std::string& msg) const {
    /*int ignore = */write(sock, msg.c_str(), msg.size());
  }

  ServerOption options_;
  FDGuard server_fd_;
  Epoll el_;
  std::unordered_map<int, std::unique_ptr<Connection>> connections_;
  std::unique_ptr<Protocol> protocol_;

  BAN_COPY_AND_ASSIGN(Server);
};

#endif  // SRC_SERVER_H
