/**
 */

#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_

#include "common/basics.h"
#include "common/shared_ptr.h"
#include "src/event_type.h"

class EventHandler;
class Event {
 public:
  Event() {}
  ~Event() {}
  inline EventHandler* GetHandler() {
    return handler_.get();
  }

 private:
  EventType et_;
  IOMask io_mask_;
  shared_ptr<EventHandler> handler_;

  DO_NOT_COPY_AND_ASSIGN(Event);
};

#endif  // SRC_EVENT_H_
