#ifndef _P_POWER_H
#define _P_POWER_H

#include "peripherals/base.h"

#define PM_PASSWORD 0x5a000000u
#define PM_RSTC     (PBASE + 0x0010001C)
#define PM_WDOG     (PBASE + 0x00100024)

#endif  /* _P_POWER_H */
