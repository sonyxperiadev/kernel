/*
 * arch/arm/mach-versatile/include/mach/ipc.h
 *
 * Copyright (c) ARM Limited 2003.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _BCM2708_IPC_H_
#define _BCM2708_IPC_H_

#define IPC_BLOCK_MAGIC			0xCAFEBABE

#define IPC_BLOCK_MAGIC_OFFSET		0x000

#define IPC_BLOCK_INFO_OFFSET		0x200

#define IPC_VC_ARM_INTERRUPT_OFFSET	0x800  /* VC  -> ARM */
#define IPC_ARM_VC_INTERRUPT_OFFSET	0x804  /* ARM -> VC */

//currently we tie the interrupt number to the user slot
#define IPC_BLOCK_MAX_NUM_USERS		32

#define IPC_BLOCK_SIZE			SZ_2M

#define IPC_IRQNUM_NONE		-1

extern void __init bcm2835_get_ipc_base(u32 *ipc_base_phys);

#endif  /* _BCM2708_IPC_H_ */
