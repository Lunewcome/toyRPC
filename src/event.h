/**
 */

#ifndef SRC_REACTOR_EVENT_H
#define SRC_REACTOR_EVENT_H

#include <memory>
#include <functional>

typedef std::function<void(int, void*)> EventHandler;

typedef int IOMask;

constexpr int IOMaskNone = 0x0;
constexpr int IOMaskRead = (1<<0);
constexpr int IOMaskWrite = (1<<1);
constexpr int IOMaskRW = (IOMaskRead | IOMaskWrite);

struct Event {
  Event() {}
  Event(int _fd, IOMask mask, EventHandler reader, EventHandler writer,
        void* data)
      : fd(_fd), io_mask(mask), read(reader), write(writer), client_data(data) {}
  Event(const Event&) = default;
  Event& operator=(const Event&) = default;
  void Reset() {
    fd = -1;
    io_mask = IOMaskNone;
    read = nullptr;
    write = nullptr;
    client_data = nullptr;
  }

  void AddReadEvent() { io_mask |= IOMaskRead; }
  void AddWriteEvent() { io_mask |= IOMaskWrite; }
  void DelReadEvent() { io_mask &= ~IOMaskRead; }
  void DelWriteEvent() { io_mask &= ~IOMaskWrite; }
  bool WaitingRead() const { return io_mask & IOMaskRead; }
  bool WaitingWrite() const { return io_mask & IOMaskWrite; }

  int fd = -1;
  IOMask io_mask = IOMaskNone;
  EventHandler read = nullptr;
  EventHandler write = nullptr;
  // managed by user!
  void* client_data = nullptr;
};

#endif  // SRC_REACTOR_EVENT_H
