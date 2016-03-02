/**
 */

#ifndef SRC_MULTIPLEXER_H_
#define SRC_MULTIPLEXER_H_

#include "common/basics.h"
#include "common/shared_ptr.h"
#include "src/event_handler.h"

#include <string>
using std::string;

enum EventType {
  Read,
  Write/*,
  Timeout
  */
};

class Multiplexer {
 public:
  Multiplexer() {}
  virtual ~Multiplexer() {}
  virtual int AddEvent(int fd,
                       EventType event_type,
                       shared_ptr<EventHandler> handler) = 0;
  virtual int DelEvent(int fd,
                       EventType event_type) = 0;
  virtual void ProcessEvents() = 0;

 private:

  DO_NOT_COPY_AND_ASSIGN(Multiplexer);
};

#endif  // SRC_MULTIPLEXER_H_
