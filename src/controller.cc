#include "controller.h"

void toyRPCController::StartCancel() {
}

bool toyRPCController::IsCanceled() const {
  return false;
}

void toyRPCController::NotifyOnCancel(google::protobuf::Closure*) {
}

toyRPCControllerPool* GetGlobaltoyRPCControllerPool() {
  static toyRPCControllerPool pool;
  return &pool;
}
