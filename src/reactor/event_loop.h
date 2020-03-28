/**
 */
#ifndef SRC_REACTOR_EVENT_LOOP_H
#define SRC_REACTOR_EVENT_LOOP_H

#include <memory>
using std::unique_ptr;

#include "common/basics.h"
#include "src/reactor/epoll.h"

// class EventLoop {
//  public:
//   EventLoop() : epoll_(new Epoll()) {};
//   ~EventLoop() {}
//   void Start() {
//     while (!stop_) {
//       epoll_->ProcessEvents();
//     }
//   }
//   void Stop() {
//     stop_ = true;
//   }
//   Epoll* MutableEpoll() {
//     return epoll_.get();
//   }
// 
//  private:
//   std::unique_ptr<Epoll> epoll_;
//   bool stop_ = false;
// 
//   DO_NOT_COPY_AND_ASSIGN(EventLoop);
// };

#endif  // SRC_REACTOR_EVENT_LOOP_H
