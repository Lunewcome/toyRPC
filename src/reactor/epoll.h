/**
 */
#ifndef SRC_REACTOR_EPOLL_H
#define SRC_REACTOR_EPOLL_H

#include <sys/epoll.h>
#include <vector>
using std::vector;

#include "common/basics.h"
#include "reactor/event_type.h"

class Event;
class EventHandler;

class Epoll {
 public:
  Epoll();
  ~Epoll();
  int AddEvent(Event* e);
  int AddEvent(int fd, IOMask mask, EventHandler* handler, void* client_data);
  int DelEvent(int fd, IOMask mask);
  void RunUtilAskedToStop() {
    while (!stop_) {
      ProcessEvents();
    }
  }
  void Stop() {
    stop_ = true;
  }

 private:
  void Init();
  int ProcessEvents();

  bool stop_ = false;
  int epoll_fd_;
  struct epoll_event* epoll_events_;
  vector<Event*> events_;

  DO_NOT_COPY_AND_ASSIGN(Epoll);
};

#endif  // SRC_REACTOR_EPOLL_H
