#include "protocol/protocol_handler.h"

#include <unistd.h>

void ProtocolHandler::Read(int fd, void* client_data) {
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
    if (CheckIntegrity(*buffer_[fd])) {
      ProcessProtocol(fd, buffer_[fd]);
    }
  }
}

void ProtocolHandler::Write(int fd, void* cd) {
}
