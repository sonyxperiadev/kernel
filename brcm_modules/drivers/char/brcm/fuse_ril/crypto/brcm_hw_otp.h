/*****************************************************************************
*  Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
#ifndef BRCM_HW_OTP_H_
#define BRCM_HW_OTP_H_

#define HUK_LEN        (128/8)         /* 128 bits */  

#define OTP_LOTP_RDATA_LOTP_RDATA_FAIL             0xE0000000       /* RO Three FAIL bits per row,  */

extern uint32 GetHuk(uint8 *huk);

#endif

