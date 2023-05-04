#ifndef SRC_CHANNEL_H
#define SRC_CHANNEL_H

#include "google/protobuf/service.h"

struct ChannelOptions {
  std::string protocol;
  // not used:
  std::string connection_type;
  int timeout_ms;
  int max_retry;
};

class toyRpcChannel : public google::protobuf::RpcChannel {
 public:
  enum class Status {
  };

  toyRpcChannel() {}

  bool Init(const std::string& ip, int port,  ChannelOptions opt);

  virtual void CallMethod(const google::protobuf::MethodDescriptor*,
                          google::protobuf::RpcController*,
                          const google::protobuf::Message*,
                          google::protobuf::Message*,
                          google::protobuf::Closure*) override;
};

#endif  // SRC_CHANNEL_H
