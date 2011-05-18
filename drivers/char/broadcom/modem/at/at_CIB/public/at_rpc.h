//***************************************************************************
/****************************************************************************
*
*     Copyright (c) 2010 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue (Buildings 1-8)
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   at_rpc.h
*
*   @brief  This file contains the function prototypes for the ATC RPC.
*
****************************************************************************/

#ifndef _AT_RPC_H_
#define _AT_RPC_H_

typedef struct
{
	UInt8			channel;		///< AP Channel Info
	Boolean 			unsolicited;		///< unsolicited flag
}AtRegisterInfo_t;


void AT_InitRpc(void);
Result_t AT_SendRpcMsg(UInt32 msgId, void *val);
Result_t AT_PostRpcCommandStr(UInt8 chan, const UInt8* atCmdStr);
Result_t AT_RegisterCPTerminal(UInt8 chan, Boolean unsolicited);


#endif //_AT_RPC_H_
