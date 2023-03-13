#include "net.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>

int CreateTcpServer(int port) {
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    return -1;
  }
  if (SetNonBlocking(sock) ||
      SetCloseOnExec(sock) ||
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

int SetCloseOnExec(int fd) {
  return fcntl(fd, F_SETFD, FD_CLOEXEC);
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


