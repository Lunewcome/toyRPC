#ifndef SRC_SERVER_H
#define SRC_SERVER_H

#include <memory>
#include <unordered_map>

#include "common/fd_guard.h"
#include "connection.h"
#include "epoll.h"

#include "protocols/http.h"

struct ServerOptions {
  uint32_t port;
  std::string protocol;
};

class Server {
 public:
  Server(const ServerOptions& options) : options_(options) {}
  void Start();
  void Stop() { GetGlobalEpoll().Stop(); }
  void RunUtilAskedToStop() { GetGlobalEpoll().RunUtilAskedToStop(); }

 private:
  static void Accept(void* _this, int sock_fd);
  static void OnNewMsgReceived(void* _this, int sock_fd);
  void AddToSocks(std::unique_ptr<SocketOptions>& options) {
    CHECK_EQ(socks_.count(options->sock_fd), 0uL);
    socks_[options->sock_fd].swap(options);
  }
  Connection* GetConnection(int fd) {
    const auto& itrt_sock = socks_.find(fd);
    return itrt_sock == socks_.end() ? nullptr : itrt_sock->second->conn.get();
  }
  void RemoveConnection(int sock_fd);
  const std::string& GetProtocolName() const { return options_.protocol; }
  void TellClient(Connection* conn, const char* msg, int code);
  void RemoveDeadConnection();

  ServerOptions options_;
  FDGuard server_fd_;
  std::unordered_map<int, std::unique_ptr<SocketOptions>> socks_;

  Http protocol_http_;
  HttpRequest http_request_;

  BAN_COPY_AND_ASSIGN(Server);
};

#endif  // SRC_SERVER_H
