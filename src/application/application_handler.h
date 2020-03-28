#ifndef SRC_APPLICATION__APPLICATION_HANDLER_H
#define SRC_APPLICATION__APPLICATION_HANDLER_H

#include <iostream>
#include <unistd.h>
using std::cout;
using std::endl;

#include "common/basics.h"
#include "iobuffer/iobuffer.h"
#include "reactor/epoll.h"
#include "reactor/event_handler.h"

class Writer : public EventHandler {
 public:
  Writer(Epoll* epoll) : EventHandler(epoll),  epoll_(epoll) {}
  virtual ~Writer() {}
  virtual void Read(int fd, void*) override {
  }
  virtual void Write(int fd, void*) override {
    const string& msg = "HTTP/1.1 200 OK\r\nContent-Lenght: 7\r\n\r\nI'm ok!";
    write(fd, msg.c_str(), msg.size());
    epoll_->DelEvent(fd, IOMaskWrite);
    Close(fd);
  }

 private:
  Epoll* epoll_;

  DO_NOT_COPY_AND_ASSIGN(Writer);
};

class ApplicationHandler : public EventHandler {
 public:
  explicit ApplicationHandler(Epoll* epoll) : EventHandler(epoll) {}
  virtual ~ApplicationHandler() {}
  virtual void Read(int fd, void* client_data) override {}
  virtual void Write(int fd, void* client_data) override {}
  virtual void Process(int32_t fd, IOBuffer* buf) {
    cout << buf->ToString() << endl;
    Writer* w = new Writer(GetEpoll());
    GetEpoll()->AddEvent(fd, IOMaskWrite, w, nullptr);
  }

 private:

  DO_NOT_COPY_AND_ASSIGN(ApplicationHandler);
};

#endif  // SRC_APPLICATION__APPLICATION_HANDLER_H
