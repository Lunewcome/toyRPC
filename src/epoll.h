/**
 */

#include "common/basics.h"
#include "common/log.h"
#include "common/shared_ptr.h"
#include "src/event.h"
#include "src/multiplexer.h"

#include <sys/epoll.h>


class Epoll : public Multiplexer {
 public:
  Epoll() {}
  virtual ~Epoll() {}
  virtual int AddEvent(shared_ptr<Event> e) {
    return 0;
  }
  virtual int DelEvent(shared_ptr<Event> e) {
    return 0;
  }
  virtual void ProcessEvents() {
    Log::WriteToDisk(DEBUG, "epoll!");
  }

 private:
  int epoll_fd_;

  DO_NOT_COPY_AND_ASSIGN(Epoll);
};
