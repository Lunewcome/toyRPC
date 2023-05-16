#ifndef SRC_SOCKET_H
#define SRC_SOCKET_H

#include <ostream>
#include <time.h>
#include <unordered_map>

#include "common/basics.h"
#include "epoll.h"
#include "iobuffer.h"
#include "google/protobuf/service.h"

struct Peer {
  std::string ip;
  int port;
  Peer(const std::string& _ip = "", int _port = -1) : ip(_ip), port(_port) {}
  friend std::ostream& operator<<(std::ostream& os, const Peer& p) {
    os << p.ip << ":" << p.port;
    return os;
  }
};

typedef void (*CallbackType)(void*, int);
struct SocketOptions {
  int fd;
  Peer peer;
  // std::string protocol_name;
  void* arg;
  CallbackType on_edge_triggered_event;
};

struct WriteConfig {
  uint64_t call_id;
  google::protobuf::Closure* on_replied;
};

class Socket {
 public:
  enum class Status {
    IDLE         = 0,
    KEEP_WRITE   = 1,
    ERROR        = 2,
    FAIL_CONNECT = 3,
    CONNECTING   = 4,
    CONNECTED    = 5,
  };

  Socket(const SocketOptions& options);
  ~Socket();
  // Called when a read event is fired.
  static void ProcessEpollInput(int fd, void* client_data);
  // Called when a write event is fired.
  static void ProcessEpollOut(int fd, void* client_data);
  // Send data in out_buff_ to peer:
  // 1. First connect if needed.
  // 2. If connected, write to fd directly.
  // 3. If all data is writen luckily in a call, return and call
  //    done->Run(); otherwise, register a write event to epoll.
  int StartWrite(WriteConfig config);
  // Write data to peer directly. Part of data that has not beed
  // sent would be discard.
  int WriteNoBuff(const char* data, int len);
  // Read from fd as much as possible until an error occurs.
  int ReadUntilFail(int* saved_errno);
  const Peer& GetPeer() const { return options_.peer; }

  // I/O buff.
  IOBuffer& ReadBuff() { return in_buff_; }
  IOBuffer& WriteBuff() { return out_buff_; }

  // For deleting inactive connections.
  int LastActiveTime() const { return last_active_timestamp_; }

 private:
  friend class SocketPool;
  const std::string StatusToString();
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
    CHECK_EQ(sock_map_.count(s->fd_), 0uL);
    sock_map_[s->fd_].reset(s);
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
