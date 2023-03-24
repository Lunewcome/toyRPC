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

  int ConsumeRange(const char** buff, int* len) {
    *buff = buff_ + start_;
    if (start_ < end_) {
      *len = end_ - start_;;
    } else {
      *len = capacity_ - start_;
    }
    return start_;
  }
  void Confirm(int offset_from_start) {
    start_ += offset_from_start;
    start_ %= capacity_;
  }

  // return : 'offset' from start_ or -1.
  int Find(char c) {
    int pos = start_;
    while (pos != end_) {
      if (buff_[pos] == c) {
        return pos - start_;
      }
      ++pos %= capacity_;
    }
    return -1;
  }

  // pos : offset from start_.
  char GetPrePosition(int pos) const {
    return --pos;
  }

  // split substring in range [start_, end_pos) with delemiter.
  void Split(int end_pos, char delemiter, std::vector<std::string>* results) {
    bool pre_is_delemiter = true;
    int start = start_;
    end_pos += start;
    end_pos %= capacity_;
    while (start != end_pos) {
      if (buff_[start] == delemiter) {
        pre_is_delemiter = true;
      } else {
        if (pre_is_delemiter) {
          results->emplace_back(buff_ + start, 1);
        } else {
          results->back().append(buff_ + start, 1);
        }
        pre_is_delemiter = false;
      }
      ++start %= capacity_;
    }
  }

  void MoveData(int len, std::string* dest) {
    dest->reserve(len);
    int pos;
    for (int i = 0; i < len; ++i) {
      pos = (start_ + i) % capacity_;
      dest->append(buff_ + pos, 1);
    }
    Confirm(len);
  }

  char operator[](int idx) const {
    // idx could be negative...
    idx = (start_ + idx + capacity_) % capacity_;
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

  int Size() const {
    return (end_ - start_ + capacity_) % capacity_;
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
