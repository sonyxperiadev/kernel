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
/**
*
*   @file   rpc_stubs.c
*
*   @brief  This file includes stubs of functions from the Nucleus AP build
*           required for RPC.
*
*
****************************************************************************/


#include "mobcom_types.h"

#define MAX_CLIENT_NUM              30		
#define AP_CLIENT_ID_START          64
#define AP_CLIENT_ID_END            99
#define	FIRST_USER_CLIENT_ID        AP_CLIENT_ID_START
#define	LAST_USER_CLIENT_ID         AP_CLIENT_ID_END

static unsigned char gClientIdIndex = 0;
unsigned char gSysClientIDs[255]={0};

// ******************************************************************************
/**	
*   Assign unique client id
*
*	@return 		    unique client ID.
*
*   @note Copy of function in taskmsgs_util.c
*
********************************************************************************/
unsigned char SYS_GenClientID(void)
{
	unsigned char clientID;
	
	clientID = FIRST_USER_CLIENT_ID + gClientIdIndex;

	gSysClientIDs[clientID] = 1;

	gClientIdIndex++;

	return clientID;
}

// ******************************************************************************
/**	
*   Check for registered client ID
*
*	@return 		    TRUE if registered client ID, FALSE otherwise
*
*   @note Copy of function in taskmsgs_util.c
*
********************************************************************************/
Boolean SYS_IsRegisteredClientID(UInt8 clientID)
{
	//Log_DebugPrintf(LOGID_MISC, "SYS_IsRegisteredClientID clientID=%d gClientIdIndex=%d gClientIdIndex2=%d", clientID, gSysClientIDs[clientID], gSysClientIDs[clientID+1]);
	return (gSysClientIDs[clientID] > 0)?TRUE: FALSE;
}
