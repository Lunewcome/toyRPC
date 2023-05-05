/**
 */
#ifndef SRC_EPOLL_H
#define SRC_EPOLL_H

#include <sys/epoll.h>
#include <vector>

#include "event.h"
#include "common/basics.h"
#include "common/fd_guard.h"

class Epoll {
 public:
  Epoll();
  ~Epoll();
  int AddEvent(const Event& e);
  int AddReadEvent(int fd, void* client_data) {
    Event e(fd, IOMaskRead, client_data);
    return AddEvent(e);
  }
  int AddWriteEvent(int fd, void* client_data) {
    Event e(fd, IOMaskWrite, client_data);
    return AddEvent(e);
  }
  int DelEvent(int fd, IOMask mask);
  void AwaitEpoll() { ProcessEvents(); }
  void RunUtilAskedToStop() {
    while (!stop_) {
      if (-1 == ProcessEvents()) {
        break;
      }
    }
  }
  void Stop() { stop_ = true; }

 private:
  int ProcessEvents();

  bool stop_ = false;
  FDGuard epoll_fd_;
  struct epoll_event* epoll_events_ = nullptr;
  std::vector<Event> events_;

  BAN_COPY_AND_ASSIGN(Epoll);
};

Epoll* GetGlobalEpoll();

#endif  // SRC_EPOLL_H
