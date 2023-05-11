#ifndef SRC_CONTROLLER_H
#define SRC_CONTROLLER_H

#include <string>

#include "common/basics.h"
#include "protocols/http.h"
#include "socket.h"

#include "google/protobuf/service.h"

class toyRPCController : public google::protobuf::RpcController {
 public:
  toyRPCController() : failed_(false) {}
  virtual ~toyRPCController() {}

  virtual void Reset() override {
    failed_ = false;
    err_txt_.clear();
  }

  virtual void SetFailed(const std::string& msg) override {
    failed_ = true;
    err_txt_ = msg;
  }
  virtual bool Failed() const override { return failed_; }
  virtual std::string ErrorText() const override { return err_txt_; }

  virtual void StartCancel() override;
  virtual bool IsCanceled() const override;
  virtual void NotifyOnCancel(google::protobuf::Closure*) override;

  HttpRequest http_request;
  HttpResponse http_response;
  Socket* current_sock;
  uint64_t call_id;

 private:
  bool failed_;
  std::string err_txt_;

  BAN_COPY_AND_ASSIGN(toyRPCController);
};

#endif //  SRC_CONTROLLER_H
