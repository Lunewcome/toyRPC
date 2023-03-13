#ifndef SRC_TOYRPC_SERVER_H
#define SRC_TOYRPC_SERVER_H

#include <memory>
#include <unordered_map>

#include "common/fd_guard.h"
#include "connection.h"
#include "epoll.h"

#include "protocols/echo.h"

struct ServerOptions {
  uint32_t port;
  std::string protocol;
};

class toyRPCServer {
 public:
  toyRPCServer(const ServerOptions& options) : options_(options) {}
  void Start();
  void Stop() { GetGlobalEpoll().Stop(); }
  void RunUtilAskedToStop() { GetGlobalEpoll().RunUtilAskedToStop(); }

 private:
  void Accept(int sock_fd);
  void OnNewMsgReceived(int sock_fd);
  void AddToSocks(std::unique_ptr<SocketOptions>& options) {
    CHECK_EQ(socks_.count(options->sock_fd), 0uL);
    socks_[options->sock_fd].swap(options);
  }
  Connection* GetConnection(int fd);
  void RemoveConnection(int sock_fd);
  const std::string& GetProtocolName() const { return options_.protocol; }

  ServerOptions options_;
  FDGuard server_fd_;
  std::unordered_map<int, std::unique_ptr<SocketOptions>> socks_;

  BAN_COPY_AND_ASSIGN(toyRPCServer);
};

#endif  // SRC_TOYRPC_SERVER_H
