#ifndef SRC_CONNECTION_H
#define SRC_CONNECTION_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#include <ostream>

#include "common/basics.h"
#include "epoll.h"
#include "iobuffer.h"

struct Peer {
  char ip[16];
  int port;
  friend std::ostream& operator<<(std::ostream& os, const Peer& p) {
    os << p.ip << ":" << p.port;
    return os;
  }
};

class Connection {
 public:
  enum class Status {
    IDLE                       = 0,
    CONNECTING                 = 1,
    CONNECTED                  = 2,
    CLOSING                    = 3,
    CLOSED                     = 4,

    READING                    = 5,

    WAITING_BUFFER_SPACE       = 6,
    WRITING                    = 7,
    KEEP_WRITE                 = 8,

    ERROR                      = 9,
  };

  Connection(Epoll& epl, int sock, const struct sockaddr_storage& addr)
      : epl_(epl),
        in_buff_(kMaxBuffLen),
        out_buff_(kMaxBuffLen),
        sock_(sock),
        status_(Status::IDLE) {
    SetPeer(addr);
  }
  // it's up to 'server.cc' to close connection.
  // ~Connection() { Close(); }

  enum Status Status() const { return status_; }
  static const std::string& StatusToString(enum Status st);

  int Sock() const { return sock_; }

  const Peer& GetPeer() const { return peer_; }

  enum Status Receive();

  int WriteImmediately(const char* data, int len);
  void KeepWrite(int fd, void* client_data);
  void SyncSend(const char* data, int sz);

  IOBuffer& GetInBuff() { return in_buff_; }
  IOBuffer& GetOutBuff() { return out_buff_; }

 private:
  void SetPeer(const struct sockaddr_storage& addr) {
    auto* sock_addr = (sockaddr_in*)(&addr);
    if (inet_ntop(AF_INET, &sock_addr->sin_addr, peer_.ip, INET_ADDRSTRLEN)) {
      peer_.port = sock_addr->sin_port;
    }
    VLOG(4) << "peer is:" << peer_;
  }

  static constexpr int kMaxBuffLen = 1024 * 1024;

  char buff_[kMaxBuffLen];

  Epoll& epl_;

  IOBuffer in_buff_;
  IOBuffer out_buff_;

  int sock_;
  enum Status status_;

  Peer peer_;

  BAN_COPY_AND_ASSIGN(Connection);
};

#endif  // SRC_CONNECTION_H
