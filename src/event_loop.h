/**
 */

#include "common/basics.h"
#include "src/multiplexer.h"

class EventLoop {
 public:
  EventLoop() {}
  ~EventLoop() {}
  void Start();
  void Stop();

 private:

  DO_NOT_COPY_AND_ASSIGN(EventLoop);
};
