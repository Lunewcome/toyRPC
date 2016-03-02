/**
 */

#include "common/basics.h"
#include "src/epoll.h"
#include "src/multiplexer.h"

class EventLoop {
 public:
  EventLoop() {}
  ~EventLoop() {}
  void Start() {
    plexer_->ProcessEvents();
  }
  void Stop() {}

 private:
  void Init() {
    plexer_.reset(new Epoll());
  }

  shared_ptr<Multiplexer> plexer_;

  DO_NOT_COPY_AND_ASSIGN(EventLoop);
};
