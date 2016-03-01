/**
 */

#include "common/basics.h"
#include "common/shared_ptr.h"


enum EventType {
  Read,
  Write
};

class Multiplexer {
 public:
  Multiplexer(const string& type) {}
  virtual ~Multiplexer() {}
  virtual int AddEvent(int fd,
                       EventType event_type,
                       shared_ptr<EventHandler> handler);
  virtual int DelEvent(int fd,
                       EventType event_type);

 private:

  DO_NOT_COPY_AND_ASSIGN(Multiplexer);
};
