#include "sock_handler.h"

#include "glog/logging.h"

void SockHandler::ProcessEpollInput(int sock_fd, void* client_data) {
  if (!client_data) {
    VLOG(3) << "error or hung fd:" << sock_fd;
    return;
  }
  auto* opt = static_cast<SocketOptions*>(client_data);
  if (opt->conn.get()) {
    opt->conn->last_active_timestamp_ = time(nullptr);
  }
  CHECK_EQ(sock_fd, opt->sock_fd);
  opt->on_level_triggered_event(opt->arg, sock_fd);
}

int SockHandler::Send(const char* data, int sz) {
  last_active_timestamp_ = time(nullptr);
  int written = WriteImmediately(data, sz);
  if (Status() == SockHandler::Status::IDLE) {
    // It's lucky that all data has been sent in 'WriteImmediately'.
    epl_.DelEvent(GetSock(), IOMaskWrite);
    return written;
  } else if (Status() == SockHandler::Status::ERROR) {
    // Close immediately or wait a moment?
    return -1;
  } else {
    CHECK(Status() == SockHandler::Status::KEEP_WRITE);
    auto& out_buff = GetOutBuff();
    out_buff.Append(data + written, sz - written);
    if (epl_.AddWriteEvent(GetSock(), this) < 0) {
      VLOG(1) << "Fail to add write event for " << GetPeer();
      out_buff.PopBack(sz - written);
      // Close immediately or wait a moment?
      return -1;
    }
    return written;
  }
}

void SockHandler::ProcessEpollOut(int sock_fd, void* client_data) {
  if (!client_data) {
    VLOG(3) << "error or hung fd:" << sock_fd;
    return;
  }
  auto* conn = static_cast<SockHandler*>(client_data);
  conn->last_active_timestamp_ = time(nullptr);
  CHECK_EQ(conn->GetSock(), sock_fd);
  while (true) {
    int rc = conn->GetOutBuff().WriteToSocek(sock_fd);
    if (rc < 0) {
      if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
        VLOG(1) << "Fail to write. " << strerror(errno);
        // Fail.
        break;
      }
    } else if (rc == 0) {
      // done.
      break;
    } else {
      conn->GetOutBuff().PopFront(rc);
      // continue;
    }
  }
  conn->epl_.DelEvent(conn->GetSock(), IOMaskWrite);
}

int SockHandler::ReadUntilFail(int* save_errno) {
  int rc;
  while (true) {
    rc = in_buff_.ReadFromSock(sock_);
    if (rc == 0 || (rc < 0 && errno != EINTR)) {
      *save_errno = errno;
      break;
    }
  }
  return rc;
}

int SockHandler::WriteImmediately(const char* data, int sz) {
  int len = write(sock_, data, sz);
  if (len < 0) {
    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
      // KEEP_WRITE needs this val.
      len = 0;
      status_ = Status::KEEP_WRITE;
    } else {
      status_ = Status::ERROR;
    }
  } else {
    status_ = (len == sz) ? Status::IDLE : Status::KEEP_WRITE;
  }
  return len;
}

void SockHandler::SyncSend(const char* data, int sz) {
  int written = 0;
  int max_tries = 5;
  while (written < sz && max_tries--) {
    int len = WriteImmediately(data, sz);
    if (len > 0) {
      written += len;
    }
    if (len == 0) {
      continue;
    }
    if (len < 0) {
      break;
    }
  }
}

#define StatusCase(st) \
  case Status::st : return #st
const std::string SockHandler::StatusToString(enum Status st) {
  switch (st) {
    StatusCase(IDLE);
    StatusCase(CONNECTING);
    StatusCase(CONNECTED);
    StatusCase(CLOSING);
    StatusCase(CLOSED);
    StatusCase(READ_OK);
    StatusCase(WAITING_BUFFER_SPACE);
    StatusCase(WRITING);
    StatusCase(KEEP_WRITE);
    StatusCase(ERROR);
    default:
      return "UNKNOWN";
  }
}
