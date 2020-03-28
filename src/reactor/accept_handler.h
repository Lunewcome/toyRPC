/**
 */
#ifndef SRC_REACTOR_ACCEPT_HANDLER_H
#define SRC_REACTOR_ACCEPT_HANDLER_H

#include "common/basics.h"
#include "protocol/protocol_handler.h"
#include "reactor/event_handler.h"

class AcceptHandler : public EventHandler {
 public:
  AcceptHandler(ProtocolHandler* proto_handler);
  virtual ~AcceptHandler();
  virtual void Read(int fd, void* client_data) override;
  virtual void Write(int fd, void* client_data) override;

 private:
  ProtocolHandler* proto_handler_;

  DO_NOT_COPY_AND_ASSIGN(AcceptHandler);
};

#endif  // SRC_REACTOR_ACCEPT_HANDLER_H
