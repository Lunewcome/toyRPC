#ifndef SRC_CONNECTION_H
#define SRC_CONNECTION_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#include <functional>
#include <ostream>
#include <time.h>

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

class Connection;
typedef void (*CallbackType)(void*, int);
struct SocketOptions {
  int sock_fd;

  // yes, it owns the connection...
  std::unique_ptr<Connection> conn;

  // TODO:protocol.
   
  void* arg;
  CallbackType on_level_triggered_event;
};

struct WriteRequest {
  int sock_fd;
  
  CallbackType on_write_done;
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

  Connection(Epoll& epl, int sock, const struct sockaddr_in& addr)
      : epl_(epl),
        // in_buff_(kMaxBuffLen),
        // out_buff_(kMaxBuffLen),
        sock_(sock),
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
  int WriteImmediately(const char* data, int len);

  char buff_[kMaxBuffLen];
  Epoll& epl_;
  IOBuffer in_buff_;
  IOBuffer out_buff_;
  int sock_;
  enum Status status_;
  Peer peer_;
  // if a connection is not active for a long time, it could be closed.
  int last_active_timestamp_;

  BAN_COPY_AND_ASSIGN(Connection);
};

#endif  // SRC_CONNECTION_H
