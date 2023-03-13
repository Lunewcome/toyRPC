#include "toyrpc_server.h"

#include "glog/logging.h"
#include "net.h"

void DebugMsg(const IOBuffer& buff);
void toyRPCServer::Start() {
  auto port = options_.port;
  server_fd_.Reset(CreateTcpServer(port));
  CHECK(!(server_fd_ < 0)) << "Port " << port << " already used?";
  // simply let it core if oom.
  std::unique_ptr<SocketOptions> options(new SocketOptions);
  options->sock_fd = server_fd_(),
  options->on_level_triggered_event = std::bind(&toyRPCServer::Accept,
      this, std::placeholders::_1);
  options->conn = nullptr;
  CHECK(GetGlobalEpoll().AddReadEvent(server_fd_(), options.get()) == 0);
  // it must be uniqe.
  AddToSocks(options);
}

Connection* toyRPCServer::GetConnection(int fd) {
  const auto& itrt_sock = socks_.find(fd);
  if (itrt_sock == socks_.end()) {
    return nullptr;
  } else {
    return itrt_sock->second->conn.get();
  }
}

void toyRPCServer::RemoveConnection(int sock_fd) {
  const auto& conn_itrt = socks_.find(sock_fd);
  if (conn_itrt == socks_.end()) {
    VLOG(1) << "trying to remove non-existing connection.";
    return;
  }
  CHECK_EQ(GetGlobalEpoll().DelEvent(sock_fd, IOMaskRW), 0);
  // How if some data has already been read from this sock?
  // Seems that sock should be closed after all read/write
  // events have been handled.
  socks_.erase(conn_itrt);
}

void toyRPCServer::OnNewMsgReceived(int sock_fd) {
  auto* conn = GetConnection(sock_fd);
  if (!conn) {
    return;
  }
  auto status = conn->ConsumeDataStream();
  if (status == Connection::Status::CLOSING) {
    VLOG(3) << "peer(" << conn->GetPeer() << ") closed connection.";
    RemoveConnection(sock_fd);
  } else if (status == Connection::Status::WAITING_BUFFER_SPACE) {
    VLOG(3) << "waiting space to receive data from " << conn->GetPeer()
        << ". This could prevent the client from sending data.";
  } else {
    CHECK(status == Connection::Status::READ_OK)
        << "what's up:" << conn->GetPeer() << ","
        << Connection::StatusToString(status);
    auto& in_buff = conn->GetInBuff();
    DebugMsg(in_buff);
    // debug output.
    const char* msg = "HTTP/1.1 200 OK\r\n" \
                      "Server:toyRPC\r\n" \
                      "Content-Type:txt\r\n" \
                      "Content-Length:19\r\n\r\nthis is response.\r\n";
    VLOG(3) << "debug:\r\n" << msg;
    // 1. what if the conn has been closed in 'RemoveConnection'?
    // 2. and what if 'RemoveConnection' and 'OnNewMsgReceived'
    //    run concurrently?
    conn->Send(msg, strlen(msg), nullptr);
  }
}

void toyRPCServer::Accept(int sock_fd) {
  CHECK_EQ(server_fd_(), sock_fd);
  struct sockaddr_storage in_addr;
  socklen_t in_len = sizeof(in_addr);
  while (true) {
    memset(&in_addr, 0, in_len);
    FDGuard fd_guard(accept4(sock_fd, (sockaddr*)&in_addr,
                             &in_len, SOCK_NONBLOCK));
    if (fd_guard < 0) {
      if (errno == EINTR) {
        continue;
      } else {
        // break if EAGAIN || EWOULDBLOCK.
        break;
      }
    }
    if (SetCloseOnExec(fd_guard()) == -1) {
      continue;
    }
    // simply let it core if oom.
    std::unique_ptr<Connection> conn(new Connection(
        GetGlobalEpoll(), fd_guard(), *(sockaddr_in*)(&in_addr)));
    std::unique_ptr<SocketOptions> options(new SocketOptions);
    options->sock_fd = fd_guard();
    options->on_level_triggered_event = std::bind(
        &toyRPCServer::OnNewMsgReceived, this, std::placeholders::_1);
    options->conn.swap(conn);
    CHECK_EQ(GetGlobalEpoll().AddReadEvent(fd_guard(), options.get()), 0);
    // conn now owns fd.
    fd_guard.Release();
    AddToSocks(options);
  }
}

void DebugMsg(const IOBuffer& buff) {
  auto debug = [&]() {
    if (buff.Empty()) {
      return;
    }
    char* buff1;
    char* buff2;
    int len1, len2;
    buff.View(&buff1, &len1, &buff2, &len2);
    VLOG(4) << std::string(buff1, len1) << std::string(buff2, len2);
  };
  debug();
}
