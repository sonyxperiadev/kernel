/*
 *
 *  Copyright (C) 2011 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __LINUX_IPC_IPC_H__
#define __LINUX_IPC_IPC_H__

extern int ipc_notify_vc_event(int ipc_id);
extern void *ipc_bus_to_virt(uint32_t bus_addr);

#endif	/* __LINUX_IPC_IPC_H__ */
