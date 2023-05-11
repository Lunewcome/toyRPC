#include "server.h"

#include <memory>

#include "glog/logging.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/service.h"
#include "net.h"

bool Server::AddService(google::protobuf::Service* service) {
  if (NULL == service) {
    LOG(ERROR) << "Parameter[service] is NULL!";
    return false;
  }
  const google::protobuf::ServiceDescriptor* sd = service->GetDescriptor();
  if (sd->method_count() == 0) {
    LOG(ERROR) << "service=" << sd->full_name() << " have no method.";
    return false;
  }
  if (full_name_service_map_.find(sd->full_name()) !=
      full_name_service_map_.end()) {
    LOG(ERROR) << "There is an existing service with full name:"
        << sd->full_name();
    return false;
  }
  for (int i = 0; i < sd->method_count(); ++i) {
    const google::protobuf::MethodDescriptor* md = sd->method(i);
    MethodProperty& mp = full_name_service_map_[sd->full_name()];
    mp.service = service;
    mp.method = md;
  }
  return true;
}

const Server::MethodProperty*
Server::GetMethodProperty(const std::string& full_name) const {
  auto itrt = full_name_service_map_.find(full_name);
  if  (itrt == full_name_service_map_.end()) {
    return nullptr;
  }
  return &itrt->second;
}

void Server::Start() {
  auto port = options_.port;
  FDGuard server_fd(CreateTcpServer(port));
  CHECK(!(server_fd < 0)) << "Port " << port << " already used?";
  SocketOptions options;
  options.sock_fd = server_fd();
  options.peer = Peer();
  options.arg = this;
  options.on_level_triggered_event = &Server::Accept;
  std::unique_ptr<Socket> s(new Socket(options));
  CHECK(GetGlobalEpoll()->AddReadEvent(server_fd.Release(), s.get()) == 0);
  GetGlobalSocketPool()->Insert(s.release());
}

void Server::OnNewMsgReceived(void* _this, int sock_fd) {
  auto* srv = static_cast<Server*>(_this);
  Socket* s = GetGlobalSocketPool()->Get(sock_fd);
  CHECK(s) << "Fatal: server losts a connection...";
  int saved_errno;
  int rc = s->ReadUntilFail(&saved_errno);
  if (rc == 0) {
    VLOG(5) << "peer(" << s->GetPeer() << ") closed connection.";
    GetGlobalSocketPool()->Remove(sock_fd);
  } else {
    CHECK(rc < 0 && saved_errno != EINTR);
    srv->cntl_.current_sock = s;
    auto& http_request = srv->cntl_.http_request;
    while (true) {
      http_request.Reset(&s->GetInBuff());
      ParseResult pr = srv->protocol_http_.Parse(s->GetInBuff(), &http_request);
      if (pr.GetStatus() == ParseStatus::OK) {
        VLOG(4) << http_request;
        srv->CallServiceMethod();
      } else if (pr.GetStatus() == ParseStatus::UNIMPLEMENTED) {
        srv->ReplyErr(s, "method not implemented.", 404);
        continue;
      } else if (pr.GetStatus() == ParseStatus::ERROR) {
        // clear && close?
        // skip this request.
        srv->ReplyErr(s, "Fail to parse request.", 404);
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
  // auto* svr = static_cast<Server*>(_this);
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
    if (SetCloseOnExec(fd_guard()) == -1 || SetNonBlocking(fd_guard()) == -1) {
      continue;
    }
    std::unique_ptr<SocketOptions> options(new SocketOptions);
    options->sock_fd = fd_guard();
    options->peer = Peer(*(sockaddr_in*)(&in_addr));
    options->arg = _this;
    options->on_level_triggered_event = &Server::OnNewMsgReceived;
    std::unique_ptr<Socket> s(new Socket(*options.get()));
    if (GetGlobalEpoll()->AddReadEvent(fd_guard(), s.get()) != 0) {
      VLOG(1) << "Fail to add event to epoll." << strerror(errno);
      continue;
    }
    // s now owns fd.
    fd_guard.Release();
    GetGlobalSocketPool()->Insert(s.release());
  }
}

void Server::ReplyErr(Socket* s, const char* err_msg, int code) {
  VLOG(2) << err_msg;
  protocol_http_.BuildResponse(err_msg, code, &cntl_.http_response);
  protocol_http_.PackResponse(cntl_, &cntl_.current_sock->GetOutBuff());
  cntl_.current_sock->StartWrite(0, nullptr);
}

void Server::CallServiceMethod() {
  static const std::string& kServiceFullName = "ServiceFullName";
  const auto& req = cntl_.http_request;
  const auto& itrt_name = req.headers.find(kServiceFullName);
  if (itrt_name == req.headers.end()) {
    ReplyErr(cntl_.current_sock, "No header named \'ServiceFullName\'", 404);
    return;
  }
  const auto* mp = GetMethodProperty(itrt_name->second);
  if (!mp) {
    const std::string& Err = "No service named " + itrt_name->second;
    ReplyErr(cntl_.current_sock, Err.c_str(), 404);
    return;
  }
  google::protobuf::Closure* done = NewCallback(this, &Server::OnDone, &cntl_);
  mp->service->CallMethod(mp->method, &cntl_, nullptr, nullptr, done);
}

void Server::OnDone(toyRPCController* cntl) {
  protocol_http_.PackResponse(*cntl, &cntl->current_sock->GetOutBuff());
  google::protobuf::Closure* done = nullptr;
  cntl->current_sock->StartWrite(0, done);
}
