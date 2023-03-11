#ifndef COMMON_FD_GUARD_H
#define COMMON_FD_GUARD_H

#include <unistd.h>

#include "glog/logging.h"

class FDGuard {
 public:
  FDGuard() : fd_(-1) {}
  FDGuard(int fd) : fd_(fd) {}
  ~FDGuard() { 
    if (fd_ > 0) {
      close(fd_);
    }
  }
  int Release() {
    int tmp = fd_;
    fd_ = 0;
    return tmp;
  }
  void Reset(int fd) { fd_ = fd; }
  int operator()() const {
    CHECK(fd_ > 0) << "trying to use an invalid fd.";
    return fd_;
  }

 private:
  int fd_;

  FDGuard(const FDGuard&) = delete;
  FDGuard& operator=(const FDGuard&) = delete;
};

#endif  // COMMON_FD_GUARD_H
