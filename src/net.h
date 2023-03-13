#ifndef SRC_NET_H
#define SRC_NET_H

int SetNonBlocking(int fd);
int SetCloseOnExec(int fd);
int SetReuse(int fd);
int SetNoDelay(int fd);
int Bind(int fd, int port);
int Listen(int fd);
int CreateTcpServer(int port);

#endif  // SRC_NET_H
