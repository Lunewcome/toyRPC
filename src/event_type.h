/**
 */

#ifndef SRC_EVENT_TYPE_H_
#define SRC_EVENT_TYPE_H_

typedef int EventType;
#define IO 0x1
#define None 0x0

typedef int IOMask;
#define NoEvent 0
#define Read 0x1
#define Write 0x2

#endif  // SRC_EVENT_TYPE_H_
