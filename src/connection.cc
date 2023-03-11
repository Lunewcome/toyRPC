#include "connection.h"

#include "glog/logging.h"

enum Connection::Status Connection::Receive() {
  status_ = Status::READING;
  if (in_buff_.Full()) {
    // Do not read to prevent peer from sending data.
    status_ = Status::WAITING_BUFFER_SPACE;
    return status_;
  }
  int free_space = in_buff_.FreeSpace();
  int in_bytes = 0;
  while (true) {
    in_bytes = read(sock_, buff_, free_space);
    if (in_bytes <= 0) {
      if (errno == EINTR) {
        continue;
      } else {
        status_ = Status::CLOSING;
        return status_;
      }
    }
    in_buff_.Append(buff_, in_bytes);
    break;
  }
  return status_;
}

void Connection::KeepWrite(int fd, void* client_data) {
  char* buff;
  int len;
  while (!out_buff_.Empty()) {
    int buff_pos = out_buff_.ConsumeRange(&buff, &len);
    int written = WriteImmediately(buff, len);
    if (Status() == Status::ERROR) {
      status_ = Status::CLOSING;
      return;
    } else if (Status() == Status::KEEP_WRITE) {
      out_buff_.Confirm(buff_pos + written);
      return;
    } else {
      continue;
    }
  }
  status_ = Status::IDLE;
  epl_.DelEvent(sock_, IOMaskWrite);
}

int Connection::WriteImmediately(const char* data, int sz) {
  status_ = Status::WRITING;
  int len = write(sock_, data, sz);
  if (len < 0 && errno != EINTR) {
    status_ = Status::ERROR;
  } else {
    status_ = (len == sz) ? Status::IDLE : Status::KEEP_WRITE;
  }
  return len;
}

void Connection::SyncSend(const char* data, int sz) {
  int written = 0;
  while (written < sz) {
    int len = WriteImmediately(data, sz);
    if (len > 0) {
      written += len;
    }
    if (len <= 0) {
      break;
    }
  }
}

const std::string& Connection::StatusToString(enum Status st) {
  static std::string status_strs[] = {
    "IDLE",
    "CONNECTING",
    "CONNECTED",
    "CLOSING",
    "CLOSED",
    "READING",
    "WAITING_BUFFER_SPACE",
    "WRITING",
    "ERROR"
  };
  return status_strs[static_cast<int>(st)];
}
