/**
 */

#ifndef SRC_EVENT_HANDLER_H_
#define SRC_EVENT_HANDLER_H_

#include "common/basics.h"
#include "common/shared_ptr.h"
#include "src/event_type.h"

class EventLoop;
class EventHandler {
 public:
  EventHandler() : el_(NULL) {}
  EventHandler(const shared_ptr<EventLoop>& el) : el_(el) {}
  virtual ~EventHandler() {}
  inline void SetEventLoop(const shared_ptr<EventLoop>& el) {
    el_ = el;
  }
  virtual void Process(int fd,
                       IOMask mask,
                       void* client_data) = 0;

 protected:
  // Handler may use EventLoop to add/del event.
  shared_ptr<EventLoop> el_;

 private:
  DO_NOT_COPY_AND_ASSIGN(EventHandler);
};

#endif  // SRC_EVENT_HANDLER_H_
