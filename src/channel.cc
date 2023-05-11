#include "channel.h"

#include "controller.h"
#include "google/protobuf/descriptor.h"

bool toyRpcChannel::Init(const std::string& ip, int port, ChannelOptions opts) {
  ip_ = ip;
  port_ = port;
  opts_ = opts;
  return true;
}

void toyRpcChannel::CallMethod(const google::protobuf::MethodDescriptor* md,
                               google::protobuf::RpcController* cntl,
                               const google::protobuf::Message* request,
                               google::protobuf::Message* response,
                               google::protobuf::Closure* done) {
  auto* toy_cntl = static_cast<toyRPCController*>(cntl);
  protocol_http_.BuildRequest(md->full_name(), &toy_cntl->http_request);
  protocol_http_.PackRequest(*toy_cntl, &toy_cntl->current_sock->GetOutBuff());
  toy_cntl->call_id = GetUniqueCallId();
  toy_cntl->current_sock->StartWrite(toy_cntl->call_id, done);
}
