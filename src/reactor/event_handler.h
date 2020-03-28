/**
 */
#ifndef SRC_REACTOR_EVENT_HANDLER_H
#define SRC_REACTOR_EVENT_HANDLER_H

#include "common/basics.h"
#include "reactor/epoll.h"

class EventHandler {
 public:
  explicit EventHandler(Epoll* epoll) : epoll_(epoll) {}
  virtual ~EventHandler() {}
  void Close(int fd) {
    close(fd);
  }
  virtual void Read(int fd, void* client_data) = 0;
  virtual void Write(int fd, void* client_data) = 0;
  Epoll* GetEpoll() {
    return epoll_;
  }

 private:
  Epoll* epoll_ = nullptr;

  DO_NOT_COPY_AND_ASSIGN(EventHandler);
};

#endif  // SRC_REACTOR_EVENT_HANDLER_H
