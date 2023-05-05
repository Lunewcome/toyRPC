#include "epoll.h"

#include <cerrno>
#include <cstring>
#include <unistd.h>

#include "connection.h"
#include "event.h"
#include "glog/logging.h"

static constexpr int32_t kEpollWaitTimeout = -1;
static constexpr int32_t kMaxWaitingEvents = 1024;

Epoll global_epl;
Epoll* GetGlobalEpoll() {
  return &global_epl;
}

Epoll::Epoll() {
  epoll_fd_.Reset(epoll_create(kMaxWaitingEvents));
  if (epoll_fd_() < 0) {
    throw(std::bad_alloc());
  }
  epoll_events_ = new struct epoll_event[kMaxWaitingEvents];
  events_.emplace_back();
}

Epoll::~Epoll() {
  delete[] epoll_events_;
}

int Epoll::ProcessEvents() {
  int num = epoll_wait(epoll_fd_(), epoll_events_, kMaxWaitingEvents,
                       kEpollWaitTimeout);
  if (num == -1) {
    return errno == EINTR ? 0 : -1;
  }
  for (int i = 0; i < num; ++i) {
    const struct epoll_event& ee = epoll_events_[i];
    int fd = ee.data.fd;
    if (ee.events & (EPOLLIN | EPOLLERR | EPOLLHUP)) {
      Connection::ProcessEpollInput(fd, events_[fd].client_data);
    }
    if (ee.events & (EPOLLOUT | EPOLLERR | EPOLLHUP)) {
      Connection::ProcessEpollOut(fd, events_[fd].client_data);
    }
  }
  return num;
}

int Epoll::AddEvent(const Event& e) {
  int fd = e.fd;
  int op = -1;
  struct epoll_event epoll_event = {0};
  epoll_event.data.fd = fd;
  if (fd >= static_cast<int>(events_.size())) {
    // a waste.
    events_.resize(fd << 1);
    op = EPOLL_CTL_ADD;
  } else {
    epoll_event.events |= events_[fd].WaitingRead() ? EPOLLIN : 0;
    epoll_event.events |= events_[fd].WaitingWrite() ? EPOLLOUT : 0;
    op = epoll_event.events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
  }
  epoll_event.events |= e.io_mask & IOMaskRead ? EPOLLIN : 0;
  epoll_event.events |= e.io_mask & IOMaskWrite ? EPOLLOUT : 0;
  int ret = epoll_ctl(epoll_fd_(), op, fd, &epoll_event);
  if (!ret) {
    events_[fd] = e;
  }
  return ret;
}

int Epoll::DelEvent(int fd, IOMask mask) {
  if (fd >= static_cast<int>(events_.size())) {
    return -1;
  }
  struct epoll_event epoll_event = {0};
  epoll_event.events = events_[fd].io_mask & ~mask;
  epoll_event.data.fd = fd;
  int op = epoll_event.events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  if (op == EPOLL_CTL_DEL) {
    events_[fd].Reset();
  }
  return epoll_ctl(epoll_fd_(), op, fd, &epoll_event);
}
