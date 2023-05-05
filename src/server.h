#ifndef SRC_SERVER_H
#define SRC_SERVER_H

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
  void Stop() { GetGlobalEpoll()->Stop(); }
  void RunUtilAskedToStop() { GetGlobalEpoll()->RunUtilAskedToStop(); }

 private:
  static void Accept(void* _this, int sock_fd);
  static void OnNewMsgReceived(void* _this, int sock_fd);
  const std::string& GetProtocolName() const { return options_.protocol; }
  void TellClient(Connection* conn, const char* msg, int code);

  ServerOptions options_;
  FDGuard server_fd_;
  Http protocol_http_;
  HttpRequest http_request_;

  BAN_COPY_AND_ASSIGN(Server);
};

#endif  // SRC_SERVER_H
