#ifndef SRC_SERVER_SERVER_H
#define SRC_SERVER_SERVER_H

#include "common/basics.h"
#include "glog/logging.h"
#include "reactor/accept_handler.h"
#include "reactor/event_loop.h"
#include "service/service.h"
#include "net/net.h"

struct ServerOption {
  uint32_t port = 0;
};

class Server {
 public:
  Server();
  ~Server();
  bool Start(const ServerOption& options);
  void Stop();
  int AddService(Service* s, const string& uri_method_mappings);

 private:
  Epoll el_;
  ServerOption options_;

  DO_NOT_COPY_AND_ASSIGN(Server);
};

#endif  // SRC_SERVER_SERVER_H
