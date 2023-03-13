#include "toyrpc_client.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_string(ip, "", "");
DEFINE_int32(port, 0, "");

class ExampleClient : public toyRPCClient {
 public:
  ExampleClient(ClientOptions& opt) : toyRPCClient(opt) {}
};

int main(int argc, char** argv) {
  google::SetUsageMessage("what are you expecting from \'toy\'RPC? ^_~");
  google::ParseCommandLineFlags(&argc, &argv, false);

  FLAGS_alsologtostderr = 1;
  FLAGS_log_dir = "./logs";
  google::InitGoogleLogging("bothasy-client");

  ClientOptions option = {
    ip : FLAGS_ip,
    port : FLAGS_port,
    timeout_ms:10,
  };
  ExampleClient c(option);

  const char* msg = "GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";

  c.DebugSend(msg, strlen(msg), nullptr);

  c.AwaitEpoll();

  return 0;
}
