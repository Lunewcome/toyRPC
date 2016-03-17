/**
 */
#ifndef SRC_EPOLL_H_
#define SRC_EPOLL_H_

#include "common/basics.h"
#include "common/log.h"
#include "common/shared_ptr.h"
#include "src/event.h"
#include "src/multiplexer.h"

#include <sys/epoll.h>

class Epoll : public Multiplexer {
 public:
  Epoll() { Init(); }
  virtual ~Epoll() {}
  virtual int AddEvent(const shared_ptr<Event>& e) {
    return AddEvent(e->GetFd(),
                    e->GetMask(),
                    e->GetReadHandler(),
                    e->GetWriteHandler(),
                    e->GetClientData());
  }
  virtual int AddEvent(
      int fd,
      IOMask mask,
      const shared_ptr<EventHandler>& read_handler,
      const shared_ptr<EventHandler>& write_handler,
      void* client_data);
  virtual int DelEvent(shared_ptr<Event> e) {
    return DelEvent(e->GetFd(), e->GetMask());
  }
  virtual int DelEvent(int fd, IOMask mask);
  virtual int ProcessEvents();

 private:
  void Init();

  int epoll_fd_;
  struct epoll_event* epoll_events_;

  DO_NOT_COPY_AND_ASSIGN(Epoll);
};

#endif  // SRC_EPOLL_H_
