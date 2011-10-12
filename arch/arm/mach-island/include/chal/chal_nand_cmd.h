/*****************************************************************************
* Copyright (c) 2007 - 2010 Broadcom Corporation.  All rights reserved.
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.  IF YOU HAVE NO
* AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
* WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
* THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use
*    all reasonable efforts to protect the confidentiality thereof, and to
*    use this information only in connection with your use of Broadcom
*    integrated circuit products.
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*****************************************************************************/
/**
 * @file    chal_nand_cmd.h
 * @brief   Generic NAND commands.
 */


#ifndef __CHAL_NAND_CMD_H__
#define __CHAL_NAND_CMD_H__

/* NAND commands */
#define NAND_CMD_READ_1ST        (0x00)
#define NAND_CMD_READ_2ND        (0x30)
#define NAND_CMD_READ_RAND_1ST   (0x05)
#define NAND_CMD_READ_RAND_2ND   (0xE0)
#define NAND_CMD_READCP_2ND      (0x35)
#define NAND_CMD_ID              (0x90)
#define NAND_CMD_RESET           (0xFF)

#define NAND_CMD_READ_PARAM      (0xEC)

#define NAND_CMD_STATUS          (0x70)
#define NAND_CMD_EDC_STATUS      (0x7B)
#define NAND_CMD_STATUS_1        (0xF1)
#define NAND_CMD_STATUS_2        (0xF2)

#define NAND_CMD_BERASE_1ST      (0x60)
#define NAND_CMD_BERASE_2ND      (0xD0)

#define NAND_CMD_PROG_1ST        (0x80)
#define NAND_CMD_PROG_2ND        (0x10)
#define NAND_CMD_PROG_RAND       (0x85)

/* NAND status result masks */
#define NAND_STATUS_FAIL         (0x01)   /* command failed */
#define NAND_STATUS_READY        (0x40)   /* chip is ready */
#define NAND_STATUS_NPRO         (0x80)   /* not protected */

#endif

