#include "socket.h"

#include "glog/logging.h"
#include "net.h"

void Socket::ProcessEpollInput(int sock_fd, void* client_data) {
  if (!client_data) {
    VLOG(3) << "error or hung fd:" << sock_fd;
    return;
  }
  auto* s = static_cast<Socket*>(client_data);
  s->last_active_timestamp_ = time(nullptr);
  CHECK_EQ(sock_fd, s->GetFD());
  s->options_.on_level_triggered_event(s->options_.arg, sock_fd);
}

int Socket::StartWrite(uint64_t call_id, google::protobuf::Closure* done) {
  // send out_buff_.
  last_active_timestamp_ = time(nullptr);
  if (Status() == Status::ERROR) {
    return -1;
  }
  if (ConnectIfNot() < 0) {
    status_ = Status::FAIL_CONNECT;
    return -1;
  }
  if (Status() == Status::Connecting) {
    return 0;
  } else {
    CHECK(Status() == Status::Connected);
    int written = out_buff_.WriteToSock(fd_);
    if (written < 0) {
      status_ = Socket::Status::ERROR;
      VLOG(2) << "Write err:" << StatusToString(Status());
      return -1;
    } else {
      out_buff_.PopFront(written);
      if (out_buff_.Empty()) {
        // Write done.
        status_ = Socket::Status::IDLE;
        done->Run();
      } else {
        status_ = Socket::Status::KEEP_WRITE;
        if (GetGlobalEpoll()->AddWriteEvent(GetFD(), this) < 0) {
          VLOG(1) << "Fail to add write event for " << GetPeer();
        }
        if (done) {
          CHECK(req_done_map_.find(call_id) == req_done_map_.end());
          req_done_map_[call_id].reset(done);
        }
      }
      return written;
    }
  }
}

bool Socket::CheckConnected(int fd) {
  int err;
  socklen_t len = sizeof(err);
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err) {
    VLOG(1) << "Fail to connect..." << strerror(errno);
    GetGlobalEpoll()->DelEvent(fd, IOMaskRW);
    return false;
  }
  return true;
}

void Socket::ProcessEpollOut(int sock_fd, void* client_data) {
  if (!client_data) {
    VLOG(3) << "error or hung fd:" << sock_fd;
    return;
  }
  auto* s = static_cast<Socket*>(client_data);
  s->last_active_timestamp_ = time(nullptr);
  CHECK_EQ(s->GetFD(), sock_fd);
  if (s->Status() == Status::ERROR) {
    return;
  }
  if (s->Status() == Status::Connecting && !s->CheckConnected(s->GetFD())) {
    return;
  }
  while (true) {
    int rc = s->GetOutBuff().WriteToSock(sock_fd);
    if (rc < 0) {
      if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
        VLOG(1) << "Fail to write. " << strerror(errno);
        s->status_ = Status::ERROR;
        break;
      }
      // simply return and retry later.
      return;
    } else if (rc == 0) {
      // done.
      break;
    } else {
      // continue;
      s->GetOutBuff().PopFront(rc);
    }
  }
  GetGlobalEpoll()->DelEvent(s->GetFD(), IOMaskWrite);
}

int Socket::ReadUntilFail(int* saved_errno) {
  int rc;
  while (true) {
    rc = in_buff_.ReadFromSock(fd_);
    if (rc == 0 || (rc < 0 && errno != EINTR)) {
      *saved_errno = errno;
      break;
    }
  }
  return rc;
}

int Socket::WriteNoBuff(const char* data, int sz) {
  int len = write(fd_, data, sz);
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

int Socket::ConnectIfNot() {
  if (fd_ > 0) {
    status_ = Status::Connected;
    return 0;
  }
  struct sockaddr_in peer_addr;
  memset(&peer_addr, 0, sizeof(peer_addr));
  if (InitSock(options_.peer, &peer_addr) != 0) {
    return -1;
  }
  while (true) {
    if (connect(fd_, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) == 0) {
      status_ = Status::Connected;
      return 0;
    } else {
      if (errno == EINTR) {
        // do not continue if EINTR.
        // https://blog.csdn.net/hnlyyk/article/details/51444617
        return -1;
      } else if (errno == EINPROGRESS) {
        if (GetGlobalEpoll()->AddWriteEvent(fd_, this) < 0) {
          return -1;
        }
        status_ = Status::Connecting;
        // TODO:what if connect timeout?
        return 0;
      } else {
        LOG(ERROR) << strerror(errno);
        return -1;
      }
    }
  }
}

int Socket::InitSock(const Peer& peer, struct sockaddr_in* peer_addr) {
  peer_addr->sin_family = AF_INET;
  peer_addr->sin_port = htons(peer.port());
  if (inet_aton(peer.ip().c_str(), &peer_addr->sin_addr) != 1) {
    VLOG(1) << "Bad ip?";
    return -1;
  }
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ < 0) {
    VLOG(1) << "Fail to create socket";
    return -1;
  }
  if (SetNonBlocking(fd_) != 0) {
    VLOG(1) << "Fail to set nonblocking";
    return -1;
  }
  if (SetCloseOnExec(fd_) == -1) {
    VLOG(1) << "Fail to set cloexec.";
    return -1;
  }
  return 0;
}

#define StatusCase(st) \
  case Status::st : return #st
const std::string Socket::StatusToString(enum Status st) {
  switch (st) {
    StatusCase(IDLE);
    StatusCase(KEEP_WRITE);
    StatusCase(ERROR);
    StatusCase(FAIL_CONNECT);
    StatusCase(Connecting);
    StatusCase(Connected);
    default:
      return "UNKNOWN";
  }
}

SocketPool* GetGlobalSocketPool() {
  static SocketPool pool;
  return &pool;
}
