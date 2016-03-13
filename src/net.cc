#include "common/log.h"

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int SetNonBlocking(int fd) {
}

int SetNoDelay(int fd) {
}

int CreateTcpClient(const string& host, int port) {
  struct sockaddr_in srv_addr;
  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(port);
  if (!inet_aton(host.c_str(), &srv_addr.sin_addr)) {
    Log::WriteToDisk(ERROR,
                     "inet_aton Fail:%s",
                     strerror(errno));
    return -1;
  }
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    Log::WriteToDisk(ERROR,
                     "socket err:%s",
                     strerror(errno));
    return -1;
  }
  if (SetNonBlocking(sock) == -1) {
    Log::WriteToDisk(ERROR,
                     "Set nonblocking err:%s",
                     strerror(errno));
    close(sock);
    return -1;
  }
  int ret = connect(sock,
                    (struct sockaddr*)&srv_addr,
                    sizeof(srv_addr));
  if (ret == -1) {
    if (ret != EINPROGRESS) {
      Log::WriteToDisk(ERROR,
                       "Fail to conn:%s",
                       strerror(errno));
      close(sock);
      return -1;
    } else {
      Log::WriteToDisk(INFO, "conn in progress.");
    }
  }
  return sock;
}

int CreateTcpServer() {
  int sock;


  return sock;
}
