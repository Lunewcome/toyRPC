#include "connection.h"

#include "glog/logging.h"

void Connection::ProcessEpollInput(int sock_fd, void* client_data) {
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

int Connection::Send(const char* data, int sz) {
  last_active_timestamp_ = time(nullptr);
  int written = WriteImmediately(data, sz);
  if (Status() == Connection::Status::IDLE) {
    // It's lucky that all data has been sent in 'WriteImmediately'.
    GetGlobalEpoll()->DelEvent(GetSock(), IOMaskWrite);
    return written;
  } else if (Status() == Connection::Status::ERROR) {
    // Close immediately or wait a moment?
    return -1;
  } else {
    CHECK(Status() == Connection::Status::KEEP_WRITE);
    auto& out_buff = GetOutBuff();
    out_buff.Append(data + written, sz - written);
    if (GetGlobalEpoll()->AddWriteEvent(GetSock(), this) < 0) {
      VLOG(1) << "Fail to add write event for " << GetPeer();
      out_buff.PopBack(sz - written);
      // Close immediately or wait a moment?
      return -1;
    }
    return written;
  }
}

int Connection::ConnectIfNot(const std::string& ip, int port) {
//   memset(&srv_addr_, 0, sizeof(srv_addr_));
//   if (InitSock(ip, port) != 0) {
//     return -1;
//   }
//   while (true) {
//     if (connect(sock_fd_(), (struct sockaddr*)&srv_addr_,
//                 sizeof(srv_addr_)) == 0) {
//       break;
//     } else {
//       if (errno == EINTR) {
//         // do not continue if EINTR.
//         // https://blog.csdn.net/hnlyyk/article/details/51444617
//         return -1;
//       } else if (errno == EINPROGRESS && FLAGS_connect_non_blocking) {
//         if (GetGlobalEpoll()->AddWriteEvent(sock_fd_(), this) < 0) {
//           return -1;
//         }
//         // what if connect timeout?
//         // TODO...
//         OnConnected();
//         return 0;
//       } else {
//         LOG(ERROR) << strerror(errno);
//         return -1;
//       }
//     }
//   }
//   OnConnected();
  return 0;
}

int Connection::InitSock(const std::string& ip, int port) {
//   srv_addr_.sin_family = AF_INET;
//   srv_addr_.sin_port = htons(port);
//   if (inet_aton(ip.c_str(), &srv_addr_.sin_addr) != 1) {
//     VLOG(1) << "Bad ip?";
//     return -1;
//   }
//   sock_fd_.Reset(socket(AF_INET, SOCK_STREAM, 0));
//   if (sock_fd_ < 0) {
//     VLOG(1) << "Fail to create socket";
//     return -1;
//   }
//   if (FLAGS_connect_non_blocking && SetNonBlocking(sock_fd_()) != 0) {
//     VLOG(1) << "Fail to set nonblocking";
//     return -1;
//   }
//   if (SetCloseOnExec(sock_fd_()) == -1) {
//     VLOG(1) << "Fail to set cloexec.";
//     return -1;
//   }
  return 0;
}

void Connection::ProcessEpollOut(int sock_fd, void* client_data) {
  if (!client_data) {
    VLOG(3) << "error or hung fd:" << sock_fd;
    return;
  }
  auto* conn = static_cast<Connection*>(client_data);
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
  GetGlobalEpoll()->DelEvent(conn->GetSock(), IOMaskWrite);
}

int Connection::ReadUntilFail(int* save_errno) {
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

int Connection::WriteImmediately(const char* data, int sz) {
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

void Connection::SyncSend(const char* data, int sz) {
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
const std::string Connection::StatusToString(enum Status st) {
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

ConnectionManager* GetGlobalConnectionManager() {
  static ConnectionManager manager;
  return &manager;
}
