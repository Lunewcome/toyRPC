#ifndef SRC_CLIENT_H
#define SRC_CLIENT_H

#include <functional>
#include <memory>
#include <string>

#include "common/basics.h"
#include "common/fd_guard.h"
#include "connection.h"
#include "controller.h"

struct ClientOptions {
  std::string ip;
  int port;
  std::string protocol;
  int timeout_ms;
  // connection_type;
};

class Client {
 public:
  typedef std::function<void(void)> OnSendDoneCallback;

  Client(ClientOptions opt) : options_(opt) {};

  void DebugSend(const char* msg, int len, OnSendDoneCallback done) {
    if (ConnectIfNot(options_.ip, options_.port) != 0) {
      return;
    }
    VLOG(4) << sock_options_->conn->Send(msg, len);
  }

  void AwaitEpoll() {
    if (Connected()) {
      GetGlobalEpoll()->AwaitEpoll();
    }
  }

  static void OnNewMsg(void* _this, int sock_fd) {
    toyRPCController cntl;

    auto* client = static_cast<Client*>(_this);
    VLOG(3) << "client new msg...";
    auto* conn = client->sock_options_->conn.get();
    int save_errno;
    int rc = conn->ReadUntilFail(&save_errno);
    if (rc == 0) {
      VLOG(4) << "peer(" << conn->GetPeer() << ") closed connection.";
      client->RemoveConnection(conn);
      return;
    } else {
      CHECK(rc < 0 && save_errno != EINTR);
      auto& in_buff = conn->GetInBuff();
      // char* buff1;
      // char* buff2;
      // int len1, len2;
      // in_buff.View(&buff1, &len1, &buff2, &len2);
      // std::string msg1(buff1, len1);
      // std::string msg2(buff2, len2);
      // VLOG(3) << msg1 << msg2;
    }
  }

 private:
  bool Connected() const {
    return !!sock_options_.get() && !!sock_options_->conn.get();
  }
  int ConnectIfNot(const std::string& ip, int port);
  int InitSock(const std::string& ip, int port);
  int Connect(const std::string& addr);
  const std::string& GetProtocolName() const { return options_.protocol; }
  static void CheckConnected(int sock_fd, void* client_data);
  void OnConnected();
  void Reconnect(Connection* close_conn);
  void OnInputOk(Connection* close_conn);
  void RemoveConnection(Connection* conn);

  ClientOptions options_;
  FDGuard sock_fd_;
  std::unique_ptr<SocketOptions> sock_options_;
  struct sockaddr_in srv_addr_;

  BAN_COPY_AND_ASSIGN(Client);
};

#endif  // SRC_CLIENT_H
