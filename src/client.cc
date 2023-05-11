#include "client.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#include "common/fd_guard.h"

DEFINE_bool(connect_non_blocking, false, "");

int SetNonBlocking(int fd);
int SetCloseOnExec(int fd);

int Client::ConnectIfNot(const std::string& ip, int port) {
  memset(&srv_addr_, 0, sizeof(srv_addr_));
  if (InitSock(ip, port) != 0) {
    return -1;
  }
  while (true) {
    if (connect(sock_fd_(), (struct sockaddr*)&srv_addr_,
                sizeof(srv_addr_)) == 0) {
      break;
    } else {
      if (errno == EINTR) {
        // do not continue if EINTR.
        // https://blog.csdn.net/hnlyyk/article/details/51444617
        return -1;
      } else if (errno == EINPROGRESS && FLAGS_connect_non_blocking) {
        if (GetGlobalEpoll()->AddWriteEvent(sock_fd_(), this) < 0) {
          return -1;
        }
        // what if connect timeout?
        // TODO...
        OnConnected();
        return 0;
      } else {
        LOG(ERROR) << strerror(errno);
        return -1;
      }
    }
  }
  OnConnected();
  return 0;
}

void Client::OnConnected() {
  sock_options_.reset(new SocketOptions);
  sock_options_->sock_fd = sock_fd_();
  sock_options_->arg = this;
  // sock_options_->conn.reset(new Socket(sock_fd_(), srv_addr_));
  sock_options_->on_level_triggered_event = &Client::OnNewMsg;
  CHECK_EQ(GetGlobalEpoll()->AddReadEvent(sock_fd_(), sock_options_.get()), 0);
}

void Client::RemoveConnection(Socket* conn) {
  CHECK_EQ(GetGlobalEpoll()->DelEvent(sock_fd_(), IOMaskRW), 0);
  sock_options_.reset(nullptr);
}

void Client::Reconnect(Socket* close_conn) {
}

void Client::OnInputOk(Socket* close_conn) {
}

void Client::CheckConnected(int sock_fd, void* client_data) {
  auto* client = static_cast<Client*>(client_data);
  CHECK_EQ(sock_fd, client->sock_fd_());
  int err;
  socklen_t len = sizeof(err);
  auto fail = [&]() {
    // close(sock_fd);
    GetGlobalEpoll()->DelEvent(sock_fd, IOMaskRead);
  };
  if (getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
    VLOG(1) << "Fail to getsockopt:" << strerror(errno);
    fail();
    return;
  }
  if (err) {
    VLOG(1) << "Fail to connect..." << strerror(errno);
    fail();
    return;
  }
  client->OnConnected();
}

int Client::InitSock(const std::string& ip, int port) {
  srv_addr_.sin_family = AF_INET;
  srv_addr_.sin_port = htons(port);
  if (inet_aton(ip.c_str(), &srv_addr_.sin_addr) != 1) {
    VLOG(1) << "Bad ip?";
    return -1;
  }
  sock_fd_.Reset(socket(AF_INET, SOCK_STREAM, 0));
  if (sock_fd_ < 0) {
    VLOG(1) << "Fail to create socket";
    return -1;
  }
  if (FLAGS_connect_non_blocking && SetNonBlocking(sock_fd_()) != 0) {
    VLOG(1) << "Fail to set nonblocking";
    return -1;
  }
  if (SetCloseOnExec(sock_fd_()) == -1) {
    VLOG(1) << "Fail to set cloexec.";
    return -1;
  }
  return 0;
}
