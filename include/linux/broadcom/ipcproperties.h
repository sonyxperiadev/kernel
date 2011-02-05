/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/ipcproperties.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef IPCProperties_CIB_h
#define IPCProperties_CIB_h
/* ============================================================ */
/* IPCProperties.h */
/* The define of the IPC PROPERTIES */
/* ============================================================ */

/* The first Property owned by the AP */
#define IPC_PROPERTY_START_AP  0
/* Add your AP Owned property IDs here in ascending order. These can be read and written by AP, but only read by the C */

/* These Property IDs are resrved for use by RPC */
#define IPC_PROPERTY_START_AP_FOR_RPC IPC_PROPERTY_START_AP
/* Put for RPC Property IDs that are owned by the AP here */

#define IPC_PROPERTY_END_AP_FOR_RPC (IPC_PROPERTY_START_AP_FOR_RPC + 8)

#define IPC_PROPERTY_END_AP    31
#define IPC_PROPERTY_START_CP 	(IPC_PROPERTY_END_AP + 1)
/* Add your CP Owned property IDs here in ascending order. These can be read and written by CP, but only read by the AP */

/* These Property IDs are resrved for use by RPC */
#define IPC_PROPERTY_START_CP_FOR_RPC IPC_PROPERTY_START_CP
/* Put for RPC Property IDs that are owned by the CP here */

#define IPC_PROPERTY_END_CP_FOR_RPC (IPC_PROPERTY_START_CP_FOR_RPC + 8)

#define IPC_PROPERTY_END_CP	  63
#define IPC_NUM_OF_PROPERTIES (IPC_PROPERTY_END_CP + 1)

#endif /* IPCProperties_CIB_h */
