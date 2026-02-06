#include "power.h"
#include "peripherals/power.h"
#include "utils.h"

void reboot(unsigned int tick)
{
	put32(PM_RSTC, PM_PASSWORD | 0x20u);
	put32(PM_WDOG, PM_PASSWORD | (tick & 0xFFFFu));
}
