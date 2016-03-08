/**
 */

#ifndef SRC_EVENT_HANDLER_H_
#define SRC_EVENT_HANDLER_H_

#include "common/basics.h"
#include "common/shared_ptr.h"
#include "src/event_type.h"
#include "src/multiplexer.h"

class Multiplexer;

class EventHandler {
 public:
  EventHandler() {
  }
  EventHandler(shared_ptr<Multiplexer> plexer) : plexer_(plexer) {
  }
  virtual ~EventHandler() {}
  inline void SetMultiplexer(shared_ptr<Multiplexer> plexer) {
    plexer_ = plexer;
  }
  virtual void Process(int fd, IOMask mask) = 0;

 protected:

 private:
  // Handler may use multiplexer to add/del event.
  shared_ptr<Multiplexer> plexer_;

  DO_NOT_COPY_AND_ASSIGN(EventHandler);
};

#endif  // SRC_EVENT_HANDLER_H_
