/*
 *
 *  Copyright (C) 2011 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __LINUX_IPC_IPC_CHIP_H__
#define __LINUX_IPC_IPC_CHIP_H__

#include <linux/types.h>

struct ipc_chip {
	u32 num_channels;
	u32 irq_base;
	u32 (*gpu_to_host_phys_addr) (u32);
	void (*clear_doorbell_irq) (void);
	void (*ring_doorbell_irq) (void);
	u32 (*read_and_ack_req_atomic) (u32);
	void (*send_req_atomic) (u32, u32);
	u32 (*read_and_ack_req) (u32);
	void (*send_req) (u32, u32);
};

#endif /* __LINUX_IPC_IPC_CHIP_H__ */
