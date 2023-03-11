#include "server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>

int CreateTcpServer(uint32_t port);
bool Server::Start() {
  auto port = options_.port;
  server_fd_.Reset(CreateTcpServer(port));
  if (server_fd_() < 0) {
    VLOG(0) << "Fail to create server on port " << port;
    return false;
  }
  auto acceptor = std::bind(&Server::Accept, this, std::placeholders::_1,
                            std::placeholders::_2);
  Event event(server_fd_(), IOMaskRead, acceptor, nullptr, nullptr);
  if (el_.AddEvent(event) < 0) {
    VLOG(0) << "Fail to start server.";
    return false;
  }
  return true;
}

void Server::Accept(int fd, void* data) {
  struct sockaddr_storage in_addr;
  socklen_t in_len = sizeof(in_addr);
  FDGuard in_fd(accept(fd, (sockaddr*)&in_addr, &in_len));
  if (in_fd() < 0) {
    VLOG(0) << "Fail to accept a new connection.";
    // WTD?
    return;
  }
  std::unique_ptr<Connection> conn(
      new(std::nothrow)Connection(el_, in_fd(), in_addr));
  if (!conn.get()) {
    // FDGuard closes fd
    SayGoodbye(in_fd(), "internal error:bad_alloc.");
    return;
  }
  auto rcv_cb = std::bind(&Server::Receive, this, std::placeholders::_1,
                          std::placeholders::_2);
  Event rcv_event(conn->Sock(), IOMaskRead, rcv_cb, nullptr, conn.get());
  if (el_.AddEvent(rcv_event) < 0){
    // FDGuard closes fd
    SayGoodbye(in_fd(), "Fail to process request(AddEvent).");
    return;
  }
  // conn owns fd now.
  in_fd.Release();
  CHECK(connections_.count(conn->Sock()) == 0)
      << "fd occupied:" << conn->Sock();
  connections_[conn->Sock()].swap(conn);
}

void Server::Receive(int fd, void* msg) {
  Connection* conn = static_cast<Connection*>(msg);
  CHECK(fd == conn->Sock()) << "bad connection!";
  CHECK(connections_.count(conn->Sock())) << "connection lost.";
  auto status = conn->Receive();
  if (status == Connection::Status::CLOSING) {
    VLOG(3) << "client(" << conn->GetPeer() << ") closed connection.";
    RemoveConnection(conn);
    return;
  } else if (status == Connection::Status::WAITING_BUFFER_SPACE) {
    VLOG(3) << "waiting space to receive data from " << conn->GetPeer();
    return;
  }
  CHECK(status == Connection::Status::READING)
      << "what's up:" << conn->GetPeer() << ","
      << Connection::StatusToString(status);
  auto& in_buff = conn->GetInBuff();
  [&]() {
    if (in_buff.Empty()) {
      return;
    }
    char* buff1;
    char* buff2;
    int len1, len2;
    in_buff.View(&buff1, &len1, &buff2, &len2);
    VLOG(4) << std::string(buff1, len1) << std::string(buff2, len2);
  }();
  const auto& pr = protocol_->Parse(in_buff);
  if (pr.OK()) {
    auto debug = [&]() {
      const char* msg = "HTTP/1.1 200 OK\r\n" \
                        "Server:toyRPC\r\n" \
                        "Content-Type:txt\r\n" \
                        "Content-Length:19\r\n\r\nthis is response.\r\n";
      Send(fd, conn, msg, strlen(msg));
    };
    debug();
  }
}

void Server::RemoveConnection(Connection* conn) {
  el_.DelEvent(conn->Sock(), IOMaskRW);
  // How if some data has already been read from this sock?
  // Seems that sock should be closed after all read/write
  // events have been handled.
  close(conn->Sock());

  CHECK(connections_.count(conn->Sock())) << "connection lost.";
  connections_.erase(conn->Sock());
}

void Server::Send(int fd, void* client_data, const char* data, int sz) {
  Connection* conn = static_cast<Connection*>(client_data);
  CHECK(fd == conn->Sock()) << "bad connection!";

  int len = conn->WriteImmediately(data, sz);
  VLOG(3) << "status:" << Connection::StatusToString(conn->Status());

  auto& out_buff = conn->GetOutBuff();
  auto writer = std::bind(&Connection::KeepWrite, conn,
                          std::placeholders::_1,
                          std::placeholders::_2);
  Event wrt_event(conn->Sock(), IOMaskWrite, nullptr, writer, nullptr);
  switch (conn->Status()) {
    case Connection::Status::IDLE:
      // 1. It's lucky to send all data by one 'write' call.
      return;
    case Connection::Status::ERROR:
      break;
    case Connection::Status::KEEP_WRITE:
      // 2. Only part of data has been sent.
      // 2.1. copy it to out buffer
      if (sz - len > out_buff.FreeSpace()) {
        LOG(ERROR) << "Data is too big to be written into buf for:"
            << conn->GetPeer();
        break;
      }
      out_buff.Append(data + len, sz - len);
      // 2.2. add a write event and go on writing when it is triggered.
      if (el_.AddEvent(wrt_event) < 0) {
        VLOG(0) << "Fail to add write event for " << conn->GetPeer();
        break;
      }
      return;
    default:
      LOG(ERROR) << "St unexpected has hannped?! peer:" << conn->GetPeer();
      break;
  }
  RemoveConnection(conn);
}

void Server::Stop() {
  el_.Stop();
}

// int AddService(Service* s, const string& uri_method_mappings) {
//   return 0;
// }

int SetNonBlocking(int fd);
int SetReuse(int fd);
int SetNoDelay(int fd);
int Bind(int fd, int port);
int Listen(int fd);
int CreateTcpServer(uint32_t port) {
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    return -1;
  }
  if (SetNonBlocking(sock) ||
      SetReuse(sock) ||
      SetNoDelay(sock) ||
      Bind(sock, port) ||
      Listen(sock)) {
    return -1;
  }
  return sock;
}

int SetNonBlocking(int fd) {
  int flg = fcntl(fd, F_GETFL);
  if (fcntl(fd, F_SETFL, flg|O_NONBLOCK) < 0) {
    return -1;
  }
  return 0;
}

int SetReuse(int fd) {
  int reuse = 1;
  int opt = setsockopt(fd,
                       SOL_SOCKET,
                       SO_REUSEADDR,
                       &reuse,
                       sizeof(reuse));
  if (opt < 0) {
  }
  return opt;
}

int SetNoDelay(int fd) {
  int yes = 1;
  if (setsockopt(fd,
                 IPPROTO_TCP,
                 TCP_NODELAY,
                 &yes,
                 sizeof(yes)) == -1) {
    return -1;
  }
  return 0;
}

int Bind(int fd, int port) {
  struct sockaddr_in sock_addr;
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sock_addr.sin_port = htons(port);
  int ret = bind(fd,
                 (struct sockaddr*)&sock_addr,
                 sizeof(sock_addr));
  if (ret < 0) {
    return -1;
  }
  return 0;
}

int Listen(int fd) {
  int ret = listen(fd, 5);
  if (ret < 0) {
    return -1;
  }
  return 0;
}
