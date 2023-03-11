#ifndef SRC_IOBUFFER_H
#define SRC_IOBUFFER_H

#include "common/basics.h"

// A ring buffer.
class IOBuffer {
 public:
  IOBuffer(int capacity) : capacity_(capacity) {
    buff_ = new char[capacity];
  }
  ~IOBuffer() {
    capacity_ = 0;
    delete []buff_;
    buff_ = nullptr;
  }

  void Flush() { start_ = end_ = 0; }

  // return: bytes written.
  int Append(const char* data, int len) {
    int written = 0;
    while (!Full() && written < len) {
      buff_[end_++] = data[written++];
      end_ %= capacity_;
    }
    return written;
  }

  // it's up to caller to ensure that buff is not empty.
  char ConsumeOneChar() {
    start_ %= capacity_;
    return buff_[start_++];
  }

  int ConsumeRange(char** buff, int* len) {
    *buff = buff_ + start_;
    if (start_ < end_) {
      *len = end_ - start_;;
    } else {
      *len = capacity_ - start_;
    }
    return start_;
  }
  void Confirm(int new_start) { start_ = new_start; }

  char operator[](int idx) const {
    idx = (start_ + idx) % capacity_;
    return buff_[idx];
  }
  // like string view?
  void View(char** buff1, int* len1, char** buff2, int* len2) const {
    *buff1 = buff_ + start_;
    if (start_ < end_) {
      *len1 = end_ - start_;

      *buff2 = nullptr;
      *len2 = 0;
    } else {
      *len1 = capacity_ - start_;

      *buff2 = buff_;
      *len2 = end_ + 1;
    }
  }

  bool Full() const { return FreeSpace() == 0; }
  bool Empty() const { return start_ == end_; }
  int FreeSpace() const {
    // one slot for sentinel.
    return (capacity_ - 1 - (end_ - start_)) % capacity_;
  }

 private:
  int capacity_;
  char* buff_;
  int start_ = 0;
  int end_ = 0;
  
  BAN_COPY_AND_ASSIGN(IOBuffer);
};

#endif  // SRC_IOBUFFER_H
