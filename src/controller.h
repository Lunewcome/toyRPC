#ifndef SRC_CONTROLLER_H
#define SRC_CONTROLLER_H

#include <string>
#include <unordered_map>

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

class toyRPCControllerPool {
 public:
  toyRPCController* Allocate(uint64_t call_id) {
    auto& ptr = call_id_cntl_map_[call_id];
    ptr.reset(new toyRPCController());
    return ptr.get();
  }
  void Remove(uint32_t call_id) {
    CHECK(Exist(call_id));
    call_id_cntl_map_.erase(call_id);
  }
  bool Exist(uint32_t call_id) const {
    const auto& itrt = call_id_cntl_map_.find(call_id);
    return itrt != call_id_cntl_map_.end();
  }

 private:
  friend toyRPCControllerPool* GetGlobaltoyRPCControllerPool();
  toyRPCControllerPool() = default;

  std::unordered_map<uint64_t, std::unique_ptr<toyRPCController>>
      call_id_cntl_map_;

  BAN_COPY_AND_ASSIGN(toyRPCControllerPool);
};

toyRPCControllerPool* GetGlobaltoyRPCControllerPool();

#endif //  SRC_CONTROLLER_H
