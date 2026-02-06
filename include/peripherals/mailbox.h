#ifndef _P_MAILBOX_H
#define _P_MAILBOX_H

#include "peripherals/base.h"

#define MAILBOX_BASE    (PBASE + 0x0000B880)

#define MAILBOX_READ    (MAILBOX_BASE + 0x00)
#define MAILBOX_STATUS  (MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE   (MAILBOX_BASE + 0x20)

#define MAILBOX_EMPTY   0x40000000u
#define MAILBOX_FULL    0x80000000u

#endif  /* _P_MAILBOX_H */
