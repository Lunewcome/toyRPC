#ifndef SRC_PROTOCOLS_DEBUG_H
#define SRC_PROTOCOLS_DEBUG_H

#include "protocol.h"

class Echo : public Protocol {
 public:
  Echo() {}
  ~Echo() {}
  virtual ParseResult Parse(IOBuffer& data) override {
    // always ok.
    ParseResult pr;
    pr.SetStatus(ParseStatus::OK);
    return pr;
  }
  virtual void PackRequest(const void* data, int sz, IOBuffer& out) override;
};

#endif  // SRC_PROTOCOLS_DEBUG_H
