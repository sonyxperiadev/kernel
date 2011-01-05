#ifndef __MACH_ISLAND_H
#define __MACH_ISLAND_H

#include <linux/init.h>
#include <linux/platform_device.h>

extern struct platform_device island_ipc_device;
void __init island_map_io(void);

#endif /* __MACH_ISLAND_H */
