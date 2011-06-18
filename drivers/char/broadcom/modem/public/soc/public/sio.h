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
// Description:  This include file specifies the interface to the serial
//				 I/O ports.
//
// $RCSfile: sio.h $
// $Revision:$
//
//******************************************************************************


#ifndef _DEV_SIO_H_
#define _DEV_SIO_H_

#include "uart_drv.h"

//******************************************************************************
//		Definition
//******************************************************************************
#define SIORXBUF_SIZE		3072 //2048  // 1024
#define SIOTXBUF_SIZE		4096*2 // needed for Bluetooth
#define HIGHTHRESHOLD		75 //60
#define LOWTHRESHOLD		40

typedef struct
{
	union
	{
		struct
		{
			BitField	rx_buf_event: 2;	// See SIOBufStatus_t
			BitField	rx_buf_overflow: 1;	// 1-Overflow once
			BitField	tx_buf_event: 2;	// See SIOBufStatus_t
			BitField	tx_buf_overflow: 1;	// 1-Overflow once
			BitField	dte_line_off: 1;	// 0-Line On, 1-Line Off
			BitField	dtr: 2;				// See SIODTRStatus_t
		} ev;
		UInt16 b;
	}u;
} SIOEvent_t;

typedef enum
{
	SIOBUFSTATUS_NORMAL			= 0x0000,
	SIOBUFSTATUS_LOW			= 0x0001,
	SIOBUFSTATUS_HIGH			= 0x0002
} SIOBufStatus_t;

typedef enum
{
	SIODTRSTATUS_NO_CHANGE		= 0x0000,
	SIODTRSTATUS_ASSERTED		= 0x0001,
	SIODTRSTATUS_NOT_ASSERTED	= 0x0002
} SIODTRStatus_t;

const UART_Config_t* UART_GetDefaultCfg(PortId_t portId);
#endif

/*============= End of File: sio.h ==================*/

