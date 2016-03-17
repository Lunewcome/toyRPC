#include "src/epoll.h"

#include "common/flags.h"
#include "src/event.h"

#include <cerrno>
#include <cstring>

DEFINE_int32(max_waiting_events, 1024, "");
DEFINE_int32(epoll_wait_timeout, -1, "");

void Epoll::Init() {
  epoll_fd_ = epoll_create(1024);
  if (epoll_fd_ < 0) {
    Log::WriteToDisk(ERROR,
                     "Fail to create epoll_fd:%s",
                     strerror(errno));
    abort();
  }
  epoll_events_ =
      new struct epoll_event[FLAGS_max_waiting_events];
  if (!epoll_events_) {
    Log::WriteToDisk(ERROR,
                     "Fail to new epoll_events_:%s",
                     strerror(errno));
    abort();
  }
}

int Epoll::ProcessEvents() {
  Log::WriteToDisk(DEBUG, "epoll now...");
  int ret = epoll_wait(epoll_fd_,
                       epoll_events_,
                       FLAGS_max_waiting_events,
                       FLAGS_epoll_wait_timeout);
  Log::WriteToDisk(DEBUG, "epoll wait finish:%d", ret);
  if (ret == 0) {
    return 0;
  }
  if (ret == -1) {
    Log::WriteToDisk(ERROR, "epoll err:%s", strerror(errno));
    return errno == EINTR ? 0 : -1;
  }
  for (int i = 0; i < ret; ++i) {
    const struct epoll_event& ee = epoll_events_[i];
    int fd = ee.data.fd;
    if (ee.events & EPOLLIN) {
      events_[fd]->GetReadHandler()->Process(
          fd,
          Read,
          events_[fd]->GetClientData());
    }
    if (ee.events & EPOLLOUT) {
      events_[fd]->GetWriteHandler()->Process(
          fd,
          Write,
          events_[fd]->GetClientData());
    }
    if (ee.events & EPOLLERR) {
      Log::WriteToDisk(ERROR,
                       "Now EPOLLERR:%s",
                       strerror(errno));
    }
    if (ee.events & EPOLLHUP) {
      Log::WriteToDisk(ERROR,
                       "Now EPOLLHUP:%s",
                       strerror(errno));
    }
  }
  return ret;
}

int Epoll::AddEvent(
    int fd,
    IOMask mask,
    const shared_ptr<EventHandler>& read_handler,
    const shared_ptr<EventHandler>& write_handler,
    void* client_data) {
  int op = EPOLL_CTL_ADD;
  struct epoll_event epoll_event;
  epoll_event.events = 0;
  epoll_event.data.fd = fd;
  // If some event(s) has been added.
  if (fd <= int(events_.size()) &&
      events_[fd].get() &&
      events_[fd]->IsValid()) {
    op = EPOLL_CTL_MOD;
    if (events_[fd]->GetMask() & Read) {
      epoll_event.events |= EPOLLIN;
    }
    if (events_[fd]->GetMask() & Write) {
      epoll_event.events |= EPOLLOUT;
    }
  }
  if (mask & Read) {
    epoll_event.events |= EPOLLIN;
  }
  if (mask & Write) {
    epoll_event.events |= EPOLLOUT;
  }
  int ret = epoll_ctl(epoll_fd_, op, fd, &epoll_event);
  if (ret) {
    Log::WriteToDisk(ERROR,
                     "epoll_ctl:%s",
                     strerror(errno));
    return ret;
  } else {
    if (op == EPOLL_CTL_ADD) {
      if (fd >= int(events_.size())) {
        events_.reserve(fd + 1);
        events_.resize(fd + 1);
      }
      events_[fd].reset(new Event(IO,
                                  mask,
                                  fd,
                                  read_handler,
                                  write_handler,
                                  client_data));
    } else {
      events_[fd]->GetMutableMask() = epoll_event.events;
    }
    // update handler
    if ((fd & Read) && read_handler.get()) {
      events_[fd]->SetReadHandler(read_handler);
    }
    if ((fd & Write) && write_handler.get()) {
      events_[fd]->SetWriteHandler(write_handler);
    }
    Log::WriteToDisk(DEBUG, "Add one event!");
    return ret;
  }
  // must be an error!
  return -1;

}

int Epoll::DelEvent(int fd, IOMask mask) {
  if (fd >= int(events_.size()) ||
      !events_[fd].get() ||
      !events_[fd]->IsValid()) {
    return -1;
  }
  struct epoll_event epoll_event;
  epoll_event.events = 0;
  epoll_event.data.fd = fd;
  int io_mask = events_[fd]->GetMask();
  if (io_mask & Read && !(mask & Read)) {
    epoll_event.events |= EPOLLIN;
  }
  if (io_mask & Write && !(mask & Write)) {
    epoll_event.events |= EPOLLOUT;
  }
  int op = !epoll_event.events ?
      EPOLL_CTL_DEL : EPOLL_CTL_MOD;
  if (op == EPOLL_CTL_DEL) {
    Log::WriteToDisk(ERROR, "Delete event!");
  } else {
    Log::WriteToDisk(ERROR, "Mod event!");
  }
  int ret = epoll_ctl(epoll_fd_,
                      op,
                      fd,
                      &epoll_event);
  if (ret) {
    Log::WriteToDisk(ERROR,
                     "epoll_ctl, Del error:%s",
                     strerror(errno));
    // should we return without
    // deleting it from events_?
    return -1;
  }
  if (op == EPOLL_CTL_DEL) {
    events_[fd].reset(NULL);
  } else {
    events_[fd]->GetMutableMask() &= ~mask;
    // reset handler
    shared_ptr<EventHandler> empty_handler(NULL);
    if (mask & Read) {
      events_[fd]->SetReadHandler(empty_handler);
    }
    if (mask & Write) {
      events_[fd]->SetWriteHandler(empty_handler);
    }
  }
  return 0;
}
