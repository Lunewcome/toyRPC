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
  virtual int AddEvent(shared_ptr<Event> e) = 0;
  virtual int DelEvent(shared_ptr<Event> e) = 0;
  virtual void ProcessEvents() = 0;

 protected:
  // indexed by fd in an event.
  vector<shared_ptr<Event> > events_;
  // time event?

 private:

  DO_NOT_COPY_AND_ASSIGN(Multiplexer);
};

#endif  // SRC_MULTIPLEXER_H_
