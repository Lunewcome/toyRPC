#ifndef SRC_TOYRPC_CLIENT_H
#define SRC_TOYRPC_CLIENT_H

#include <functional>
#include <memory>
#include <string>

#include "common/basics.h"
#include "common/fd_guard.h"
#include "connection.h"

struct ClientOptions {
  std::string ip;
  int port;
  std::string protocol;
  int timeout_ms;
  // connection_type;
};

class toyRPCClient {
 public:
  typedef std::function<void(void)> OnSendDoneCallback;

  toyRPCClient(ClientOptions opt) : options_(opt) {};

  void DebugSend(const char* msg, int len, OnSendDoneCallback done) {
    if (ConnectIfNot(options_.ip, options_.port) != 0) {
      return;
    }
    VLOG(4) << sock_options_->conn->Send(msg, len, nullptr);
  }

  void AwaitEpoll() {
    if (Connected()) {
      GetGlobalEpoll().AwaitEpoll();
    }
  }

  void OnNewMsg(int sock_fd) {
    VLOG(3) << "client new msg...";
    auto* conn = sock_options_->conn.get();
    auto status = conn->ConsumeDataStream();
    if (status == Connection::Status::CLOSING) {
      VLOG(4) << "peer(" << conn->GetPeer() << ") closed connection.";
      RemoveConnection(conn);
      return;
    } else if (status == Connection::Status::WAITING_BUFFER_SPACE) {
      VLOG(3) << "waiting space to receive data from " << conn->GetPeer()
          << ". This could prevent the client from sending data.";
      return;
    } else {
      CHECK(status == Connection::Status::READ_OK)
          << "what's up:" << conn->GetPeer() << ","
          << Connection::StatusToString(status);
      auto& in_buff = conn->GetInBuff();
      char* buff1;
      char* buff2;
      int len1, len2;
      in_buff.View(&buff1, &len1, &buff2, &len2);
      std::string msg1(buff1, len1);
      std::string msg2(buff2, len2);
      VLOG(3) << msg1 << msg2;
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

  BAN_COPY_AND_ASSIGN(toyRPCClient);
};

#endif  // SRC_TOYRPC_CLIENT_H
