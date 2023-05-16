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
  options.fd = server_fd();
  options.peer = Peer();
  options.arg = this;
  options.on_edge_triggered_event = &Server::Accept;
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
    uint64_t call_id;
    while (true) {
      HttpRequest http_request(&s->ReadBuff());
      ParseResult pr = srv->protocol_http_.Parse(s->ReadBuff(), &http_request);
      if (pr.GetStatus() == ParseStatus::OK) {
        VLOG(5) << http_request;
        if (!http_request.GetCallId(&call_id)) {
          VLOG(1) << "Fail to get CallId in request.";
          continue;
        }
        auto* cntl_pool = GetGlobaltoyRPCControllerPool();
        CHECK(!cntl_pool->Exist(call_id));
        auto* cntl = cntl_pool->Allocate(call_id);
        cntl->http_request.Swap(&http_request);
        cntl->current_sock = s;
        cntl->call_id = call_id;
        srv->CallServiceMethod(cntl);
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
  std::string ip;
  int port;
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
    if (!GetIpPortFromSockAddr(*(sockaddr_in*)(&in_addr), &ip, &port)) {
      VLOG(2) << "Fail to get ip/port.";
      continue;
    }
    std::unique_ptr<SocketOptions> options(new SocketOptions);
    options->fd = fd_guard();
    options->peer = Peer(ip, port);
    options->arg = _this;
    options->on_edge_triggered_event = &Server::OnNewMsgReceived;
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
  HttpResponse resp;
  std::string buf;
  protocol_http_.BuildResponse(err_msg, code, &resp);
  protocol_http_.PackResponse(resp, &buf);
  s->WriteNoBuff(buf.c_str(), buf.size());
}

void Server::CallServiceMethod(toyRPCController* cntl) {
  static const std::string& kServiceFullName = "ServiceFullName";
  const auto& req = cntl->http_request;
  const auto& itrt_name = req.headers.find(kServiceFullName);
  if (itrt_name == req.headers.end()) {
    ReplyErr(cntl->current_sock, "No header named \'ServiceFullName\'", 404);
    return;
  }
  const auto* mp = GetMethodProperty(itrt_name->second);
  if (!mp) {
    const std::string& Err = "No service named " + itrt_name->second;
    ReplyErr(cntl->current_sock, Err.c_str(), 404);
    return;
  }
  mp->service->CallMethod(mp->method, cntl, nullptr, nullptr,
                          NewCallback(this, &Server::IssueRPC, cntl));
}

void Server::IssueRPC(toyRPCController* cntl) {
  protocol_http_.PackResponse(*cntl, &cntl->current_sock->WriteBuff());
  // Do nothing when writing is done.
  WriteConfig config = {
    .call_id = 0,
    .on_replied = nullptr
  };
  if (cntl->current_sock->StartWrite(config) != 0) {
    // delete config.done;
  }
}
