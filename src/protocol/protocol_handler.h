#ifndef SRC_PROTOCOL_PROTOCOL_HANDLER_H
#define SRC_PROTOCOL_PROTOCOL_HANDLER_H

#include "common/basics.h"
#include "application/application_handler.h"
#include "iobuffer/iobuffer.h"
#include "reactor/event_handler.h"

class ProtocolHandler : public EventHandler {
 public:
  explicit ProtocolHandler(ApplicationHandler* app_handler)
      : EventHandler(app_handler->GetEpoll()),
        app_handler_(app_handler) {}
  virtual ~ProtocolHandler() {}
  virtual void Read(int fd, void* cd) override;
  virtual void Write(int fd, void* cd) override;
  bool CheckIntegrity(const IOBuffer& buf) const {
    return true;
  }

 private:
  void ProcessProtocol(int32_t fd, IOBuffer* buf) {
    buf->SeekTo(protocol_data_len_);
    app_handler_->Process(fd, buf);
    buf->Clear();
  }

  vector<IOBuffer*> buffer_;
  ApplicationHandler* app_handler_;
  int32_t protocol_data_len_ = 0;

  DO_NOT_COPY_AND_ASSIGN(ProtocolHandler);
};

#endif  // SRC_PROTOCOL_PROTOCOL_HANDLER_H
