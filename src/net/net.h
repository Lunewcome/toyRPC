/**
 */
#ifndef SRC_NET_NET_H
#define SRC_NET_NET_H

int CheckConnection(int sock);
int CreateTcpClient(const string& host, int port);
int CreateTcpServer(uint32_t port);

#endif  // SRC_NET_NET_H
