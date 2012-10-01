/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

#ifndef __RPC_WAKELOCK_H
#define __RPC_WAKELOCK_H

#define ENABLE_RPC_WAKELOCK

#if defined(CONFIG_HAS_WAKELOCK) && defined(ENABLE_RPC_WAKELOCK)

void rpc_wake_lock_init(void);
void rpc_wake_lock_add(UInt32 elem);
void rpc_wake_lock_remove(UInt32 elem);
void rpc_wake_lock_reset(void);

#else
static inline void rpc_wake_lock_init(void) {};
static inline void rpc_wake_lock_add(UInt32 elem) {};
static inline void rpc_wake_lock_remove(UInt32 elem) {};
static inline void rpc_wake_lock_reset(void) {};

#endif

#endif
