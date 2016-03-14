#include "common/log.h"

int main() {
  int sock = CreateTcpClient("127.0.0.1", 8899);
  return 0;
}
