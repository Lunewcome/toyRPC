#ifndef SRC_IOBUFFER_IOBUFFER_H
#define SRC_IOBUFFER_IOBUFFER_H

#include <string>
using std::string;

#include "common/basics.h"

class IOBuffer {
 public:
  IOBuffer() {}
  explicit IOBuffer(const string& buf) {
    buf_ = buf;
  }
  explicit IOBuffer(const char* buf) {
    buf_.assign(buf);
  }
  ~IOBuffer() {}
  void Append(const IOBuffer& buf) {
    buf_.append(buf.buf_);
  }
  void Append(const string& buf) {
    buf_.append(buf);
  }
  void Append(const char* buf) {
    buf_.append(buf);
  }
  const string& ToString() const {
    return buf_;
  }
  const char* ToCString() const {
    return buf_.c_str();
  }
  void SeekTo(int idx) {
    // buf_
  }
  void Clear() {
    buf_.clear();
  }

 private:
  string buf_;

  DO_NOT_COPY_AND_ASSIGN(IOBuffer);
};

#endif  // SRC_IOBUFFER_H
