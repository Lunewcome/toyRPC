#include "server/server.h"

Server::Server() {
}

Server::~Server() {
}

bool Server::Start(const ServerOption& options) {
  options_ = options;
  auto port = options_.port;
  int conn_fd = CreateTcpServer(port);
  if (conn_fd < 0) {
    LOG(ERROR) << "Failed to create server on port " << port;
    return false;
  }
  ApplicationHandler* app_handler = new ApplicationHandler(&el_);

  ProtocolHandler* protocol_handler = new ProtocolHandler(app_handler);

  AcceptHandler* accept_handler = new AcceptHandler(protocol_handler);
  el_.AddEvent(conn_fd, IOMaskRead, accept_handler, nullptr);
  el_.RunUtilAskedToStop();
  return true;
}

void Server::Stop() {
  el_.Stop();
}

int AddService(Service* s, const string& uri_method_mappings) {
  return 0;
}
