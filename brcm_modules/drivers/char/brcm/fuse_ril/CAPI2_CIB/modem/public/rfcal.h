/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
//
// Description:  This file ...
//
// $RCSfile: rfcal.h $
// $Revision: 1.2 $
// $Date: 2000/04/04 15:04:08 $
// $Author: lubin $
//
//******************************** History *************************************
//
// $Log: rfcal.h $
// Revision 1.2  2000/04/04 15:04:08  lubin
// Used a memory pool to save the script file. Printing result is done in 
// rfcalout task now to avoid speed problem.
// Revision 1.1  1999/12/23 17:26:42Z  awong
// Initial revision
//
//******************************************************************************

#ifndef _CAL_RFCAL_H_
#define _CAL_RFCAL_H_

#include "mobcom_types.h"
#include "consts.h"

#define CMDLINES_BUF_SIZE		50

typedef 	UInt8		Cmd_t[32];

typedef		Int32		Parm_t[512];

typedef struct
{
	Cmd_t 	cmd;
	UInt8	num_parm;
	Parm_t	parm;
}CmdLine_t;

#define   CAL_MODE 	0x02
#define   RFCAL_MEM_SIZE		CMDLINES_BUF_SIZE * sizeof(CmdLine_t) + 4

void RFCAL_Run( void );
void RFCAL_Init( void );
void RFCAL_PrintResult(
	UInt8	ave_rxlev
	);

void RFCAL_InitLocHeap( 				// Initialize a local heap for storing script file
	UInt8 *local_heap
	);

#if defined(_BCM2124_) || defined(_BCM2152_) || defined(_BCM213x1_) || defined(_BCM2153_) || defined (_BCM21551_) || defined(_ATHENA_)
Boolean RFCAL_SaveCalDataToFlash(void *flash_addr, UInt32 size, void *cal_data);
#else
Boolean RFCAL_SaveCalDataToFlash( void* cal_data, void* flash_addr, UInt32 size );
#endif

void RFCAL_PutString(UInt8 *b);

void RFCAL_SendDataToComPort( void *data, UInt32 size );

#endif

