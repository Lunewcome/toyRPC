#include "src/server/server.h"

#include "glog/logging.h"

int main(int argc, char** argv) {
  google::InitGoogleLogging("bothasy");
  Server s;
  ServerOption option;
  option.port = 8080;
  s.Start(option);

  return -1;
}
