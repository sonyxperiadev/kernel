/*
 * Copyright (c) 2012-2013 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**=========================================================================

  \file  macTrace.h

  \brief definition for trace related APIs

  \author Sunit Bhatia

   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

   Qualcomm Confidential and Proprietary.

  ========================================================================*/



#ifndef __MAC_TRACE_H
#define __MAC_TRACE_H

#include "aniGlobal.h"


eHalStatus pe_AcquireGlobalLock( tAniSirLim *psPe);
eHalStatus pe_ReleaseGlobalLock( tAniSirLim *psPe);

#ifdef TRACE_RECORD

#define MAC_TRACE_GET_MODULE_ID(data) ((data >> 8) & 0xff)
#define MAC_TRACE_GET_MSG_ID(data)       (data & 0xffff)


#define eLOG_NODROP_MISSED_BEACON_SCENARIO 0
#define eLOG_PROC_DEAUTH_FRAME_SCENARIO 1

void macTraceReset(tpAniSirGlobal pMac);
void macTrace(tpAniSirGlobal pMac,  tANI_U8 code, tANI_U8 session, tANI_U32 data);
void macTraceNew(tpAniSirGlobal pMac,  tANI_U8 module, tANI_U8 code, tANI_U8 session, tANI_U32 data);
tANI_U8* macTraceGetCfgMsgString( tANI_U16 cfgMsg );
tANI_U8* macTraceGetLimMsgString( tANI_U16 limMsg );
tANI_U8* macTraceGetWdaMsgString( tANI_U16 wdaMsg );
tANI_U8* macTraceGetSmeMsgString( tANI_U16 smeMsg );
tANI_U8* macTraceGetModuleString( tANI_U8 moduleId);
tANI_U8* macTraceGetInfoLogString( tANI_U16 infoLog );

tANI_U8* macTraceGetHDDWlanConnState(tANI_U16 connState);

#ifdef WLAN_FEATURE_P2P_DEBUG
tANI_U8* macTraceGetP2PConnState(tANI_U16 connState);
#endif

tANI_U8* macTraceGetNeighbourRoamState(tANI_U16 neighbourRoamState);
tANI_U8* macTraceGetcsrRoamState(tANI_U16 csrRoamState);
tANI_U8* macTraceGetcsrRoamSubState(tANI_U16 csrRoamSubState);
tANI_U8* macTraceGetLimSmeState(tANI_U16 limState);
tANI_U8* macTraceGetLimMlmState(tANI_U16 mlmState);
tANI_U8* macTraceGetTLState(tANI_U16 tlState);

#else
#define macTraceReset(x)
#define macTrace(x, y, z, a)
#define macTraceNew(x, y, z, a, b)
#define macTraceGetCfgMsgString( x ) (tANI_U8*)"TRACE_DIS"
#define macTraceGetLimMsgString( x ) (tANI_U8*)"TRACE_DIS"
#define macTraceGetWdaMsgString( x ) (tANI_U8*)"TRACE_DIS"
#define macTraceGetSmeMsgString( x ) (tANI_U8*)"TRACE_DIS"
#define macTraceGetModuleString( x ) (tANI_U8*)"TRACE_DIS"
#define macTraceGetInfoLogString( x ) (tANI_U8*)"TRACE_DIS"

#define macTraceGetHDDWlanConnState( x ) (tANI_U8*)"TRACE_DIS"

#ifdef WLAN_FEATURE_P2P_DEBUG
#define macTraceGetP2PConnState( x )
#endif

#define macTraceGetNeighbourRoamState(x) (tANI_U8*)"TRACE_DIS"
#define macTraceGetcsrRoamState(x) (tANI_U8*)"TRACE_DIS"
#define macTraceGetcsrRoamSubState(x) (tANI_U8*)"TRACE_DIS"
#define macTraceGetLimSmeState(x) (tANI_U8*)"TRACE_DIS"
#define macTraceGetLimMlmState(x) (tANI_U8*)"TRACE_DIS"
#define macTraceGetTLState(x) (tANI_U8*)"TRACE_DIS"
#endif

#endif

