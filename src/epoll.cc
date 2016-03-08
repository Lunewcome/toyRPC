#include "src/epoll.h"

#include <cerrno>
#include <cstring>

void Epoll::Init() {
  epoll_fd_ = epoll_create(1024);
  if (epoll_fd_ < 0) {
    Log::WriteToDisk(ERROR,
                     "Fail to create epoll_fd:%s",
                     strerror(errno));
    abort();
  }
}

void Epoll::ProcessEvents() {
  Log::WriteToDisk(DEBUG, "epoll!");
  // epoll_wait()...
  // for () {
  //   event->GetHandler()->Process();
  // }
}

int Epoll::AddEvent(shared_ptr<Event> e) {
  int op = EPOLL_CTL_ADD;
  int old_events = 0;
  if (e->GetFd() <= int(events_.size()) &&
      events_[e->GetFd()]->IsValid()) {
    op = EPOLL_CTL_MOD;
    if (events_[e->GetFd()]->GetMask() & Read) {
      old_events |= EPOLLIN;
    }
    if (events_[e->GetFd()]->GetMask() & Write) {
      old_events |= EPOLLOUT;
    }
  }
  struct epoll_event* epoll_event = NULL;
  epoll_event->events = old_events;
  epoll_ctl(epoll_fd_, op, e->GetFd(), epoll_event);
  return 0;
}

int Epoll::DelEvent(shared_ptr<Event> e) {
  return 0;
}

