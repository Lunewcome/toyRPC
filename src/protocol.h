#ifndef SRC_PROTOCOL_H
#define SRC_PROTOCOL_H

#include <string>

#include "common/basics.h"
#include "common/registerer.h"
#include "iobuffer.h"

enum class ParseStatus {
  // beginning of parsing.
  INIT = 0,
  // parse done.
  OK = 1,
  // need more data on socket.
  NEED_MORE_DATA = 2,
  // continue to read and parse buff.
  CONTINUE = 3,
  // protocol method is not supported.
  UNIMPLEMENTED = 4,
  // error. now no error is considered....
  ERROR = 5,
};

class ParseResult {
 public:
  void SetStatus(ParseStatus st) { st_ = st; }
  ParseStatus GetStatus() const { return st_; }
  bool OK() const { return st_ == ParseStatus::OK; }

 private:
  ParseStatus st_ = ParseStatus::INIT;
};

class Protocol {
 public:
  Protocol() {}
  ~Protocol() {}
  virtual ParseResult Parse(IOBuffer& in) = 0;
  virtual void PackRequest(const void* data, int sz, IOBuffer& out) = 0;

 private:
  BAN_COPY_AND_ASSIGN(Protocol);
};

REGISTER_REGISTERER(Protocol);
#define REGISTER_PROTOCOL(clazz) REGISTER_CLASS(Protocol, clazz);

#endif  // SRC_PROTOCOL_H
