/**
 */

#include "common/basics.h"
#include "common/shared_ptr.h"

class EventHandler {
 public:
  EventHandler() {
    Init();
  }
  virtual ~EventHandler() {}
  void SetMultiplexer(shared_ptr<Multiplexer> plexer);
  virtual void Process() = 0;

 protected:
  virtual void Init() = 0;

 private:
  // Handler may use multiplexer to add/del event.
  shared_ptr<Multiplexer> plexer_;

  DO_NOT_COPY_AND_ASSIGN(EventHandler);
};
