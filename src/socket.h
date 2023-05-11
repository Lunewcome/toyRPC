#ifndef SRC_SOCKET_H
#define SRC_SOCKET_H

#include <arpa/inet.h>

#include <ostream>
#include <time.h>
#include <unordered_map>

#include "common/basics.h"
#include "epoll.h"
#include "iobuffer.h"
#include "google/protobuf/service.h"

struct Peer {
  Peer() = default;
  Peer(const std::string& ip, int port) : local_ip(ip), local_port(port) {}
  Peer(const struct sockaddr_in& sock_addr) {
    char ip[16];
    memset(ip, '\0', 16);
    if (inet_ntop(AF_INET, &sock_addr.sin_addr, ip, INET_ADDRSTRLEN)) {
      local_ip.assign(ip);
      local_port = ntohs(sock_addr.sin_port);
    }
  }
  const std::string& ip() const { return local_ip; }
  int port() const { return local_port; }
  friend std::ostream& operator<<(std::ostream& os, const Peer& p) {
    os << p.local_ip << ":" << p.local_port;
    return os;
  }

 private:
  std::string local_ip;
  int local_port = -1;
};

typedef void (*CallbackType)(void*, int);
struct SocketOptions {
  int sock_fd;
  Peer peer;

  std::string protocol_name;

  void* arg;
  CallbackType on_level_triggered_event;
};

class Socket {
 public:
  enum class Status {
    IDLE         = 0,
    KEEP_WRITE   = 1,
    ERROR        = 2,
    FAIL_CONNECT = 3,
    Connecting   = 4,
    Connected    = 5,
  };

  Socket(const SocketOptions& options)
      : status_(Status::IDLE),
        last_active_timestamp_(time(nullptr)),
        options_(options) {
    fd_ = options_.sock_fd;
  }
  ~Socket() {
    if (fd_ > 0) {
      VLOG(4) << "Closing connection to " << options_.peer;
      close(fd_);
      fd_ = -1;
    }
    // ignore in_buff_ / out_buff_ ?
  }
  static void ProcessEpollInput(int fd, void* client_data);
  static void ProcessEpollOut(int fd, void* client_data);
  int StartWrite(uint64_t call_id, google::protobuf::Closure* done);
  int WriteNoBuff(const char* data, int len);
  int ReadUntilFail(int* saved_errno);
  enum Status Status() const { return status_; }
  static const std::string StatusToString(enum Status st);
  int GetFD() const { return fd_; }
  const Peer& GetPeer() const { return options_.peer; }

  IOBuffer& GetInBuff() { return in_buff_; }
  IOBuffer& GetOutBuff() { return out_buff_; }

  int LastActiveTime() const { return last_active_timestamp_; }
  const SocketOptions& GetOptions() const { return options_; }

 private:
  int ConnectIfNot();
  int InitSock(const Peer& peer, struct sockaddr_in* peer_addr);
  bool CheckConnected(int fd);

  int fd_;
  enum Status status_;
  // close a inactive connection.
  int last_active_timestamp_;
  SocketOptions options_;
  IOBuffer in_buff_;
  IOBuffer out_buff_;
  std::unordered_map<uint64_t, std::unique_ptr<google::protobuf::Closure>>
      req_done_map_;

  BAN_COPY_AND_ASSIGN(Socket);
};

class SocketPool {
 public:
  SocketPool() {}
  void Insert(Socket* s) {
    CHECK_EQ(sock_map_.count(s->GetFD()), 0uL);
    sock_map_[s->GetFD()].reset(s);
  }
  Socket* Get(int fd) const {
    const auto& itrt_sock = sock_map_.find(fd);
    return itrt_sock == sock_map_.end() ? nullptr : itrt_sock->second.get();
  }
  void Remove(int fd) {
    const auto& conn_itrt = sock_map_.find(fd);
    CHECK(conn_itrt != sock_map_.end())
        << "trying to remove a non-existing socket.";
    CHECK_EQ(GetGlobalEpoll()->DelEvent(fd, IOMaskRW), 0);
    // How if some data has already been read from this sock?
    // Seems that sock should be closed after all read/write
    // events have been handled.
    sock_map_.erase(conn_itrt);
  }

 private:
  void RemoveDeadSocks();

  std::unordered_map<int, std::unique_ptr<Socket>> sock_map_;

  BAN_COPY_AND_ASSIGN(SocketPool);
};

SocketPool* GetGlobalSocketPool();

#endif  // SRC_SOCKET_H
