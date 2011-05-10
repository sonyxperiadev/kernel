#ifndef __ISLAND_H
#define __ISLAND_H

#include <linux/init.h>
#include <linux/platform_device.h>

extern struct platform_device island_ipc_device;
void __init island_init_machine(void);
void __init island_map_io(void);
void __init island_add_common_devices(void);

#endif /* __ISLAND_H */
