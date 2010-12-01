#ifndef _BCM2708_SMI_REGS_H
#define _BCM2708_SMI_REGS_H

#define BCM2708_SMI_DEBUG

#ifdef BCM2708_SMI_DEBUG
#define refmt(fmt) "[%s]: " fmt, __func__
#define bcm2708_smi_print(fmt, ...) \
	printk(KERN_ERR refmt(fmt), ##__VA_ARGS__)
#else
#define bcm2708_smi_print(fmt, ...)
#endif


/* Secondary Memory Interface */
#define SMICS	0x00
#define SMIL	0x04
#define SMIA	0x08
#define SMID	0x0C
#define SMIDSR0	0x10
#define SMIDSW0	0x14
#define SMIDSR1	0x18
#define SMIDSW1	0x1C
#define SMIDSR2	0x20
#define SMIDSW2	0x24
#define SMIDSR3	0x28
#define SMIDSW3 0x2C
#define SMIDC	0x30
#define SMIDCS	0x34
#define SMIDA	0x38
#define SMIDD	0x3C
#define SMIFD	0x40

// SMI control register bits
#define SMICS_ENABLE    0
#define SMICS_DONE      1
#define SMICS_ACTIVE    2
#define SMICS_START     3
#define SMICS_CLEARFIFO 4
#define SMICS_WRITE     5
#define SMICS_PAD       6
#define SMICS_TEEN      8
#define SMICS_INTD      9
#define SMICS_INTT      10
#define SMICS_INTR      11
#define SMICS_PVMODE    12
#define SMICS_SETERR    13
#define SMICS_PXLDAT    14
#define SMICS_EDREQ     15
#define SMICS_AFERR     25
#define SMICS_TXW       26
#define SMICS_RXR       27
#define SMICS_TXD       28
#define SMICS_RXD       29
#define SMICS_TXE       30
#define SMICS_RXF       31

// SMI address and direct address register bits.
#define SMIA_DEVICE     8
#define SMIDA_DEVICE    8

// SMI DSR* and DSW* common fields
#define SMIDS_STROBE    0
#define SMIDS_DREQ      7
#define SMIDS_PACE      8
#define SMIDS_PACEALL   15
#define SMIDS_HOLD      16
#define SMIDS_SETUP     24
#define SMIDS_WIDTH     30

// SMI DSR* register specific
#define SMIDS_FSETUP    22
#define SMIDS_MODE68    23
// SMI DSW* register specific
#define SMIDS_SWAP      22
#define SMIDS_FORMAT    23

// SMI direct control/status register bits.
#define SMIDCS_ENABLE   0
#define SMIDCS_START    1
#define SMIDCS_DONE     2
#define SMIDCS_WRITE    3

#endif /* _BCM2708_SMI_REGS_H */
