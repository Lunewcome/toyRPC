/**
 */
#ifndef SRC_NET_H_
#define SRC_NET_H_


int CheckConnection(int sock);
int CreateTcpClient(const string& host, int port);
int CreateTcpServer(int port);

#endif  // SRC_NET_H_
