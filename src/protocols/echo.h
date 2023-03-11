#ifndef SRC_PROTOCOLS_DEBUG_H
#define SRC_PROTOCOLS_DEBUG_H

#include "protocol.h"

class Echo : public Protocol {
 public:
  Echo() {}
  ~Echo() {}
  virtual ParseResult Parse(IOBuffer& data) override {
    // always ok.
    return ParseStatus(ParseStatus::OK);
  }
};

#endif  // SRC_PROTOCOLS_DEBUG_H
