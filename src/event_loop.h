/**
 */

#include "common/basics.h"
#include "common/log.h"
#include "src/epoll.h"
#include "src/multiplexer.h"

DEFINE_string(multiplexer, "epoll", "epoll,select,poll");

class EventLoop {
 public:
  EventLoop(const shared_ptr<Multiplexer>& plexer)
      : stop_(0),
        plexer_(plexer) {
    Init();
  }
  ~EventLoop() {}
  inline void Start() {
    while (!stop_) {
      plexer_->ProcessEvents();
    }
  }
  inline void Stop() { stop_ = 1; }

 private:
  void Init() {}

  int stop_;
  shared_ptr<Multiplexer> plexer_;

  DO_NOT_COPY_AND_ASSIGN(EventLoop);
};
