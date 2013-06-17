/******************************************************************************
*
* Copyright 2009 - 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/
/**
*
*   @file   csl_bt_tap.h
*
*   @brief  This file contains CSL (DSP) API to BT TAP interface
*
****************************************************************************/
#ifndef _CSL_BT_TAP_H_
#define _CSL_BT_TAP_H_

/*---- Include Files ----------------------------------------------------*/
#include "mobcom_types.h"

extern AP_SharedMem_t *vp_shared_mem;

/*---- Function Declarations -----------------------------------------*/
/*********************************************************************/
/**
*
*   CSL_BTTAP_Init initializes  BT TAP output buffer.
*
*
**********************************************************************/
void CSL_BTTAP_Init(void);

/*********************************************************************/
/**
*
*   CSL_BTTAP_Read reads PCM data from shared memory for BT TAP voice record.
*
*   @param    outBuf		(out)	destination buffer
*   @param    outSize		(in)	data size to read
*   @param    readIndex		(in)	index of ping-pong buffer
*   @param    channelSelect	(in)	channel (L or R)
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_BTTAP_Read(UInt8 *outBuf, UInt32 outSize, UInt16 readIndex,
					  UInt16 channelSelect);

#endif /* _CSL_BT_TAP_H_ */
