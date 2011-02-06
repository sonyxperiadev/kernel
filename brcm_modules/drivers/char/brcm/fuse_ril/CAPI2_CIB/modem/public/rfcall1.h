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
// Description:  This file contains type definitions and constants for 
//				 factory calibration task.
//
// $RCSfile: rfcall1.h $
// $Revision: 1.3 $
// $Date: 2000/02/16 13:50:28 $
// $Author: lubin $
//
//******************************** History *************************************
//
// $Log: rfcall1.h $
// Revision 1.3  2000/02/16 13:50:28  lubin
// Modified code for debuging L1 in calibration mode.
// Revision 1.2  2000/01/27 12:03:19Z  awong
// Add some new messages
// Revision 1.1  1999/12/23 09:26:41  awong
// Initial revision
//
//******************************************************************************

#ifndef _RFCALL1_H_
#define _RFCALL1_H_

#include "consts.h"
#include "mobcom_types.h"

typedef enum
{
	 RFCALL1MSG_FRAME_INT,
	 RFCALL1MSG_RX_FULL,
	 RFCALL1MSG_TX_FREE,
	 RFCALL1MSG_READ_STATUS,
	 RFCALL1MSG_OAK_REG,
	 RFCALL1MSG_STATUS_SYNC
} RFCALL1MSG_Name_t;

typedef struct
{
	 RFCALL1MSG_Name_t name;	 
	 UInt16 arg0;
	 UInt16 arg1;
	 UInt16 arg2;
	 UInt16 arg3;
	 UInt16 arg4;
	 UInt8 *arg5;
} RFCALL1Msg_t;

void RFCALL1_PostMsg(
	RFCALL1Msg_t *rfcall1_task_msg
	);

void RFCALL1_Run( void );

void RFCALL1_Init( void );

#endif

