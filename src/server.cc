#include "server.h"

#include <memory>

#include "glog/logging.h"
#include "net.h"

void Server::Start() {
  auto port = options_.port;
  server_fd_.Reset(CreateTcpServer(port));
  CHECK(!(server_fd_ < 0)) << "Port " << port << " already used?";
  // simply let it core if oom.
  std::unique_ptr<SocketOptions> options(new SocketOptions);
  options->sock_fd = server_fd_(),
  options->arg= this;
  options->on_level_triggered_event = &Server::Accept;
  options->conn = nullptr;
  CHECK(GetGlobalEpoll()->AddReadEvent(server_fd_(), options.get()) == 0);
  // it must be uniqe.
  GetGlobalConnectionManager()->Insert(options.release());
}

void Server::OnNewMsgReceived(void* _this, int sock_fd) {
  auto* srv = static_cast<Server*>(_this);
  auto* conn = GetGlobalConnectionManager()->GetConnection(sock_fd);
  CHECK(conn) << "Fatal: server losts a connection...";
  int save_errno;
  int rc = conn->ReadUntilFail(&save_errno);
  if (rc == 0) {
    VLOG(3) << "peer(" << conn->GetPeer() << ") closed connection.";
    GetGlobalConnectionManager()->Remove(sock_fd);
  } else {
    CHECK(rc < 0 && save_errno != EINTR);
    while (true) {
      // Do this reset here?
      srv->http_request_.Reset(&conn->GetInBuff());
      ParseResult pr = srv->protocol_http_.Parse(conn->GetInBuff(),
                                                 &srv->http_request_);
      if (pr.GetStatus() == ParseStatus::OK) {
        // process http_request...
        VLOG(4) << srv->http_request_;
        srv->TellClient(conn, "I've received.\r\n", 200);
      } else if (pr.GetStatus() == ParseStatus::UNIMPLEMENTED) {
        srv->TellClient(conn, "method not implemented.\r\n", 404);
        continue;
      } else if (pr.GetStatus() == ParseStatus::ERROR) {
        // clear && close?
        // skip this request.
        srv->TellClient(conn, "Fail to parse request.\r\n", 404);
        continue;
      } else if (pr.GetStatus() == ParseStatus::NEED_MORE_DATA) {
        break;
      } else {
        CHECK(false) << "What happend?";
      }
    }
  }
}

void Server::Accept(void* _this, int sock_fd) {
  auto* svr = static_cast<Server*>(_this);
  CHECK_EQ(svr->server_fd_(), sock_fd);
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
        fd_guard(), *(sockaddr_in*)(&in_addr)));
    std::unique_ptr<SocketOptions> options(new SocketOptions);
    options->sock_fd = fd_guard();
    options->arg = _this;
    options->on_level_triggered_event = &Server::OnNewMsgReceived;
    options->conn.swap(conn);
    if (GetGlobalEpoll()->AddReadEvent(fd_guard(), options.get()) != 0) {
      VLOG(1) << "Fail to add event to epoll." << strerror(errno);
      continue;
    }
    // conn now owns fd.
    fd_guard.Release();
    GetGlobalConnectionManager()->Insert(options.release());
  }
}

void Server::TellClient(Connection* conn, const char* msg, int code) {
  HttpResponse resp;
  resp.version = "HTTP/1.1";
  resp.status_code = code;
  resp.status_text = "Not Found";
  resp.content.append(msg, strlen(msg));

  std::string sz;
  StringPrintf(&sz, "%d", resp.content.size());
  resp.headers["Content-Length"] = sz;
  resp.headers["Content-Type"] = "text/plain";

  std::string buf;
  protocol_http_.PackRequest(resp, &buf);
  conn->SyncSend(buf.c_str(), buf.size());
}
