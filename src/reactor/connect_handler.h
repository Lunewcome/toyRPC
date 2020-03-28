/**
 */
#ifndef SRC_REACTOR_CONNECT_HANDLER_H
#define SRC_REACTOR_CONNECT_HANDLER_H

#include "common/basics.h"
#include "reactor/event_handler.h"
#include "iobuffer/iobuffer.h"
#include "protocol/protocol_handler.h"

class ConnectHandler : public EventHandler {
 public:
  ConnectHandler(ProtocolHandler* protocol_handler)
       : protocol_handler_(protocol_handler) {}
  virtual ~ConnectHandler() {}
  virtual void Read(int fd, void* client_data) override;
  virtual void Write(int fd, void* client_data) override;
  ProtocolHandler* GetProtoHandler() {
    return protocol_handler_;
  }

 private:
  vector<IOBuffer*> buffer_;
  ProtocolHandler* protocol_handler_ = nullptr;

  DO_NOT_COPY_AND_ASSIGN(ConnectHandler);
};

#endif  // SRC_REACTOR_CONNECT_HANDLER_H
