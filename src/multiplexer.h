/**
 */

#ifndef SRC_MULTIPLEXER_H_
#define SRC_MULTIPLEXER_H_

#include "common/basics.h"
#include "common/shared_ptr.h"
#include "src/event.h"

#include <string>
using std::string;


class Multiplexer {
 public:
  Multiplexer() {}
  virtual ~Multiplexer() {}
  virtual int AddEvent(shared_ptr<Event> e) = 0;
  virtual int DelEvent(shared_ptr<Event> e) = 0;
  virtual void ProcessEvents() = 0;

 private:

  DO_NOT_COPY_AND_ASSIGN(Multiplexer);
};

#endif  // SRC_MULTIPLEXER_H_
