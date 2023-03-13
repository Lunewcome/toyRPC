#include "connection.h"

#include "glog/logging.h"

void Connection::ProcessEpollInput(int sock_fd, void* client_data) {
  if (!client_data) {
    VLOG(3) << "error or hung fd:" << sock_fd;
    return;
  }
  auto* opt = static_cast<SocketOptions*>(client_data);
  CHECK_EQ(sock_fd, opt->sock_fd);
  opt->on_level_triggered_event(sock_fd);
}

int Connection::Send(const char* data, int sz, WriteRequest* req) {
  int written = WriteImmediately(data, sz);
  auto& out_buff = GetOutBuff();
  if (Status() == Connection::Status::IDLE) {
    // 1. It's lucky that all data has been sent in 'WriteImmediately'.
    GetGlobalEpoll().DelEvent(GetSock(), IOMaskWrite);
    return written;
  } else if (Status() == Connection::Status::ERROR) {
    // Close?
//    if (cb_interface_.on_write_error) {
//      cb_interface_.on_write_error(this);
//    }
    return -1;
  } else {
    CHECK(Status() == Connection::Status::KEEP_WRITE);
    // 2. Only part of data has been sent.
    // 2.1. copy it to out buffer
    if (sz - written > out_buff.FreeSpace()) {
      VLOG(1) << "Data is too big to be written into buf for:" << GetPeer();
      // fix this 'error'.
      return written;
    }
    out_buff.Append(data + written, sz - written);
    // 2.2. add a write event and go on writing when it is triggered.
    if (GetGlobalEpoll().AddWriteEvent(GetSock(), this) < 0) {
      VLOG(1) << "Fail to add write event for " << GetPeer();
      return written;
    }
    return written;
  }
}

void Connection::ProcessEpollOut(int sock_fd, void* client_data) {
  if (!client_data) {
    VLOG(3) << "error or hung fd:" << sock_fd;
    return;
  }
  auto* conn = static_cast<Connection*>(client_data);
  CHECK_EQ(conn->GetSock(), sock_fd);
  char* buff;
  int len;
  while (!conn->GetOutBuff().Empty()) {
    int buff_pos = conn->GetOutBuff().ConsumeRange(&buff, &len);
    int written = conn->WriteImmediately(buff, len);
    if (conn->Status() == Status::ERROR) {
//       if (conn->cb_interface_.on_write_error) {
//         conn->cb_interface_.on_write_error(conn);
//       }
      break;
    } else if (conn->Status() == Status::KEEP_WRITE) {
      conn->GetOutBuff().Confirm(buff_pos + written);
    } else {
      CHECK(conn->Status() == Status::IDLE);
    }
    // max_tries?
  }
  conn->epl_.DelEvent(conn->GetSock(), IOMaskWrite);
}

enum Connection::Status Connection::ConsumeDataStream() {
  if (in_buff_.Full()) {
    // Do not read to prevent peer from sending data.
    status_ = Status::WAITING_BUFFER_SPACE;
    return status_;
  }
  int free_space = in_buff_.FreeSpace();
  int in_bytes = 0;
  while (true) {
    in_bytes = read(sock_, buff_, free_space);
    if (in_bytes == 0) {
      status_ = Status::CLOSING;
      break;
    } else if (in_bytes < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // no more data.
        status_ = Status::READ_OK;
        break;
      } else {
        status_ = Status::CLOSING;
        break;
      }
    } else {
      status_ = Status::READ_OK;
      in_buff_.Append(buff_, in_bytes);
      break;
    }
  }
  return status_;
}

int Connection::WriteImmediately(const char* data, int sz) {
  // status_ = Status::WRITING;
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
