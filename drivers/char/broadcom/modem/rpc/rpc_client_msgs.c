/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   rpc_client_msgs.c
*
*   @brief  This file builds lookup table for client registered messages.
*
****************************************************************************/
#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/delay.h>	/* udelay */
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/poll.h>

#include <linux/unistd.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <linux/kthread.h>

#include "mobcom_types.h"
#include "rpc_global.h"

#include "resultcode.h"
#include "taskmsgs.h"
#include "rpc_client_msgs.h"

static UInt32 **tableBase = NULL;

#undef _DBG_
#define _DBG_(a)

static Boolean rpc_build_lookup_msgs(UInt16 *tbl, UInt32 table_size,
			UInt32 mask)
{
	Boolean bRetVal = TRUE;
	if (tableBase == NULL) {
		tableBase = (UInt32 **) capi2_malloc(256 * sizeof(UInt32));

		if (tableBase) {
			int i;
			for (i = 0; i < 256; i++)
				tableBase[i] = 0;
		}
	}

	if (tableBase) {
		int i;
		for (i = 0; i < (int)(table_size); i++) {
			unsigned short id = (unsigned short)tbl[i];
			unsigned short id1 =
			    ((unsigned short)((id & (unsigned short)0xff00) >>
					      8));
			unsigned short id2 = id & ((unsigned short)0x00ff);
			UInt32 *tableBase2 = NULL;
			if (tableBase[id1] == 0) {
				int j;
				tableBase[id1] =
				    (UInt32 *) capi2_malloc(256 *
							    sizeof(UInt32));
				tableBase2 = tableBase[id1];
				if (tableBase2) {
					for (j = 0; j < 256; j++)
						tableBase2[j] = 0xFFFF;
				}
			}

			tableBase2 = tableBase[id1];

			if (tableBase2 == 0) {
				bRetVal = FALSE;
			} else {
				if (tableBase2[id2] == 0xFFFF)
				    tableBase2[id2] = 0;

				tableBase2[id2] |= mask;

				/*_DBG_(RPC_TRACE("rpc_build_lookup = 0x%x, 0x%x, 0x%x\r\n", id, id1, id2));*/
			}
		}
	} else {
		bRetVal = FALSE;
	}
	return bRetVal;
}

Boolean rpc_reset_client_msgs(UInt8 clientId)
{
	int i, k;
	UInt32 mask = 1;

	if (!tableBase)
		return FALSE;


	if (clientId >= 60 && clientId < 92) {
		mask = mask << (clientId - 60);
		_DBG_(RPC_TRACE
		      ("rpc_register_client_msgs cid=%d mask=%x \r\n",
		       clientId, mask));

		for (i = 0; i < 256; i++) {
			if (tableBase[i] != 0) {
				UInt32 *tableBase2 = tableBase[i];
				for (k = 0; k < 256; k++) {
					if (tableBase2[k] != 0xFFFF &&
							tableBase2[k] & mask)
						tableBase2[k] &= ~mask;
				}
			}
		}
	} else
		return FALSE;

	return TRUE;
}

Boolean rpc_register_client_msgs(UInt8 clientId, UInt16 *tbl,
					UInt32 table_size)
{
	UInt32 mask = 1;

	if (clientId >= 60 && clientId < 92) {
		mask = mask << (clientId - 60);
		_DBG_(RPC_TRACE
		      ("rpc_register_client_msgs cid=%d sz=%d mask=%x \r\n",
		       clientId, table_size, mask));

/*		printk("rpc_register_client_msgs cid=%d sz=%d mask=%x \r\n", */
/*		       (int)clientId, (int)table_size, (int)mask); */

		return rpc_build_lookup_msgs(tbl, table_size, mask);
	}
	return FALSE;
}

int rpc_is_registered_msg(UInt16 dscm, UInt8 clientId)
{
	UInt32 nodeVal = 0;
	unsigned short id = (unsigned short)dscm;
	unsigned short id1 =
	    ((unsigned short)((id & (unsigned short)(((unsigned short)0xff00)))
			      >> 8));
	unsigned short id2 = id & ((unsigned short)0x00ff);
	UInt32 *tableBase2 = NULL;

	/* Make sure the table is built up */
	if (!tableBase)
		return -1;

	tableBase2 = tableBase[id1];

	if (tableBase2 == NULL) {
		/* message not handled in RPC */
		_DBG_(RPC_TRACE
		      ("rpc_is_registered_msg no RPC entry (tableBase2 is NULL) = 0x%x, 0x%x, 0x%x\r\n",
		       id, id1, id2));
		return -1;
	} else if (tableBase2[id2] != 0xFFFF) {
		nodeVal = tableBase2[id2];
	} else {
		/* message not handled in RPC */
		_DBG_(RPC_TRACE
		      ("rpc_is_registered_msg no RPC entry = 0x%x, 0x%x, 0x%x\r\n",
		       id, id1, id2));
		return -1;
	}

	if (clientId >= 60 && clientId < 92) {
		UInt32 mask = 1;
		mask = mask << (clientId - 60);
		_DBG_(RPC_TRACE
		    ("rpc_is_registered_msg cid=%d msgId=%x mask=%x val=%x ret=%d\r\n",
		    clientId, dscm, mask, nodeVal, (nodeVal & mask) ? 1 : 0));
		return (nodeVal & mask) ? 1 : 0;
	}

	return 0;
}



