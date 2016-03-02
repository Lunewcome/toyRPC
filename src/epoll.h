/**
 */

#include "common/basics.h"
#include "src/multiplexer.h"

#include <sys/epoll.h>


class Epoll : public Multiplexer {
 public:
  Epoll() {}
  virtual ~Epoll() {}
  virtual int AddEvent(int fd,
                       EventType event_type,
                       shared_ptr<EventHandler> handler);
  virtual int DelEvent(int fd,
                       EventType event_type);
  virtual void ProcessEvents() {}

 private:

  DO_NOT_COPY_AND_ASSIGN(Epoll);
};
