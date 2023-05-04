#include "channel.h"

void toyRpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                               google::protobuf::RpcController* cntl,
                               const google::protobuf::Message* request,
                               google::protobuf::Message* response,
                               google::protobuf::Closure* done) {
}
