#ifndef SRC_CHANNEL_H
#define SRC_CHANNEL_H

#include "google/protobuf/service.h"
#include "protocols/http.h"

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

  bool Init(const std::string& ip, int port, ChannelOptions opts);

  virtual void CallMethod(const google::protobuf::MethodDescriptor*,
                          google::protobuf::RpcController*,
                          const google::protobuf::Message*,
                          google::protobuf::Message*,
                          google::protobuf::Closure*) override;
 private:
  static uint64_t GetUniqueCallId() {
    static uint64_t id = 0;
    return ++id;
  }

  std::string ip_;
  int port_;
  ChannelOptions opts_;
  Http protocol_http_;
};

#endif  // SRC_CHANNEL_H
