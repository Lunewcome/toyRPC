#ifndef SRC_PROTOCOL_H
#define SRC_PROTOCOL_H

#include "common/basics.h"
#include "common/registerer.h"
#include "iobuffer.h"

enum class ParseStatus {
  OK = 0,
  NEED_MORE_DATA = 1,
  WRONG = 2,
};

class ParseResult {
 public:
  ParseResult(ParseStatus st) : st_(st) {}
  bool OK() const { return st_ == ParseStatus::OK; }

 private:
  ParseStatus st_ = ParseStatus::OK;
};

class Protocol {
 public:
  Protocol() {}
  ~Protocol() {}
  virtual ParseResult Parse(IOBuffer& data) = 0;

 private:
  BAN_COPY_AND_ASSIGN(Protocol);
};

REGISTER_REGISTERER(Protocol);
#define REGISTER_PROTOCOL(clazz) REGISTER_CLASS(Protocol, clazz);

#endif  // SRC_PROTOCOL_H
