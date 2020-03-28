/**
 */

#ifndef SRC_REACTOR_EVENT_TYPE_H
#define SRC_REACTOR_EVENT_TYPE_H

typedef int IOMask;
#define  IOMaskNone   0x0
#define  IOMaskRead   (1<<0)
#define  IOMaskWrite  (1<<1)

#endif  // SRC_REACTOR_EVENT_TYPE_H
