/**
 */

#ifndef SRC_REACTOR_EVENT_H
#define SRC_REACTOR_EVENT_H

#include "common/basics.h"
#include "src/reactor/event_type.h"
#include "src/reactor/event_handler.h"

struct Event {
  Event() {}
  Event(int _fd, void* data) : fd(_fd), client_data(data) {}
  ~Event() {
    delete handler;
  }

  int fd = 0;
  IOMask io_mask = 0;
  EventHandler* handler = nullptr;
  // managed by user!
  void* client_data = nullptr;

 private:
  DO_NOT_COPY_AND_ASSIGN(Event);
};

void SetEvents(Event* e, int events) {
  e->io_mask |= events;
}

void ClearAllEvent(Event* e) {
  e->io_mask = 0;
}

void AddReadEvent(Event* e) {
  e->io_mask |= IOMaskRead;
}

void AddWriteEvent(Event* e) {
  e->io_mask |= IOMaskWrite;
}

void AddRDEvent(Event* e) {
  AddReadEvent(e);
  AddWriteEvent(e);
}

void DelReadEvent(Event* e) {
  e->io_mask &= ~(IOMaskRead);
}

void DelWriteEvent(Event* e) {
  e->io_mask &= ~(IOMaskWrite);
}

void DelRWEvent(Event* e) {
  DelReadEvent(e);
  DelWriteEvent(e);
}

bool HasReadEvent(const Event& e) {
  return e.io_mask & IOMaskRead;
}

bool HasWriteEvent(const Event& e) {
  return e.io_mask & IOMaskWrite;
}

bool HasRWEvent(const Event& e) {
  return HasReadEvent(e) || HasWriteEvent(e);
}

#endif  // SRC_REACTOR_EVENT_H
