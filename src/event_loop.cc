#include "src/event_loop.h"

#include "common/flags.h"
#include "src/epoll.h"

DEFINE_string(multiplexer,
              "epoll",
              "epoll,select,poll");

EventLoop::EventLoop(const string& plexer)
    : stop_(0),
      plexer_(NULL) {
  Init(plexer);
}

void EventLoop::Init(const string& plexer) {
  if (plexer == FLAGS_multiplexer) {
    plexer_.reset(new Epoll());
  }
}
