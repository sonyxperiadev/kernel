#ifndef _MACH_BCM2708_VCIO_H
#define _MACH_BCM2708_VCIO_H

// routines to handle I/O via the VideoCore "ARM control" registers
// (semaphores, doorbells, mailboxes)

// [but only writes to mailbox currently supported]

#define BCM_VCIO_DRIVER_NAME "bcm2708_vcio"

#define MBOX_CHAN_FB    1 // for use by the frame buffer
#define MBOX_CHAN_VUART 2 // for use by the virtual UART
#define MBOX_CHAN_VCHIQ 3 // for use by the VCHIQ interface

extern int /*rc*/ bcm_mailbox_write(unsigned chan, uint32_t data25);

#endif

