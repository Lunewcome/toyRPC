#include "reactor/accept_handler.h"

#include <sys/socket.h>

#include "reactor/event_type.h"

AcceptHandler::AcceptHandler(ProtocolHandler* proto_handler)
    : EventHandler(proto_handler->GetEpoll()),
      proto_handler_(proto_handler) {
}

AcceptHandler::~AcceptHandler() {
  // delete proto_handler_;
}

void AcceptHandler::Read(int fd, void* client_data) {
  int conn_fd = accept(fd, nullptr, nullptr);
  if (conn_fd < 0) {
    return;
  }
  GetEpoll()->AddEvent(conn_fd, IOMaskRead, proto_handler_, nullptr);
}

void AcceptHandler::Write(int fd, void* client_data) {
  // do nothing? Maybe tell client that its request is rejected
  // when traffic is too heavy!
}
