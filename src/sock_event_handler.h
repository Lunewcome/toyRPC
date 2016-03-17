/**
 */
#ifndef SRC_SOCK_EVENT_HANDLER_H_
#define SRC_SOCK_EVENT_HANDLER_H_

#include "common/basics.h"
#include "src/event_handler.h"

#include <unistd.h>

class SockEventHandler : public EventHandler {
 public:
  SockEventHandler() {}
  SockEventHandler(const shared_ptr<EventLoop>& el)
      : EventHandler(el) {}
  virtual ~SockEventHandler() {}
  virtual void Process(int fd,
                       IOMask mask,
                       void* client_data) {
    const string& str = "Wrote some str to peer~~~";
    write(fd, str.c_str(), str.size());
    el_->DelEvent(fd, mask);
  }

 private:

  DO_NOT_COPY_AND_ASSIGN(SockEventHandler);
};

#endif  // SRC_SOCK_EVENT_HANDLER_H_
