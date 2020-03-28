#include "reactor/epoll.h"

#include <cerrno>
#include <cstring>

#include "common/flags.h"
#include "glog/logging.h"
#include "reactor/event.h"

DEFINE_int32(epoll_wait_timeout, -1, "");

static const int32_t kMaxWaitingEvents = 1024;

Epoll::Epoll() {
 Init();
}

Epoll::~Epoll() {
  delete[] epoll_events_;
  for (auto e : events_) {
    delete e;
  }
}

void Epoll::Init() {
  epoll_fd_ = epoll_create(kMaxWaitingEvents);
  if (epoll_fd_ < 0) {
    abort();
  }
  epoll_events_ = new struct epoll_event[kMaxWaitingEvents];
  if (!epoll_events_) {
    close(epoll_fd_);
    abort();
  }
}

int Epoll::ProcessEvents() {
  int num = epoll_wait(epoll_fd_,
                       epoll_events_,
                       kMaxWaitingEvents,
                       FLAGS_epoll_wait_timeout);
  if (num == 0) {
    return 0;
  }
  if (num == -1) {
    return errno == EINTR ? 0 : -1;
  }
  for (int i = 0; i < num; ++i) {
    const struct epoll_event& ee = epoll_events_[i];
    int fd = ee.data.fd;
    if (ee.events & EPOLLIN) {
      events_[fd]->handler->Read(fd, events_[fd]->client_data);
    }
    if (ee.events & EPOLLOUT) {
      events_[fd]->handler->Write(fd, events_[fd]->client_data);
    }
    if (ee.events & EPOLLERR) {
      // close connection?
    }
    if (ee.events & EPOLLHUP) {
    }
  }
  // handle timeout.
  return num;
}

int Epoll::AddEvent(int fd, IOMask mask,
                    EventHandler* handler,
                    void* client_data) {
  int op = EPOLL_CTL_ADD;
  struct epoll_event epoll_event;
  epoll_event.events = 0;
  epoll_event.data.fd = fd;
  if (fd < static_cast<int>(events_.size())) {
    if (events_[fd] && HasReadEvent(*events_[fd])) {
      epoll_event.events |= EPOLLIN;
      op = EPOLL_CTL_MOD;
    }
    if (events_[fd] && HasWriteEvent(*events_[fd])) {
      epoll_event.events |= EPOLLOUT;
      op = EPOLL_CTL_MOD;
    }
  }
  if (mask & IOMaskRead) {
    epoll_event.events |= EPOLLIN;
  }
  if (mask & IOMaskWrite) {
    epoll_event.events |= EPOLLOUT;
  }
  int ret = epoll_ctl(epoll_fd_, op, fd, &epoll_event);
  if (ret) {
    return ret;
  } else {
    if (op == EPOLL_CTL_ADD) {
      if (fd >= static_cast<int>(events_.size())) {
        events_.resize(fd << 1);
        size_t itrt = fd;
        while (itrt < events_.size()) {
          events_[itrt++] = nullptr;
        }
      }
      events_[fd] = new Event(fd, client_data);
    }
    SetEvents(events_[fd], epoll_event.events);
    if (handler) {
      if (events_[fd]->handler) {
        // how?
//        delete events_[fd]->handler;
      }
      events_[fd]->handler = handler;
    }
    return ret;
  }
}

int Epoll::DelEvent(int fd, IOMask mask) {
  if (fd >= static_cast<int>(events_.size())) {
    return -1;
  }
  struct epoll_event epoll_event;
  epoll_event.events = events_[fd]->io_mask & mask;
  epoll_event.data.fd = fd;

  int op = EPOLL_CTL_DEL;
  if (epoll_event.events) {
    op = EPOLL_CTL_MOD;
  }

  int ret = epoll_ctl(epoll_fd_, op, fd, &epoll_event);
  if (ret) {
    // should we return without deleting it from events_?
    return -1;
  }
  if (op == EPOLL_CTL_DEL) {
    // a clearing should be sufficient.
    ClearAllEvent(events_[fd]);
  }
  return 0;
}
