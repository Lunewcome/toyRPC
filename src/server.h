#ifndef SRC_SERVER_H
#define SRC_SERVER_H

#include "common/fd_guard.h"
#include "controller.h"
#include "epoll.h"
#include "socket.h"

struct ServerOptions {
  uint32_t port;
  std::string protocol;
};

using google::protobuf::Service;
class Server {
 public:
  Server(const ServerOptions& options) : options_(options) {}
  bool AddService(google::protobuf::Service* service);
  void Start();
  void Stop() { GetGlobalEpoll()->Stop(); }
  void RunUtilAskedToStop() { GetGlobalEpoll()->RunUtilAskedToStop(); }

 private:
  struct MethodProperty {
    google::protobuf::Service* service;
    const google::protobuf::MethodDescriptor* method;
  };

  static void Accept(void* _this, int sock_fd);
  static void OnNewMsgReceived(void* _this, int sock_fd);
  const std::string& GetProtocolName() const { return options_.protocol; }
  void ReplyErr(Socket* conn, const char* err, int code);
  void CallServiceMethod(toyRPCController* cntl);
  const MethodProperty* GetMethodProperty(const std::string& full_name) const;
  void IssueRPC(toyRPCController* cntl);

  ServerOptions options_;
  Http protocol_http_;
  std::unordered_map<std::string, MethodProperty> full_name_service_map_;

  BAN_COPY_AND_ASSIGN(Server);
};

#endif  // SRC_SERVER_H
