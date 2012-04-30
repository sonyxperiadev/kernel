/*
 * pcicfg.h: PCI configuration constants and structures.
 *
 * Copyright (C) 1999-2011, Broadcom Corporation
 * 
 *         Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 * $Id: pcicfg.h 277737 2011-08-16 17:54:59Z $
 */

#ifndef	_h_pcicfg_
#define	_h_pcicfg_

/* A structure for the config registers is nice, but in most
 * systems the config space is not memory mapped, so we need
 * field offsetts. :-(
 */
#define	PCI_CFG_VID		0
#define	PCI_CFG_CMD		4
#define	PCI_CFG_REV		8
#define	PCI_CFG_BAR0		0x10
#define	PCI_CFG_BAR1		0x14
#define	PCI_BAR0_WIN		0x80	/* backplane addres space accessed by BAR0 */
#define	PCI_INT_STATUS		0x90	/* PCI and other cores interrupts */
#define	PCI_INT_MASK		0x94	/* mask of PCI and other cores interrupts */
/* PCIE Enhanced CAPABILITY DEFINES */
#define PCIE_EXTCFG_OFFSET	0x100
#define	PCI_SPROM_CONTROL	0x88	/* sprom property control */
#define	PCI_BAR1_CONTROL	0x8c	/* BAR1 region burst control */
#define PCI_TO_SB_MB		0x98	/* signal backplane interrupts */
#define PCI_BACKPLANE_ADDR	0xa0	/* address an arbitrary location on the system backplane */
#define PCI_BACKPLANE_DATA	0xa4	/* data at the location specified by above address */
#define	PCI_CLK_CTL_ST		0xa8	/* pci config space clock control/status (>=rev14) */
#define	PCI_BAR0_WIN2		0xac	/* backplane addres space accessed by second 4KB of BAR0 */
#define	PCI_GPIO_IN		0xb0	/* pci config space gpio input (>=rev3) */
#define	PCI_GPIO_OUT		0xb4	/* pci config space gpio output (>=rev3) */
#define	PCI_GPIO_OUTEN		0xb8	/* pci config space gpio output enable (>=rev3) */

#define	PCI_BAR0_SHADOW_OFFSET	(2 * 1024)	/* bar0 + 2K accesses sprom shadow (in pci core) */
#define	PCI_BAR0_SPROM_OFFSET	(4 * 1024)	/* bar0 + 4K accesses external sprom */
#define	PCI_BAR0_PCIREGS_OFFSET	(6 * 1024)	/* bar0 + 6K accesses pci core registers */
#define	PCI_BAR0_PCISBR_OFFSET	(4 * 1024)	/* pci core SB registers are at the end of the
						 * 8KB window, so their address is the "regular"
						 * address plus 4K
						 */

#define PCI_BAR0_WINSZ		(16 * 1024)	/* bar0 window size Match with corerev 13 */

/* On pci corerev >= 13 and all pcie, the bar0 is now 16KB and it maps: */
#define	PCI_16KB0_PCIREGS_OFFSET (8 * 1024)	/* bar0 + 8K accesses pci/pcie core registers */
#define	PCI_16KB0_CCREGS_OFFSET	(12 * 1024)	/* bar0 + 12K accesses chipc core registers */
#define PCI_16KBB0_WINSZ	(16 * 1024)	/* bar0 window size */

/* On AI chips we have a second window to map DMP regs are mapped: */
#define	PCI_16KB0_WIN2_OFFSET	(4 * 1024)	/* bar0 + 4K is "Window 2" */


/* PCI_SPROM_CONTROL */
#define SPROM_SZ_MSK		0x02	/* SPROM Size Mask */
#define SPROM_LOCKED		0x08	/* SPROM Locked */
#define	SPROM_BLANK		0x04	/* indicating a blank SPROM */
#define SPROM_WRITEEN		0x10	/* SPROM write enable */
#define SPROM_BOOTROM_WE	0x20	/* external bootrom write enable */
#define SPROM_BACKPLANE_EN	0x40	/* Enable indirect backplane access */
#define SPROM_OTPIN_USE		0x80	/* device OTP In use */

#endif	/* _h_pcicfg_ */
