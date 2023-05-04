#ifndef SRC_IOBUFFER_H
#define SRC_IOBUFFER_H

#include <string>

#include "common/basics.h"
#include "glog/logging.h"

#include "google/protobuf/io/zero_copy_stream.h" // ZeroCopyInputStream

// The underlying structure is a doublely-linked list of arrays.
// Initially, the list is empty. When data is written into an IOBuffer,
// a new array is allocated, and another one is allocated and linked as
// 'next' of the first one when more data should be written. Reading
// starts at the head of the list, and the array is recycled for later
// allocation when all data in it has been consumed. Memory allocation
// and deallocation maybe improved this way.
class IOBuffer {
friend class IOBufferAsZeroCopyInputStream;
friend class IOBufferAsZeroCopyOutputStream;
 private:
  struct Block;

 public:
  IOBuffer() : head_(nullptr), tail_(nullptr), recycled_(nullptr) {}
  ~IOBuffer();
  // On succeeding, bytes number is returned;
  // otherwise -1 and errno is set properly.
  int ReadFromSock(int sock_fd);
  // Send buffer to sock.
  // On succeeding, bytes written is returned; return 0 when buffer is empty.
  // -1 and errno is set.
  int WriteToSocek(int sock_fd) const;
  // Pop 'len' charactars from front of the buffer.
  // Do nothing if 'len <= 0' or buffer is empty.
  // Pop all data if len >= size of buffer.
  void PopFront(int len);
  // Pop all par before pos(included).
  // It's up to caller to ensure that head_ <= pos <= tail_.
  // Do nothing if buffer is empty.
  class iterator;
  void PopFront(const iterator& pos);
  // Pop 'len' bytes from back of the buffer.
  // Do nothing if 'len <= 0' or buffer is empty.
  // Pop all data if len >= size of buffer.
  void PopBack(int len);
  // Append another IOBuffer or block could lead to a blank hole
  // in current IOBuffer. This should be ok.
  // Append another IOBuffer to the current one.
  // Note that other will be cleared after this call. There is no internal
  // counter for references, so the underlying data would be freed when other
  // is destroyed. That is why other MUST be cleared.
  void Append(IOBuffer& other);
  // Append a block.
  void Append(Block* other);
  // Append a std::string or c_str.
  void Append(const std::string& data) {
    Append(data.c_str(), data.size());
  }
  void Append(const char* buff, size_t len);

  // It's up to caller to ensure safe range.
  class iterator {
   public:
    iterator() : blk_(nullptr), offset_(-1) {}
    iterator& operator=(const iterator& other) =  default;
    iterator(const iterator& other) =  default;
    bool IsValid() const {
      return blk_ != nullptr && offset_ >= 0;
    }
    bool operator==(const iterator& other) const {
      return (blk_ == other.blk_) && (offset_ == other.offset_);
    }
    bool operator!=(const iterator& other) const {
      return !(*this == other);
    }
    char operator*() const;
    void operator++();
    iterator operator+(int step) const;
    // Return the steps from 'other' to 'this'.
    // It's up to caller to ensure that other is behind 'this'.
    int operator-(const iterator& other) const;
  
   private:
    // only IOBuffer can initialize a valid iterator.
    friend class IOBuffer;
    iterator(Block* blk, int offset) {
      blk_ = blk;
      offset_ = offset;
    }
  
    // Does not own it.
    Block* blk_;
    // Note that offset is relative to start of blk_.
    // TODO: change offset_ to be relative to 0 in blk_. This is somehow
    // convenient
    int offset_;
    static iterator kNull;
  };

  // IOBuffer's valid range is [Begin, End).
  iterator Begin() const;
  iterator End() const;
  // Skip all 'ch', starting from 'begin'. On succeeding, return the
  // first iterator that does not equal to 'ch', otherwise return the
  // last iterator that equals to 'ch'.
  // Normally, *begin == 'ch'.
  iterator Skip(const iterator& begin, char ch);

  struct ParseOptions {
    // cur, pre
    typedef bool (*MatcherType)(void*, IOBuffer::iterator, IOBuffer::iterator);
    // cur, pre
    typedef void (*HitCallbackType)(void*, IOBuffer::iterator,
                                    IOBuffer::iterator);
    // pre
    typedef void (*MissCallbackType)(void*, IOBuffer::iterator);
  
    // The one who calls Parse.
    void* caller;
    // Parse the buffer with this as starting position. It is used to avoid
    // parsing the same data more than once.
    iterator starting_point;
    // Determine whether a condition is satisfied, for example, '\r\n' is
    // reached when parsing http protocol.
    MatcherType matcher;
    // Called when matcher returns true.
    HitCallbackType on_hit;
    // Called when the condition is not satisfied.
    MissCallbackType on_miss;
    // not used for now.
    MissCallbackType on_error;
  };
  // Return true if parse is done; otherwise false.
  bool Parse(const ParseOptions& opts) const;
  // Interpret [begin, end) as a string.
  std::string AsString(const iterator& begin, const iterator& end) const;
  // clear without freeing memory.
  void Clear() {
    head_ = nullptr;
    tail_ = nullptr;
  }
  bool Empty() const;

 private:
  friend class iterator;

  Block* AllocateBlock();
  void RecycleBlock(Block* blk);
  int FreeSpaceWrapper(Block* blk) const;
  int SizeWrapper(Block* blk) const;
  void ForwardHeadByOneBlock();
  void BackwardTailByOneBlock();

  // experimental....
  // Cut the buffer from head to pos(including).
  void MarkFilled(int len);

  Block* head_;
  Block* tail_;
  Block* recycled_;

  BAN_COPY_AND_ASSIGN(IOBuffer);
};

struct IOBuffer::Block {
  ~Block() {
    delete []data;
    data = nullptr;
    start = end = 0;
  }
  bool Full() const { return end == kDefaultBlockBytes; }
  bool Empty() const { return end == start; }
  int FreeSpace() const { return kDefaultBlockBytes - end; }
  int Size() const { return end - start; }
  char* ReadEntry() { return data + start; }
  char* WriteEntry() { return data + end; }
  void MarkFull() { end = kDefaultBlockBytes; }

  char* data;
  // default size of a block.
  constexpr static int kDefaultBlockBytes = 8192;
  Block* next;
  Block* pre;
  int start;
  // the next position to write.
  int end;

 private:
  // Make it a unique way to allocate a block.
  friend Block* IOBuffer::AllocateBlock();
  Block() {
    data = new char[kDefaultBlockBytes];
    start = end = 0;
    pre = next = nullptr;
  }

  BAN_COPY_AND_ASSIGN(Block);
};

inline IOBuffer::iterator IOBuffer::Begin() const {
  return head_ ? iterator(head_, head_->start) : iterator::kNull;
}

inline IOBuffer::iterator IOBuffer::End() const {
  return tail_ ? iterator(tail_, tail_->end) : iterator::kNull;
}

inline bool IOBuffer::Empty() const {
  return ((head_ == nullptr) && (tail_ == nullptr)) || head_->Empty();
}

inline int IOBuffer::FreeSpaceWrapper(Block* blk) const {
  return blk ? blk->FreeSpace() : 0;
}

inline int IOBuffer::SizeWrapper(Block* blk) const {
  return blk ? blk->Size() : 0;
}

inline void IOBuffer::MarkFilled(int len) {
  CHECK(tail_);
  CHECK(false);
}

inline void IOBuffer::RecycleBlock(IOBuffer::Block* blk) {
  // Recyle list does not consider 'pre'.
  blk->start = blk->end = 0;
  blk->pre = nullptr;
  blk->next = recycled_;
  recycled_ = blk;
}

inline char IOBuffer::iterator::operator*() const {
  return blk_->data[offset_];
}

inline void IOBuffer::iterator::operator++() {
 ++offset_;
 if (offset_ >= Block::kDefaultBlockBytes) {
   // do not recycle
   blk_ = blk_->next;
   offset_ = 0;
 }
}

inline IOBuffer::iterator IOBuffer::iterator::operator+(int step) const {
  CHECK(step >= 0);
  auto ret = *this;
  while (step--) { ++ret; }
  return ret;
}

inline int IOBuffer::iterator::operator-(const iterator& other) const {
  auto oth = other;
  int steps = 0;
  while (oth != *this) {
    ++steps;
    ++oth;
  }
  return steps;
}

using ZeroCopyInputStream = google::protobuf::io::ZeroCopyInputStream;
using ZeroCopyOutputStream = google::protobuf::io::ZeroCopyOutputStream;
class IOBufferAsZeroCopyInputStream : public ZeroCopyInputStream {
 public:
  explicit IOBufferAsZeroCopyInputStream(const IOBuffer& buf)
      : add_offset_(0),
        byte_count_(0),
        block_(buf.head_) {
    if (buf.Empty()) {
      block_ = nullptr;
    }
  }

  bool Next(const void** data, int* size) override;
  void BackUp(int count) override;
  bool Skip(int count) override;
  google::protobuf::int64 ByteCount() const override { return byte_count_; }

private:
  int add_offset_;
  google::protobuf::int64 byte_count_;
  const IOBuffer::Block* block_;
};

class IOBufferAsZeroCopyOutputStream : public ZeroCopyOutputStream {
 public:
  explicit IOBufferAsZeroCopyOutputStream(IOBuffer* buf)
      : buf_(buf), cur_block_(nullptr), byte_count_(0) {}

  bool Next(void** data, int* size) override;
  void BackUp(int count) override;
  google::protobuf::int64 ByteCount() const override { return byte_count_; }

 private:
  IOBuffer* buf_;
  IOBuffer::Block* cur_block_;
  google::protobuf::int64 byte_count_;
};

#endif  // SRC_IOBUFFER_H
