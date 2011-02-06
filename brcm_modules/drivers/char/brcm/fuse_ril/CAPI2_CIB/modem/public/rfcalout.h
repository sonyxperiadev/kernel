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
// Description: This file contains type definitions and constants for 
//				 factory calibration task.

// $RCSfile: rfcalout.h $
// $Revision: 1.4 $
// $Date: 2000/04/04 15:04:08 $
// $Author: lubin $
//
//******************************** History *************************************
//
// $Log: rfcalout.h $
// Revision 1.4  2000/04/04 15:04:08  lubin
// Used a memory pool to save the script file. Printing result is done in 
// rfcalout task now to avoid speed problem.
// Revision 1.3  2000/02/16 21:50:19Z  lubin
// Modified code for debuging L1 in calibration mode.
// Revision 1.2  1999/12/23 18:07:57Z  awong
// Revision 1.1  1999/12/23 09:26:39  awong
// Initial revision
//
//******************************************************************************

#ifndef _CAL_RFCALOUT_H_
#define _CAL_RFCALOUT_H_

#include "consts.h"
#include "mobcom_types.h"
#include "rfcal.h"

typedef enum
{
	MAIN_PORT,
	AUX_PORT
} RfcalPort_t;

typedef enum
{
	RFCALOUT_DUMP_HEX,
	RFCALOUT_PRINT,
	RFCALOUT_PRINT_RESULT
} RFCALOUTMSG_Name_t;

typedef struct
{
	 RFCALOUTMSG_Name_t name;	 
	 RfcalPort_t port_num;
	 UInt8 *msg_p;
	 UInt16 size;
	 UInt16 arg3;
} RFCALOUTMsg_t;

void RFCALOUT_HexDump( 
	RfcalPort_t port_id,
	UInt8 *buf, 
	UInt16 size 
	);

void RFCALOUT_PostMsg(
	RFCALOUTMsg_t *rfcalout_task_msg
	);

void RFCALOUT_Run( void );

void RFCALOUT_Init( void );

#endif

