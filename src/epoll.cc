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
      events_[e->GetFd()].get() &&
      events_[e->GetFd()]->IsValid()) {
    op = EPOLL_CTL_MOD;
    if (events_[e->GetFd()]->GetMask() & Read) {
      old_events |= EPOLLIN;
    }
    if (events_[e->GetFd()]->GetMask() & Write) {
      old_events |= EPOLLOUT;
    }
  }
  struct epoll_event* epoll_event;
  if (op == EPOLL_CTL_ADD) {
    epoll_event = new struct epoll_event;
    e->SetPlexerPrivateData(
        static_cast<void*>(epoll_event));
  } else {
    epoll_event =
        static_cast<struct epoll_event*>(
            e->GetPlexerPrivateData());
  }
  epoll_event->events = old_events | e->GetMask();
  epoll_event->data.fd = e->GetFd();
  int ret = epoll_ctl(epoll_fd_, op, e->GetFd(), epoll_event);
  if (ret) {
    Log::WriteToDisk(ERROR,
                     "epoll_ctl:%s",
                     strerror(errno));
  } else {
    // keep at least ...
    events_.reserve(e->GetFd());
    events_[e->GetFd()] = e;
  }
  return ret;
}

int Epoll::DelEvent(shared_ptr<Event> e) {
  if (e->GetFd() >= int(events_.size()) ||
      !events_[e->GetFd()].get() ||
      !events_[e->GetFd()]->IsValid()) {
    return -1;
  }
  struct epoll_event* epoll_events = NULL;
  int io_mask = events_[e->GetFd()]->GetMask();
  if (io_mask & Read && !(e->GetMask() & Read)) {
    epoll_events->events |= EPOLLIN;
  }
  if (io_mask & Write && !(e->GetMask() & Write)) {
    epoll_events->events |= EPOLLOUT;
  }
  if (!epoll_events->events) {
    // no event to wait!
    int ret = epoll_ctl(epoll_fd_,
                        EPOLL_CTL_DEL,
                        e->GetFd(),
                        epoll_events);
    if (ret) {
      Log::WriteToDisk(ERROR,
                       "epoll_ctl, Del error:%s",
                       strerror(errno));
      // should we return without
      // deleting it from events_?
      return -1;
    }
    events_[e->GetFd()].reset(NULL);
  }
  int ret = epoll_ctl(epoll_fd_,
                      EPOLL_CTL_MOD,
                      e->GetFd(),
                      epoll_events);
  if (ret) {
    Log::WriteToDisk(ERROR,
                     "epoll_ctl, Mod err:%s",
                     strerror(errno));
    return -1;
  }
  return 0;
}
