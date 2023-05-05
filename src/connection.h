#ifndef SRC_CONNECTION_H
#define SRC_CONNECTION_H

#include <arpa/inet.h>

#include <ostream>
#include <time.h>
#include <unordered_map>

#include "common/basics.h"
#include "epoll.h"
#include "iobuffer.h"

struct Peer {
  Peer(const struct sockaddr_in& sock_addr) {
    if (inet_ntop(AF_INET, &sock_addr.sin_addr, ip, INET_ADDRSTRLEN)) {
      port = ntohs(sock_addr.sin_port);
    }
  }
  friend std::ostream& operator<<(std::ostream& os, const Peer& p) {
    os << p.ip << ":" << p.port;
    return os;
  }

  char ip[16];
  int port;
};

struct WriteRequest {
  int sock_fd;
  
  // CallbackType on_write_done;
};

class Connection {
 public:
  enum class Status {
    IDLE                       = 0,
    CONNECTING                 = 1,
    CONNECTED                  = 2,
    CLOSING                    = 3,
    CLOSED                     = 4,

    READ_OK                    = 5,

    WAITING_BUFFER_SPACE       = 6,
    WRITING                    = 7,
    KEEP_WRITE                 = 8,

    ERROR                      = 9,
  };

  Connection(int sock, const struct sockaddr_in& addr)
        // in_buff_(kMaxBuffLen),
        // out_buff_(kMaxBuffLen),
      : sock_(sock),
        status_(Status::IDLE),
        peer_(addr),
        last_active_timestamp_(time(nullptr)) {
    VLOG(4) << "peer:" << peer_;
  }
  ~Connection() { close(GetSock()); }

  static void ProcessEpollInput(int fd, void* client_data);
  static void ProcessEpollOut(int fd, void* client_data);
  int Send(const char* dat, int len);
  int ReadUntilFail(int* save_errno);
  enum Status Status() const { return status_; }
  static const std::string StatusToString(enum Status st);
  int GetSock() const { return sock_; }
  const Peer& GetPeer() const { return peer_; }

  void SyncSend(const char* data, int sz);

  IOBuffer& GetInBuff() { return in_buff_; }
  IOBuffer& GetOutBuff() { return out_buff_; }

  int LastActiveTime() const { return last_active_timestamp_; }

 private:
  static constexpr int kMaxBuffLen = 1024 * 1024;

  int ConnectIfNot(const std::string& ip, int port);
  int InitSock(const std::string& ip, int port);
  int WriteImmediately(const char* data, int len);

  char buff_[kMaxBuffLen];
  IOBuffer in_buff_;
  IOBuffer out_buff_;
  int sock_;
  enum Status status_;
  Peer peer_;
  // if a connection is not active for a long time, it could be closed.
  int last_active_timestamp_;

  BAN_COPY_AND_ASSIGN(Connection);
};

typedef void (*CallbackType)(void*, int);
struct SocketOptions {
  int sock_fd;

  // yes, it owns the connection...
  std::unique_ptr<Connection> conn;

  // TODO:protocol.
   
  void* arg;
  CallbackType on_level_triggered_event;
};

class ConnectionManager {
 public:
  ConnectionManager() {}
  void Insert(SocketOptions* options) {
    CHECK_EQ(conn_map_.count(options->sock_fd), 0uL);
    conn_map_[options->sock_fd].reset(options);
  }
  Connection* GetConnection(int fd) const {
    const auto& itrt_sock = conn_map_.find(fd);
    return itrt_sock == conn_map_.end() ? nullptr : itrt_sock->second->conn.get();
  }
  void Remove(int fd) {
    const auto& conn_itrt = conn_map_.find(fd);
    CHECK(conn_itrt != conn_map_.end())
        << "trying to remove a non-existing connection.";
    CHECK_EQ(GetGlobalEpoll()->DelEvent(fd, IOMaskRW), 0);
    // How if some data has already been read from this sock?
    // Seems that sock should be closed after all read/write
    // events have been handled.
    conn_map_.erase(conn_itrt);
  }

 private:
  void RemoveDeadConnection();

  std::unordered_map<int, std::unique_ptr<SocketOptions>> conn_map_;

  BAN_COPY_AND_ASSIGN(ConnectionManager);
};

ConnectionManager* GetGlobalConnectionManager();

#endif  // SRC_CONNECTION_H
