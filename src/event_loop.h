/**
 */
#ifndef SRC_EVENT_LOOP_H_
#define SRC_EVENT_LOOP_H_

#include "common/basics.h"
#include "common/log.h"
#include "src/event.h"
#include "src/event_handler.h"
#include "src/multiplexer.h"

class EventLoop {
 public:
  EventLoop(const string& plexer);
  ~EventLoop() {}
  inline void Start();
  inline void Stop();
  inline int AddEvent(const shared_ptr<Event>& e);
  inline int AddEvent(
      int fd,
      IOMask mask,
      const shared_ptr<EventHandler>& read_handler,
      const shared_ptr<EventHandler>& write_handler,
      void* client_data);
  inline int DelEvent(shared_ptr<Event> e);
  inline int DelEvent(int fd, IOMask mask);

 private:
  void Init(const string& plexer);

  int stop_;
  shared_ptr<Multiplexer> plexer_;

  DO_NOT_COPY_AND_ASSIGN(EventLoop);
};

inline void EventLoop::Start() {
  while (!stop_) {
    plexer_->ProcessEvents();
  }
}

inline void EventLoop::Stop() {
  stop_ = 1;
}

inline int EventLoop::AddEvent(
    const shared_ptr<Event>& e) {
  return plexer_->AddEvent(e);
}

inline int EventLoop::AddEvent(
    int fd,
    IOMask mask,
    const shared_ptr<EventHandler>& read_handler,
    const shared_ptr<EventHandler>& write_handler,
    void* client_data) {
  return plexer_->AddEvent(fd,
                           mask,
                           read_handler,
                           write_handler,
                           client_data);
}

inline int EventLoop::DelEvent(shared_ptr<Event> e) {
  return plexer_->DelEvent(e);
}

inline int EventLoop::DelEvent(int fd, IOMask mask) {
  return plexer_->DelEvent(fd, mask);
}

#endif  // SRC_EVENT_LOOP_H_
