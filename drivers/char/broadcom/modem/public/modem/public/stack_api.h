/****************************************************************************
*
*     Copyright (c) 2005-2008 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/

#ifndef _STACK_API_H_
#define _STACK_API_H_

#include "mobcom_types.h"
#include "ms.h"
#include "dsp_feature_def.h"

#ifdef DSP_COMMS_INCLUDED
// AT_MTEST_LOCK_ARFCN (30)
Boolean
STACKAPI_LockArfcn( Boolean, UInt16 );

// AT_MTEST_UNLOCK_ARFCN (31)
void
STACKAPI_UnLockArfcn( void );
#endif

#if defined(UMTS) && defined(DEVELOPMENT_ONLY)
// AT_MTEST_TX_PWR (43)
int
STACKAPI_Get_Adjust_Tx_Power( void );

void
STACKAPI_Set_Adjust_Tx_Power( int );

// AT_MTEST_CMU_TEST (37)
Boolean
STACKAPI_Get_Is_Cmu_Test( void );

void
STACKAPI_Set_Is_Cmu_Test( Boolean );
#endif

#if defined(DEVELOPMENT_ONLY)
// AT_MTEST_SCELL_RXQUAL_ADJ (57)
void
STACKAPI_Set_Scell_Rxqual_Adj( Boolean );

// AT_MTEST_SCELL_RXLEV_ADJ (58)
void
STACKAPI_Set_Scell_Rxlev_Adj( Boolean );
#endif

// AT_MTEST_MS_AB (61)
#ifdef DSP_COMMS_INCLUDED
void
STACKAPI_EnableMSAB(Boolean);
#endif

// AT_MTEST_WCDMA_SLEEP (62)
#ifdef UMTS
void
STACKAPI_SetFwSleepEnable( BOOL8 );
#endif

// AT_MTEST_3G_RESEL_OPT_THRESHOLDS (102)
#ifdef UMTS
void
STACKAPI_Get_3G_Resel_Opt_Thresholds(char*);

void
STACKAPI_Set_3G_Resel_Opt_Thresholds(int, int, int);
#endif

// AT_MTEST_3G_RESEL_OPT_THRESHOLDS2 (103)
#ifdef UMTS
void
STACKAPI_Get_3G_Resel_Opt2_Thresholds(char*);
void
STACKAPI_Set_3G_Resel_Opt2_Thresholds(int, int, int, int);
#endif

#ifdef DSP_COMMS_INCLUDED
#ifdef RFIC_REGISTER_READ

void STACKAPI_ATRFICRegisterReadArray(
	UInt8 chan, 
	char *buf,
	UInt16	*parm
	);

void STACKAPI_StartRFICDumpDirRegs(UInt8 chan,char *buf,UInt16 *parm);

void STACKAPI_StartRFICDumpIndRegs(UInt8 chan,char *buf,UInt16 *parm);


void STACKAPI_ATRFICRegisterWrite(
	UInt8 chan, 
	char *buf,
	UInt16	*parm
	);
#endif
#endif

#endif
