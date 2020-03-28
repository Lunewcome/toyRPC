#include "common/log.h"

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>

int SetNonBlocking(int fd);
int SetReuse(int fd);
int SetNoDelay(int fd);
int Bind(int fd, int port);
int Listen(int fd);

int CreateTcpClient(const string& host, int port) {
  struct sockaddr_in srv_addr;
  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(port);
  if (!inet_aton(host.c_str(), &srv_addr.sin_addr)) {
    return -1;
  }
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    return -1;
  }
  if (SetNonBlocking(sock) == -1) {
    close(sock);
    return -1;
  }
  int ret = connect(sock,
                    (struct sockaddr*)&srv_addr,
                    sizeof(srv_addr));
  if (ret == -1) {
    if (errno != EINPROGRESS) {
      close(sock);
      return -1;
    } else {
    }
  }
  return sock;
}

int CreateTcpServer(uint32_t port) {
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    return -1;
  }
  if (SetNonBlocking(sock) ||
      SetReuse(sock) ||
      SetNoDelay(sock) ||
      Bind(sock, port) ||
      Listen(sock)) {
    return -1;
  }
  return sock;
}

int SetNonBlocking(int fd) {
  int flg = fcntl(fd, F_GETFL);
  if (fcntl(fd, F_SETFL, flg|O_NONBLOCK) < 0) {
    return -1;
  }
  return 0;
}

int SetReuse(int fd) {
  int reuse = 1;
  int opt = setsockopt(fd,
                       SOL_SOCKET,
                       SO_REUSEADDR,
                       &reuse,
                       sizeof(reuse));
  if (opt < 0) {
  }
  return opt;
}

int SetNoDelay(int fd) {
  int yes = 1;
  if (setsockopt(fd,
                 IPPROTO_TCP,
                 TCP_NODELAY,
                 &yes,
                 sizeof(yes)) == -1) {
    return -1;
  }
  return 0;
}

int Bind(int fd, int port) {
  struct sockaddr_in sock_addr;
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sock_addr.sin_port = htons(port);
  int ret = bind(fd,
                 (struct sockaddr*)&sock_addr,
                 sizeof(sock_addr));
  if (ret < 0) {
    return -1;
  }
  return 0;
}

int Listen(int fd) {
  int ret = listen(fd, 5);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

int CheckConnection(int sock) { 
  int err = -1;
  socklen_t len = sizeof(int);
  if (getsockopt(sock,
                 SOL_SOCKET,
                 SO_ERROR,
                 &err,
                 &len) < 0 ){
    // close(sock);
    return -1;
  }
  if (!err) {
    return -1;
  } else {
    return 0;
  }
}
