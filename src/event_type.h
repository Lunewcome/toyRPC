/**
 */

#ifndef SRC_EVENT_TYPE_H_
#define SRC_EVENT_TYPE_H_
enum EventType {
  IO,
  None
};

enum IOMask {
  NoEvent = 0,
  Read = 0x1,
  Write = 0x2
};

#endif  // SRC_EVENT_TYPE_H_
