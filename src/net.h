#ifndef SRC_NET_H
#define SRC_NET_H

#include <arpa/inet.h>

#include <string>

int SetNonBlocking(int fd);
int SetCloseOnExec(int fd);
int SetReuse(int fd);
int SetNoDelay(int fd);
int Bind(int fd, int port);
int Listen(int fd);
int CreateTcpServer(int port);
bool GetIpPortFromSockAddr(const sockaddr_in& sock_addr, std::string* ip, int* port);

#endif  // SRC_NET_H
