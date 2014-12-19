/*
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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
 * */
/**=========================================================================
  
  \file  rrmApi.h
  
  \brief RRM APIs
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/* $Header$ */

#ifndef __RRM_API_H__
#define __RRM_API_H__

#define RRM_MIN_TX_PWR_CAP    13
#define RRM_MAX_TX_PWR_CAP    19

#define RRM_BCN_RPT_NO_BSS_INFO    0
#define RRM_BCN_RPT_MIN_RPT        1

tANI_U8 rrmGetMinOfMaxTxPower(tPowerdBm regMax, tPowerdBm apTxPower);

extern tSirRetStatus rrmInitialize(tpAniSirGlobal pMac);

extern tSirRetStatus rrmCleanup(tpAniSirGlobal pMac);


extern tSirRetStatus rrmProcessLinkMeasurementRequest( tpAniSirGlobal pMac, 
                                  tANI_U8 *pRxPacketInfo,
                                  tDot11fLinkMeasurementRequest *pLinkReq,
                                  tpPESession pSessionEntry );

extern tSirRetStatus rrmProcessRadioMeasurementRequest( tpAniSirGlobal pMac, 
                                  tSirMacAddr peer,
                                  tDot11fRadioMeasurementRequest *pRRMReq,
                                  tpPESession pSessionEntry );

extern tSirRetStatus rrmProcessNeighborReportResponse( tpAniSirGlobal pMac, 
                                  tDot11fNeighborReportResponse *pNeighborRep,
                                  tpPESession pSessionEntry );

extern void rrmProcessMessage(tpAniSirGlobal pMac, 
                                  tpSirMsgQ pMsg);

extern tSirRetStatus rrmSendSetMaxTxPowerReq ( tpAniSirGlobal pMac, 
                                  tPowerdBm txPower, 
                                  tpPESession pSessionEntry );

extern tPowerdBm rrmGetMgmtTxPower ( tpAniSirGlobal pMac, 
                                  tpPESession pSessionEntry );

extern void rrmCacheMgmtTxPower ( tpAniSirGlobal pMac, 
                                  tPowerdBm txPower, 
                                  tpPESession pSessionEntry );

extern tpRRMCaps rrmGetCapabilities ( tpAniSirGlobal pMac,
                                  tpPESession pSessionEntry );

extern void rrmUpdateConfig ( tpAniSirGlobal pMac,
                                  tpPESession pSessionEntry );

extern void rrmGetStartTSF ( tpAniSirGlobal pMac, 
                                  tANI_U32 *pStartTSF );

extern void rrmUpdateStartTSF ( tpAniSirGlobal pMac, 
                                  tANI_U32 startTSF[2] );

extern tSirRetStatus rrmSetMaxTxPowerRsp ( tpAniSirGlobal pMac, 
                                  tpSirMsgQ limMsgQ );

extern tSirRetStatus
rrmProcessNeighborReportReq( tpAniSirGlobal pMac,
                            tpSirNeighborReportReqInd pNeighborReq );
extern tSirRetStatus
rrmProcessBeaconReportXmit( tpAniSirGlobal pMac,
                            tpSirBeaconReportXmitInd pBcnReport);
#endif
