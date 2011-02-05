/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/bcm_otp.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/**
 *
 *   @file   bcm_otp.h
 *
 *   @brief  Broadcom header for the OTP subsystem.
 *
 ****************************************************************************/

#ifndef __BCM_OTP__
#define __BCM_OTP__

#define MAX_ROWS 96

/* Register offsets from Base
 * Base address to be determined at run-time */
#define OTP_PC         0x0
#define OTP_WDATA      0x4
#define OTP_ADDR       0x8
#define OTP_CTRL       0xC
#define OTP_STATUS     0x10
#define OTP_RDATA      0x14
#define OTP_COUNTER    0x18
#define OTP_BADADDR    0x1C
#define OTP_SBD        0x38
#define OTP_JTAG       0x3C

/* Offsets within registers */

/* OTP Programming control  */
#define LOTP_CLK_DIV_EN         (1 << 22)
#define LOTP_CLK_DIV_RATIO      0x00300000
#define OTP_CPU_MODE_EN         (1 << 15)
#define RESET                    (1 << 14)
#define OTP_ENABLE              (1 << 12)
#define OTP_PROG_UNSUCCESS      (1 << 3)
#define OTP_PROG_SUCCESS        (1 << 2)
#define OTP_UNPROG              (1 << 1)
#define OTP_CRC_DONE            (1 << 0)

/* OTP Write data register*/
#define WR_FAIL_BIT_MASK         0xE0000000
#define WR_FAIL_BIT_SHIFT        29
#define WR_ECC_MASK              0x1F000000
#define WR_ECC_SHIFT             24
#define WR_DATA_MASK             0x00FFFFFF

/* OTP Address register*/
#define ADDR_MASK                0x1FE0
#define ADDR_SHIFT               5
#define BITSEL_MASK              0x1F

/* OTP CTRL register*/
#define EN_ECC                   (1 << 25)
#define EN_WREN                  (1 << 24)
#define BURST_STAT               (1 << 21)
#define ACCESS_MODE_SHIFT        19
#define ACCESS_MODE_MASK         0x00180000
#define CONTINUE_ON_FAIL         (1 << 17)
#define PROG_SEL                 (1 << 4)
#define OTP_CMD_MASK             0xF

/* OTP STATUS register*/
#define ECC_COR                  (1 << 8)
#define INV_PROG_REQ             (1 << 5)
#define OTP_FAIL                 (1 << 4)
#define OTP_BUSY                 (1 << 3)
#define OTP_DOUT                 (1 << 2)
#define DATA_RDY                 (1 << 1)
#define CMD_DONE                 (1 << 0)

/* Commands  */
#define OTP_CMD_READ             0x0
#define OTP_CMD_BITINTCLK        0x1
#define OTP_CMD_PRGWORD          0x2
#define OTP_CMD_VERIFY           0x3
#define OTP_CMD_INIT             0x4
#define OTP_CMD_SET              0x5
#define OTP_CMD_RESET            0x6
#define OTP_CMD_OCST             0x7
#define OTP_CMD_PRGLOCKBITS      0x8
#define OTP_CMD_PRESCRTEST       0x9
#define OTP_CMD_BITEXTCLK        0xA
#define OTP_CMD_WORDEXTCLK       0xB

/* OTP Read data register*/
#define RD_FAIL_BIT_MASK         0xE0000000
#define RD_FAIL_BIT_SHIFT        29
#define RD_ECC_MASK              0x1F000000
#define RD_ECC_SHIFT             24
#define RD_DATA_MASK             0x00FFFFFF

/* OTP Counter */
#define ECC_CTHRESH_MASK         0xFFFF0000
#define ECC_CTHRESH_SHIFT        16
#define ECC_CCNT                 0xFFFF

/* OTP Bad addr */
#define OTP_BAD_ADDR             0xFF

/* Extern function prototypes*/
extern int otp_read_word(int row, int ecc, u32 *rd);

#endif /* __BCM_OTP__ */
