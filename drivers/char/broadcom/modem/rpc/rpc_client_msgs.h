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
*   software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

#ifndef _BCM_CLIENT_MSGS_H
#define _BCM_CLIENT_MSGS_H

int rpc_is_registered_msg(UInt16 dscm, UInt8 clientId);
Boolean rpc_register_client_msgs(UInt8 clientId, UInt16 *tbl,
					UInt16 table_size);
Boolean rpc_reset_client_msgs(UInt8 clientId);


#endif /* _BCM_CLIENT_MSGS_H */
