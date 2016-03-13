/**
 */

#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_

#include "common/basics.h"
#include "common/shared_ptr.h"
#include "src/event_type.h"
#include "src/event_handler.h"

class Event {
 public:
  Event()
      : et_(None),
        io_mask_(NoEvent),
        fd_(-1),
        read_handler_(NULL),
        write_handler_(NULL),
        client_data_(NULL) {}
  Event(EventType et,
        IOMask mask,
        int fd,
        const shared_ptr<EventHandler>& read_handler,
        const shared_ptr<EventHandler>& write_handler,
        void* client_data)
      : et_(et),
        io_mask_(mask),
        fd_(fd),
        read_handler_(read_handler),
        write_handler_(write_handler),
        client_data_(client_data) {}
  ~Event() { /*delete client_data_;*/}
  inline shared_ptr<EventHandler>& GetReadHandler() {
    return read_handler_;
  }
  inline shared_ptr<EventHandler>& GetWriteHandler() {
    return write_handler_;
  }
  inline void SetReadHandler(
      const shared_ptr<EventHandler>& read_handler) {
    read_handler_ = read_handler;
  }
  inline void SetWriteHandler(
      const shared_ptr<EventHandler>& write_handler) {
    write_handler_ = write_handler;
  }
  inline bool IsValid() const {
    return (fd_ > 0 &&
            et_ != None &&
            io_mask_ != NoEvent);
  }
  inline int GetFd() const { return fd_; }
  inline int GetType() const { return et_; }
  inline IOMask GetMask() const { return io_mask_; }
  inline IOMask& GetMutableMask() { return io_mask_; }
  inline void* GetClientData() { return client_data_; }

 private:
  EventType et_;
  IOMask io_mask_;
  int fd_;
  shared_ptr<EventHandler> read_handler_;
  shared_ptr<EventHandler> write_handler_;
  // managed by user!
  void* client_data_;

  DO_NOT_COPY_AND_ASSIGN(Event);
};

#endif  // SRC_EVENT_H_
