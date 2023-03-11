#include "server.h"

#include "gflags/gflags.h"

class ExampleServer : public Server {
 public:
  ExampleServer(const ServerOption& options) : Server(options) {}
};

int main(int argc, char** argv) {
  google::SetUsageMessage("what are you expecting from \'toy\'RPC? ^_~");
  google::ParseCommandLineFlags(&argc, &argv, false);

  FLAGS_alsologtostderr = 1;
  FLAGS_log_dir = "./logs";
  google::InitGoogleLogging("bothasy");

  ServerOption option = { port:8080 };
  ExampleServer s(option);
  if (!s.Start()) {
    return -1;
  }
  VLOG(2) << "server started.";
  s.RunUtilAskedToStop();
  return 0;
}
