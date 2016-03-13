/**
 */

#ifndef SRC_MULTIPLEXER_H_
#define SRC_MULTIPLEXER_H_

#include "common/basics.h"
#include "common/shared_ptr.h"
#include "src/event.h"

#include <string>
#include <vector>
using std::string;
using std::vector;


class Multiplexer {
 public:
  Multiplexer() {}
  virtual ~Multiplexer() {}
  virtual int AddEvent(const shared_ptr<Event>& e) = 0;
  virtual int AddEvent(
      int fd,
      IOMask mask,
      const shared_ptr<EventHandler>& read_handler,
      const shared_ptr<EventHandler>& write_handler,
      void* client_data) = 0;
  virtual int DelEvent(shared_ptr<Event> e) = 0;
  virtual int DelEvent(int fd, IOMask mask) = 0;
  virtual int ProcessEvents() = 0;

 protected:
  // indexed by fd in an event.
  vector<shared_ptr<Event> > events_;
  // time event?

 private:

  DO_NOT_COPY_AND_ASSIGN(Multiplexer);
};

#endif  // SRC_MULTIPLEXER_H_
