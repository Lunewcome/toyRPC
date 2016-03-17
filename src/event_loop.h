/**
 */
#ifndef SRC_EVENT_LOOP_H_
#define SRC_EVENT_LOOP_H_

#include "common/basics.h"
#include "common/log.h"
#include "src/event.h"
#include "src/event_handler.h"
#include "src/epoll.h"
#include "src/multiplexer.h"

DEFINE_string(multiplexer, "epoll", "epoll,select,poll");

class EventLoop {
 public:
  EventLoop(const string& plexer)
      : stop_(0),
        plexer_(NULL) {
    Init(plexer);
  }
  ~EventLoop() {}
  inline void Start() {
    while (!stop_) {
      plexer_->ProcessEvents();
    }
  }
  inline void Stop() { stop_ = 1; }

  int AddEvent(const shared_ptr<Event>& e) {
    return plexer_->AddEvent(e);
  }
  int AddEvent(
      int fd,
      IOMask mask,
      const shared_ptr<EventHandler>& read_handler,
      const shared_ptr<EventHandler>& write_handler,
      void* client_data) {
    return plexer_->AddEvent(fd,
                             mask,
                             read_handler,
                             write_handler,
                             client_data);
  }
  int DelEvent(shared_ptr<Event> e) {
    return plexer_->DelEvent(e);
  }
  int DelEvent(int fd, IOMask mask) {
    return plexer_->DelEvent(fd, mask);
  }

 private:
  void Init(const string& plexer) {
    if (plexer == "epoll") {
      plexer_.reset(new Epoll());
    }
  }

  int stop_;
  shared_ptr<Multiplexer> plexer_;

  DO_NOT_COPY_AND_ASSIGN(EventLoop);
};

#endif  // SRC_EVENT_LOOP_H_
