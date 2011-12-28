//*********************************************************************
//
// (c)1999-2011 Broadcom Corporation
//
// Unless you and Broadcom execute a separate written software license agreement governing use of this software,
// this software is licensed to you under the terms of the GNU General Public License version 2,
// available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
//
//*********************************************************************
/**
*
*   @file   csl_bt_tap.c
*
*   @brief  This file contains CSL (DSP) API to BT TAP interface
*
****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "csl_bttap.h"

extern AP_SharedMem_t	*vp_shared_mem;

//*********************************************************************
/**
*
*   CSL_BTTAP_Init initializes  BT TAP output buffer.
*
* 
**********************************************************************/
void CSL_BTTAP_Init(void)
{
    memset(&vp_shared_mem->shared_BTnbdinLR[0][0], 0, (2*640)*sizeof(UInt16)); // clean buffer 

} // CSL_BTTAP_Init


//*********************************************************************
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
UInt32 CSL_BTTAP_Read(UInt8 *outBuf, UInt32 outSize, UInt16 readIndex, UInt16 channelSelect)
{
 UInt8	*buffer;
		
	if(readIndex != 0)
		buffer = (UInt8 *)&vp_shared_mem->shared_BTnbdinLR[0][0];      //Left channel, one of ping-pong buffer
	else
		buffer = (UInt8 *)&vp_shared_mem->shared_BTnbdinLR[0][320];    //Left channel, one of ping-pong buffer

	if(channelSelect == 0)
		buffer += 640*2;		//read from R channel of BT NB buffer

	memcpy(outBuf, buffer, outSize);
	
	// return the number of bytes has been copied
	return outSize;

} // CSL_BTTAP_Read

