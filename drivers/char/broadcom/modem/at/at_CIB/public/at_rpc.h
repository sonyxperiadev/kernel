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

typedef struct
{
	Boolean 		turn_on;		///< on/off
	UInt8		tone_id;			///< tone ID
	UInt32		duration;		///< duration
}AtToneInfo_t;

typedef struct 
{
	UInt8			channel;		///< CP Channel Info
	UInt8 			contextId;		///< context Id
	UInt8			simID;			///< SIM Id
}AtDUNInfo_t;

typedef struct 
{
	UInt8			channel;		///< AP Channel Info
	Boolean 			flight_mode;		///< unsolicited flag
}AtFlightMode_t;

void AT_InitRpc(void);
Result_t AT_SendRpcMsg(UInt32 tid, UInt8 chan, UInt32 msgId, void *val);
Result_t AT_PostRpcCommandStr(UInt8 chan, const UInt8* atCmdStr);
Result_t AT_RegisterCPTerminal(UInt8 chan, Boolean unsolicited);

Result_t AT_RPCTone(UInt8 chan, Boolean turn_on, UInt8 tone, UInt32 duration);
Result_t AT_RPCAudioPath(UInt8 chan, Boolean turn_on);
Result_t AT_RPCMicMute(Boolean mute_on);
Result_t AT_RPCSpeakerMute(Boolean mute_on);
Result_t AT_RPCSetSpeakerGain(UInt32 val);
UInt32 AT_RPCGetSpeakerGain(void);
Result_t AT_RPCSetMicGain(UInt32 val);
UInt32 AT_RPCGetMicGain(void);

Result_t AT_RPC_DUNConnect(UInt8 chan, UInt8 contextId);
Result_t AT_RPC_DUNDisconnect(UInt8 chan, UInt8 contextId);
Result_t AT_RPC_APCommandStr(UInt8 chan, const UInt8* atCmdStr);
Result_t AT_RPC_FlightMode(UInt8 chan, Boolean flight_mode);
Result_t  AT_RPC_RegisterAPTerminal(UInt8 chan, UInt8 clientID);
#endif //_AT_RPC_H_
