#ifndef __ISLAND_H
#define __ISLAND_H

#include <linux/init.h>
#include <linux/platform_device.h>

extern struct platform_device island_ipc_device;

void __init island_map_io(void);
void __init board_pmu_init(void);

#endif /* __ISLAND_H */
