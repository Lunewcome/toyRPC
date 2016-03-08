/**
 */

#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_

#include "common/basics.h"
#include "common/shared_ptr.h"
#include "src/event_type.h"
#include "src/event_handler.h"

class Event {
 public:
  Event() : et_(None), io_mask_(NoEvent), fd_(-1) {}
  Event(EventType et, IOMask mask, int fd)
      : et_(et), io_mask_(mask), fd_(fd) {}
  ~Event() {}
  inline EventHandler* GetHandler() {
    return handler_.get();
  }
  inline bool IsValid() const {
    return (fd_ > 0 &&
            et_ != None &&
            io_mask_ != NoEvent);
  }
  inline int GetFd() const { return fd_; }
  inline int GetType() const { return et_; }
  inline int GetMask() const { return io_mask_; }

 private:
  EventType et_;
  IOMask io_mask_;
  int fd_;
  shared_ptr<EventHandler> handler_;

  DO_NOT_COPY_AND_ASSIGN(Event);
};

#endif  // SRC_EVENT_H_
