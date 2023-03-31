#include "server.h"

#include "gflags/gflags.h"

class ExampleServer : public Server {
 public:
  ExampleServer(const ServerOptions& options) : Server(options) {}
};

int main(int argc, char** argv) {
  google::SetUsageMessage("what are you expecting from \'toy\'RPC? ^_~");
  google::ParseCommandLineFlags(&argc, &argv, false);

  FLAGS_alsologtostderr = 1;
  FLAGS_log_dir = "./logs";
  google::InitGoogleLogging("bothasy-server");

  ServerOptions option = { port:8080 };
  ExampleServer s(option);
  s.Start();
  VLOG(2) << "server started.";
  s.RunUtilAskedToStop();
  return 0;
}
