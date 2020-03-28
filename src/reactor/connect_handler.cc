#include "reactor/connect_handler.h"

#include <unistd.h>
#include <glog/logging.h>

void ConnectHandler::Read(int fd, void* client_data) {
  char buf[1024 + 1];
  ssize_t len = read(fd, buf, 1024);
  if (len < 0) {
    // error.
  } else if (len == 0) {
    // end of connection.
    Close(fd);
  } else {
    buf[len] = '\0';
    if (fd >= static_cast<int>(buffer_.size())) {
      buffer_.resize(fd * 2);
    }
    if (buffer_[fd]) {
      buffer_[fd]->Append(buf);
    } else {
      buffer_[fd] = new IOBuffer(buf);
    }
    if (protocol_handler_->CheckIntegrity(*buffer_[fd])) {
      protocol_handler_->ProcessProtocol(buffer_[fd]);
    }
  }
}

void ConnectHandler::Write(int fd, void* client_data) {
}
