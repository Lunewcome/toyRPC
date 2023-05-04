#include "iobuffer.h"

#include <sys/uio.h>

#include "glog/logging.h"

IOBuffer::iterator IOBuffer::iterator::kNull = iterator(nullptr, -1);
IOBuffer::~IOBuffer() {
  // All data must have been processed...
  if (!Empty()) {
    VLOG(1) << "There is still data unprocessed.";
  }
  while (recycled_) {
    auto* next = recycled_->next;
    delete recycled_;
    recycled_ = next;
  }
}

int IOBuffer::ReadFromSock(int sock_fd) {
  if (Empty()) {
    Append(AllocateBlock());
  }
  static const int iovcnt = 2;
  struct iovec vecs[iovcnt];
  vecs[0].iov_base = tail_->WriteEntry();
  vecs[0].iov_len = tail_->FreeSpace();
  auto* blk = AllocateBlock();
  vecs[1].iov_base = blk->WriteEntry();
  vecs[1].iov_len = blk->FreeSpace();
  int rc = readv(sock_fd, vecs, iovcnt);
  if (rc <= 0) {
    RecycleBlock(blk);
    return rc;
  } else if (static_cast<size_t>(rc) < vecs[0].iov_len) {
    // tail_ is not full, recycle 'blk'.
    RecycleBlock(blk);
    tail_->end += rc;
  } else {
    // tali_ is full now.
    tail_->MarkFull();
    Append(blk);
  }
  return rc;
}

int IOBuffer::WriteToSocek(int sock_fd) const {
  if (Empty()) {
    return 0;
  }
  constexpr static int kMaxVCnt = 4;
  int iovcnt = 0;
  struct iovec vecs[kMaxVCnt];
  auto blk = head_;
  while (blk && iovcnt < kMaxVCnt) {
    vecs[iovcnt].iov_base = blk->ReadEntry();
    vecs[iovcnt].iov_len = blk->Size();
    blk = blk->next;
    ++iovcnt;
  }
  return writev(sock_fd, vecs, iovcnt);
}

void IOBuffer::Append(IOBuffer& other) {
  if (other.Empty()) {
    return;
  }
  if (Empty()) {
    head_ = other.head_;
    tail_ = other.tail_;
  } else {
    tail_->next = other.head_;
    other.head_->pre = tail_;
    tail_ = other.tail_;
  }
  other.Clear();
}

void IOBuffer::Append(Block* blk) {
  if (!blk) {
    return;
  }
  if (Empty()) {
    head_ = blk;
    tail_ = blk;
    // neccesary?
    blk->next = blk->pre = nullptr;
  } else {
    tail_->next = blk;
    blk->pre = tail_;
    tail_ = blk;
  }
}

void IOBuffer::Append(const char* buff, size_t len) {
  size_t var_len = len;
  for (size_t idx = 0; idx < len;) {
    size_t fs = FreeSpaceWrapper(tail_);
    if (fs == 0) {
      Append(AllocateBlock());
    } else if (fs <= var_len) {
      memcpy(tail_->WriteEntry(), buff + idx, fs);
      tail_->MarkFull();
      idx += fs;
      var_len -= fs;
    } else {
      // enouth space.
      memcpy(tail_->WriteEntry(), buff + idx, var_len);
      tail_->end += var_len;
      return;
    }
  }
}

IOBuffer::Block* IOBuffer::AllocateBlock() {
  if (recycled_) {
    auto* free_blk = recycled_;
    recycled_ = recycled_->next;
    // recycle consider next only.
    free_blk->next = nullptr;
    return free_blk;
  } else {
    return new Block();
  }
}

void IOBuffer::PopFront(int len) {
  while (len > 0) {
    if (Empty()) {
      break;
    }
    int fs = head_->Size();
    if (fs <= len) {
      len -= fs;
      ForwardHeadByOneBlock();
    } else {
      head_->start += len;
    }
  }
}

void IOBuffer::PopBack(int len) {
  while (len > 0) {
    if (Empty()) {
      break;
    }
    int fs = tail_->Size();
    if (fs <= len) {
      len -= fs;
      BackwardTailByOneBlock();
    } else {
      tail_->end -= len;
    }
  }
}

void IOBuffer::PopFront(const iterator& pos) {
  if (Empty()) {
    return;
  }
  while (head_ != pos.blk_) {
    ForwardHeadByOneBlock();
  }
  // pos.offset_ should also be cut.
  head_->start += pos.offset_ + 1;
  if (head_->start >= Block::kDefaultBlockBytes) {
    ForwardHeadByOneBlock();
  }
  // head_ overruns tail_.
  if (head_ == nullptr) {
    tail_ = nullptr;
  }
}

void IOBuffer::ForwardHeadByOneBlock() {
  if (Empty()) {
    return;
  }
  auto* tmp = head_;
  head_ = head_->next;
  if (head_) {
    head_->pre = nullptr;
  } else {
    // Let Empty() make sense...
    tail_ = nullptr;
  }
  RecycleBlock(tmp);
}

void IOBuffer::BackwardTailByOneBlock() {
  if (Empty()) {
    return;
  }
  auto* tmp = tail_;
  tail_ = tail_->pre;
  if (tail_) {
    tail_->next = nullptr;
  } else {
    head_ = nullptr;
  }
  RecycleBlock(tmp);
}

bool IOBuffer::Parse(const ParseOptions& opts) const {
  iterator pre = iterator::kNull;
  iterator cur = opts.starting_point;
  const auto& end = End();
  while (cur != end) {
    if (pre != iterator::kNull && opts.matcher(opts.caller, pre, cur)) {
      opts.on_hit(opts.caller, pre, cur);
      return true;
    }
    pre = cur;
    ++cur;
  }
  CHECK(cur == end);
  opts.on_miss(opts.caller, pre);
  return false;
}

IOBuffer::iterator IOBuffer::Skip(const iterator& begin, char ch) {
  const auto& end = End();
  iterator itrt = begin;
  CHECK_EQ(*itrt, ch);
  while (*itrt == ch && (itrt + 1) != end) {
    ++itrt;
  }
  return itrt;
}

std::string IOBuffer::AsString(const iterator& begin,
                               const iterator& end) const {
  std::string ret;
  auto itrt = begin;
  const auto& buff_end = End();
  while (itrt != end && itrt != buff_end) {
    ret += *itrt;
    ++itrt;
  }
  return ret;
}

bool IOBufferAsZeroCopyInputStream::Next(const void** data, int* size) {
  if (!block_) {
    return false;
  }
  *data = block_->data + block_->start + add_offset_;
  *size = block_->Size() - add_offset_;
  byte_count_ += block_->Size() - add_offset_;
  add_offset_ = 0;
  block_ = block_->next;
  return true;
}

void IOBufferAsZeroCopyInputStream::BackUp(int count) {
  if (block_ && block_->pre) {
    const IOBuffer::Block* pre = block_->pre;
    CHECK(add_offset_ == 0 && pre->Size() >= count)
        << "BackUp() is not after a Next()";
    add_offset_ = pre->Size() - count;
    byte_count_ -= count;
    block_ = pre;
  } else {
    LOG(FATAL) << "BackUp an empty ZeroCopyInputStream";
  }
}

bool IOBufferAsZeroCopyInputStream::Skip(int count) {
  while (block_) {
    const int left_bytes = block_->Size() - add_offset_;
    if (count < left_bytes) {
      add_offset_ += count;
      byte_count_ += count;
      return true;
    }
    count -= left_bytes;
    add_offset_ = 0;
    byte_count_ += left_bytes;
    block_ = block_->next;
  }
  return false;
}

bool IOBufferAsZeroCopyOutputStream::Next(void** data, int* size) {
  if (buf_->Empty() || buf_->tail_->Full()) {
    // core if AllocateBlock fails.
    cur_block_ = buf_->AllocateBlock();
    buf_->Append(cur_block_);
  }
  *data = (void*)(cur_block_->WriteEntry());
  *size = cur_block_->FreeSpace();
  byte_count_ += cur_block_->FreeSpace();
  return true;
}

void IOBufferAsZeroCopyOutputStream::BackUp(int count) {
  if (cur_block_ && !buf_->Empty()) {
    CHECK(cur_block_ == buf_->tail_) << "cur_block_ must match buf_->tail_.";
  }
  buf_->PopBack(count);
}
